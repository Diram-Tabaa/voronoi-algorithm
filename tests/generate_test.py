import random 
import numpy as np
from svg.path import parse_path
from xml.dom import minidom


def get_random_point(xmin, xmax, ymin, ymax, fd):
    x = random.uniform(xmin, xmax)
    y = random.uniform(ymin, ymax)
    print("%.6f %.6f" % (x, y), file = fd)

def get_circular_point(radius, center_x, center_y, offset, npts, fd):
    angle = (offset/npts)*2*np.pi
    x = radius*np.cos(angle) 
    y = radius*np.sin(angle) 
    print("%.4f %.4f" % (x + center_x, y + center_y), file = fd)
    #return x + center_x, y + center_y

def get_convex_points(scale, start_x, start_y, offset, npts, fd):
    x = (offset / npts)*scale + start_x
    y = 0.2*x**2 + start_y
    print("%.4f %.4f" % (x, y), file = fd)

def space_fill_triangles(fd):
    for i in range(10):
        for j in range(10):
            x = (5*j - 22) 
            y = 5*i - 22 
            print("%.8f %.8f" % (x, y), file = fd)



def get_point_at(path, distance, scale, offset):
    pos = path.point(distance)
    pos += offset
    pos *= scale
    return pos.real*np.cos(0) - pos.imag*np.sin(0), pos.imag*np.cos(0) + pos.real*np.sin(0)


def points_from_path(path, density, scale, offset):
    step = int(path.length() * density)
    last_step = step - 1

    if last_step == 0:
        yield get_point_at(path, 0, scale, offset)
        return

    for distance in range(step):
        yield get_point_at(
            path, distance / last_step, scale, offset)


def points_from_doc(doc, density=5, scale=1, offset=0):
    offset = offset[0] + offset[1] * 1j
    points = []
    for element in doc.getElementsByTagName("path"):
        for path in parse_path(element.getAttribute("d")):
            points.extend(points_from_path(
                path, density, scale, offset))

    return points


string = """<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
    <path fill="none" stroke="red"
        d="M 10,30
            A 20,20 0,0,1 50,30
            A 20,20 0,0,1 90,30
            Q 90,60 50,90
            Q 10,60 10,30 z" />
</svg>"""

doc = minidom.parseString(string)
points = points_from_doc(doc, density=1, scale=0.2, offset=(-5, -5))
doc.unlink()


if __name__ == "__main__":

    
    
    xmin, xmax = -19, 19
    ymin, ymax = -19, 19
    fd = open("python_test", "w")
    npts = 100#len(points)
    print("%d" % npts, file = fd)
    space_fill_triangles(fd)
    #for i in range(npts):
        #print("%.10f %.10f" % (points[i][0], points[i][1]), file = fd)
        #get_circular_point(5, -5, -5, i, npts, fd)
        #get_circular_point(4, 4, 4, i, npts // 3, fd)
        #get_circular_point(10, 8, 4, i, npts // 3, fd)
        #get_convex_points(9, 0, 0, i, npts, fd)
        #get_random_point(xmin, xmax, ymin, ymax, fd)
