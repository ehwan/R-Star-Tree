#pragma once

/*
References:
Antonin Guttman, R-Trees: A Dynamic Index Structure for Spatial Searching, University if California Berkeley
*/

#include <iterator>
#include <limits>
#include <vector>
#include <utility>
#include <algorithm>

#include "global.hpp"
#include "iterator.hpp"
#include "node.hpp"

namespace eh { namespace rtree {

template < typename BoundType, typename KeyType, typename MappedType >
class RTree
{
public:
  using node_base_type = node_base_t<BoundType,KeyType,MappedType>;
  using node_type = node_t<BoundType,KeyType,MappedType>;
  using leaf_type = leaf_node_t<BoundType,KeyType,MappedType>;

  using size_type = unsigned int;

  using bound_type = BoundType;
  using key_type = KeyType;
  using mapped_type = MappedType;

  using value_type = std::pair<key_type, mapped_type>;

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

  using iterator = iterator_t<leaf_type>;
  using const_iterator = iterator_t<leaf_type const>;

  using node_iterator = node_iterator_t<node_type>;
  using const_node_iterator = node_iterator_t<node_type const>;
  
  using leaf_iterator = node_iterator_t<leaf_type>;
  using const_leaf_iterator = node_iterator_t<leaf_type const>;

protected:
  node_base_type *_root = nullptr;
  int _leaf_level = 0;

  void delete_if()
  {
    if( _root )
    {
      if( _leaf_level == 0 )
      {
        reinterpret_cast<leaf_type*>( _root )->delete_recursive();
        delete reinterpret_cast<leaf_type*>( _root );
      }else {
        reinterpret_cast<node_type*>( _root )->delete_recursive( _leaf_level );
        delete reinterpret_cast<node_type*>( _root );
      }
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
      _root = new leaf_type;
      _leaf_level = 0;
    }
  }


  // search for appropriate node in target_level to insert bound
  node_type *choose_insert_target( bound_type const& bound, int target_level )
  {
    assert( target_level <= _leaf_level );
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

    node_type *n = reinterpret_cast<node_type*>(_root);
    for( int level=0; level<target_level; ++level )
    {
      area_type min_area_enlarge = MAX_AREA;
      typename node_type::iterator chosen = n->end();

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
      assert( chosen != n->end() );
      n = reinterpret_cast<node_type*>(chosen->second);
    }
    return n;
  }
  void broadcast_new_bound( node_type *N )
  {
    while( N->parent() )
    {
      N->entry().first = N->calculate_bound();
      N = N->parent();
    }
  }
  void broadcast_new_bound( leaf_type *leaf )
  {
    if( leaf->parent() )
    {
      leaf->entry().first = leaf->calculate_bound();
      broadcast_new_bound( leaf->parent() );
    }
  }

