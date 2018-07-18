class face:
    def __init__(self, ptIndices, faceIndex):
        self.ptIndices = [ptIndex for ptIndex in ptIndices]
        self.elemIndices = []
        self.faceIndex = faceIndex
