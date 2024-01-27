#pragma once

#include <cstdint>
#include <utility>
#include <cassert>
#include <iterator>

#include "global.hpp"
#include "geometry_traits.hpp"

namespace eh { namespace rtree {

template < typename TreeType >
struct static_node_t;

template < typename TreeType >
struct static_leaf_node_t;

template < typename TreeType >
struct static_node_base_t
{
  using node_base_type = static_node_base_t;
  using node_type = static_node_t<TreeType>;
  using leaf_type = static_leaf_node_t<TreeType>;

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
struct static_node_t
  : public static_node_base_t<TreeType>
{
  using parent_type = static_node_base_t<TreeType>;
  using node_base_type = parent_type;
  using node_type = static_node_t;
  using leaf_type = static_leaf_node_t<TreeType>;
  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;
  using value_type = std::pair<geometry_type,node_base_type*>;

  using iterator = value_type*;
  using const_iterator = value_type const*;

  alignas(value_type) uint8_t _data[ sizeof(value_type) * TreeType::MAX_ENTRIES ];
  size_type _size = 0;

  static_node_t() = default;
  static_node_t( static_node_t const& ) = delete;
  static_node_t& operator=( static_node_t const& ) = delete;
  static_node_t( static_node_t && ) = delete;
  static_node_t& operator=( static_node_t && ) = delete;

  ~static_node_t()
  {
    for( auto &c : *this )
    {
      c.~value_type();
    }
  }

  // add child node with bounding box
  void insert( value_type child )
  {
    assert( size() < TreeType::MAX_ENTRIES );
    child.second->_parent = this;
    child.second->_index_on_parent = size();
    new (data()+size()) value_type( std::move(child) );
    ++_size;
  }
  void erase( node_base_type *node )
  {
    assert( node->_parent == this );
    assert( size() > 0 );
    if( node->_index_on_parent < size()-1 )
    {
      back().second->_index_on_parent = node->_index_on_parent;
      at(node->_index_on_parent) = std::move( back() );
    }
    node->_parent = nullptr;
    pop_back();
  }
  void erase( iterator pos )
  {
    erase( pos->second );
  }

  void clear()
  {
    for( auto &c : *this )
    {
      c.~value_type();
    }
    _size = 0;
  }

  // swap two different child node (i, j)
  void swap( size_type i, size_type j )
  {
    assert( i != j );
    assert( i < size() );
    assert( j < size() );

    std::swap( at(i), at(j) );
    at(i).second->_index_on_parent = i;
    at(j).second->_index_on_parent = j;
  }
  void pop_back()
  {
    assert( size() > 0 );
    back().~value_type();
    --_size;
  }

  // child count
  size_type size() const
  {
    return _size;
  }
  bool empty() const
  {
    return _size == 0;
  }
  // child iterator
  iterator begin()
  {
    return data();
  }
  // child iterator
  const_iterator begin() const
  {
    return data();
  }
  // child iterator
  iterator end()
  {
    return data() + size();
  }
  // child iterator
  const_iterator end() const
  {
    return data() + size();
  }

  value_type& at( size_type i )
  {
    assert( i >= 0 );
    assert( i < size() );
    return data()[i];
  }
  value_type const& at( size_type i ) const
  {
    assert( i >= 0 );
    assert( i < size() );
    return data()[i];
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

  value_type *data()
  {
    return reinterpret_cast<value_type*>( _data );
  }
  value_type const* data() const
  {
    return reinterpret_cast<value_type const*>( _data );
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
    if( leaf_level == 1 )
    {
      // child is leaf node
      for( auto &c : *this )
      {
        new_node->insert({
          c.first, 
          c.second->as_leaf()->clone_recursive( tree )
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
struct static_leaf_node_t
  : public static_node_base_t<TreeType>
{
  using parent_type = static_node_base_t<TreeType>;
  using node_base_type = parent_type;
  using node_type = static_node_t<TreeType>;
  using leaf_type = static_leaf_node_t;
  using size_type = typename TreeType::size_type;
  using geometry_type = typename TreeType::geometry_type;
  using key_type = typename TreeType::key_type;
  using mapped_type = typename TreeType::mapped_type;
  using value_type = std::pair<key_type,mapped_type>;

  using iterator = value_type*;
  using const_iterator = value_type const*;

  alignas(value_type) uint8_t _data[ sizeof(value_type) * TreeType::MAX_ENTRIES ];
  size_type _size = 0;

  static_leaf_node_t() = default;
  static_leaf_node_t( static_leaf_node_t const& ) = delete;
  static_leaf_node_t& operator=( static_leaf_node_t const& ) = delete;
  static_leaf_node_t( static_leaf_node_t && ) = delete;
  static_leaf_node_t& operator=( static_leaf_node_t && ) = delete;

  ~static_leaf_node_t()
  {
    for( auto &c : *this )
    {
      c.~value_type();
    }
  }

  // add child node with bounding box
  void insert( value_type child )
  {
    assert( size() < TreeType::MAX_ENTRIES );
    new (data()+size()) value_type( std::move(child) );
    ++_size;
  }
  void erase( value_type *pos )
  {
    assert( size() > 0 );
    assert( std::distance(data(),pos) < size() );
    if( std::distance(data(),pos) < size()-1 )
    {
      *pos = std::move( back() );
    }
    pop_back();
  }

  void clear()
  {
    for( auto &c : *this )
    {
      c.~value_type();
    }
    _size = 0;
  }

  // swap two different child node (i, j)
  void swap( size_type i, size_type j )
  {
    assert( i != j );
    assert( i < size() );
    assert( j < size() );

    std::swap( at(i), at(j) );
  }
  void pop_back()
  {
    assert( size() > 0 );
    back().~value_type();
    --_size;
  }

  // child count
  size_type size() const
  {
    return _size;
  }
  bool empty() const
  {
    return _size == 0;
  }
  // child iterator
  iterator begin()
  {
    return data();
  }
  // child iterator
  const_iterator begin() const
  {
    return data();
  }
  // child iterator
  iterator end()
  {
    return data() + size();
  }
  // child iterator
  const_iterator end() const
  {
    return data() + size();
  }

  value_type& at( size_type i )
  {
    assert( i >= 0 );
    assert( i < size() );
    return data()[i];
  }
  value_type const& at( size_type i ) const
  {
    assert( i >= 0 );
    assert( i < size() );
    return data()[i];
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
  value_type *data()
  {
    return reinterpret_cast<value_type*>( _data );
  }
  value_type const* data() const
  {
    return reinterpret_cast<value_type const*>( _data );
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
  leaf_type *clone_recursive( TreeType &tree ) const
  {
    leaf_type *new_node = tree.template construct_node<leaf_type>();
    for( auto &c : *this )
    {
      new_node->insert( c );
    }
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