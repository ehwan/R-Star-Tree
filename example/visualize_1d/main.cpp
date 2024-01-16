#include <RTree.hpp>

#include <ios>
#include <limits>
#include <random>
#include <iostream>
#include <fstream>

int main( int argc, char **argv )
{
  using bound_type = eh::rtree::bound_t<double>;
  using rtree_type = eh::rtree::RTree< bound_type, int >;

  std::mt19937 mt_engine{ std::random_device{}() };

  // normal distribution random generator
  // mu = 0, sigma = 5
  std::normal_distribution<float> normal_distribute( 0, 5 );

  if( argc < 2 )
  {
    std::cerr << "Invalid Arguments:\n";
    std::cerr << argv[0] << " (Number of Points)\n";
    return 0;
  }

  const int N = std::atoi( argv[1] );

  rtree_type rtree;
  for( int i=0; i<N; ++i )
  {
    double point = normal_distribute(mt_engine);
    double end_point = point + 1e-9;

    rtree.insert( {point,end_point}, i+1 );
  }

  // print tree structures to stdout
  std::ostream& output = std::cout;

  output << rtree.leaves_level() << "\n";

  std::vector<rtree_type::node_type*> bfs_nodes, bfs_pong;
  int level = 0;
  bfs_nodes.push_back( rtree.root_node() );

  while( level <= rtree.leaves_level() )
  {
    bfs_pong.clear();
    int count = 0;
    for( auto *n : bfs_nodes )
    {
      count += n->size();
    }
    output << count;

    for( auto *n : bfs_nodes )
    {
      for( auto &c : *n )
      {
        output << " " << c.first.min_bound() << " " << c.first.max_bound();
        bfs_pong.push_back( c.second );
      }
    }
    output << "\n";

    std::swap( bfs_nodes, bfs_pong );
    ++level;
  }

  return 0;
}