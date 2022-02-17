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
using namespace std;

vector<Graph> gen_graphs(int nbVert, vector<Graph> &startingGraphs);

vector<Graph> load_from_file(const string &filename, long long nbGraphToRead=-1);


//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico);
bool check_if_seen_and_add(Graph& g, vector<char> &degreeList, sparse_hash_map<vector<char>, vector<Graph>> &dico, int idThread = 0);


//void save_to_file(const string &filename, const unordered_map<vector<int>, vector<Graph>, vector_hash> &graphList, int nbGraph);
//void save_to_file(const string &filename, const sparse_hash_map<vector<char>, vector<Graph>> &graphList, long long nbGraph);




/** Internal functions **/

vector<Graph> gen_graphs_thread(vector<Graph> &listMinus, vector<Graph> &startingGraphs, int **isTwinCompat, vector<int> &sizesToDo, ogzstream &outFile, int idThread, mutex &lock); //TTAADDAA à mettre juste dans le .cpp ?

//TTAADDAA mettre dans misc.hh/cpp ?
void gen_subsets(int k, int n, vector<vector<int>> &listRes);

void gen_twin_list(const Graph &g, vector<long long> &twinLists, int nbVert);
void gen_twin_list2(const Graph &g, vector<long long> &twinLists, int nbVert);
bool can_discard_edgelist(const vector<long long> &twinLists, int *isTwinCompat, int nbVert);

void gen_P2_list(const Graph &g, vector<long long> &pathList, int nbVert);
bool detect_C4(const vector<long long> &pathList, int code);

#endif
