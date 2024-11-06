def getHTML(page, zim):
    entry = zim.get_entry_by_path(page)

    return bytes(entry.get_item().content).decode("UTF-8")
