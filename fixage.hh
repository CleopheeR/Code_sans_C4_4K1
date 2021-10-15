#ifndef DEF_FIXAGE_HH
#define DEF_FIXAGE_HH

#include <vector>
#include <map>
#include "Graph.hh"
#include "sparsepp/spp.h"

using spp::sparse_hash_map;

bool is_fixeur(const Graph& g, const vector<vector<int>> &listSubsetsEdges);

bool is_pre_or_fixeur(Graph &g, vector<int> &degreeList, bool prefixeurTest, sparse_hash_map<vector<int>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat);


sparse_hash_map<vector<int>, vector<Graph>> gen_fixeurs(int nbVert);
#endif
