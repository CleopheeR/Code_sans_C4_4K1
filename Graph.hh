#ifndef DEF_GRAPH_HH
#define DEF_GRAPH_HH

#include <cstdlib>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>
#include <sstream>

#include "sparsepp/spp.h"
#include "gzstream/gzstream.h"

using spp::sparse_hash_map;
using namespace std;

//TYDY: dans misc.
// Hash function to have spp work. Hash a vector<char> into a size_t
namespace std
{
    template<> struct hash<vector<char>> {
        inline size_t operator()(const vector<char> &v) const
        {
            int ret = 0;
            for (const int x : v)
                ret ^= x + 0x9e3779b9 + (ret << 6) + (ret >> 2);
            return ret;
        }
    };
}


// Included in every file. AdjListGlobal converts an encoded neighbourhood into the actuel sets of
// ids of the vertices.
extern vector<int> *adjListGlobal;
extern int nbProc;

#define NBMAXPROC 200
#define NBMAXVERT 32

#define are_neighb(g, u, v) (g.adjMat[u]&(1<<v))

// Insted of converting to stringstream
inline int read_int(const char *str, int &index)
{
    int res = 0;
    while (str[index] >= '0' && str[index] <= '9')
    {
        res *= 10;
        res += str[index]-'0';
        index++;
    }
    return res;
}

// adjMat[i] contains the encoded neighbourhood (an int) of vertex i
// vertsCol is used to get a fingerprint, when doing the isomorphis test.
class Graph
{
    public:
        Graph() {
            adjMat = NULL;
            vertsCol = NULL;
            nbVert = 0;
            nbEdge = 0;
        }

        // Copy constructor. Checks wheter vertsCol is allocated or not.
        Graph(const Graph& g)
        {
            init(g.nbVert, g.nbEdge);
            for (int u = 0; u < g.nbVert; u++)
                adjMat[u] = g.adjMat[u];

            if (g.vertsCol)
            {
                vertsCol = (char*) malloc(g.nbVert*sizeof(char));
                for (int u = 0; u < g.nbVert; u++)
                    vertsCol[u] = g.vertsCol[u];
            }
            else
                vertsCol = NULL;
        }

        //TTAADDAA factoriser avec constructeur ?!
        Graph& operator=(const Graph& g)
        {
            this->~Graph();
            init(g.nbVert, g.nbEdge);
            for (int u = 0; u < g.nbVert; u++)
                adjMat[u] = g.adjMat[u];

            if (g.vertsCol)
            {
                vertsCol = (char*) malloc(g.nbVert*sizeof(char));
                for (int u = 0; u < g.nbVert; u++)
                    vertsCol[u] = g.vertsCol[u];
            }
            else
                vertsCol = NULL;
            return *this;
        }


        Graph (igzstream &f)
        {
            string toto;
            getline(f, toto);
            int n, m;
            int pos = 0;
            const char* totoStr = toto.c_str();
            n = read_int(totoStr, pos);
            pos++;
            m = read_int(totoStr, pos);
            pos++;


            init(n, m);

            // Reading the edges as "v1,v2" list, separated by one space.
            for (int i = 0; i < m; i++)
            {
                int u, v;
                u = read_int(totoStr, pos);
                pos++;
                v = read_int(totoStr, pos);
                pos++;

                adjMat[u] ^= (1<<v);
                adjMat[v] ^= (1<<u);
            }
            vertsCol = NULL;
        }

        ~Graph()
        {
            if (adjMat)
                free(adjMat);
            if (vertsCol)
                free(vertsCol);
        }

        // Allocates adjMat and fills nbVert and nbEdge attributes.
        void init(int n, int m);

        // Copies g into the current object, but with a new vertex. Does not perform allocation, because
        // *this is a temporary variable reused, and already allocated, once and for all.
        void copy_and_add_new_vertex_noalloc(const Graph& g, int puissNew, int code);

        // Removes the vertex passed in argument. Swaps it with last vertex.
        Graph subgraph_removing_vertex(int idToRemove) const;

        inline const vector<int>& get_neighb(int u) const
        {
            return adjListGlobal[adjMat[u]];
        }

        // Adds/deletes edges, modyfing the number of edges.
        void add_edge(int u, int v); // Does not change any degreeList
        void delete_edge(int u, int v); // Does not change any degreeList

//TYDY unused for official
        void add_new_edges(int u, const vector<int> &adj);

        // Computes the graph hash, but also writes it to its parameter.
        void compute_hashes(vector<char> &degreeeList);

        void print_in_file(ogzstream &f) const;
        // Adds the graph to the stringstream. Enables to reduce the number of
        // writes to the (gz) files, hence to gain time. The writing is done at the end.
        void print_in_string(stringstream &str) const;
        void pretty_print(void) const;



        // The attributes.
        int nbVert;
        int nbEdge;

        int *adjMat;
        char *vertsCol;

};



void init_adjListGlobal(int n);
void free_adjListGlobal(void);

// Reads a list of graphs from the file named fName. Adds them to the collection deglist2Graphs,
// Indexed by their fingerprint, that the function computes.
void read_prefixeurs_compute_hash(const string &fName, int nbVert, sparse_hash_map<vector<char>, vector<Graph>> &deglist2Graphs);



#endif


