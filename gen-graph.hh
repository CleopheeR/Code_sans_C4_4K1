#ifndef DEF_GENGRAPH_HH
#define DEF_GENGRAPH_HH

#include <vector>
#include "Graph.hh"


vector<Graph> gen_graphs(int nbVert);

bool check_if_seen_and_add(const Graph& g, map<vector<int>, vector<Graph>> &dico);

#endif
