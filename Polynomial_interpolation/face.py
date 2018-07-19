import numpy as np
import math

def extractUniquePts(face1, face2, face3):
    ret = []
    for pt in face1.ptIndices:
        ret.append(pt)
    for pt in face2.ptIndices:
        if pt not in ret:
            ret.append(pt)
    for pt in face3.ptIndices:
        if pt not in ret:
            ret.append(pt)
    return ret

def solveSecondOrderEquation(coef):
    a = coef[0]
    b = coef[1]
    c = coef[2]
    return (-1*b-np.sqrt(b**2-4*a*c))/(2*a), (-1*b+np.sqrt(b**2-4*a*c))/(2*a)

'''
    Helper function. Get the point on quadratic surface given a point and the normal of a plane.
    @author: Hongru Yang
    @date: 07/16/2018
    @param coef coefficients of the quadratic surface, ordered as follow: ax+by+cz+dx^2+ey^2+fz^2
    @param point midpoint of an edge
    @param normal normal vector of the plane where the midpoint is
'''
def getSurfacePoint(coef,const, point, normal): # Need to accomadate for const is one case
    # return a point on the surface that is perpendicular to the plane
    secondOrder = coef[5]*normal[2]**2 + coef[4]*normal[1]**2 + coef[3]*normal[0]**2
    firstOrder = coef[0]*normal[0] + coef[1]*normal[1] + coef[2]*normal[2] + 2*normal[0]*point[0]*coef[3] + 2*normal[1]*point[1]*coef[4] + 2*normal[2]*point[2]*coef[5]
    const = -1*const + coef[0]*point[0]+coef[1]*point[1]+coef[2]*point[2]+ coef[3]*point[0]**2+ coef[4]*point[1]**2+ coef[5]*point[2]**2
    p = np.array([secondOrder, firstOrder, const])
    root = solveSecondOrderEquation(p)
    root_abs = np.absolute(root)
    index = np.argmin(root_abs)
    return root[index]*normal+point

def findFace(faces, edge):
    for face in faces:
        if edge[0] in face.adjacentFaces and edge[1] in face.adjacentFaces:
            return face
    return None

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
        self.newpts = []


    def shareEdge(self, ptidx1, ptidx2):
        '''
        Check whether the face share the given edge.
        @author Hongru Yang
        @date 07/18/2018
        @param ptidx1 integer index of point1
        @param ptidx2 integer index of point2
        '''
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
                for face in elem.faces:
                    if (face.visited is False) and (face.shareEdge(pt1, pt2)):
                        return face.findExternalFace(pt1, pt2)


    def clearMarks(self):
        '''
        Recursively clear all marked faces.
        @author Hongru Yang
        @date 07/18/2018
        '''
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
            pt = (self.globalNodes[idx-1])
            x,y,z = pt
            temp = [x, y, z, x**2, y**2, z**2]
            m.append(temp)
        m = np.array(m)
        #print(m.shape)
        try:
            y = np.ones(6)
            self.neighborCoefficients = np.linalg.solve(m, y)
            self.neighborConst = 1
            return 1
        except np.linalg.linalg.LinAlgError:
            return 0
            '''#print(ptList)
            r = np.array([10,10,10])
            m = []
            for idx in ptList:
                pt = self.globalNodes[idx-1]
                x, y, z = pt + r
                temp = [x, y, z, x**2, y**2, z**2]
                m.append(temp)
            m = np.array(m)
            #print(m)
            y = np.ones(6)
            coef = np.linalg.solve(m, y)
            coef[0] = coef[0]+2*coef[3]*r[0]
            coef[1] = coef[1]+2*coef[4]*r[1]
            coef[2] = coef[2]+2*coef[5]*r[2]
            self.neighborCoefficients = coef
            self.neighborConst = 0'''

    def getFirstOrderCoef(self):
        if self.firstOrderCoef is None:
            m = []
            for idx in self.ptIndices:
                pt = self.globalNodes[idx-1]
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
                    pt = self.globalNodes[idx-1]
                    m.append(pt+r)
                m = np.array(m)
                y = np.ones(3)
                self.firstOrderCoef = np.linalg.solve(m, y)
                self.firstOrderConst = 0


    def getNewPoint(self):
        '''
        This function has to be called after neighborInterpolation and getFirstOrderCoef.
        '''
        if self.neighborCoefficients is None:
            for i in range(len(self.ptIndices)):
                edge = [self.ptIndices[i], self.ptIndices[(i+1)%len(self.ptIndices)]]
                mid = np.average([self.globalNodes[edge[0]-1], self.globalNodes[edge[1]-1]], axis = 0)
                self.newpts.append(mid)
            return
        for i in range(len(self.ptIndices)):
            edge = [self.ptIndices[i], self.ptIndices[(i+1)%len(self.ptIndices)]]
            face = findFace(self.adjacentFaces, edge)
            mid = np.average([self.globalNodes[edge[0]-1], self.globalNodes[edge[1]-1]], axis = 0)
            if face is None:
                newpt = getSurfacePoint(self.neighborCoefficients, self.neighborConst, mid, self.firstOrderCoef)
                self.newpts.append(newpt)
                continue
            elif face.neighborCoefficients is None:
                self.newpts.append(mid)
                continue

            newpoint1 = getSurfacePoint(self.neighborCoefficients, self.neighborConst, mid, self.firstOrderCoef)
            newpoint2 = getSurfacePoint(face.neighborCoefficients, face.neighborConst, mid, face.firstOrderCoef)
            if math.isnan(newpoint1[0]) or math.isnan(newpoint2[0]):
                self.newpts.append(mid)
            else:
                self.newpts.append(np.average([newpoint1, newpoint2], axis = 0))
