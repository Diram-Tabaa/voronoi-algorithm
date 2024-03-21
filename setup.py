#!/usr/bin/env python3

from setuptools import setup, Extension

setup(
	name = "voronoi",
	version = "1.0",
	ext_modules = [Extension("voronoi", ["uarray.c", "bst.c", "geometry.c", "priority_queue.c", "voronoi.c", "voronoipy.c"])]
	)