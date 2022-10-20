#ifndef DEF_ARRAY_HH
#define DEF_ARRAY_HH

#include <vector>
#include <string>
#include "sparsepp/spp.h"

#include "Graph.hh"


using namespace std;

// Takes a list of vertices which are part of the initial graph, but which can also be neighbour (or not) with the new vertex. Returns all possible neighbourhoods given the initial neighbourhood of the new vertex (defined in curG).
// Recursive function.
void getPossibleFreeNeighourhoods(int nbVert, const vector<int> &freeVerts, vector<Graph> &ret, Graph &curG, int pos, vector<Graph> &obstructions, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread);

vector<vector<char>> compute_cleophee_arrays(const Graph &g, const vector<vector<int>> &adjSets, const vector<vector<int>> &antiCompleteSets, const vector<string> &setsNames, const vector<int> &freeVerts, vector<Graph> &obstructions, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread, bool print=false);


// Tests if there is an obstruction of C4 or 4K1, or it is isomorphic to a prefixeur
bool is_graph_ok(Graph& g, vector<Graph> &obstructions, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread, bool print = false);
#endif
