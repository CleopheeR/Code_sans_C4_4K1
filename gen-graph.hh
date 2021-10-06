#ifndef DEF_GENGRAPH_HH
#define DEF_GENGRAPH_HH

#include <vector>
#include <unordered_map>
#include <map>
#include "sparsepp/spp.h"
#include "Graph.hh"

namespace std
{
    template<>
struct hash<vector<char>> {
    inline size_t operator()(const vector<char> &v) const {
        int ret = 0;
        //int nbElem = v.size();
    //for (int i = 0; i < nbElem; i++)
    for (const int x : v)
        //ret ^= (ret << 5) + (ret >> 2) + x;//v[i];
        ret ^= x + 0x9e3779b9 + (ret << 6) + (ret >> 2);
    //long long d = v.back();
    //ret ^= (d << 32);
    return ret;
    }
};
}


using spp::sparse_hash_map;
vector<Graph> gen_graphs(int nbVert);

//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico);
bool check_if_seen_and_add(Graph& g, vector<int> &degreeList, sparse_hash_map<vector<char>, vector<Graph>> &dico);


vector<Graph> load_from_file(const string &filename);

//void save_to_file(const string &filename, const unordered_map<vector<int>, vector<Graph>, vector_hash> &graphList, int nbGraph);
void save_to_file(const string &filename, const sparse_hash_map<vector<char>, vector<Graph>> &graphList, int nbGraph);


void gen_subsets(int k, int n, vector<vector<int>> &listRes);


void gen_twin_list(const Graph &g, vector<long long> &twinLists, int nbVert);
bool can_discard_edgelist(const vector<long long> &twinLists, int *isTwinCompat, int nbVert);

void gen_P2_list(const Graph &g, vector<long long> &pathList, int nbVert);
bool detect_C4(const vector<long long> &pathList, int code);

#endif
