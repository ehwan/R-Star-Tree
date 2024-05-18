#! /usr/bin/env python
import matplotlib.pyplot as plt
import numpy as np
import sys


def parse_tree_file( file_path ):
  with open( file_path, 'r') as file:
    leaf_level = int(file.readline())
    tree = []
    # parse non-leaf nodes
    for level in range(leaf_level):
      str_values = file.readline().split(' ')
      nodecount = int(str_values[0])
      nodes = []
      for i in range(nodecount):
        xmin = float(str_values[6*i+1])
        ymin = float(str_values[6*i+2])
        zmin = float(str_values[6*i+3])
        xmax = float(str_values[6*i+4])
        ymax = float(str_values[6*i+5])
        zmax = float(str_values[6*i+6])
        nodes.append( [xmin, ymin, zmin, xmax, ymax, zmax] )
        # nodes.insert( len(nodes), [a0, a1, a2, a3] )
      tree.append( nodes )

    points = []
    str_values = file.readline().split(' ')
    nodecount = int(str_values[0])
    nodes = []
    for i in range(nodecount):
      x = float(str_values[3*i+1])
      y = float(str_values[3*i+2])
      z = float(str_values[3*i+3])
      nodes.append( [x, y, z] )
    tree.append( nodes )
  return tree


colors = [ 'k', 'tab:orange', 'tab:blue', 'tab:purple' ]
linewidths = [ 1.5, 0.5, 0.5 ]

def plot_tree( tree ):
  ax = plt.axes(projection='3d')

  leaf_level = len(tree)-1
  points = tree[-1]
  Xs = [0] * len(points)
  Ys = [0] * len(points)
  Zs = [0] * len(points)
  for i, (x, y, z) in enumerate(points):
    Xs[i] = x
    Ys[i] = y
    Zs[i] = z
  ax.plot3D( Xs, Ys, Zs, '.', color=colors[-1] )

  for level in range(leaf_level-1,-1,-1):
    for (xmin,ymin,zmin,xmax,ymax,zmax) in tree[level]:
      Xs = [ xmin, xmax, xmax, xmin, xmin, xmin, xmax, xmax, xmin, xmin, xmax, xmax, xmax, xmax, xmin, xmin ]
      Ys = [ ymin, ymin, ymax, ymax, ymin, ymin, ymin, ymax, ymax, ymin, ymin, ymin, ymax, ymax, ymax, ymax ]
      Zs = [ zmin, zmin, zmin, zmin, zmin, zmax, zmax, zmax, zmax, zmax, zmax, zmin, zmin, zmax, zmax, zmin ]
      ax.plot3D( Xs, Ys, Zs, '-', color=colors[level], linewidth=linewidths[level] )



tree = parse_tree_file( sys.argv[1] )
plot_tree( tree )
plt.show()