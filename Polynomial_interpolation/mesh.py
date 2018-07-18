'''
    @file: mesh.py
    @author: Hongru Yang
    @date: 07/16/2018
    @brief: definition of class mesh
'''

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

'''
    Helper function. Get the point on quadratic surface given a point and the normal of a plane.
    @author: Hongru Yang
    @date: 07/16/2018
    @param coef coefficients of the quadratic surface, ordered as follow: ax+by+cz+dx^2+ey^2+fz^2
    @param point midpoint of an edge
    @param normal normal vector of the plane where the midpoint is
'''
def getSurfacePoint(coef, point, normal): # Need to accomadate for const is one case
    # return a point on the surface that is perpendicular to the plane
    secondOrder = coef[5]*normal[2]**2 + coef[4]*normal[1]**2 + coef[3]*normal[0]**2
    firstOrder = coef[0]*normal[0] + coef[1]*normal[1] + coef[2]*normal[2] + 2*normal[0]*point[0]*coef[3] + 2*normal[1]*point[1]*coef[4] + 2*normal[2]*point[2]*coef[5]
    const = -1 + coef[0]*point[0]+coef[1]*point[1]+coef[2]*point[2]+ coef[3]*point[0]**2+ coef[4]*point[1]**2+ coef[5]*point[2]**2
    p = np.array([secondOrder, firstOrder, const])
    root = np.roots(p)
    root_abs = np.absolute(root)
    index = np.argmin(root_abs)
    return root[index]*normal+point

'''
    Helper function. Check whether a point is in an array of points
    @author: Hongru Yang
    @date: 07/16/2018
    @param arr a list of points
    @param pt a point (type: numpy array of shape of (3,))
'''
def inArray(arr, pt):
    for e in arr:
        if np.array_equal(e, pt):
            return True
    return False

'''
    Helper function. Pick the point in array2 that is different from points in array1
    @author: Hongru Yang
    @date: 07/16/2018
    @param arr1 a list of points
    @param arr2 a list of points
'''
def findNewNode(arr1, arr2):
    for pt in arr2:
        if not inArray(arr1, pt):
            return pt
    return None

'''
    Helper function. Find a mesh in meshes list that has pt1 and pt2
    @author: Hongru Yang
    @date: 07/16/2018
    @param meshes a list of mesh objects
    @param pt1 point1 type numpy array of shape (3,)
    @param pt2 point2 type numpy array of shape (3,)
'''
def findMesh(meshes, pt1, pt2):
    for mesh in meshes:
        if inArray(mesh.pts, pt1) and inArray(mesh.pts, pt2):
            return mesh
    return None

'''
    Mesh class
    @author: Hongru Yang
    @date: 07/16/2018
    @param vertexNum the number of node that the mesh have. Currenly default to 3 (i.e. the inital setting is linear triangle)
    @param pts list of points
    @param edgeNeighbor a list of edge neighbor (edge neighbor is defined by elements that share an edge with the current element)
    @param coefficients coefficients of second order elements (has to be computed after we have neighborCoefficients)
    @param const the right hand side of equation when we calculating coefficients (either 1 or 0)
    @param neighborCoefficients the coefficients of the surface of neighbor interpolation
    @param neighborConst the right hand side of equation when we calculating neighborCoefficients
    @param firstOrderCoef the coefficients of the plane where our triangle element is (ax+by+cz) can also be used as the normal of our plane
'''
class mesh:
    def __init__(self, pts):
        self.vertexNum = 3
        self.pts = [pt for pt in pts]
        self.edgeNeighbor = [] # one mesh can only have 3 edgeNeighbor; store mesh
        self.coefficients = None # store the coefficients of second order interpolation
        self.const = None
        self.neighborCoefficients = None # store the coefficients of neighbor interpolation
        self.neighborConst = None
        self.firstOrderCoef = None # store the coefficients of the first order surface

'''
    Calculate the error of our mesh with respect to the true function.
    @author: Hongru Yang
    @date: 07/16/2018
    @param realFunc real shape of our mesh
    @param num number of pieces we want to divide our edge into (thus we divide our triangle into num^2 pieces)
'''
    def getErr(self, realFunc, num):
        # the function has to be called after we get firstOrderCoef, and coefficients'
        # realFunc receive point and normal return the value on the surface
        vec1 = (self.pts[1] - self.pts[0])/num
        vec2 = (self.pts[2] - self.pts[1])/num/2
        area = np.linalg.norm(np.cross(vec1, vec2))
        err1 = 0
        err2 = 0
        for i in range(num):
            for j in range(1, 2*i+1):
                curr = self.pts[0]+vec1/2+vec1*i+vec2*j
                realPt = realFunc(curr, self.firstOrderCoef)
                surfacePt = self.getQuadraticSurfacePoint(curr, self.firstOrderCoef)
                err1 += np.linalg.norm(realPt-curr)*area
                err2 += np.linalg.norm(realPt-surfacePt)*area
        return err1, err2

