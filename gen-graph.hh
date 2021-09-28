#ifndef DEF_GENGRAPH_HH
#define DEF_GENGRAPH_HH

#include <vector>
#include "Graph.hh"


vector<Graph> gen_graphs(int nbVert);

bool check_if_seen_and_add(const Graph& g, map<vector<int>, vector<Graph>> &dico);


vector<Graph> load_from_file(const string &filename);
void save_to_file(const string &filename, const map<vector<int>, vector<Graph>> &graphList, int nbGraph);


void gen_subsets(int k, int n, vector<vector<int>> &listRes);

#endif
