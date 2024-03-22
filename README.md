# Voronoi: Fortune's Algorithm Implementation in C

## Introduction

This respository contains a C implementation of Fortune's algorithm for the construction of the voronoi diagram for a set of points. 

This repository also comes with a python API for integration with python GUI animations

## Installation

### Python 3.10 API (Recommended)

1. Compile the source code into a python module by running the following, this will generate a local shared object ```.so``` file that python would use at runtime
```
make python
```

2. Import voronoi library into your python script. This library provides one function, namely ``` voronoi```, which takes in a list of points, each being a list of x and y coordinates, and returns the computed segments for both the voronoi and delaunay triangulation 

```python
import voronoi

#pack your points as list of lists
points = [[1.0,-1.0], [1.23, -4.5], [2.1, 4.1], [7.1, -9.0]]
#run the vornoni algorithm 
voronoi, delaunay = voronoi.voronoi(points)
#unpack the voronoi into segments and rays 
voronoi_segments, voronoi_rays = voronoi
# voronoi_rays are of the form [(x_1, y_1, grad_1), ...]
# voronoi_segments and delaunay are of the form [((x_1, y_1), (x'_1, y'_1)), ...]
```

3. As an example you can try running ``` voronoi_animation.py ```, which computes and renders delaunay/voronoi of a set of randomly generated points in real time and displays an animation of that as the points move around 

```shell
#ensure you have X Window System enabled in your session
python3 voronoi_animation.py 
```

### I/O-Interface Executable 

1. Compile the source code into an executable by running ```make```

```
make
```
2. List all your input points in the following format as an input file for the executable, where N is the number of points, and where each line contains the x and y coordinate respectively.

```
N
1.023 1.045
1.312 1.312 
...
```


3. Run the executable with the input points file, and pass ```stdout``` output into ```outputs.txt```, which will contain both the points as well as the segments in python list format, that can be parsed by ```visualize.py```

```
./voronoi input_file > outputs.txt 
```


4. Visualize the points by running ```visualize.py```, this will generate an image ```result.png``` of the voronoi diagram

```
python3 visualize.py
```

## Known Issues

1. Bug that involves the deletion of certain arc intersections for edge cases, where the respective intersections are not found in the beachline tree

