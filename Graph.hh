#include <cstdlib>
#include <vector>
#include <utility>

#ifndef DEF_GRAPH_HH
#define DEF_GRAPH_HH


using namespace std;

class Graph
{
    public:
        Graph() {}


        Graph(const Graph& g)
        {

            init(g.nbVert, g.nbEdge);
            for (int u = 0; u < g.nbVert; u++)
            {
                for (int v : g.adjList[u])
                    adjMat[u][v] = true;
            }

            for (int u = 0; u < g.nbVert; u++)
            {
                adjList[u] = g.adjList[u];
                degreeList[u] = g.degreeList[u];
            }
        }


        ~Graph()
        {
            for (int i = 0; i < nbVert; i++)
                free(adjMat[i]);
            free(adjMat);
        }


        void init(int n, int m);
        int nbVert;
        int nbEdge;
        vector<vector<int>> adjList;

        bool **adjMat;
        vector<int> degreeList;


        void copy_and_add_new_vertex(const Graph&); //TODO ou bien renvoie un Graphe autre
        void add_edge(int u, int v);
        void remove_last_edge(int u, int v);

        void print(void) const;
        //TODO copie...
};

#endif


