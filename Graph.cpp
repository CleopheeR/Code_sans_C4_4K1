#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "Graph.hh"

void Graph::init(int n, int m)
{
    nbVert = n;
    nbEdge = m;
    //adjMat = (bool**) malloc(sizeof(bool*)*n);
    adjMat = (int*) calloc(sizeof(int),n);
    vertsCol = NULL;

    //for (int i = 0; i < nbVert; i++)
    //    adjMat[i] = (bool*) calloc(sizeof(bool),n);

    //degreeList = (int*) calloc(n,sizeof(int));
    degreeList.resize(n+1, 0);

    //adjList = (vector<int>*) malloc(sizeof(vector<int>)*n);
    adjList.resize(n);
}



void Graph::copy_and_add_new_vertex(const Graph& g)
{
    init(g.nbVert+1, g.nbEdge);

    for (int u = 0; u < g.nbVert; u++)
    {
        /*
        for (int v : g.adjList[u])
            adjMat[u] |= (1<<v);
            //adjMat[u][v] = true;
        */
        adjMat[u] = g.adjMat[u];
    }


    for (int u = 0; u < g.nbVert; u++)
    {
        adjList[u] = g.adjList[u];
        degreeList[u+1] = g.degreeList[u];
    }
    degreeList[0] = 0;
    degreeList.back() = g.degreeList.back(); //for the hash
}

void Graph::add_edge(int u, int v)
{
    nbEdge++;
    int nbNeighbU = adjList[u].size();
    int nbNeighbV = adjList[v].size();


    adjMat[u] |= (1<<v);
    adjMat[v] |= (1<<u);
    //adjMat[u][v] = true;
    //adjMat[v][u] = true;
    adjList[u].push_back(v);
    adjList[v].push_back(u);

    int n1 = min(nbNeighbU, nbNeighbV);
    int n2 = max(nbNeighbU, nbNeighbV);

    int i = 0;
    while (i < nbVert && degreeList[i] <= n1)
        i++;
    degreeList[i-1]++;
    i--;
    while(i < nbVert && degreeList[i] <= n2)
        i++;
    degreeList[i-1]++;
}

void Graph::remove_last_edge(int u, int v)
{
    nbEdge--;
    int nbNeighbU = adjList[u].size();
    int nbNeighbV = adjList[v].size();

    adjMat[u] ^= (1<<v);
    adjMat[v] ^= (1<<u);
    //adjMat[u][v] = false;
    //adjMat[v][u] = false;
    adjList[u].pop_back();
    adjList[v].pop_back();

    int n1 = min(nbNeighbU, nbNeighbV);
    int n2 = max(nbNeighbU, nbNeighbV);

    int i = 0;
    while (degreeList[i++] < n1);
    degreeList[i-1]--;
    while(degreeList[i++] < n2);
    degreeList[i-1]--;

    //TODO ça peut s'optimiser quels endroits on a enlever ou rajouté un pour faire l'opération inverse, si jamais
}


void Graph::print_in_file(ofstream &f) const
{
    f << nbVert << " " << nbEdge;
    for (int i = 0; i < nbVert; i++)
    {
        for (int y : adjList[i])
            if (i > y)
                f << "\t" << i << ";" << y;
    }
    f << endl;
}

void Graph::print(void) const
{
    cout << nbVert << " vertices and " << nbEdge << " edges.\n";
    for (int x : degreeList)
        cout << x << " ";
    cout << endl;
    for (int i = 0; i < nbVert; i++)
    {
        for (int y : adjList[i])
            if (i > y)
                cout << i << ";" << y << "\t";
    }
    cout << endl;
}


inline int my_hash2(int colours[], const vector<int> &adjList)
{
    const int nbMaxVertices = 30;
    int tmpVals[nbMaxVertices];
    for (int i = 0; i < adjList.size(); i++)
        tmpVals[i] = colours[adjList[i]];
    sort(tmpVals, tmpVals+adjList.size());

    int newCol = 0;
    for (int i = 0; i < adjList.size(); i++)
    {
        //newCol = (newCol + (324723947 + tmpVals[i])) ^93485734985;
        newCol ^= (newCol << 7) + (newCol >> 2) + tmpVals[i];
        //newCol ^= std::hash<int>()(tmpVals[i]);
    }
    return newCol;
}

inline int my_hash3(int colours[], const vector<int> &adjList)
{
    int newCol = 0;
    for (int i = 0; i < adjList.size(); i++)
    {
        //newCol ^=colours[adjList[i]];
        int curCol = colours[adjList[i]];
        if (curCol%2)
            newCol ^= (curCol << 16);
        else
            newCol ^= curCol;

    }
    return newCol;
}

void Graph::compute_hashes(void)
{
    const int nbMaxVertices = 30;
    int cols[nbMaxVertices], prevCols[nbMaxVertices];
    for (int u = 0; u < nbVert; u++)
        cols[u] = adjList[u].size();

    for (int i = 0; i < 3; i++)
    {
        std::swap(cols, prevCols);
        for (int u = 0; u < nbVert; u++)
            cols[u] = my_hash2(prevCols, adjList[u]);

    }
    if (vertsCol == NULL)
        vertsCol = (int*) malloc(nbVert*sizeof(int));
    int xorAll = 0;
    for (int u = 0; u < nbVert; u++)
    {
        vertsCol[u] = cols[u];
        xorAll ^= prevCols[u];
    }
    //degreeList.back() = xorAll;
    degreeList.back() = xorAll;
}
