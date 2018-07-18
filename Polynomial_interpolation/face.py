def extractUniquePts(face1, face2, face3):
    ret = []
    for pt in face1.ptIndices:
        ret.append(pt)
    for pt in face2.ptIndices if pt not in ret:
        ret.append(pt)
    for pt in face3.ptIndices if pt not in ret:
        ret.append(pt)
    return ret


class face:
    def __init__(self, ptIndices, faceIndex, nodes):
        self.globalNodes = nodes
        self.ptIndices = [ptIndex for ptIndex in ptIndices]
        self.elements = []
        self.faceIndex = faceIndex
        self.neighborCoefficients = None # store the coefficients of neighbor interpolation
        self.neighborConst = None
        self.firstOrderCoef = None
        self.firstOrderConst = None
        self.visited = False
        self.adjacentFaces = []

'''
    Check whether the face share the given edge.
    @author Hongru Yang
    @date 07/18/2018
    @param ptidx1 integer index of point1
    @param ptidx2 integer index of point2
'''
    def shareEdge(self, ptidx1, ptidx2):
        return (ptidx1 in self.ptIndices) and (ptidx2 in self.ptIndices)


    def findAjacentFaces(self):
        if len(self.elements)>1: # a face that is internal don't need this function
            return
        else:
            self.visited = True
            face1 = self.findExternalFace(self.ptIndices[0], self.ptIndices[1])
            face2 = self.findExternalFace(self.ptIndices[1], self.ptIndices[2])
            face3 = self.findExternalFace(self.ptIndices[0], self.ptIndices[2])
            if len(self.adjacentFaces) == 0:
                self.adjacentFaces.append(face1)
                self.adjacentFaces.append(face2)
                self.adjacentFaces.append(face3)
            return face1, face2, face3

    def findExternalFace(self, pt1, pt2):
        if self.visited == False and len(self.elements)==1:
            return self
        else:
            self.visited = True
            for elem in self.elements:
                for face in elem.faces if (face.visited is False) and (face.shareEdge(pt1, pt2)):
                    return face.findExternalFace(pt1, pt2)

'''
    Recursively clear all marked faces.
    @author Hongru Yang
    @date 07/18/2018
'''
    def clearMarks(self):
        if self.visited == False:
            return
        else:
            self.visited = False
            for elem in self.elements:
                for face in elem.faces:
                    face.clearMarks()

    def neighborInterpolation(self):
        if self.neighborCoefficients is not None:
            return

        face1, face2, face3 = self.findAjacentFaces()
        self.clearMarks()
        ptList = extractUniquePts(face1, face2, face3)

        m = []
        for idx in ptList:
            pt = (self.globalNodes[idx])
            x,y,z = pt
            temp = [x, y, z, x**2, y**2, z**2]
            m.append(temp)
        m = np.array(m)

        try:
            y = np.ones(6)
            self.neighborCoefficients = np.linalg.solve(m, y)
            self.neighborConst = 1
        except np.linalg.linalg.LinAlgError:
            r = np.random.rand(3)
            m = []
            for idx in ptList:
                pt = self.globalNodes[idx]
                x, y, z = pt + r
                temp = [x, y, z, x**2, y**2, z**2]
                m.append(temp)
            m = np.array(m)
            y = np.ones(6)
            coef = np.linalg.solve(m, y)
            coef[0] = coef[0]+2*coef[3]*r[0]
            coef[1] = coef[1]+2*coef[4]*r[1]
            coef[2] = coef[2]+2*coef[5]*r[2]
            self.neighborCoefficients = coef
            self.neighborConst = 0

    def getFirstOrderCoef(self):
        # ax + by + cz + d = 0 ==> (assume d is nonzero)
        # ax + by + cz = 1
        if self.firstOrderCoef is None:
            m = []
            for idx in self.ptIndices:
                pt = self.globalNodes[idx]
                m.append(pt)
            m = np.array(m)
            y = np.ones(3)

            try:
                self.firstOrderCoef = np.linalg.solve(m, y)
                self.firstOrderConst = 1
            except np.linalg.linalg.LinAlgError:
                r = np.random.rand(3)
                m = []
                for idx in self.ptIndices:
                    pt = self.globalNodes[idx]
                    m.append(pt+r)
                m = np.array(m)
                y = np.ones(3)
                self.firstOrderCoef = np.linalg.solve(m, y)
                self.firstOrderConst = 0

'''
    This function has to be called after neighborInterpolation and getFirstOrderCoef.
'''
    def getNewPoint(self):
        # Neet to change
        for i in range(self.vertexNum):
            shareNode = [self.pts[i], self.pts[(i+1)%self.vertexNum]]
            mesh = findMesh(self.edgeNeighbor, self.pts[i], self.pts[(i+1)%self.vertexNum])
            mid = np.average(shareNode, axis = 0)
            if mesh is None:
                newpt = getSurfacePoint(self.neighborCoefficients, mid, self.firstOrderCoef)
                self.pts.append(newpt)
                continue

            newpoint1 = getSurfacePoint(self.neighborCoefficients, mid, self.firstOrderCoef)
            newpoint2 = getSurfacePoint(mesh.neighborCoefficients, mid, mesh.firstOrderCoef)
            self.pts.append(np.average([newpoint1, newpoint2], axis = 0))
