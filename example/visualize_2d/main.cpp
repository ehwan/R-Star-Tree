#include <RTree.hpp>

#include <ios>
#include <limits>
#include <random>
#include <iostream>

int main( int argc, char **argv )
{
  using point_type = eh::rtree::point_t<double,2>;
  using bound_type = eh::rtree::bound_t<point_type>;
  using rtree_type = eh::rtree::RTree< bound_type, int >;

  std::mt19937 mt_engine{ std::random_device{}() };

  // normal distribution random generator for radius
  // mu = 0, sigma = 5
  std::normal_distribution<double> normal_distribute( 0, 5 );

  // uniform distribution in [0,\pi] for theta
  std::uniform_real_distribution<double> uniform_distribute( 0, std::atan2(1,1)*4 );

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
    double r = normal_distribute( mt_engine );
    double theta = uniform_distribute( mt_engine );

    const double epsilon = 1e-6;
    point_type point = { r*std::cos(theta), r*std::sin(theta) };
    point_type end_point = { point[0]+epsilon, point[1]+epsilon };

    rtree.insert( bound_type{point,end_point}, i+1 );
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
        output << " " << c.first.min_bound()[0] << " " << c.first.min_bound()[1];
        output << " " << c.first.max_bound()[0] << " " << c.first.max_bound()[1];
        bfs_pong.push_back( c.second );
      }
    }
    output << "\n";

    std::swap( bfs_nodes, bfs_pong );
    ++level;
  }

  return 0;
}