def getHTML(page, zim):
    if zim.has_entry_by_path(page):
        entry = zim.get_entry_by_path(page)
    else:
        return False

    return bytes(entry.get_item().content).decode("UTF-8")
