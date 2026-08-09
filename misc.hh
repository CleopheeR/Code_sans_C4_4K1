#ifndef DEF_MISC_HH
#define DEF_MISC_HH

#include <vector>

#include "sparsepp/spp.h"
#include "Graph.hh"


using spp::sparse_hash_map;
using namespace std;

// Prints the list of graphs in list1 not isomorphic to any graph in list2.
// Also checks for invalid graphs (containing a C4 or a 4K1).
void compare_two_graphs_sets(const sparse_hash_map<vector<char>, vector<Graph>> &list1,
        const sparse_hash_map<vector<char>, vector<Graph>> &list2);

// Print the count of minimal graphs among smallGraphs. That is, none in biggerGraphs is a supergraph of them.
// Also removes the non-minimal graphs from biggerGraphs.
// Note that graphs in biggerGraphs should have exactly one vertex more than the ones in smallGraphs.
void get_minimal_graphs(const vector<Graph> &smallGraphs, sparse_hash_map<vector<char>, vector<Graph>> &biggerGraphs);

/** Internal functions **/

// Removes from biggerGraphs any graph which is a supergraph of g. They must have precisely one more vertex than g.
void remove_nonminimal_graphs(const Graph &g, sparse_hash_map<vector<char>, vector<Graph>> &biggerGraphs, int idThread);
#endif
