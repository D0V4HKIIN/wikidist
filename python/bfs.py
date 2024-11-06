from collections import deque
from linkparser import LinkParser
from getHTML import getHTML

def bfs(start, end, zim, max_depth=10):

    end = end.lower()
    queue = deque([(start, 0)])
    visited = set([start])
    parent = {start: None}

    while queue:
        current, depth = queue.popleft()

        # print(f"Visiting: {current} at depth {depth}")

        if depth >= max_depth:
            continue

        parser = LinkParser()
        html = getHTML(current, zim)
        if html is False:
            print(current, "doesn't exist in db")
            continue
        parser.feed(html)

        for link in parser.links:
            if link.lower() == end:
                print("Found", end, "at", current)
                print("dist is", depth)
                path = []
                while current is not None:
                    path.append(current)
                    current = parent[current]
                return path[::-1]

            if link not in visited:
                visited.add(link)
                parent[link] = current
                queue.append((link, depth + 1))
