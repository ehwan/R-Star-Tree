#pragma once

/*
References:
Antonin Guttman, R-Trees: A Dynamic Index Structure for Spatial Searching, University if California Berkeley
*/

#include <limits>
#include <vector>
#include <utility>
#include <algorithm>
#include <cassert>

#include "global.hpp"
#include "iterator.hpp"
#include "node.hpp"
#include "static_node.hpp"
#include "quadratic_split.hpp"
#include "sequence_split.hpp"
#include "geometry_traits.hpp"

namespace eh { namespace rtree {

template < typename GeometryType, typename KeyType, typename MappedType, unsigned int MinEntry=4, unsigned int MaxEntry=8 >
class RTree
{
public:
  // using node_base_type = node_base_t<RTree>;
  // using node_type = node_t<RTree>;
  // using leaf_type = leaf_node_t<RTree>;

  // using stack allocator instead of std::vector
  using node_base_type = static_node_base_t<RTree>;
  using node_type = static_node_t<RTree>;
  using leaf_type = static_leaf_node_t<RTree>;

  using size_type = unsigned int;

  using geometry_type = GeometryType;
  using traits = geometry_traits<GeometryType>;
  using key_type = KeyType;
  using mapped_type = MappedType;
  using value_type = std::pair<key_type, mapped_type>;

  // type for area
  using area_type = typename geometry_traits<geometry_type>::area_type;
  constexpr static area_type MAX_AREA = std::numeric_limits<area_type>::max();
  constexpr static area_type LOWEST_AREA = std::numeric_limits<area_type>::lowest();

