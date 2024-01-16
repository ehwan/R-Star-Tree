# RTree
**Currently Working in Progress.**

C++ template RTree header only library.

## References
 Guttman, A. (1984). "R-Trees: A Dynamic Index Structure for Spatial Searching" (PDF). Proceedings of the 1984 ACM SIGMOD international conference on Management of data – SIGMOD '84. p. 47.


## Visualization Examples
### 1-dimensional R-Tree structure visualization
`example/visualize_1d`

![](example/visualize_1d/images/N300Quadratic.png)

The x-axis illustrates different levels within the R-tree, and the y-axis displays the size of bounding boxes (in one dimension) for each node.

On the far right, there are purple dots representing input points (N = 300). These points are generated from a normal distribution with a mean ($\mu$) of 0 and a standard deviation ($\sigma$) of 5.