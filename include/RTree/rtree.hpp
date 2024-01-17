#pragma once

/*
References:
Antonin Guttman, R-Trees: A Dynamic Index Structure for Spatial Searching, University if California Berkeley
*/

#include <limits>
#include <vector>
#include <utility>
#include <algorithm>

#include "global.hpp"
#include "iterator.hpp"
#include "node.hpp"

namespace eh { namespace rtree {

template < typename BoundType, typename ValueType >
class RTree
{
public:
  using size_type = unsigned int;

  // data type
  using value_type = ValueType;

  // multiple scalar; N-dimension
  using bound_type = BoundType;

  using point_type = typename bound_type::point_type;

  // type for area
  using area_type = typename bound_type::area_type;
  constexpr static area_type MAX_AREA = std::numeric_limits<area_type>::max();
  constexpr static area_type LOWEST_AREA = std::numeric_limits<area_type>::lowest();

  // M
  constexpr static size_type MAX_ENTRIES = 8;
  // m <= M/2
  constexpr static size_type MIN_ENTRIES = 4;
  static_assert( MIN_ENTRIES <= MAX_ENTRIES/2, "Invalid MIN_ENTRIES count" );
  static_assert( MIN_ENTRIES >= 1, "Invalid MIN_ENTRIES count" );

  using node_type = node_t<bound_type,value_type>;

  using iterator = level_node_iterator_t<node_type>;
  using const_iterator = level_node_iterator_t<node_type const>;

protected:
  node_type *_root = nullptr;
  int _leaf_level = 0;

  void delete_if()
  {
    if( _root )
    {
      _root->delete_recursive();
      delete _root;
    }
  }
  void set_null()
  {
    _root = nullptr;
    _leaf_level = 0;
  }

  void init_root()
  {
    if( _root == nullptr )
    {
      _root = new node_type;
      _leaf_level = 0;
    }
  }

  // search for appropriate parent in target_level to insert bound
  node_type *choose_insert_parent( bound_type const& bound, int target_level )
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
    for( int level=0; level<target_level; ++level )
    {
      area_type min_area_enlarge = MAX_AREA;
      typename node_type::child_iterator chosen = n->end();

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
  // insert node to given parent
  void insert_node( bound_type const& bound, node_type *node, node_type *parent )
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
    parent->add_child( bound, node );
    node_type *pair = nullptr;

    if( parent->size() > MAX_ENTRIES )
    {
      pair = split( parent );
    }
    adjust_tree( parent );
    if( pair )
    {
      if( parent == _root )
      {
        node_type *new_root = new node_type;
        new_root->add_child( parent->calculate_bound(), parent );
        new_root->add_child( pair->calculate_bound(), pair );
        _root = new_root;
        ++_leaf_level;
      }else {
        insert_node( pair->calculate_bound(), pair, parent->parent() );
      }
    }
  }

  void adjust_tree( node_type *N )
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
    while( 1 )
    {
      if( N->parent() == nullptr ){ break; }
      N->parent()->_child[ N->_index_on_parent ].first = N->calculate_bound();
      N = N->parent();
    }
  }

  // node must be leaf node
  void condense_tree( node_type *node )
  {
    std::vector< std::pair<int,node_type*> > reinsert_nodes;
    for( int level=_leaf_level; level>0; --level )
    {
      if( node->size() < MIN_ENTRIES )
      {
        // delete node from node's parent
        if( node->_index_on_parent < node->parent()->size()-1 )
        {
          node->parent()->_child.back().second->_index_on_parent = node->_index_on_parent;
          node->parent()->_child[ node->_index_on_parent ] = node->parent()->_child.back();
        }
        node->parent()->_child.pop_back();
        // insert node to set
        reinsert_nodes.emplace_back( _leaf_level-level, node );
      }else {
        node->parent()->_child[ node->_index_on_parent ].first = node->calculate_bound();
      }
      node = node->parent();
    }

    // root adjustment
    if( _root->size() == 1 && _leaf_level > 0 )
    {
      node_type *child = _root->_child[0].second;
      delete _root;
      child->_parent = nullptr;
      _root = child;
      --_leaf_level;
    }

    // reinsert entries
    for( auto reinsert : reinsert_nodes )
    {
      for( auto &c : *reinsert.second )
      {
        node_type *chosen = choose_insert_parent( c.first, _leaf_level-reinsert.first );
        c.second->_parent = nullptr;
        insert_node( c.first, c.second, chosen );
      }
      delete reinsert.second;
    }


  }

public:

