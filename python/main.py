from libzim.reader import Archive
from libzim.search import Query, Searcher

from bfs import bfs
from explore import explore
from adj_matrix import compute_adj


# zim = Archive("/home/jonas/.local/share/kiwix/wikipedia_en_all_nopic_2024-06.zim")
zim = Archive(
    "/home/jonas/.var/app/org.kiwix.desktop/data/kiwix/wikipedia_en_100_nopic_2024-06.zim")
# zim = Archive(
    # "/home/jonas/.var/app/org.kiwix.desktop/data/kiwix/wikipedia_en_all_nopic_2024-06.zim")

# print(f"Main entry is at {zim.main_entry.get_item().path}")

entry = zim.main_entry.get_item().path

pages = explore(entry, zim)
# print(zim.article_count)

# print("Found", len(pages), "pages")
# print(len(set(pages)))

adj = compute_adj(pages, zim)


# for a in pages:
# for b in pages:
# print("from", a, "to", b, bfs(a, b, zim))

# print(bfs("Thirty_Years'_War", "Tautology_(language)", zim))


# print(pages)
# print(len(pages))

# print(zim.entry_count)
# print(zim.all_entry_count)
# print(zim.media_count)
