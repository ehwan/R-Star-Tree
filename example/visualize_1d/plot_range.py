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
        beg = float(str_values[2*i+1])
        end = float(str_values[2*i+2])
        nodes.insert( len(nodes), [beg,end] )
      tree.insert( len(tree), nodes )

    points = []
    str_values = file.readline().split(' ')
    rangecount = int(str_values[0])
    nodes = []
    for i in range(rangecount):
      beg = float(str_values[2*i+1])
      nodes.insert( len(nodes), beg )
    tree.insert( len(tree), nodes )
  return tree

def plot_tree( tree ):
  leaf_level = len(tree)-1
  for level in range(leaf_level):
    bounds = tree[level]
    for bi,bound in enumerate(bounds):
      offset = (bi/len(bounds) * 0.85) % 0.5 - 0.25
      Xs = [ level+offset, level+offset ]
      Ys = [ bound[0], bound[1] ]
      plt.plot( Xs, Ys, 'o-' )
  points = tree[-1]
  Xs = [ leaf_level ] * len(points)
  Ys = points
  plt.plot( Xs, Ys, '.', label='Input Points' )



tree = parse_tree_file( sys.argv[1] )
plot_tree( tree )
plt.xlabel( 'Level' )
plt.ylabel( 'BoundingBox' )
plt.xticks( range(len(tree)) )
plt.show()