  RTree()
  {
    init_root();
  }
  RTree( RTree const& rhs )
  {
    _root = rhs._root->clone_recursive();
    _leaf_level = rhs._leaf_level;
  }
  RTree& operator=( RTree const& rhs )
  {
    delete_if();
    _root = rhs._root->clone_recursive();
    _leaf_level = rhs._leaf_level;
    return *this;
  }
  RTree( RTree &&rhs )
  {
    _root = rhs._root;
    _leaf_level = rhs._leaf_level;
    rhs.set_null();
    rhs.init_root();
  }
  RTree& operator=( RTree &&rhs )
  {
    delete_if();
    _root = rhs._root;
    _leaf_level = rhs._leaf_level;
    rhs.set_null();
    rhs.init_root();
    return *this;
  }

  iterator begin()
  {
    node_type *n = _root;
    for( int level=0; level<_leaf_level; ++level )
    {
      n = n->_child[0].second;
    }
    if( n->empty() ){ return {nullptr}; }
    return {n->_child[0].second};
  }
  const_iterator cbegin() const
  {
    node_type const* n = _root;
    for( int level=0; level<_leaf_level; ++level )
    {
      n = n->_child[0].second;
    }
    if( n->empty() ){ return {nullptr}; }
    return {n->_child[0].second};
  }
  const_iterator begin() const
  {
    return cbegin();
  }

  iterator begin( int target_level )
  {
    node_type *n = _root;
    for( int level=0; level<target_level; ++level )
    {
      n = n->_child[0].second;
    }
    return {n};
  }
  const_iterator cbegin( int target_level ) const
  {
    node_type const* n = _root;
    for( int level=0; level<target_level; ++level )
    {
      n = n->_child[0].second;
    }
    return {n};
  }
  const_iterator begin( int target_level ) const
  {
    return cbegin( target_level );
  }
  iterator end()
  {
    return {nullptr};
  }
  const_iterator cend() const
  {
    return {nullptr};
  }
  const_iterator end() const
  {
    return cend();
  }


  node_type *root() const
  {
    return _root;
  }


  int leaves_level() const
  {
    return _leaf_level;
  }


  // node must be data node;
  // which is, node's level is leaf_level+1
  void erase( node_type *node )
  {
    if( node->_index_on_parent < node->parent()->size()-1 )
    {
      node->parent()->_child.back().second->_index_on_parent = node->_index_on_parent;
      node->parent()->_child[ node->_index_on_parent ] = node->parent()->_child.back();
    }
    node->parent()->_child.pop_back();

    condense_tree( node->parent() );
    delete node;
  }

  // insert new data to appropriate parent
  void insert( bound_type const& bound, value_type val )
  {
    node_type *new_data_node = new node_type;
    new_data_node->_data = std::move(val);
    node_type *chosen = choose_insert_parent( bound, _leaf_level );
    insert_node( bound, new_data_node, chosen );
  }


  // split nodes 'as-is'
  struct just_split_t
  {
    node_type *operator()( node_type *node ) const
    {
      node_type *new_node = new node_type;
      for( int i=node->size()-MIN_ENTRIES; i<node->size(); ++i )
      {
        new_node->add_child( node->_child[i].first, node->_child[i].second );
      }
      node->_child.erase( node->_child.end()-MIN_ENTRIES, node->_child.end() );

      return new_node;
    }
  };

