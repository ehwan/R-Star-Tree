#! /bin/python3
import matplotlib.pyplot as plt
import sys


def parse_tree_file( file_path ):
  with open( file_path, 'r') as file:
    leaf_level = int(file.readline())
    tree = []
    # parse non-leaf nodes
    for node_id in range(leaf_level):
      str_values = file.readline().split(' ')
      rangecount = int(str_values[0])
      nodes = []
      for i in range(rangecount):
        a0 = float(str_values[4*i+1])
        a1 = float(str_values[4*i+2])
        a2 = float(str_values[4*i+3])
        a3 = float(str_values[4*i+4])
        nodes.insert( len(nodes), [a0, a1, a2, a3] )
      tree.insert( len(tree), nodes )

    points = []
    str_values = file.readline().split(' ')
    rangecount = int(str_values[0])
    nodes = []
    for i in range(rangecount):
      a0 = float(str_values[2*i+1])
      a1 = float(str_values[2*i+2])
      nodes.insert( len(nodes), [a0, a1] )
    tree.insert( len(tree), nodes )
  return tree


colors = [ 'k', 'tab:orange', 'tab:blue', 'tab:purple' ]
linewidths = [ 1.5, 1, 0.5 ]

def plot_tree( tree, N ):
  leaf_level = len(tree)-1
  level_offset = 3 - leaf_level
  points = tree[-1]
  Xs = [0] * len(points)
  Ys = [0] * len(points)
  for i, (x, y) in enumerate(points):
    Xs[i] = x
    Ys[i] = y
  # leaf
  plt.plot( Xs, Ys, '.', label='Input Points', color=colors[3] )

  for level in range(leaf_level-1,-1,-1):
    bounds = tree[level]
    for bi,bound in enumerate(bounds):
      Xs = [ bound[0], bound[2], bound[2], bound[0], bound[0] ]
      Ys = [ bound[1], bound[1], bound[3], bound[3], bound[1] ]
      plt.plot( Xs, Ys, '-', color=colors[level_offset+level], linewidth=linewidths[level_offset+level] )

  plt.title(f'N = {N}')

for i in range(1,1001):
  print(i)
  plt.cla()
  plt.clf()
  plt.cla()
  tree = parse_tree_file(f"../outs/Point{i}.txt")
  plot_tree(tree,i)
  plt.savefig(f"Point{i:04d}.png")