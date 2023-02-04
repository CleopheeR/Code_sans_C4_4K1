#ifndef DEF_PROBLEMARRAY_HH
#define DEF_PROBLEMARRAY_HH

#include <cstdlib>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>
#include <set>

#include "sparsepp/spp.h"
#include "gzstream/gzstream.h"

using spp::sparse_hash_map;
using namespace std;

extern vector<int> *adjListGlobal;
extern int nbProc;

#define NBMAXPROC 200
#define NBMAXVERT 32

class ProblemArraySet
{
    public:
    int id;
    vector<int> neighbInBaseGraph;
    //Ou bien map v ?
    vector<pair<int, bool>> forcedNeighbInOtherSets; //and true if adjacent, false if not
    set<int> specialAdjSets, specialNonAdjSets;
};


class ProblemArray
{
    public:

        Graph add_vertices_to_base_graph(const ProblemArraySet* adjVertsToAdd[], int nbSets) const;

        bool can_NN_be_solved_method1(const ProblemArraySet &setA, const ProblemArraySet &setB, const ProblemArraySet &setC) const;

        bool is_true_N_between_two(const ProblemArraySet &setA, const ProblemArraySet &setB) const;

        bool can_3sets_be_possible(const ProblemArraySet &setA, const ProblemArraySet &setB, const ProblemArraySet &setC) const;

        void gen_default_partition(void);
        void compute_partition_array(void);

        void get_possible_free_neighbourhoods(int newVert, const vector<int> &freeVerts, Graph &curG, int pos, vector<Graph> &ret) const;

        bool is_graph_ok(const Graph &g, bool print) const;

        char get_sets_compatibility(int i1, int i2) const;

        void print_array(void) const;

        bool check_that_set_is_clique(const ProblemArraySet &set) const;

        ProblemArray(void)
        {
            int idThread = 0;
            deglist2ObstructionsBySize = NULL;
        }


        Graph baseGraph;
        vector<ProblemArraySet> partitionSets;
        vector<vector<char>> partitionArray;
        vector<sparse_hash_map<vector<char>, vector<Graph>>> *deglist2ObstructionsBySize;
        int idThread;
};

bool is_magic_graph(const Graph &g);

#endif


