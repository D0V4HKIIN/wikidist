from collections import deque
from linkparser import LinkParser
from getHTML import getHTML


def bfs(start, end, zim, max_depth=20):

    end = end.lower()
    queue = deque([(start, 0)])
    visited = set([start])
    parent = {start: None}

    while queue:
        current, depth = queue.popleft()

        # print(f"Visiting: {current} at depth {depth}")

        if depth >= max_depth:
            continue

        parser = LinkParser(zim)
        html = getHTML(current, zim)
        parser.feed(html)

        for link in parser.links:
            if link.lower() == end:
                # print("Found", end, "at", current)
                # print("dist is", depth)
                path = [link]
                while current is not None:
                    path.append(current)
                    current = parent[current]
                return path[::-1], depth

            if link not in visited:
                visited.add(link)
                parent[link] = current
                queue.append((link, depth + 1))

    return [], -1
