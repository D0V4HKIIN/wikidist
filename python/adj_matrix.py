import numpy as np
import scipy
from linkparser import LinkParser
from getHTML import getHTML


def compute_adj(pages, zim):
    print("computing indexes")
    indexes = {}
    for page, idx in enumerate(pages):
        indexes[page] = idx

    print("computing matrix")
    n = len(pages)
    # mat = np.zeros((n, n))
    mat_sparse = scipy.sparse.csr_matrix((n, n))
    # print(mat.shape)
    print(mat_sparse.shape)
    for x in pages:
        parser = LinkParser(zim)
        html = getHTML(x)
        parser.feed(html)
        neighbors = parser.links
        for neighbor in neighbors:
            mat_sparse[]