#include <iostream>
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

using namespace std;

void printVector(vector<string> vector)
{
  std::copy(vector.begin(), vector.end(), std::ostream_iterator<string>(std::cout, ", "));
  cout << endl;
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

void bfs(string from, string to, zim::Archive a)
{
  cout << "searching from " << from << " to " << to << endl;
  assert(a.hasEntryByPath(from));
  assert(a.hasEntryByPath(to));

  // queue of titles
  string from_title = a.getEntryByPath(from).getItem(true).getTitle();
  string to_title = a.getEntryByPath(to).getItem(true).getTitle();
  deque<string> queue{from_title};
  unordered_set<string> visited{from_title};
  unordered_map<string, string> parents{std::make_pair(from_title, "")};

  while (!queue.empty())
  {
    string current = queue.front();
    queue.pop_front();

    cout << "current " << current << endl;

    zim::Item item = a.getEntryByTitle(current).getItem(true);
    vector<string> links = extractLinks(item.getData());
    for (const string &link : links)
    {
      if (link == to_title)
      {
        cout << "found path at " << current << endl;
        vector<string> path{link};
        while (current != "")
        {
          path.push_back(current);
          current = parents.at(current);
        }
        reverse(path.begin(), path.end());
        printVector(path);
        return;
      }
      if (a.hasEntryByTitle(link) && visited.find(link) == visited.end())
      {
        visited.insert(link);
        parents.insert(make_pair(link, current));
        queue.push_back(link);
      }
    }
  }

  cout << "couldn't find a path between " << from_title << " to " << to_title << endl;
}

unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> neighbors(zim::Archive a)
{
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

    if (entry.getIndex() != a.getEntryByClusterOrder(entry.getIndex()).getItem(true).getIndex())
    {
      cout << "different index" << endl;
    }

    ids.insert(item.getIndex());
  }

  // compute neighbors map
  unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> neighbors{};
  for (zim::entry_index_type id : ids)
  {
    // cout << "getting " << id << endl;
    zim::Entry entry = a.getEntryByClusterOrder(id);
    zim::Item item = entry.getItem(true);

    if (item.getIndex() != id)
    {
      // cout << "index not matching " << entry.isRedirect() << endl;
    }
    vector<string> links = extractLinks(item.getData());

    vector<zim::entry_index_type> indexes{};
    for (string link : links)
    {
      // cout << "getting n " << link << endl;
      if (a.hasEntryByTitle(link))
      {
        indexes.push_back(a.getEntryByTitle(link).getIndex());
      }
    }
    if (indexes.empty())
    {
      cout << "is empty" << endl;
    }
    // neighbors.insert(make_pair(item.getIndex(), indexes));
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
    zim::Archive a("/home/jonas/.var/app/org.kiwix.desktop/data/kiwix/"
                   "wikipedia_en_100_nopic_2024-06.zim");
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
      bfs(from, to, a);
    }
    else if (argc == 2 && strcmp(argv[1], "precompute") == 0)
    {
      unordered_map<zim::entry_index_type, vector<zim::entry_index_type>> n = neighbors(a);
      // for (auto a : n)
      // {
      // cout << a.first << ": ";
      // for (auto b : a.second)
      // {
      // cout << b << ", ";
      // }
      // cout << "\n";
      // }
      cout << "found " << n.size() << " neighbors" << endl;
      // write to disk
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }
}
