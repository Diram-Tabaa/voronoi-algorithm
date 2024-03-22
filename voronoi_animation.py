#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import ast

from matplotlib.collections import LineCollection
from matplotlib.animation import FuncAnimation
import voronoi
import time


# setting up the points and their velocities
np_points = np.random.uniform(low=-20, high= 20, size=(100,2))
velocities = np.random.randn(100, 2) * 0.2 
points = np_points.tolist()
points= list(map(tuple, points))

#computing voronoi/deluanay
vor, deluan = voronoi.voronoi(points)
vor_segments, vor_rays = vor

# setting up the graph
fig, ax = plt.subplots()
ax.set_xlim(-25, 25)
ax.set_ylim(-25, 25)
scatter = ax.scatter(list(map(lambda x: x[0], points)), list(map(lambda x: x[1], points)), s = 1)
line_segments = LineCollection(deluan, linestyles='solid', linewidths = (0.3))
ax.add_collection(line_segments)
ax.set_title('Voronoi diagram/Deluanay triangulation of randomly generated points')

def update(frame):

    global np_points
    global velocities

    np_points += velocities
    for i, (x, y) in enumerate(np_points):
        if x <= -25 or x >= 25:
            velocities[i, 0] = -velocities[i, 0]
        if y <= -25 or y >= 25:
            velocities[i, 1] = -velocities[i, 1]
        velocities[i] += np.random.uniform(low=-1,high=1, size= (2,)) * 0.005

    points = np_points.tolist()

    scatter.set_offsets(points)

    #computing voronoi/deluanay
    points= list(map(tuple, points))
    vor, deluan = voronoi.voronoi(points)
    vor_segments, vor_rays = vor

    line_segments.set_segments(deluan)
    
    return scatter, line_segments

ani = FuncAnimation(fig, update, frames=range(100), blit=True, interval=20)
plt.show()
