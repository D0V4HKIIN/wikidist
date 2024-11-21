#include <iostream>
#include <fstream>
#include <zim/archive.h>
#include <zim/search_iterator.h>
#include <zim/item.h>
#include <unordered_set>
#include <unordered_map>
#include <cassert>
#include <deque>
#include <regex>
#include <vector>
#include <algorithm>
#include <omp.h>

using namespace std;

template <typename T>
void printVector(vector<T> &vector)
{
  std::copy(vector.begin(), vector.end(), std::ostream_iterator<T>(std::cout, ", "));
  cout << endl;
}

void write(ostream &out, size_t s)
{
  out.write(reinterpret_cast<const char *>(&s), sizeof(s));
}

void serialize(ostream &out, unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> const &map)
{
  write(out, map.size());
  for (auto const &p : map)
  {
    write(out, p.first);
    write(out, p.second.size());
    for (zim::entry_index_type const &e : p.second)
    {
      write(out, e);
    }
  }
}

size_t read(istream &in)
{
  size_t value;
  in.read(reinterpret_cast<char *>(&value), sizeof(value));
  return value;
}

unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> deserialize(istream &in)
{
  unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> map{};

  size_t size = read(in);

  for (size_t i = 0; i < size; i++)
  {
    zim::entry_index_type key = read(in);
    size_t value_size = read(in);
    vector<zim::entry_index_type> value;

    for (size_t j = 0; j < value_size; j++)
    {
      zim::entry_index_type val = read(in);
      value.push_back(val);
    }
    map[key] = value;
  }

  return map;
}

// thx chatgpt
std::vector<std::string> extractLinks(const std::string &html)
{
  std::vector<std::string> links;

  // Regular expression to match href attributes
  std::regex linkRegex(R"(<a[^>]*\s+title=["']([^"']+)["'][^>]*>)");
  std::smatch match;

  std::string::const_iterator searchStart(html.cbegin());
  while (std::regex_search(searchStart, html.cend(), match, linkRegex))
  {
    // match[1] contains the href value
    links.push_back(match[1]);
    searchStart = match.suffix().first; // Move past the current match
  }

  return links;
}

void bfs(string const &from_path, string const &to_path, zim::Archive &a, unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> &map)
{
  cout << "searching from " << from_path << " to " << to_path << endl;

  // queue of titles
  zim::entry_index_type from = a.getEntryByPath(from_path).getItem(true).getIndex();
  zim::entry_index_type to = a.getEntryByPath(to_path).getItem(true).getIndex();
  deque<zim::entry_index_type> queue{from};
  unordered_set<zim::entry_index_type> visited{from};
  unordered_map<zim::entry_index_type, zim::entry_index_type> parents{std::make_pair(from, from)};

  while (!queue.empty())
  {
    zim::entry_index_type current = queue.front();
    queue.pop_front();

    cout << "current " << current << endl;

    for (const zim::entry_index_type &n : map[current])
    {
      if (n == to)
      {
        cout << "found path at " << current << endl;
        vector<string> path{a.getEntryByPath(n).getTitle()};
        while (current != from)
        {
          path.push_back(a.getEntryByPath(current).getTitle());
          current = parents[current];
        }
        reverse(path.begin(), path.end());
        printVector(path);
        return;
      }
      if (visited.find(n) == visited.end())
      {
        visited.insert(n);
        parents[n] = current;
        queue.push_back(n);
      }
    }
  }

  cout << "couldn't find a path between " << from_path << " to " << to_path << endl;
}

unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> neighbors(zim::Archive &a)
{
  cout << "finding all ids" << endl;
  // get all relevant ids
  unordered_set<zim::entry_index_type> ids{};
  for (zim::Entry entry : a.iterEfficient())
  {
    if (entry.isRedirect())
    {
      continue;
    }
    zim::Item item = entry.getItem();
    if (item.getMimetype() != "text/html")
    {
      continue;
    }
    // never happens but a nice check imo
    if (ids.find(item.getIndex()) != ids.end())
    {
      cout << item.getTitle() << " already in db" << endl;
      continue;
    }

    ids.insert(item.getIndex());
  }

  cout << "copying set to vector" << endl;
  vector<zim::entry_index_type> ids_vec{};
  ids_vec.insert(ids_vec.end(), ids.begin(), ids.end());

  cout << "finding neighbors" << endl;
  // compute neighbors map
  unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> neighbors{};

// #pragma omp declare reduction(merge : unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> : omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))
#pragma omp parallel for // reduction(merge : neighbors)
  for (zim::entry_index_type id : ids_vec)
  {
    zim::Entry entry = a.getEntryByPath(id);
    zim::Item item = entry.getItem(true);

    vector<string> links = extractLinks(item.getData());

    vector<zim::entry_index_type> indexes{};
    for (string link : links)
    {
      if (a.hasEntryByTitle(link))
      {
        indexes.push_back(a.getEntryByTitle(link).getIndex());
      }
    }
    if (indexes.empty())
    {
      cout << a.getEntryByPath(id).getTitle() << " is has no neighbors" << endl;
    }
#pragma omp critical
    neighbors[id] = indexes;
  }

  cout << neighbors.size() << " " << ids.size() << endl;
  return neighbors;
}

int main(int argc, char *argv[])
{
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  try
  {
    std::cout << "loading archive" << std::endl;
    zim::Archive a("/home/jonas/.local/share/kiwix/wikipedia_en_all_nopic_2024-06.zim");
    std::cout << "archive loaded" << std::endl;

    unordered_set<string> titles{};
    unordered_set<string> paths{};

    std::cout << a.getEntryCount() << " " << a.getMediaCount() << " "
              << a.getAllEntryCount() << " " << a.iterByTitle().size() << " "
              << a.getArticleCount() << " " << a.iterEfficient().size() << " "
              << titles.size() << " " << paths.size()
              << endl;

    if (argc >= 3)
    {
      string from = argv[1];
      string to = argv[2];

      assert(a.hasEntryByPath(from));
      assert(a.hasEntryByPath(to));

      cout << "reading file" << endl;
      ifstream file;
      file.open("neighbors.txt");
      unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> n = deserialize(file);
      file.close();
      cout << "there are " << n.size() << "articles in file" << endl;

      bfs(from, to, a, n);
    }
    else if (argc == 2 && strcmp(argv[1], "precompute") == 0)
    {
      unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> n = neighbors(a);
      cout << "found " << n.size() << " neighbors" << endl;
      // write to disk
      ofstream file;
      file.open("neighbors.txt");
      serialize(file, n);
      file.close();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }
}
