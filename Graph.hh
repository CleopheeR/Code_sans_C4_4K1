#include <cstdlib>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>
#include <cassert>

#ifndef DEF_GRAPH_HH
#define DEF_GRAPH_HH


#define are_neighb(g, u, v) (g.adjMat[u]&(1<<v))
using namespace std;

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
            {
                adjMat[u] = g.adjMat[u];
            }

            if (g.vertsCol)
            {
                vertsCol = (int*) malloc(g.nbVert*sizeof(int));
                for (int u = 0; u < g.nbVert; u++)
                    vertsCol[u] = g.vertsCol[u];
            }
            else
                vertsCol = NULL;
        }

        Graph& operator=(const Graph& g)
        {
            init(g.nbVert, g.nbEdge);
            for (int u = 0; u < g.nbVert; u++)
            {
                adjMat[u] = g.adjMat[u];
            }

            if (g.vertsCol)
            {
                vertsCol = (int*) malloc(g.nbVert*sizeof(int));
                for (int u = 0; u < g.nbVert; u++)
                    vertsCol[u] = g.vertsCol[u];
            }
            else
                vertsCol = NULL;
            return *this;
        }


        Graph (ifstream &f)
        {
            int n, m;
            f >> n >> m;
            init(n, m);
            nbEdge = m; // car ajouté par add_edge
            for (int i = 0; i < m; i++)
            {
                char virgule;
                int u, v;
                f >> u >> virgule >> v;

                adjMat[u] |= (1<<v);
                adjMat[v] |= (1<<u);
            }
            vertsCol = NULL;
        }


        ~Graph()
        {
            if (adjMat)
                free(adjMat);
            if (vertsCol)
                free(vertsCol);
            /*if (adjMat == NULL)
                return;
            for (int i = 0; i < nbVert; i++)
                free(adjMat[i]);
            free(adjMat);*/
        }

        void print_in_file(ofstream &f) const;

        void init(int n, int m);



        int nbVert;
        int nbEdge;

        int *adjMat;
        int *vertsCol;

        const vector<int>& get_neighb(int u) const;

        void copy_and_add_new_vertex(const Graph&, vector<int> &degreeList); //TODO ou bien renvoie un Graphe autre
        void add_edge(int u, int v, vector<int> &degreeeList);
        void remove_last_edge(int u, int v, vector<int> &degreeeList);


        void compute_hashes(vector<int> &degreeeList);

        void print(void) const;
        //TODO copie...
};



void init_adjListGlobal(int n);
void free_adjListGlobal(void);

#endif


