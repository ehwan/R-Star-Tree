#pragma once

#include <vector>
#include <utility>
#include <cassert>

#include "global.hpp"
#include "geometry_traits.hpp"

namespace eh { namespace rtree {

template < typename TreeType >
struct node_t;

template < typename TreeType >
struct leaf_node_t;

template < typename TreeType >
struct node_base_t
{
  using node_base_type = node_base_t;
  using node_type = node_t<TreeType>;
  using leaf_type = leaf_node_t<TreeType>;

  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;

  node_type *_parent = nullptr;
  size_type _index_on_parent;

  // parent node's pointer
  node_type *parent() const
  {
    return _parent;
  }
  bool is_root() const
  {
    return _parent == nullptr;
  }


  auto& entry()
  {
    return parent()->at( _index_on_parent );
  }
  auto const& entry() const
  {
    return parent()->at( _index_on_parent );
  }

  inline node_type *as_node()
  {
    return reinterpret_cast<node_type*>( this );
  }
  inline node_type const* as_node() const
  {
    return reinterpret_cast<node_type const*>( this );
  }
  inline leaf_type *as_leaf()
  {
    return reinterpret_cast<leaf_type*>( this );
  }
  inline leaf_type const* as_leaf() const
  {
    return reinterpret_cast<leaf_type const*>( this );
  }

