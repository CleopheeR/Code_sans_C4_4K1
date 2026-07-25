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

const int STEP_RATIO = 1000;

using spp::sparse_hash_map;
using namespace std;

// Generate supergraphs of the generated graphs of size nbVert-1 (inside a file), and adds possible ones in startingGraphs.
void gen_graphs(int nbVert, vector<Graph> &startingGraphs);

vector<Graph> load_from_file(const string &filename, long long nbGraphToRead=-1);


// Takes a graphs and returns false if already seen. Otherwise returns true and adds the graph to dico, keyed by a hash (inside a vector<char>).
bool check_if_seen_and_add(Graph& g, const vector<char> &degreeList, sparse_hash_map<vector<char>, vector<Graph>> &dico, int idThread = 0);


//void save_to_file(const string &filename, const unordered_map<vector<int>, vector<Graph>, vector_hash> &graphList, int nbGraph);
//void save_to_file(const string &filename, const sparse_hash_map<vector<char>, vector<Graph>> &graphList, long long nbGraph);




/** Internal functions **/

vector<Graph> gen_graphs_thread(const vector<Graph> &listMinus, vector<Graph> &startingGraphs, vector<pair<long long, long long>> &indicesToDo, ogzstream &outFile, int idThread, vector<mutex> &locksTests, mutex &lockToDo, sparse_hash_map<vector<char>, vector<Graph>> *deglists2GraphsToAdd, bool keepTwins);

//TTAADDAA mettre dans misc.hh/cpp ?
// Generates all subsets of size k from the [1;n] interval. No longer used
void gen_subsets(int k, int n, vector<vector<int>> &listRes);


// Enumerate the list of induced P_2's from g, and precomputes a quantity to
// then detect C_4's quick.
void gen_P2_list(const Graph &g, vector<long long> &pathList, int nbVert);
// Detects if the new vertex (whose neighbourhood is encoded in code) indeces a C_4.
bool detect_C4(const vector<long long> &pathList, int code);

// Same for generating O_3's, and detecting O_4's (but not so useful, so unused).
void gen_O3_list(const Graph &g, vector<int> &indepList, int nbVert);
bool detect_O4(const vector<int> &indepList, int code);

#endif
