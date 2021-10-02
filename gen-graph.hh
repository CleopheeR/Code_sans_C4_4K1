#ifndef DEF_GENGRAPH_HH
#define DEF_GENGRAPH_HH

#include <vector>
#include <unordered_map>
#include "Graph.hh"

struct vector_hash {
    size_t operator()(const vector<int> &v) const {
        int ret = 0;
        int nbElem = v.size();
    for (int i = 0; i < nbElem-1; i++)
        ret ^= (ret << 5) + (ret >> 2) + v[i];
    long long d = v.back();
    ret ^= (d << 32);
    return ret;
    }
};



vector<Graph> gen_graphs(int nbVert);

//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico);
bool check_if_seen_and_add(Graph& g, vector<int> &degreeList, map<vector<int>, vector<Graph>> &dico);


vector<Graph> load_from_file(const string &filename);

//void save_to_file(const string &filename, const unordered_map<vector<int>, vector<Graph>, vector_hash> &graphList, int nbGraph);
void save_to_file(const string &filename, const map<vector<int>, vector<Graph>> &graphList, int nbGraph);


void gen_subsets(int k, int n, vector<vector<int>> &listRes);

#endif
