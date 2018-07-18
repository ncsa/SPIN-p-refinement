'''
    @file: write_vtk.py
    @author: Hongru Yang
    @date: 07/16/2018
    @brief: definition of vtk writer function
'''

import numpy as np

'''
    Create a brand new vtk file function. If the file already exist, the function overwrites it.
    @author: Hongru Yang
    @date: 07/16/2018
    @param filename is a string;
    @param header is a string;
    @param meshes is a list of mesh objects
    @param type is the type of element (reference to vtk file format guide)
'''
def createVTKFile(filename, header, meshes, type):
    points = []
    point2index = {}
    i = 0
    for mesh in meshes:
        for pt in mesh.pts:
            if str(pt) not in point2index.keys():
                points.append(pt)
                point2index[str(pt)] = i
                i += 1

    f = open(filename, "w+")
    f.write("# vtk DataFile Version 1.0\n")
    f.write(header+"\n")
    f.write("ASCII\n\n")
    f.write("DATASET UNSTRUCTURED_GRID\n")
    f.write("POINTS       %d float\n" %len(points))

    for point in points:
        f.write("  %f       %f       %f    \n" %(point[0], point[1], point[2]))
    f.write("\n")

    size = len(meshes[0].pts)+1
    f.write("CELLS         %d          %d\n" %(len(meshes), size*len(meshes)))
    for mesh in meshes:
        l = [len(mesh.pts)]
        for pt in mesh.pts:
            l.append(point2index[str(pt)]) # assume points are in order
        f.write(" "+"        ".join(map(str, l))+"\n")
    f.write("\n")

    f.write("CELL_TYPES         %d\n" %len(meshes))
    cellType = str(type)+'\n'
    for i in range(len(meshes)):
        f.write(cellType)
