'''
    @file: read_gmsh.py
    @author: Hongru Yang
    @date: 07/16/2018
    @brief: definition of gmsh reader, writer, vtk writer function
'''

import numpy as np
from face import Face
from element import Element
from multiprocessing import Pool

'''
    Generate 4 faces of a tetrahedron.
    @autor Hongru Yang
    @date 07/18/2018
    @param ptsIndex a list of point indices (int)
'''
def generateFaces(ptsIndex):
    temp = [ptsIndex[0:3], ptsIndex[1:4], [ptsIndex[0], ptsIndex[2], ptsIndex[3]], [ptsIndex[0], ptsIndex[1], ptsIndex[3]]]
    return temp

'''
    Helper function. Generate keys (string) for dictionary.
    @autor Hongru Yang
    @date 07/18/2018
    @param ptsIndex a list of point indices (int)
'''
def getKey(ptsIndex):
    temp = map(str, ptsIndex)
    return ' '.join(temp)

'''
    Read .gmsh file and store the information. Nodes, faces, elements and face2index are about to be filled in.
    @autor Hongru Yang
    @date 07/18/2018
    @param filename string
    @param nodes an empty list
    @param faces an empty list
    @param elements an empty list
    @param face2index an empty dictionary
'''
def read_gmsh(filename, nodes, faces, elements, otherElements, face2index):
    f = open(filename, "r")

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
                tempElem = Element(nodes)
                tempElem.ptIndices = sorted(map(int, line[5:9]))
                elements.append(tempElem)
                tempFaces = generateFaces(tempElem.ptIndices)
                for tempFace in tempFaces:
                    k = getKey(tempFace)
                    if k not in face2index:
                        face2index[k] = faceIndex
                        faceObj = Face(tempFace, faceIndex, nodes)
                        faceObj.elements.append(tempElem)
                        faces.append(faceObj)
                        tempElem.faces.append(faceObj)
                        faceIndex += 1
                    else:
                        # Insert faces and elements
                        tempFaceIdx = face2index[k]
                        tempElem.faces.append(faces[tempFaceIdx])
                        faces[tempFaceIdx].elements.append(tempElem)
            else:
                otherElements.append(lines[i])

        i+=1

'''
    A simple function to write .gmsh file. Can be further optimized to reduce the number of system calls.
    @autor Hongru Yang
    @date 07/18/2018
    @param filename string
    @param nodes a list of node objects
    @param elements a list of element objects
    @param otherElements a list of other type of elements (in our case anything except tetrahedron)
'''
def write_gmsh(filename, nodes, elements, otherElements):
    element_type = 11
    point2index = {} # here the indices are the 1-based index
    i = len(nodes)+1
    for element in elements:
        element.orderNodes()
        # add new nodes to nodes array
        for node in element.newpts:
            k = str(node)
            if k not in point2index:
                nodes.append(node)
                point2index[k] = i
                i += 1
            element.ptIndices.append(point2index[k])

    f = open(filename, "w+")
    f.write("$MeshFormat\n2.2 0 8\n$EndMeshFormat\n$Nodes\n")
    f.write("%d\n" %len(nodes))
    j = 1
    for node in nodes:
        f.write("%d %f %f %f\n" %(j, node[0], node[1], node[2]))
        j += 1
    f.write("$EndNodes\n$Elements\n%d\n" %(len(elements)+len(otherElements)))
    for otherElement in otherElements:
        f.write(otherElement)
    k = len(otherElements)+1
    for element in elements:
        f.write("%d %d 2 0 1 %d %d %d %d %d %d %d %d %d %d\n" %(k,element_type, element.ptIndices[0],element.ptIndices[1],element.ptIndices[2],element.ptIndices[3], element.ptIndices[4],element.ptIndices[5],element.ptIndices[6],element.ptIndices[7],element.ptIndices[8],element.ptIndices[9]))
        k += 1
    f.write("$EndElements\n")


'''
    A simple function to write .vtk file for visualization. Can be further optimized to reduce the number of system calls.
    @autor Hongru Yang
    @date 07/18/2018
    @param filename string
    @param nodes a list of node objects
    @param elements a list of element objects
    @param otherElements a list of other type of elements (in our case anything except tetrahedron)
'''
def write_vtk(filename, header, nodes, elements):
    #Note VTK file is 0-based indexed
    element_type = "24"
    point2index = {}
    i = len(nodes)+1
    for element in elements:
        element.orderNodes()
        # add new nodes to nodes array
        for node in element.newpts:
            k = str(node)
            if k not in point2index:
                nodes.append(node)
                point2index[k] = i
                i += 1
            element.ptIndices.append(point2index[k])

    f = open(filename, "w+")
    f.write("# vtk DataFile Version 1.0\n")
    f.write(header)
    f.write("\nASCII\n\nDATASET UNSTRUCTURED_GRID\n")
    f.write("POINTS       %d float\n" %len(nodes))
    for node in nodes:
        f.write("  %f       %f       %f    \n" %(node[0], node[1], node[2]))
    f.write("\n")

    size = 11
    f.write("CELLS         %d          %d\n" %(len(elements), size*len(elements)))
    for element in elements:
        l = [10]
        for pt in element.ptIndices:
            l.append(pt-1)
        f.write(" "+"        ".join(map(str, l))+"\n")
    f.write("\n")

    f.write("CELL_TYPES         %d\n" %len(elements))
    cellTypes = (element_type+"\n")*len(elements)
    f.write(cellTypes)

'''
    A wrapper function to read a gmsh file, refine all of the tetrahedron elements and write the result into a vtk file.
    @autor Hongru Yang
    @date 07/18/2018
    @param filename string
'''
def refine_gmsh(filename):
    nodes = []
    faces = []
    elements = []
    otherElements = []
    face2index = {}
    read_gmsh(filename, nodes, faces, elements, otherElements, face2index)

    for face in faces:
        if len(face.elements)==1:
            face.neighborInterpolation()
            face.getFirstOrderCoef()

    for face in faces:
        if len(face.elements)==1:
            face.getNewPoint()

    write_vtk("test_rocket.vtk", "test_rocket", nodes, elements)

refine_gmsh("rocket.msh")
