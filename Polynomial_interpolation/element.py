class Element:
    def __init__(self, nodes):
        self.globalNodes = nodes
        self.ptIndices = None
        self.faces = []
        self.newpts = []

    def findExternalFace(self, pt1, pt2):
        for face in self.faces:
            if len(face.elements)==1 and face.shareEdge(pt1, pt2):
                return face
        return None


    def orderNodes(self):
        '''
        This function has to be called after refinement of faces.
        '''
        edges = [(self.ptIndices[0],self.ptIndices[1]), (self.ptIndices[1],self.ptIndices[2]), (self.ptIndices[0],self.ptIndices[2]),(self.ptIndices[0],self.ptIndices[3]),(self.ptIndices[2],self.ptIndices[3]),(self.ptIndices[3],self.ptIndices[1])]
        for edge in edges:
            face = self.findExternalFace(edge[0], edge[1])
            if face is None:
                mid = (self.globalNodes[edge[0]-1] + self.globalNodes[edge[1]-1])/2
                self.newpts.append(mid)
            else:
                pt = face.getNewEdgePoint(edge[0], edge[1])
                self.newpts.append(pt)