'''
    Wrapper function. Get the point on the quadratic surface.
    @author: Hongru Yang
    @date: 07/16/2018
    @param pt a point type of numpy array of shape (3,)
    @param normal normal vector typr numpy array of shape (3,)
'''
    def getQuadraticSurfacePoint(self, pt, normal):
        return getSurfacePoint(self.coefficients, pt, normal)

'''
    def getPlaneZValue(self, x, y): # return only positive value; used for error measuring
        z = (1-self.firstOrderCoef[0]*x-self.firstOrderCoef[1]*y)/self.firstOrderCoef[2]
        if z<0:
            return 0
        else:
            return z

    def getSurfaceZValue(self, x, y):
        a = self.coefficients[0]
        b = self.coefficients[1]
        c = self.coefficients[2]
        d = self.coefficients[3]
        e = self.coefficients[4]
        f = self.coefficients[5]

        secondOrder = self.coefficients[5]
        firstOrder = self.coefficients[2]
        const = -1 + a*x + b*y + d*x**2 + e*y**2
        p = np.array([secondOrder, firstOrder, const])
        root = np.roots(p)
        return np.amax(root)
'''

'''
    Compute the first order coefficients. (The first order coefficients are the normal vector of the plane, ax+by+cz).
    @author: Hongru Yang
    @date: 07/16/2018
'''
    def getFirstOrderCoef(self):
        # ax + by + cz + d = 0 ==> (assume d is nonzero)
        # ax + by + cz = 1
        if self.firstOrderCoef is None:
            m = []
            for i in range(3):
                pt = self.pts[i]
                temp = [pt[0], pt[1], pt[2]]
                m.append(temp)
            m = np.array(m)
            y = np.ones(3)
            self.firstOrderCoef = np.linalg.solve(m, y)

'''
    Compute the coefficients of polynomial interpolation running with its edge neighbor.
    @author: Hongru Yang
    @date: 07/16/2018
'''
    def neighborInterpolation(self):
        if len(self.edgeNeighbor)<3:
            if self.edgeNeighbor[0].neighborCoefficients is not None:
                self.neighborCoefficients = self.edgeNeighbor[0].neighborCoefficients
                return True
            else:
                return False

        m = []
        Y = []
        for pt in self.pts:
            #print(pt)
            x = pt[0]
            y = pt[1]
            z = pt[2]
            temp = [x, y, z, x**2, y**2, z**2]
            m.append(temp)

        for mesh in self.edgeNeighbor:
            pt = findNewNode(self.pts, mesh.pts)
            #print(pt)
            x = pt[0]
            y = pt[1]
            z = pt[2]
            temp = [x, y, z, x**2, y**2, z**2]
            m.append(temp)
        m = np.array(m)
        #print(m)
        #print("\n")
        Y = np.ones(6)
        self.neighborCoefficients = np.linalg.solve(m, Y)
        return True

'''
    Compute the coefficients of the quadratic surface that the second order element is on.
    @author: Hongru Yang
    @date: 07/16/2018
'''
    def getCoefficients(self):
        # self.pts has to have length 6
        if len(self.pts)<6:
            raise ValueError("less than 6 points.")
            return
        else:
            m = []
            Y = []
            for pt in self.pts:
                x = pt[0]
                y = pt[1]
                z = pt[2]
                temp = [x, y, z, x**2, y**2, z**2]
                m.append(temp)
            m = np.array(m)
            Y = np.ones(6)
            self.coefficients = np.linalg.solve(m, Y)

'''
    This function has to be called when all meshes have neighborCoefficients
    and first order coefficients.
    For all meshes in neighborMesh, first calculate the midpoint of the share
    edge, then calculate the point at both surfaces. Finally take the average.
    Get new point in the following order: 0,1; 1,2; 0,2
    @author: Hongru Yang
    @date: 07/16/2018
'''
    def getNewPoint(self):
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

'''
    Draw all of the points of the current element in 3D.
    @author: Hongru Yang
    @date: 07/16/2018
    @param ax plot parameter. Can be generated as follows: ax = plt.axes(projection='3d')
'''
    def drawPoint(self, ax):
        X = np.array([pt[0] for pt in self.pts])
        Y = np.array([pt[1] for pt in self.pts])
        Z = np.array([pt[2] for pt in self.pts])
        ax.scatter3D(X, Y, Z, color = 'b')
