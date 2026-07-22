#ifndef DEF_TEST_PPTY_HH
#define DEF_TEST_PPTY_HH

#include "Graph.hh"

// The following functions return true if the graph is free of C4 and O4, of O4, or of C4.
bool free_C4_O4(const Graph& g, int n);
bool free_O4(const Graph& g, int n);
bool free_C4(const Graph& g, int n);

// Test if two graphs are isomorphic. The "colors" (fingerprints) of the vertices must have been
// computed before calling are_isomorphic.
bool are_isomorphic(const Graph& g1, const Graph& g2, int idThread);

// The first function looks for any pair of twin, the second one looks for one involving its parameter v.
// Usually v is a new vertex (because the rest of the graph is guaranteed twin-free).
bool has_twin(const Graph& g);
bool has_twin(const Graph& g, int v);

// Returns the number of connected componends in g.
int nb_connected_comp(const Graph& g);

// Tests if g contains an induced copy of targetGraph.
bool is_supergraph_of(const Graph &g, Graph &targetGraph, int idThread);


/** Internal functions **/

// Recursive function called by are_isomorphic, trying to match g1 and g2's vertices together.
bool gen_iso_matching(const Graph &g1, const Graph &g2, int i, int idThread);

// Removes vertices from g to try to become isomorphic to targetGraph. The targetHash parameter is used as a
// pre-isomorphic test. tmpHash is where the hash of the g will be computed.
// pos is the id of the vertex we will try to remove at this step.
bool is_supergraph_of_aux(Graph &g, const Graph &targetGraph, const vector<char> &targetHash, vector<char> &tmpHash, int pos, int idThread);
#endif
