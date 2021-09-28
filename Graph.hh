#include <cstdlib>
#include <vector>
#include <utility>
#include <fstream>
#include <iostream>

#ifndef DEF_GRAPH_HH
#define DEF_GRAPH_HH


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
                //for (int v : g.adjList[u])
                //    adjMat[u][v] = true;
            }

            for (int u = 0; u < g.nbVert; u++)
            {
                adjList[u] = g.adjList[u];
                degreeList[u] = g.degreeList[u];
            }
            degreeList.back() = g.degreeList.back(); //for the hash
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
                //for (int v : g.adjList[u])
                //    adjMat[u][v] = true;
            }

            for (int u = 0; u < g.nbVert; u++)
            {
                adjList[u] = g.adjList[u];
                degreeList[u] = g.degreeList[u];
            }
            degreeList.back() = g.degreeList.back(); //for the hash
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
            nbEdge = 0; // car ajouté par add_edge
            for (int i = 0; i < m; i++)
            {
                char virgule;
                int u, v;
                f >> u >> virgule >> v;

                add_edge(u,v);
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
        vector<vector<int>> adjList;

        int *adjMat;
        vector<int> degreeList;
        int *vertsCol;


        void copy_and_add_new_vertex(const Graph&); //TODO ou bien renvoie un Graphe autre
        void add_edge(int u, int v);
        void remove_last_edge(int u, int v);


        void compute_hashes(void);

        void print(void) const;
        //TODO copie...
};

#endif


