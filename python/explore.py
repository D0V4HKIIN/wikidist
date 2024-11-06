from linkparser import LinkParser
from getHTML import getHTML

# basically dfs to find all pages


def explore(start, zim):

    stack = [start]
    visited = set([start])

    while stack:
        current = stack.pop()

        # print(f"Visiting: {current}")

        parser = LinkParser(zim)
        html = getHTML(current, zim)
        parser.feed(html)

        for link in parser.links:
            if not zim.has_entry_by_path(link):
                continue

            if link not in visited:
                visited.add(link)
                stack.append(link)
    return visited