  // quadratic split algorithm
  struct quadratic_split_t
  {
    std::pair<typename node_type::child_iterator,typename node_type::child_iterator> pick_seed( node_type *node ) const
    {
      /*
      PS1. [Calculate inefficiency of grouping entries together.]
      For each pair of entries E1 and E2, compose a rectangle J including E1.I and E2.I. 
      Calculate d = area(J) - area(E1.I) - area(E2.I)

      PS2. [Choose the most wasteful pair.]
      Choose the pair with the largest d.
      */
      typename node_type::child_iterator n1 = node->end(), n2;
      area_type max_wasted_area = LOWEST_AREA;

      // choose two nodes that would waste the most area if both were put in the same group
      for( auto ci=node->begin(); ci!=node->end()-1; ++ci )
      {
        for( auto cj=ci+1; cj!=node->end(); ++cj )
        {
          const auto J = ci->first.merged( cj->first );
          const area_type wasted_area = J.area() - ci->first.area() - cj->first.area();

          if( wasted_area > max_wasted_area )
          {
            max_wasted_area = wasted_area;
            n1 = ci;
            n2 = cj;
          }
          // if same wasted area, choose pair with small intersection area
          else if( wasted_area == max_wasted_area )
          {
            if( ci->first.intersection(cj->first).area() < n1->first.intersection(n2->first).area() )
            {
              n1 = ci;
              n2 = cj;
            }
          }
        }
      }
      if( n1 == node->end() )
      {
        n1 = node->begin();
        n2 = node->begin()+1;
      }

      return { n1, n2 };
    }

    node_type* operator()( node_type *parent )
    {
      /*
      QS1. [Pick first entry for each group.]
      Apply Algorithm PickSeeds to choose two entries to be the first elements of the groups. 
      Assign each to a group.

      QS2. [Check if done.]
      If all entries have been assigned, stop. 
      If one group has so few entries that all the rest must
      be assigned to it in order for it to have the minimum number m, assign them and stop.

      QS3. [Select entry to assign.]
      Invoke Algorithm PickNext to choose the next entry to assign. 
      Add it to the group whose covering rectangle will have to be enlarged least to accommodate it.
      Resolve ties by adding the entry to the group with smaller area,
      then to the one with fewer entries, then to either. 
      Repeat from QS2.
      */
      std::vector<std::pair<bound_type,node_type*>> entry1, entry2;
      
      auto seeds = pick_seed( parent );
      entry1.push_back( *seeds.first );
      entry2.push_back( *seeds.second );
      bound_type bound1 = seeds.first->first;
      bound_type bound2 = seeds.second->first;
      parent->_child.erase( seeds.second );
      parent->_child.erase( seeds.first );

      while( parent->_child.empty() == false )
      {
        /*
        PN1. [Determine cost of putting each entry in each group.]
        For each entry E not yet in a group, 
        calculate d1 = the area increase required in the covering rectangle of Group 1 to include E.I. 
        Calculate d2  similarly for Group 2.

        PN2. [Find entry with greatest preference for one group.]
        Choose any entry with the maximum difference between d1 and d2.
        */

        const auto old_node_count = parent->size();
        if( entry1.size() + old_node_count == MIN_ENTRIES )
        {
          entry1.insert( entry1.end(), parent->begin(), parent->end() );
          parent->_child.clear();
        }else if( entry2.size() + old_node_count == MIN_ENTRIES )
        {
          entry2.insert( entry2.end(), parent->begin(), parent->end() );
          parent->_child.clear();
        }else {
          typename node_type::child_iterator picked = parent->end();
          int picked_to = 0;
          area_type maximum_difference = LOWEST_AREA;

          for( auto ci=parent->begin(); ci!=parent->end(); ++ci )
          {
            const area_type d1 = bound1.merged( ci->first ).area() - bound1.area();
            const area_type d2 = bound2.merged( ci->first ).area() - bound2.area();
            const auto diff = std::abs(d1 - d2);

            if( diff > maximum_difference )
            {
              picked = ci;
              maximum_difference = diff;
              picked_to = d1 < d2 ? 0 : 1;
            }
          }

          if( picked_to == 0 )
          {
            entry1.push_back( *picked );
            bound1 = bound1.merged( picked->first );
          }else {
            entry2.push_back( *picked );
            bound2 = bound2.merged( picked->first );
          }

          parent->_child.erase( picked );
        }
      }

      for( auto &e1 : entry1 )
      {
        parent->add_child( e1.first, e1.second );
      }
      node_type *node_pair = new node_type;
      for( auto &e2 : entry2 )
      {
        node_pair->add_child( e2.first, e2.second );
      }

      return node_pair;
    }
  };

  using splitter_t = quadratic_split_t;

  // 'parent' contains MAX_ENTRIES+1 nodes;
  // split into two nodes
  // so that two nodes' child count is in range [ MIN_ENTRIES, MAX_ENTRIES ]
  node_type* split( node_type *parent )
  {
    // @TODO another split scheme
    splitter_t spliter;
    return spliter( parent );
  }
};



}} // namespace eh rtree