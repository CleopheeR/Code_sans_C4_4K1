#ifndef DEF_GRAPH_HH
#define DEF_GRAPH_HH

#include <cstdlib>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>

#include "sparsepp/spp.h"
#include "gzstream/gzstream.h"

using spp::sparse_hash_map;
using namespace std;

namespace std
{
    template<>
struct hash<vector<char>> {
    inline size_t operator()(const vector<char> &v) const {
        int ret = 0;
    for (const int x : v)
        ret ^= x + 0x9e3779b9 + (ret << 6) + (ret >> 2);
    return ret;
    }
};
}


extern vector<int> *adjListGlobal;
extern int nbProc;

#define NBMAXPROC 200
#define NBMAXVERT 32

#define are_neighb(g, u, v) (g.adjMat[u]&(1<<v))

inline int read_int(const char *str, int &index)
{
    //TTAADDAA pas boucle car nb sur deux chiffres maxi...
    //Ou en fait faire deux fonctions
    int res = 0;
    while (str[index] >= '0' && str[index] <= '9')
    {
        res *= 10;
        res += str[index]-'0';
        index++;
    }
    return res;
}

class Graph
{
    public:
        Graph() {
            adjMat = NULL;
            vertsCol = NULL;
        }

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
            nbEdge = m; // car ajouté par add_edge
            for (int i = 0; i < m; i++)
            {
                char virgule;
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

        //TTAADDAA factoriser avec au-dessus ?
        void fill_from_file(igzstream &f)
        {
            static string toto;
            getline(f, toto);
            int n, m;

            int pos = 0;
            const char* totoStr = toto.c_str();
            n = read_int(totoStr, pos);
            pos++;
            m = read_int(totoStr, pos);
            pos++;

            init(n, m);

            nbEdge = m;
            for (int i = 0; i < m; i++)
            {
                char virgule;
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

        void init(int n, int m);

        void copy_and_add_new_vertex_bis(const Graph& g, const vector<int> &newEdges, int puissNew, int code);//TODO ou bien renvoie un Graphe autre

        inline const vector<int>& get_neighb(int u) const
        {
            return adjListGlobal[adjMat[u]];
        }

        void add_edge(int u, int v); // Does not change any degreeList

        void compute_hashes(vector<char> &degreeeList);

        void print_in_file(ogzstream &f) const;
        void print(void) const;





        int nbVert;
        int nbEdge;

        int *adjMat;
        char *vertsCol;

};



void init_adjListGlobal(int n);
void free_adjListGlobal(void);

void read_prefixeurs_compute_hash(const string &fName, int nbVert,sparse_hash_map<vector<char>, vector<Graph>> &deglist2Graphs);

#endif


