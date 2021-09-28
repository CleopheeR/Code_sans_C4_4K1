#ifndef DEF_TEST_PPTY_HH
#define DEF_TEST_PPTY_HH

#include "Graph.hh"


int nb_connected_comp(const Graph& g);

bool free_O4(const Graph& g, int n);


bool free_C4(const Graph& g, int n);


bool free_C4_O4(const Graph& g, int n);


int nb_twin(const Graph& g, int v);
bool has_twin(const Graph& g, int v);

bool are_isomorphic(const Graph& g1, const Graph& g2);



#endif
