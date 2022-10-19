#ifndef DEF_ARRAY_HH
#define DEF_ARRAY_HH

#include <vector>
#include <string>

#include "Graph.hh"


using namespace std;

// Takes a list of vertices which are part of the initial graph, but which can also be neighbour (or not) with the new vertex. Returns all possible neighbourhoods given the initial neighbourhood of the new vertex (defined in curG).
// Recursive function.
void getPossibleFreeNeighourhoods(int nbVert, const vector<int> &freeVerts, vector<Graph> &ret, Graph &curG, int pos);

vector<vector<char>> compute_cleophee_arrays(const Graph &g, const vector<vector<int>> &adjSets, const vector<vector<int>> &antiCompleteSets, const vector<string> &setsNames, const vector<int> &freeVerts, bool print=false);

bool is_graph_ok(const Graph& g, bool print); // Tests if there is an obstruction of C4 or 4K1
#endif
