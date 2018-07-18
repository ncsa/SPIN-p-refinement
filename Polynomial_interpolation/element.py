class element:
    def __init__(self):
        self.ptIndices = None
        self.elemIndex = None
        self.faces = []

    def shareEdge(self, ptidx1, ptidx2):
        return (ptidx1 in self.ptIndices) and (ptidx2 in self.ptIndices)