  // insert node to given parent
  void insert_node( bound_type const& bound, node_base_type *node, node_type *parent )
  {
    parent->add_child( bound, node );
    node_type *pair = nullptr;

    if( parent->size() > MAX_ENTRIES )
    {
      pair = split( parent );
    }
    broadcast_new_bound( parent );
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


public:

  void insert( value_type new_val )
  {
    leaf_type *chosen = reinterpret_cast<leaf_type*>(choose_insert_target( new_val.first, _leaf_level ));
    chosen->insert( std::move(new_val) );
    leaf_type *pair = nullptr;
    if( chosen->size() > MAX_ENTRIES )
    {
      pair = split( chosen );
    }
    broadcast_new_bound( chosen );
    if( pair )
    {
      if( chosen->parent() == nullptr )
      {
        node_type *new_root = new node_type;
        new_root->add_child( chosen->calculate_bound(), chosen );
        new_root->add_child( pair->calculate_bound(), pair );
        _root = new_root;
        ++_leaf_level;
      }else {
        insert_node( pair->calculate_bound(), pair, chosen->parent() );
      }
    }
  }

  void erase( iterator pos )
  {
    leaf_type *leaf = pos._leaf;
    leaf->_child.erase( leaf->_child.begin()+std::distance(leaf->_child.data(),pos._pointer) );

    if( leaf == _root ){ return; }

    // relative level from leaf, node's children will be reinserted later.
    std::vector< std::pair<int,node_base_type*> > reinsert_nodes;

    node_type *node = leaf->parent();
    if( leaf->size() < MIN_ENTRIES )
    {
      // delete node from node's parent
      node->erase_child( leaf );

      // insert node to set
      reinsert_nodes.emplace_back( 0, leaf );
    }else {
      leaf->entry().first = leaf->calculate_bound();
    }
    for( int level=_leaf_level-1; level>0; --level )
    {
      node_type *parent = node->parent();
      if( node->size() < MIN_ENTRIES )
      {
        // delete node from node's parent
        parent->erase_child( node );
        // insert node to set
        reinsert_nodes.emplace_back( _leaf_level-level, node );
      }else {
        node->entry().first = node->calculate_bound();
      }
      node = parent;
    }

    // root adjustment
    if( _leaf_level > 0 )
    {
      if( reinterpret_cast<node_type*>(_root)->size() == 1 )
      {
        node_base_type *child = reinterpret_cast<node_type*>(_root)->_child[0].second;
        reinterpret_cast<node_type*>(_root)->erase_child( child );
        delete reinterpret_cast<node_type*>(_root);
        _root = child;
        --_leaf_level;
      }
    }

    // reinsert entries
    // sustain the relative level from leaf
    for( auto reinsert : reinsert_nodes )
    {
      // leaf node
      if( reinsert.first == 0 )
      {
        for( auto &c : *reinterpret_cast<leaf_type*>(reinsert.second) )
        {
          insert( std::move( c ) );
        }
        delete reinterpret_cast<leaf_type*>(reinsert.second);
      }else {
        for( auto &c : *reinterpret_cast<node_type*>(reinsert.second) )
        {
          node_type *chosen = choose_insert_target( c.first, _leaf_level-reinsert.first );
          insert_node( c.first, c.second, chosen );
        }
        delete reinterpret_cast<node_type*>(reinsert.second);
      }
    }
  }

  RTree()
  {
    init_root();
  }

  // @TODO
  // mapped_type copy-assignable
  RTree( RTree const& rhs )
  {
    if( rhs._leaf_level == 0 )
    {
      _root = reinterpret_cast<leaf_type*>(rhs._root)->clone_recursive();
      _leaf_level = 0;
    }else {
      _root = reinterpret_cast<node_type*>(rhs._root)->clone_recursive(rhs._leaf_level);
      _leaf_level = rhs._leaf_level;
    }
  }
  // @TODO
  // mapped_type copy-assignable
  RTree& operator=( RTree const& rhs )
  {
    delete_if();
    if( rhs._leaf_level == 0 )
    {
      _root = reinterpret_cast<leaf_type*>(rhs._root)->clone_recursive();
      _leaf_level = 0;
    }else {
      _root = reinterpret_cast<node_type*>(rhs._root)->clone_recursive(rhs._leaf_level);
      _leaf_level = rhs._leaf_level;
    }
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
    node_type *n = reinterpret_cast<node_type*>(_root);
    for( int level=0; level<_leaf_level; ++level )
    {
      n = reinterpret_cast<node_type*>(n->_child[0].second);
    }
    if( reinterpret_cast<leaf_type*>(n)->empty() ){ return {}; }
    return { 
      &reinterpret_cast<leaf_type*>(n)->_child[0],
      reinterpret_cast<leaf_type*>(n)
    };
  }
  const_iterator cbegin() const
  {
    node_type const* n = reinterpret_cast<node_type const*>(_root);
    for( int level=0; level<_leaf_level; ++level )
    {
      n = reinterpret_cast<node_type const*>(n->_child[0].second);
    }
    if( reinterpret_cast<leaf_type const*>(n)->empty() ){ return {}; }
    return { 
      &reinterpret_cast<leaf_type const*>(n)->_child[0],
      reinterpret_cast<leaf_type const*>(n)
    };
  }
  const_iterator begin() const
  {
    return cbegin();
  }

  iterator end()
  {
    return {};
  }
  const_iterator cend() const
  {
    return {};
  }
  const_iterator end() const
  {
    return {};
  }

  node_iterator begin( int level )
  {
    node_type *n = reinterpret_cast<node_type*>(_root);
    for( int l=0; l<level; ++l )
    {
      n = reinterpret_cast<node_type*>(n->_child[0].second);
    }
    return { n };
  }
  const_node_iterator begin( int level ) const
  {
    node_type const* n = reinterpret_cast<node_type const*>(_root);
    for( int l=0; l<level; ++l )
    {
      n = reinterpret_cast<node_type const*>(n->_child[0].second);
    }
    return { n };
  }
  const_node_iterator cbegin( int level ) const
  {
    return begin(level);
  }

  node_iterator end( int level )
  {
    return {};
  }
  const_node_iterator end( int level ) const
  {
    return {};
  }
  const_node_iterator cend( int level ) const
  {
    return {};
  }

  leaf_iterator leaf_begin()
  {
    node_type *n = reinterpret_cast<node_type*>(_root);
    for( int l=0; l<_leaf_level; ++l )
    {
      n = reinterpret_cast<node_type*>(n->_child[0].second);
    }
    return { reinterpret_cast<leaf_type*>(n) };
  }
  const_leaf_iterator leaf_begin() const
  {
    node_type const* n = reinterpret_cast<node_type const*>(_root);
    for( int l=0; l<_leaf_level; ++l )
    {
      n = reinterpret_cast<node_type const*>(n->_child[0].second);
    }
    return { reinterpret_cast<leaf_type const*>(n) };
  }
  const_leaf_iterator leaf_cbegin() const
  {
    return leaf_begin();
  }
  leaf_iterator leaf_end()
  {
    return {};
  }
  const_leaf_iterator leaf_end() const
  {
    return {};
  }
  const_leaf_iterator leaf_cend() const
  {
    return {};
  }

  node_type *root() const
  {
    return _root;
  }

  int leaves_level() const
  {
    return _leaf_level;
  }




  // split nodes 'as-is'
  struct just_split_t
  {
    node_type *operator()( node_type *node ) const
    {
      node_type *new_node = new node_type;
      for( auto i=node->_child.end()-MIN_ENTRIES; i!=node->_child.end(); ++i )
      {
        new_node->add_child( i->first, i->second );
      }
      node->_child.erase( node->_child.end()-MIN_ENTRIES, node->_child.end() );

      return new_node;
    }
    leaf_type *operator()( leaf_type *node ) const
    {
      leaf_type *new_node = new leaf_type;
      for( auto i=node->_child.end()-MIN_ENTRIES; i!=node->_child.end(); ++i )
      {
        new_node->insert( std::move(*i) );
      }
      node->_child.erase( node->_child.end()-MIN_ENTRIES, node->_child.end() );

      return new_node;
    }
  };

  // quadratic split algorithm
  struct quadratic_split_t
  {
    template < typename NodeType >
    std::pair<typename NodeType::iterator,typename NodeType::iterator>
    pick_seed( NodeType *node ) const
    {
      /*
      PS1. [Calculate inefficiency of grouping entries together.]
      For each pair of entries E1 and E2, compose a rectangle J including E1.I and E2.I. 
      Calculate d = area(J) - area(E1.I) - area(E2.I)

      PS2. [Choose the most wasteful pair.]
      Choose the pair with the largest d.
      */
      typename NodeType::iterator n1 = node->end(), n2;
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
      assert( n1 != node->end() );

      return { n1, n2 };
    }

    template < typename NodeType >
    NodeType* operator()( NodeType *node ) const
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
      std::vector<typename NodeType::value_type> entry1, entry2;

      auto seeds = pick_seed( node );
      entry1.push_back( std::move(*seeds.first) );
      entry2.push_back( std::move(*seeds.second) );
      bound_type bound1 = entry1.front().first;
      bound_type bound2 = entry2.front().first;
      node->_child.erase( seeds.second );
      node->_child.erase( seeds.first );

      while( node->_child.empty() == false )
      {
        /*
        PN1. [Determine cost of putting each entry in each group.]
        For each entry E not yet in a group, 
        calculate d1 = the area increase required in the covering rectangle of Group 1 to include E.I. 
        Calculate d2  similarly for Group 2.

        PN2. [Find entry with greatest preference for one group.]
        Choose any entry with the maximum difference between d1 and d2.
        */

        const auto old_node_count = node->size();
        if( entry1.size() + old_node_count == MIN_ENTRIES )
        {
          entry1.insert(
            entry1.end(),
            std::make_move_iterator(node->begin()),
            std::make_move_iterator(node->end())
          );
          node->_child.clear();
        }else if( entry2.size() + old_node_count == MIN_ENTRIES )
        {
          entry2.insert(
            entry2.end(),
            std::make_move_iterator(node->begin()),
            std::make_move_iterator(node->end())
          );
          node->_child.clear();
        }else {
          typename NodeType::iterator picked = node->end();
          int picked_to = 0;
          area_type maximum_difference = LOWEST_AREA;

          for( auto ci=node->begin(); ci!=node->end(); ++ci )
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
            entry1.push_back( std::move(*picked) );
            bound1 = bound1.merged( entry1.back().first );
          }else {
            entry2.push_back( std::move(*picked) );
            bound2 = bound2.merged( entry2.back().first );
          }

          node->_child.erase( picked );
        }
      }

      for( auto &e1 : entry1 )
      {
        node->insert( std::move(e1) );
      }
      NodeType *node_pair = new NodeType;
      for( auto &e2 : entry2 )
      {
        node_pair->insert( std::move(e2) );
      }

      return node_pair;
    }
  };

  using splitter_t = quadratic_split_t;

  
  // 'node' contains MAX_ENTRIES+1 nodes;
  // split into two nodes
  // so that two nodes' child count is in range [ MIN_ENTRIES, MAX_ENTRIES ]
  template < typename NodeType >
  NodeType* split( NodeType *node )
  {
    // @TODO another split scheme
    splitter_t spliter;
    return spliter( node );
  }
};



}} // namespace eh rtree