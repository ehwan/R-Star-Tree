#pragma once

/*
References:
Antonin Guttman, R-Trees: A Dynamic Index Structure for Spatial Searching, University if California Berkeley
*/

#include <limits>
#include <vector>
#include <utility>

#include "global.hpp"

namespace eh { namespace rtree {

class bound_t;
class node_t;
class RTree;

// bounding box representation
class bound_t
{
public:
  using point_type = int;

protected:
  point_type _min, _max;

public:
  bound_t( point_type const& __min, point_type const& __max )
    : _min(__min),
      _max(__max)
  {
  }


  inline auto const& min() const
  {
    return _min;
  }
  inline auto const& max() const
  {
    return _max;
  }

  // area of this bounding box
  auto area() const
  {
    return ( max() - min() );
  }
  bool is_inside( point_type const& p ) const
  {
    return (min() <= p) && (p < max());
  }
  bool is_inside( bound_t const& b ) const
  {
    return (min() <= b.min()) && (b.max() <= max());
  }
  bool is_overlap( point_type const& p ) const
  {
    return is_inside( p );
  }
  bool is_overlap( bound_t const& b ) const
  {
    if( !(b.min() < max()) ){ return false; }
    if( !(min() < b.max()) ){ return false; }
    return true;
  }

  bound_t merged( bound_t const& b ) const
  {
    return { std::min(_min,b.min()), std::max(_max,b.max()) };
  }
  bound_t merged( point_type const& p ) const
  {
    return { std::min(_min,p), std::max(_max,p) };
  }
  bound_t intersection( bound_t const& b ) const
  {
    const auto ret_min = std::max( min(), b.min() );
    return { ret_min, std::max( ret_min, std::min(max(),b.max()) ) };
  }
};
class node_t
{
friend class RTree;
public:
  using bound_type = bound_t;

  constexpr static int TYPE_NODE = 0;
  constexpr static int TYPE_LEAF = 1;
  constexpr static int TYPE_DATA = 2;

protected:
  // normal, leaf or entry
  int _type;
  std::vector<std::pair<bound_t,node_t*>> _child;
  node_t *_parent = nullptr;
  int _data;

public:
  using child_iterator = decltype(_child)::iterator;
  using const_child_iterator = decltype(_child)::const_iterator;

  node_t( int type )
    : _type( type )
  {
  }

  bool is_root() const
  {
    return _parent == nullptr;
  }
  bool is_node() const
  {
    return _type == TYPE_NODE;
  }
  bool is_leaf() const
  {
    return _type == TYPE_LEAF;
  }
  bool is_data() const
  {
    return _type == TYPE_DATA;
  }

  auto size() const
  {
    return _child.size();
  }
  child_iterator begin()
  {
    return _child.begin();
  }
  const_child_iterator begin() const
  {
    return _child.begin();
  }
  child_iterator end()
  {
    return _child.end();
  }
  const_child_iterator end() const
  {
    return _child.end();
  }
};


class RTree
{
public:
  using size_type = unsigned int;

  // data type
  using value_type = int;

  // single scalar
  using scalar_type = int;

  using node_type = node_t;

  // type for area
  using area_type = int;
  constexpr static area_type MAX_AREA = std::numeric_limits<area_type>::max();

  // multiple scalar; N-dimension
  using point_type = int;
  using bound_type = bound_t;

  // M
  constexpr static size_type MAX_ENTRIES = 8;
  // m <= M/2
  constexpr static size_type MIN_ENTRIES = 4;


protected:
  node_type *_root = nullptr;

public:

  void insert( bound_type const& bound, node_type const& node )
  {
    /*
    I1. [Find position for new record.]
    Invoke ChooseLeaf to select a leaf node L in which to place E.

    I2. [Add record to leaf node.] 
    If L has room for another entry, install E.
    Otherwise invoke SplitNode to obtain L and LL containing E and all the old entries of L.

    I3. [Propagate changes upward.] 
    Invoke AdjustTree on L, also passing LL if a split was performed.

    I4. [Grow tree taller.]
    If node split propagation caused the root to split, create a new root whose children are the two resulting nodes.
    */
  }

