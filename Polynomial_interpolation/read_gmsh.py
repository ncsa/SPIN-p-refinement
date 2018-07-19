import numpy as np
from face import face
from element import element

'''
    Generate 4 faces of a tetrahedron.
    @autor Hongru Yang
    @data 07/18/2018
    @param ptsIndex a list of point indices (int)
'''
def generateFaces(ptsIndex):
    temp = [ptsIndex[0:3], ptsIndex[1:4], [ptsIndex[0], ptsIndex[2], ptsIndex[3]], [ptsIndex[0], ptsIndex[1], ptsIndex[3]]]
    return temp

'''
    Helper function. Generate keys (string) for dictionary.
    @autor Hongru Yang
    @data 07/18/2018
    @param ptsIndex a list of point indices (int)
'''
def getKey(ptsIndex):
    temp = map(str, ptsIndex)
    return ' '.join(temp)

'''
    Read .gmsh file and store the information. Nodes, faces, elements and face2index are about to be filled in.
    @autor Hongru Yang
    @data 07/18/2018
    @param filename string
    @param nodes an empty list
    @param faces an empty list
    @param elements an empty list
    @param face2index an empty dictionary
'''
def read_gmsh(filename, nodes, faces, elements, face2index):
    f = open(filename, "r")
    #nodes = []
    #faces = []
    #elements = []
    #face2index = {}

    lines = f.readlines()
    #print(lines)
    isNode = False
    isElement = False

    faceIndex = 0
    i = 0
    while i < (len(lines)):
        if lines[i] == "$Nodes\n":
            isNode = True
            i+=1
        elif lines[i] == "$EndNodes\n":
            isNode = False
        elif lines[i] == "$Elements\n":
            isElement = True
            i+=1
        elif lines[i] == "$EndElements\n":
            isElement = False
            break
        elif isNode == True:
            line = lines[i].split(" ")
            temp = np.zeros(3)
            temp[0] = float(line[1])
            temp[1] = float(line[2])
            temp[2] = float(line[3])
            nodes.append(temp)
        elif isElement == True:
            line = lines[i].split(" ")
            if line[1] == '4':
                tempElem = element()
                tempElem.ptIndices = sorted(map(int, line[5:9]))
                elements.append(tempElem)
                tempFaces = generateFaces(tempElem.ptIndices)
                for tempFace in tempFaces:
                    k = getKey(tempFace)
                    if k not in face2index:
                        face2index[k] = faceIndex
                        faceObj = face(tempFace, faceIndex, nodes)
                        faceObj.elements.append(tempElem)
                        faces.append(faceObj)
                        tempElem.faces.append(faceObj)
                        faceIndex += 1
                    else:
                        # Insert faces and elements
                        tempFaceIdx = face2index[k]
                        tempElem.faces.append(faces[tempFaceIdx])
                        faces[tempFaceIdx].elements.append(tempElem)

        i+=1

def refine_gmsh(filename):
    nodes = []
    faces = []
    elements = []
    face2index = {}
    read_gmsh(filename, nodes, faces, elements, face2index)

    unique_faces = [face for face in faces if len(face.elements)==1]
    for face in unique_faces:
        face.neighborInterpolation()
        face.getFirstOrderCoef()
    for face in unique_faces:
        face.getNewPoint()

refine_gmsh("rocket.msh")