  // get next node on same level
  // this would return node across different parent
  // if it is last node, return nullptr
  node_base_type *next()
  {
    // if n is root
    if( _parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( _index_on_parent == parent()->size()-1 )
    {
      node_type *n = parent()->next();
      if( n == nullptr ){ return nullptr; }
      return n->front().second;
    }else {
      // else; return next node in same parent
      return parent()->at( _index_on_parent+1 ).second;
    }
  }
  node_base_type const* next() const
  {
    // if n is root
    if( this->_parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( this->_index_on_parent == parent()->size()-1 )
    {
      node_type const* n = parent()->next();
      if( n == nullptr ){ return nullptr; }
      return n->front().second;
    }else {
      // else; return next node in same parent
      return parent()->at( _index_on_parent+1 ).second;
    }
  }
  // get prev node on same level
  // this would return node across different parent
  // if it is first node, return nullptr
  node_base_type *prev()
  {
    // if n is root
    if( this->_parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( this->_index_on_parent == 0 )
    {
      node_type *n = parent()->prev();
      if( n == nullptr ){ return nullptr; }
      return n->back().second;
    }else {
      // else; return next node in same parent
      return parent()->at( _index_on_parent-1 ).second;
    }
  }
  node_base_type const* prev() const
  {
    // if n is root
    if( this->_parent == nullptr ){ return nullptr; }

    // if n is last node on parent
    // return parent's next's 0th child node
    if( this->_index_on_parent == 0 )
    {
      node_type const* n = parent()->prev();
      if( n == nullptr ){ return nullptr; }
      return n->back().second;
    }else {
      // else; return next node in same parent
      return parent()->at( _index_on_parent-1 ).second;
    }
  }
};

template < typename TreeType >
struct node_t
  : public node_base_t<TreeType>
{
  using parent_type = node_base_t<TreeType>;
  using node_base_type = parent_type;
  using node_type = node_t;
  using leaf_type = leaf_node_t<TreeType>;
  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;
  using value_type = std::pair<geometry_type,node_base_type*>;

  node_t() = default;
  node_t( node_t const& ) = delete;
  node_t& operator=( node_t const& ) = delete;
  node_t( node_t && ) = delete;
  node_t& operator=( node_t && ) = delete;

  std::vector<value_type> _child;

  using iterator = typename decltype(_child)::iterator;
  using const_iterator = typename decltype(_child)::const_iterator;

  // add child node with bounding box
  void insert( value_type child )
  {
    assert( size() < TreeType::MAX_ENTRIES );
    child.second->_parent = this;
    child.second->_index_on_parent = size();
    _child.push_back( std::move(child) );
  }
  void erase( node_base_type *node )
  {
    assert( size() > 0 );
    if( node->_index_on_parent < size()-1 )
    {
      _child.back().second->_index_on_parent = node->_index_on_parent;
      _child[node->_index_on_parent] = std::move(_child.back());
    }
    node->_parent = nullptr;
    _child.pop_back();
  }
  void erase( iterator pos )
  {
    erase( pos->second );
  }

  void clear()
  {
    _child.clear();
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
  iterator begin()
  {
    return _child.begin();
  }
  // child iterator
  const_iterator begin() const
  {
    return _child.begin();
  }
  // child iterator
  iterator end()
  {
    return _child.end();
  }
  // child iterator
  const_iterator end() const
  {
    return _child.end();
  }

  value_type& at( size_type i )
  {
    assert( i >= 0 );
    assert( i < size() );
    return _child[i];
  }
  value_type const& at( size_type i ) const
  {
    assert( i >= 0 );
    assert( i < size() );
    return _child[i];
  }
  value_type& operator[]( size_type i )
  {
    return at(i);
  }
  value_type const& operator[]( size_type i ) const
  {
    return at(i);
  }
  value_type& front()
  {
    assert( size() > 0 );
    return at( 0 );
  }
  value_type const& front() const
  {
    assert( size() > 0 );
    return at( 0 );
  }
  value_type& back()
  {
    assert( size() > 0 );
    return at( size()-1 );
  }
  value_type const& back() const
  {
    assert( size() > 0 );
    return at( size()-1 );
  }
  value_type* data()
  {
    return _child.data();
  }
  value_type const* data() const
  {
    return _child.data();
  }

  // union bouinding box of children
  geometry_type calculate_bound() const
  {
    assert( empty() == false );
    geometry_type merged = at(0).first;
    for( size_type i=1; i<size(); ++i )
    {
      merged = geometry_traits<geometry_type>::merge( merged, at(i).first );
    }
    return merged;
  }

  // delete its child recursively
  void delete_recursive( int leaf_level, TreeType& tree )
  {
    if( leaf_level == 1 )
    {
      // child is leaf node
      for( auto &c : *this )
      {
        c.second->as_leaf()->delete_recursive( tree );
        tree.destroy_node( c.second->as_leaf() );
      }
    }else {
      for( auto &c : *this )
      {
        c.second->as_node()->delete_recursive( leaf_level-1, tree );
        tree.destroy_node( c.second->as_node() );
      }
    }
  }
  node_type *clone_recursive( int leaf_level, TreeType& tree ) const
  {
    node_type *new_node = tree.template construct_node<node_type>();
    new_node->_child.reserve( size() );
    if( leaf_level == 1 )
    {
      // child is leaf node
      for( auto &c : *this )
      {
        new_node->insert({
          c.first, 
          c.second->as_leaf()->clone_recursive(tree)
        });
      }
    }else {
      for( auto &c : *this )
      {
        new_node->insert({
          c.first,
          c.second->as_node()->clone_recursive( leaf_level-1, tree )
        });
      }
    }
    return new_node;
  }
  size_type size_recursive( int leaf_level ) const
  {
    size_type ret = 0;
    if( leaf_level == 1 )
    {
      // child is leaf node
      for( auto &c : *this )
      {
        ret += c.second->as_leaf()->size_recursive();
      }
    }else {
      for( auto &c : *this )
      {
        ret += c.second->as_node()->size_recursive( leaf_level-1 );
      }
    }
    return ret;
  }

  node_type* next()
  {
    return parent_type::next()->as_node();
  }
  node_type const* next() const
  {
    return parent_type::next()->as_node();
  }
  node_type* prev()
  {
    return parent_type::prev()->as_node();
  }
  node_type const* prev() const
  {
    return parent_type::prev()->as_node();
  }


};

template < typename TreeType >
struct leaf_node_t
  : public node_base_t<TreeType>
{
  using parent_type = node_base_t<TreeType>;
  using node_base_type = parent_type;
  using node_type = node_t<TreeType>;
  using leaf_type = leaf_node_t;
  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;
  using value_type = std::pair<key_type,mapped_type>;

  std::vector<value_type> _child;

  using iterator = typename decltype(_child)::iterator;
  using const_iterator = typename decltype(_child)::const_iterator;

  leaf_node_t() = default;
  leaf_node_t( leaf_node_t const& ) = delete;
  leaf_node_t& operator=( leaf_node_t const& ) = delete;
  leaf_node_t( leaf_node_t && ) = delete;
  leaf_node_t& operator=( leaf_node_t && ) = delete;

  // add child node with bounding box
  void insert( value_type child )
  {
    assert( size() < TreeType::MAX_ENTRIES );
    _child.push_back( std::move(child) );
  }
  void erase( iterator pos )
  {
    erase( &*pos );
  }
  void erase( value_type *data )
  {
    assert( size() > 0 );
    if( std::distance(_child.data(),data) != size()-1 )
    {
      *data = std::move( _child.back() );
    }
    _child.pop_back();
  }

  void clear()
  {
    _child.clear();
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
  iterator begin()
  {
    return _child.begin();
  }
  // child iterator
  const_iterator begin() const
  {
    return _child.begin();
  }
  // child iterator
  iterator end()
  {
    return _child.end();
  }
  // child iterator
  const_iterator end() const
  {
    return _child.end();
  }

  value_type& at( size_type i )
  {
    assert( i >= 0 );
    assert( i < size() );
    return _child[i];
  }
  value_type const& at( size_type i ) const
  {
    assert( i >= 0 );
    assert( i < size() );
    return _child[i];
  }
  value_type& operator[]( size_type i )
  {
    return at(i);
  }
  value_type const& operator[]( size_type i ) const
  {
    return at(i);
  }
  value_type& front()
  {
    assert( size() > 0 );
    return at( 0 );
  }
  value_type const& front() const
  {
    assert( size() > 0 );
    return at( 0 );
  }
  value_type& back()
  {
    assert( size() > 0 );
    return at( size()-1 );
  }
  value_type const& back() const
  {
    assert( size() > 0 );
    return at( size()-1 );
  }
  value_type* data()
  {
    return _child.data();
  }
  value_type const* data() const
  {
    return _child.data();
  }

  // union bouinding box of children
  geometry_type calculate_bound() const
  {
    assert( empty() == false );
    geometry_type merged = at(0).first;
    for( int i=1; i<size(); ++i )
    {
      merged = geometry_traits<geometry_type>::merge( merged, at(i).first );
    }
    return merged;
  }

  // delete its child recursively
  void delete_recursive( TreeType& tree )
  {
  }
  leaf_type *clone_recursive( TreeType& tree ) const
  {
    leaf_type *new_node = tree.template construct_node<leaf_type>();
    new_node->_child = _child;
    return new_node;
  }
  size_type size_recursive() const
  {
    return size();
  }

  leaf_type* next()
  {
    return parent_type::next()->as_leaf();
  }
  leaf_type const* next() const
  {
    return parent_type::next()->as_leaf();
  }
  leaf_type* prev()
  {
    return parent_type::prev()->as_leaf();
  }
  leaf_type const* prev() const
  {
    return parent_type::prev()->as_leaf();
  }
};

}} // namespace eh rtree