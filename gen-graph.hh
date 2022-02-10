#ifndef DEF_GENGRAPH_HH
#define DEF_GENGRAPH_HH

#include <vector>
#include <unordered_map>
#include <map>
#include <thread>
#include <mutex>

#include "sparsepp/spp.h"
#include "gzstream/gzstream.h"
#include "Graph.hh"


using spp::sparse_hash_map;
vector<Graph> gen_graphs(int nbVert);

//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico);
bool check_if_seen_and_add(Graph& g, vector<char> &degreeList, sparse_hash_map<vector<char>, vector<Graph>> &dico, int idThread = 0);


vector<Graph> load_from_file(const string &filename, long long nbGraphMinus=-1);

//void save_to_file(const string &filename, const unordered_map<vector<int>, vector<Graph>, vector_hash> &graphList, int nbGraph);
void save_to_file(const string &filename, const sparse_hash_map<vector<char>, vector<Graph>> &graphList, long long nbGraph);


void gen_subsets(int k, int n, vector<vector<int>> &listRes);


void gen_twin_list(const Graph &g, vector<long long> &twinLists, int nbVert);
bool can_discard_edgelist(const vector<long long> &twinLists, int *isTwinCompat, int nbVert);

void gen_P2_list(const Graph &g, vector<long long> &pathList, int nbVert);
bool detect_C4(const vector<long long> &pathList, int code);


vector<Graph> gen_graphs_thread(vector<Graph> &listMinus, int **isTwinCompat, vector<int> &sizesToDo, ogzstream &outFile, int idThread, mutex &lock);
#endif