  // M
  constexpr static size_type MAX_ENTRIES = MaxEntry;
  // m <= M/2
  constexpr static size_type MIN_ENTRIES = MinEntry;
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
        // root is leaf node
        _root->as_leaf()->delete_recursive();
        delete _root->as_leaf();
      }else {
        _root->as_node()->delete_recursive( _leaf_level );
        delete _root->as_node();
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
  node_type *choose_insert_target( geometry_type const& bound, int target_level )
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

    node_type *n = _root->as_node();
    for( int level=0; level<target_level; ++level )
    {
      area_type min_area_enlarge = MAX_AREA;
      typename node_type::iterator chosen = n->end();

      for( auto ci=n->begin(); ci!=n->end(); ++ci )
      {
        const auto area_enlarge = 
          traits::area( traits::merge(ci->first,bound) ) - traits::area(ci->first);
        if( area_enlarge < min_area_enlarge )
        {
          min_area_enlarge = area_enlarge;
          chosen = ci;
        }else if( area_enlarge == min_area_enlarge )
        {
          if( traits::area(ci->first) < traits::area(chosen->first) )
          {
            chosen = ci;
          }
        }
      }
      assert( chosen != n->end() );
      n = chosen->second->as_node();
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
  void insert_node( geometry_type const& bound, node_base_type *node, node_type *parent )
  {
    node_type *pair = nullptr;
    if( parent->size() == MAX_ENTRIES )
    {
      pair = split( parent, typename node_type::value_type{bound,node} );
    }else {
      parent->insert( {bound, node} );
    }
    broadcast_new_bound( parent );
    if( pair )
    {
      if( parent == _root )
      {
        node_type *new_root = new node_type;
        new_root->insert( {parent->calculate_bound(), parent} );
        new_root->insert( {pair->calculate_bound(), pair} );
        _root = new_root;
        ++_leaf_level;
      }else {
        insert_node( pair->calculate_bound(), pair, parent->parent() );
      }
    }
  }

  // using splitter_t = sequence_split_t<RTree>;
  using splitter_t = quadratic_split_t<RTree>;
  
  // 'node' contains MAX_ENTRIES+1 nodes;
  // split into two nodes
  // so that two nodes' child count is in range [ MIN_ENTRIES, MAX_ENTRIES ]
  template < typename NodeType >
  NodeType* split( NodeType *node, typename NodeType::value_type child )
  {
    NodeType *pair = new NodeType;
    // @TODO another split scheme
    splitter_t spliter;
    return spliter( node, std::move(child), pair );
  }


public:

  void insert( value_type new_val )
  {
    leaf_type *chosen = choose_insert_target( new_val.first, _leaf_level )->as_leaf();
    leaf_type *pair = nullptr;
    if( chosen->size() == MAX_ENTRIES )
    {
      pair = split( chosen, std::move(new_val) );
    }else {
      chosen->insert( std::move(new_val) );
    }
    broadcast_new_bound( chosen );
    if( pair )
    {
      if( chosen->parent() == nullptr )
      {
        node_type *new_root = new node_type;
        new_root->insert( {chosen->calculate_bound(), chosen} );
        new_root->insert( {pair->calculate_bound(), pair} );
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
    leaf->erase( pos._pointer );

    if( leaf == _root ){ return; }

    // relative level from leaf, node's children will be reinserted later.
    std::vector< std::pair<int,node_base_type*> > reinsert_nodes;

    node_type *node = leaf->parent();
    if( leaf->size() < MIN_ENTRIES )
    {
      // delete node from node's parent
      node->erase( leaf );

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
        parent->erase( node );
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
      if( _root->as_node()->size() == 1 )
      {
        node_base_type *child = _root->as_node()->at(0).second;
        _root->as_node()->erase( child );
        delete _root->as_node();
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
        for( auto &c : *reinsert.second->as_leaf() )
        {
          insert( std::move( c ) );
        }
        delete reinsert.second->as_leaf();
      }else {
        for( auto &c : *reinsert.second->as_node() )
        {
          node_type *chosen = choose_insert_target( c.first, _leaf_level-reinsert.first );
          insert_node( c.first, c.second, chosen );
        }
        delete reinsert.second->as_node();
      }
    }
  }

  size_type size() const
  {
    if( _leaf_level == 0 )
    {
      return _root->as_leaf()->size_recursive();
    }else {
      return _root->as_node()->size_recursive( _leaf_level );
    }
  }

  void clear()
  {
    delete_if();
    set_null();
    init_root();
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
      _root = rhs._root->as_leaf()->clone_recursive();
      _leaf_level = 0;
    }else {
      _root = rhs._root->as_node()->clone_recursive( rhs._leaf_level );
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
      _root = rhs._root->as_leaf()->clone_recursive();
      _leaf_level = 0;
    }else {
      _root = rhs._root->as_node()->clone_recursive( rhs._leaf_level );
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
  ~RTree()
  {
    delete_if();
  }

  iterator begin()
  {
    node_type *n = _root->as_node();
    for( int level=0; level<_leaf_level; ++level )
    {
      n = n->at(0).second->as_node();
    }
    if( n->as_leaf()->empty() ){ return {}; }
    return {
      &n->as_leaf()->at(0),
      n->as_leaf()
    };
  }
  const_iterator cbegin() const
  {
    node_type const* n = _root->as_node();
    for( int level=0; level<_leaf_level; ++level )
    {
      n = n->at(0).second->as_node();
    }
    if( n->as_leaf()->empty() ){ return {}; }
    return { 
      &n->as_leaf()->at(0),
      n->as_leaf()
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
    node_type *n = _root->as_node();
    for( int l=0; l<level; ++l )
    {
      n = n->at(0).second->as_node();
    }
    return { n };
  }
  const_node_iterator begin( int level ) const
  {
    node_type const* n = _root->as_node();
    for( int l=0; l<level; ++l )
    {
      n = n->at(0).second->as_node();
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
    node_type *n = _root->as_node();
    for( int l=0; l<_leaf_level; ++l )
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
  }
  const_leaf_iterator leaf_begin() const
  {
    node_type const* n = _root->as_node();
    for( int l=0; l<_leaf_level; ++l )
    {
      n = n->at(0).second->as_node();
    }
    return { n->as_leaf() };
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
    return _root->as_node();
  }

  int leaf_level() const
  {
    return _leaf_level;
  }

protected:
  template < typename _GeometryType, typename Functor >
  bool search_overlap_wrapper( node_type *node, int leaf, _GeometryType const& search_range, Functor functor )
  {
    if( leaf == 0 )
    {
      if( search_overlap_leaf_wrapper(node->as_leaf(), search_range, functor) ){ return true; }
    }else {
      for( auto &c : *node )
      {
        if( traits::is_overlap(c.first, search_range) == false ){ continue; }
        if( search_overlap_wrapper( c.second->as_node(), leaf-1, search_range, functor ) ){ return true; }
      }
    }
    return false;
  }

  template < typename _GeometryType, typename Functor >
  bool search_inside_wrapper( node_type *node, int leaf, _GeometryType const& search_range, Functor functor )
  {
    if( leaf == 0 )
    {
      if( search_inside_leaf_wrapper(node->as_leaf(), search_range, functor) ){ return true; }
    }else {
      for( auto &c : *node )
      {
        if( traits::is_overlap(c.first, search_range) == false ){ continue; }
        if( search_inside_wrapper( c.second->as_node(), leaf-1, search_range, functor ) ){ return true; }
      }
    }
    return false;
  }
  template < typename _GeometryType, typename Functor >
  bool search_overlap_leaf_wrapper( leaf_type *node, _GeometryType const& search_range, Functor functor )
  {
    for( auto &c : *node )
    {
      if( traits::is_overlap(c.first, search_range) == false ){ continue; }
      if( functor( c ) ){ return true; }
    }
    return false;
  }
  template < typename _GeometryType, typename Functor >
  bool search_inside_leaf_wrapper( leaf_type *node, _GeometryType const& search_range, Functor functor )
  {
    for( auto &c : *node )
    {
      if( traits::is_inside(search_range, c.first) == false ){ continue; }
      if( functor( c ) ){ return true; }
    }
    return false;
  }

public:

  template < typename _GeometryType, typename Functor >
  void search_inside( _GeometryType const& search_range, Functor functor )
  {
    search_inside_wrapper( _root->as_node(), _leaf_level, search_range, functor );
  }
  template < typename _GeometryType, typename Functor >
  void search_overlap( _GeometryType const& search_range, Functor functor )
  {
    search_overlap_wrapper( _root->as_node(), _leaf_level, search_range, functor );
  }

};



}} // namespace eh rtree