#ifndef DEF_ARRAY_HH
#define DEF_ARRAY_HH

#include <vector>
#include <string>

#include "Graph.hh"


using namespace std;

void compute_cleophee_arrays(const Graph &g, const vector<vector<int>> &adjSets, const vector<vector<int>> &antiCompleteSets, const vector<string> &setsNames, const vector<int> &freeVerts);

#endif
