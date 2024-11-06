from html.parser import HTMLParser


class LinkParser(HTMLParser):
    def __init__(self, zim):
        super().__init__()
        self.links = []
        self.zim = zim

    def handle_starttag(self, tag, attrs):
        # Only parse the 'anchor' tag.
        if tag == "a":
            # Check the list of defined attributes.
            for name, value in attrs:
                # If href is defined, print it.
                if name == "href":
                    if self.zim.has_entry_by_path(value):
                        self.links.append(value)
