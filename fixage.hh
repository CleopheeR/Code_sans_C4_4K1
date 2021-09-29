#ifndef DEF_FIXAGE_HH
#define DEF_FIXAGE_HH

#include <vector>
#include <map>
#include "Graph.hh"


bool is_fixeur(const Graph& g, const vector<vector<int>> &listSubsetsEdges);

bool is_pre_or_fixeur(const Graph &g, const vector<vector<int>> &listSubsetsEdges, bool prefixeurTest, map<vector<int>, vector<Graph>> &fixeurDict, map<vector<int>, vector<Graph>> &prefixeurDict);


map<vector<int>, vector<Graph>> gen_fixeurs(int nbVert);
#endif
