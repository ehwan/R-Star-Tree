#pragma once

#include <vector>
#include <utility>

#include "global.hpp"

namespace eh { namespace rtree {

template < typename BoundType, typename ValueType >
class node_t
{
  using node_type = node_t;

  template < typename _BoundType, typename _ValueType >
  friend class RTree;

public:
  using bound_type = BoundType;
  using value_type = ValueType;
  using size_type = unsigned int;

protected:
  node_type *_parent = nullptr;
  std::vector<std::pair<bound_type,node_type*>> _child;
  size_type _index_on_parent;
  value_type _data;

public:
  using child_iterator = typename decltype(_child)::iterator;
  using const_child_iterator = typename decltype(_child)::const_iterator;

  bool is_root() const
  {
    return _parent == nullptr;
  }

  // user data
  value_type& data()
  {
    return _data;
  }
  // user data
  value_type const& data() const
  {
    return _data;
  }

  // parent node's pointer
  node_type *parent() const
  {
    return _parent;
  }

  // this node's level; ( root = 0 )
  int level() const
  {
    int ret = 0;
    node_type const *n = this;
    while( n )
    {
      ++ret;
      n = n->_parent;
    }
    return ret-1;
  }

  // add child node with bounding box
  void add_child( bound_type const& bound, node_type *node )
  {
    _child.emplace_back( bound, node );
    node->_parent = this;
    node->_index_on_parent = _child.size()-1;
  }

  // child count
  size_type size() const
  {
    return _child.size();
  }
  bool empty() const
  {
    return _child.empty();
  }
  // child iterator
  child_iterator begin()
  {
    return _child.begin();
  }
  // child iterator
  const_child_iterator begin() const
  {
    return _child.begin();
  }
  // child iterator
  child_iterator end()
  {
    return _child.end();
  }
  // child iterator
  const_child_iterator end() const
  {
    return _child.end();
  }

  // union bouinding box of children
  bound_type calculate_bound() const
  {
    bound_type merged = _child[0].first;
    for( int i=1; i<_child.size(); ++i )
    {
      merged = merged.merged( _child[i].first );
    }
    return merged;
  }

  // delete its child recursively
  void delete_recursive()
  {
    for( auto &c : _child )
    {
      c.second->delete_recursive();
      delete c.second;
    }
  }
  node_type *clone_recursive() const
  {
    node_type *new_node = new node_type;
    new_node->_data = _data;
    new_node->_child.reserve( _child.size() );
    for( auto &c : *this )
    {
      new_node->add_child( c.first, c.second->clone_recursive() );
    }
    return new_node;
  }

  // get next node on same level
  // this would return node across different parent
  // if it is last node, return nullptr
  node_type *next()
  {
    // if n is root
    if( _parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( _index_on_parent == _parent->size()-1 )
    {
      node_type *n = _parent->next();
      if( n == nullptr ){ return nullptr; }
      return n->_child[0].second;
    }else {
      // else; return next node in same parent
      return _parent->_child[ _index_on_parent+1 ].second;
    }
  }
  node_type const* next() const
  {
    // if n is root
    if( _parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( _index_on_parent == _parent->size()-1 )
    {
      node_type const* n = _parent->next();
      if( n == nullptr ){ return nullptr; }
      return n->_child[0].second;
    }else {
      // else; return next node in same parent
      return _parent->_child[ _index_on_parent+1 ].second;
    }
  }
  // get prev node on same level
  // this would return node across different parent
  // if it is first node, return nullptr
  node_type *prev()
  {
    // if n is root
    if( _parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( _index_on_parent == 0 )
    {
      node_type *n = _parent->prev();
      if( n == nullptr ){ return nullptr; }
      return n->_child.back().second;
    }else {
      // else; return next node in same parent
      return _parent->_child[ _index_on_parent-1 ].second;
    }
  }
  node_type const* prev() const
  {
    // if n is root
    if( _parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( _index_on_parent == 0 )
    {
      node_type const* n = _parent->prev();
      if( n == nullptr ){ return nullptr; }
      return n->_child.back().second;
    }else {
      // else; return next node in same parent
      return _parent->_child[ _index_on_parent-1 ].second;
    }
  }
};

}} // namespace eh rtree