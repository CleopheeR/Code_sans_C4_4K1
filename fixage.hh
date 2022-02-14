#ifndef DEF_FIXAGE_HH
#define DEF_FIXAGE_HH

#include <vector>
#include <map>
#include <mutex>

#include "Graph.hh"
#include "sparsepp/spp.h"

using spp::sparse_hash_map;

sparse_hash_map<vector<char>, vector<Graph>> gen_fixeurs(int nbVert);

void get_minimal_fixeurs(const vector<Graph> &prefixeurMinusList, sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict);

bool is_pre_or_fixeur(const Graph &g, bool prefixeurTest, const sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat, int idThread);


/** Internal functions **/

void gen_fixeurs_thread(int nbVert, const vector<Graph> &graphList, int** isTwinCompat, vector<Graph> &fixeursList, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, mutex &mutInsert, int idThread);

void remove_nonminimal_fixeurs(const Graph &g, sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat, int idThread);
#endif
