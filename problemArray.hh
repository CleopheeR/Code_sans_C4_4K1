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

        // Advanced means not stantard magic, i.e. special relationships foreseen and enforced for future vertices in specific sets.
        inline bool is_advanced(void) const
        {
            return !(forcedNeighbInOtherSets.empty() && specialAdjSets.empty() && specialNonAdjSets.empty());
        }


        int id;
        vector<int> neighbInBaseGraph;
        //Ou bien map v ?
        vector<pair<int, bool>> forcedNeighbInOtherSets; //and true if adjacent, false if not
        set<int> specialAdjSets, specialNonAdjSets;
};


class ProblemArray
{
    public:
        // Returns a new graphs with nbSets new vertices, their neighbourhood defined by adjVertsToAdd. Does not care about possible adjacencies between the newly added vertices.
        Graph add_vertices_to_base_graph(const ProblemArraySet* adjVertsToAdd[], int nbSets) const;
        // Tries to split A into A_{N0B} and A_{1B} : the first one contains vertices which *necessarily* have a non-adjacent vertex in B (this implies the existence of a vertex in B). The second one contains vertices which are complete to B (works if B is empty).
        // Also tries to split A into A_{N1B} and A_{0B} following the same logic: each vertiex in A_{1NB} *has* a neighbour in B, and A_{0B} is anticomplete to B (may be empty).
        bool can_NN_be_solved_method1(const ProblemArraySet &setA, const ProblemArraySet &setB, const ProblemArraySet &setC) const;


        // Method 2 : let A N B and A N C (true N's) be *any* problematic thing, we say there is a bad triplet (A,B,C). If B 1 C then we could merge B and C (and others which are N with A) and A is solved. We furthermore require that the intersection of all second and third members of the triplet is empty. This condition makes sure that there is no D such that BC is N with A and D in the end.
        bool can_NN_be_solved_method2(void) const;

        // Returns false if A can be split into A_{0B} and A_{1B}, i.e. if having a neighbour in B implies being complete to B, and if having a non-adjacent vertex in B means implies being anticomplete to B.
        bool is_true_N_between_two(const ProblemArraySet &setA, const ProblemArraySet &setB) const;

        // Returns false if a \in A, b \in B and c \in C cannot exist at the same time.
        bool can_3sets_be_possible(const ProblemArraySet &setA, const ProblemArraySet &setB, const ProblemArraySet &setC) const;

        // Given the base graph, generates the possible sets: one for each possible adjacency to the base graph.
        // Maybe TODO: take into account some free vertices?
        void gen_default_partition(void);

        // Computes the array, i.e. all the compatibility between the different sets.
        void compute_partition_array(void);

        // Takes a graph with a "new" vertex, alreay added with its adjacency to the base graph. Generates all possibles graphs depending on its adjacency to the "free vertices".
        void get_possible_free_neighbourhoods(int newVert, const vector<int> &freeVerts, Graph &curG, int pos, vector<Graph> &ret) const;

        // Returns false if the graph contains a C4, a 4K1 or an "obstruction".
        bool is_graph_ok(const Graph &g, bool print) const;

        // Returns the compatibility between two sets.
        char get_sets_compatibility(int i1, int i2) const;

        void print_array(void) const;

        // Returns true if two vertices with the adjacency to the base graph defined by set are necessarily adjacent, hence this set is a clique.
        bool check_that_set_is_clique(const ProblemArraySet &set) const;

        vector<string> solve_array_problems(void) const;

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

bool is_magic_graph(const Graph &g, bool special, vector<sparse_hash_map<vector<char>, vector<Graph>>> *deglist2ObstructionsBySize = NULL);

sparse_hash_map<vector<char>, vector<Graph>> gen_magic_graphs(int nbVert);
#endif


