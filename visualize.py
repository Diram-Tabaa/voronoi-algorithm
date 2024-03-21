import matplotlib.pyplot as plt
import numpy as np
import ast

from matplotlib.collections import LineCollection


def read_outputs(filename):
    fd = open(filename, "r")
    vertices = fd.readline()
    vertices = vertices.strip(", \n")
    segments = fd.readline()
    segments = segments.strip(", \n")
    vertices = ast.literal_eval(vertices)
    segments = ast.literal_eval(segments)
    return vertices, segments

if __name__ == "__main__":

    fig, ax = plt.subplots()
    ax.set_xlim(-25, 25)
    ax.set_ylim(-25, 25)

    points, segments = read_outputs("outputs.txt")
    line_segments = LineCollection(segments, 
                                linestyles='solid', linewidths = (0.3))
    ax.add_collection(line_segments)
    ax.scatter(list(map(lambda x: x[0], points)), list(map(lambda x: x[1], points)), s = 1)
    ax.set_title('Line Collection with mapped colors')
    plt.savefig("result.png", dpi=500)