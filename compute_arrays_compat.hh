#ifndef DEF_ARRAY_HH
#define DEF_ARRAY_HH

#include <vector>
#include <string>
#include "sparsepp/spp.h"

#include "Graph.hh"


using namespace std;

bool is_quasi_fixer(const Graph &g, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlusPlus, int idThread);

void gen_klmpartition_default_sets(const Graph &g, vector<vector<int>> &listPossibleNeighbs, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread);

// Takes a list of vertices which are part of the initial graph, but which can also be neighbour (or not) with the new vertex. Returns all possible neighbourhoods given the initial neighbourhood of the new vertex (defined in curG).
// Recursive function.
void getPossibleFreeNeighourhoods(int nbVert, const vector<int> &freeVerts, vector<Graph> &ret, Graph &curG, int pos, vector<Graph> &obstructions, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread);

vector<vector<char>> compute_cleophee_arrays(const Graph &g, const vector<vector<int>> &adjSets, const vector<vector<int>> &antiCompleteSets, const vector<string> &setsNames, const vector<int> &freeVerts, vector<Graph> &obstructions, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlusPlus, int idThread, bool print=false);


// Tests if there is an obstruction of C4 or 4K1, or it is isomorphic to a prefixeur
bool is_graph_ok(Graph& g, vector<Graph> &obstructions, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread, bool print = false);


// Given the original graphs and the adjacencies of A, B, C, determines if vertices a \in A, b \in B, c \in C can exist simulaneously
bool can_3sets_be_possible(const Graph &g, const vector<int> *adjA, const vector<int> *adjB, const vector<int> *adjC, vector<Graph> &obstructions);

//See .cpp for explanation => TODO put here
bool isTrueNBetweenTwo(const Graph &g, const vector<int> &adjA, const vector<int> &adjB, vector<Graph> &obstructions);
#endif
