import numpy as np
import scipy
from linkparser import LinkParser
from getHTML import getHTML


def compute_adj(pages, zim):
    n = len(pages)
    mat = np.zeros((n, n))
    mat_sparse = scipy.sparse.csr_matrix((n, n))
    print(mat.shape)
    print(mat_sparse.shape)
    for x in pages:
        parser = LinkParser(zim)
        parser.feed(x)
        neighbors = parser.links