  node_type *choose_leaf( bound_type const& bound )
  {
    /*
    CLl. [Initialize.]
    Set N to be the root node.

    CL2. [Leaf check.]
    If N is a leaf, return N.

    CL3. [Choose subtree.]
    If N is not a leaf, let F be the entry in N whose rectangle F.I needs least enlargement to
    include E.I. Resolve ties by choosing the entry with the rectangle of smallest area.

    CL4. [Descend until a leaf is reached.]
    Set N to be the child node pointed to by F.p and repeat from CL2.
    */

    node_type *n = _root;
    while( n->is_leaf() == false )
    {
      area_type min_area_enlarge = MAX_AREA;
      node_type::child_iterator chosen = n->end();

      for( auto ci=n->begin(); ci!=n->end(); ++ci )
      {
        const auto area_enlarge = ci->first.merged(bound).area() - ci->first.area();
        if( area_enlarge < min_area_enlarge )
        {
          min_area_enlarge = area_enlarge;
          chosen = ci;
        }else if( area_enlarge == min_area_enlarge )
        {
          if( ci->first.area() < chosen->first.area() )
          {
            chosen = ci;
          }
        }
      }
      n = chosen->second;
    }
    return n;
  }

  void adjust_tree( node_type *N, bound_type const& new_bound, node_type *NN=nullptr )
  {
  /*
  AT1. [Initialize.] 
  Set N=L. If L was split previously, set NN to be the resulting second node.

  AT2. [Check if done.]
  If N is the root, stop.

  AT3. [Adjust covering rectangle in parent entry.]
  Let P be the parent node of N, and let E_N be N's entry in P.
  Adjust En.I so that it tightly encloses all entry rectangles in N.

  AT4. [Propagate node split upward.]
  If N has a partner NN resulting from an earlier split, create a new entry E_NN with E_NN.p pointing to NN and E_NN.I
  enclosing all rectangles in NN. 
  Add E_nn to P if there is room. 
  Otherwise, invoke SplitNode to produce P and PP containing E_NN and all P’s old entries.

  AT5. [Move up to next level.]
  Set N=P and set NN=PP if a split occurred.
  Repeat from AT2.
  */
  /*
    while( 1 )
    {
      if( N->is_root() ){ break; }

      auto *parent = N->_parent;
      node_type::child_iterator entry_on_parent = parent->end();
      for( auto ci=parent->begin(); ci!=parent->end(); ++ci )
      {
        if( &ci->second == N )
        {
          entry_on_parent = ci;
          break;
        }
      }
    }
    */
  }

#if 0
  struct split_quadratic_t
  {
    static std::pair<node_t::child_iterator,node_t::child_iterator> peek_seed( node_type *node )
    {
      node_type *n1 = nullptr, *n2 = nullptr;
      area_type max_wasted_area = 0;

      // choose two nodes that would waste the most area if both were put in the same group
      for( auto ci=node->begin(); ci!=node->end()-1; ++ci )
      {
        for( auto cj=ci+1; cj!=node->end(); ++cj )
        {
          auto union_area = ci->first.merged( cj->first ).area();
          const area_type wasted_area = union_area - ci->first.area() - cj->first.area();

          if( wasted_area > max_wasted_area )
          {
            max_wasted_area = wasted_area;
            n1 = ci->second;
            n2 = cj->second;
          }
        }
      }
      if( n1 == nullptr )
      {
        n1 = parent->begin()->second;
        n2 = (parent->begin()+1)->second;
      }
    }
  };

  // 'parent' contains M+1 nodes;
  // split into two nodes
  std::pair<node_type*,node_type*> split_quadratic( node_type *parent )
  {
  }
  #endif


};



}} // namespace eh rtree