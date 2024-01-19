#include <RTree.hpp>

#include <random>
#include <iostream>

int main( int argc, char **argv )
{
  using point_type = eh::rtree::point_t<double,2>;
  using bound_type = eh::rtree::bound_t<point_type>;
  using rtree_type = eh::rtree::RTree< bound_type, bound_type, int >;

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

    rtree.insert( {bound_type{point,point}, i+1} );
  }

  // print tree structures to stdout
  std::ostream& output = std::cout;

  output << rtree.leaves_level() << "\n";



  for( int level=0; level<rtree.leaves_level(); ++level )
  {
    int count = 0;
    for( auto ni=rtree.begin(level); ni!=rtree.end(level); ++ni )
    {
      rtree_type::node_type *node = *ni;
      count += node->size();
    }
    output << count;

    for( auto ni=rtree.begin(level); ni!=rtree.end(level); ++ni )
    {
      rtree_type::node_type *node = *ni;

      for( rtree_type::node_type::value_type &c : *node )
      {
        output << " " << c.first.min_bound()[0] << " " << c.first.min_bound()[1];
        output << " " << c.first.max_bound()[0] << " " << c.first.max_bound()[1];
      }
    }
    output << "\n";
  }
  int count = 0;
  for( auto ni=rtree.leaf_begin(); ni!=rtree.leaf_end(); ++ni )
  {
    rtree_type::leaf_type *leaf = *ni;
    count += leaf->size();
  }
  output << count;
  for( auto ni=rtree.leaf_begin(); ni!=rtree.leaf_end(); ++ni )
  {
    rtree_type::leaf_type *leaf = *ni;
    for( rtree_type::leaf_type::value_type &c : *leaf )
    {
      output << " " << c.first.min_bound()[0] << " " << c.first.min_bound()[1];
      output << " " << c.first.max_bound()[0] << " " << c.first.max_bound()[1];
    }
  }
  output << "\n";

  return 0;
}