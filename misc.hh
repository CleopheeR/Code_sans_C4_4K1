#ifndef DEF_MISC_HH
#define DEF_MISC_HH

#include <vector>

#include "sparsepp/spp.h"
#include "Graph.hh"


using spp::sparse_hash_map;
using namespace std;

void compare_two_fixeurs_sets(const sparse_hash_map<vector<char>, vector<Graph>> &list1,
        const sparse_hash_map<vector<char>, vector<Graph>> &list2);

#endif
