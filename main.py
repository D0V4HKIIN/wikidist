from libzim.reader import Archive
from libzim.search import Query, Searcher

from bfs import bfs
from explore import explore


zim = Archive("/home/jonas/.local/share/kiwix/wikipedia_en_100_nopic_2024-06.zim")

print(f"Main entry is at {zim.main_entry.get_item().path}")

entry = zim.main_entry.get_item().path

# bfs(entry, "List_of_presidents_of_the_United_States", zim)

pages = explore(entry, zim)

print(pages)
print(len(pages))

print(zim.entry_count)
print(zim.all_entry_count)
print(zim.media_count)