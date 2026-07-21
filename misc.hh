#ifndef DEF_MISC_HH
#define DEF_MISC_HH

#include <vector>

#include "sparsepp/spp.h"
#include "Graph.hh"


using spp::sparse_hash_map;
using namespace std;

void compare_two_fixeurs_sets(const sparse_hash_map<vector<char>, vector<Graph>> &list1,
        const sparse_hash_map<vector<char>, vector<Graph>> &list2);


void get_minimal_fixeurs(const vector<Graph> &prefixeurMinusList, sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict);

/** Internal functions **/
void remove_nonminimal_fixeurs(const Graph &g, sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat, int idThread);
#endif
