#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "Graph.hh"

vector<int> *adjListGlobal;


void Graph::init(int n, int m)
{
    nbVert = n;
    nbEdge = m;
    adjMat = (int*) calloc(sizeof(int),n);
    vertsCol = NULL;
}


void Graph::copy_and_add_new_vertex_bis(const Graph& g, const vector<int> &newEdges, int puissNew, int code)
{
    if (adjMat == NULL)
        init(g.nbVert+1, g.nbEdge);
    else
    {
        nbVert = g.nbVert+1;
        nbEdge = g.nbEdge+newEdges.size();
    }

    for (int u = 0; u < g.nbVert; u++)
        adjMat[u] = g.adjMat[u];

    for (int x : newEdges)
        adjMat[x] ^= puissNew;
    adjMat[g.nbVert] = code;



}

void Graph::copy_and_add_new_vertex(const Graph& g)//, vector<int> &degreeList)
{
    if (adjMat == NULL)
        init(g.nbVert+1, g.nbEdge);
    else
    {
        nbVert = g.nbVert+1;
        nbEdge = g.nbEdge;
    }

    for (int u = 0; u < g.nbVert; u++)
        adjMat[u] = g.adjMat[u];
    adjMat[g.nbVert] = 0;


    //for (int u = g.nbVert; u > 0; u--)
    //    degreeList[u] = degreeList[u-1];
    //degreeList[0] = 0;
}

void Graph::add_edge(int u, int v)//, vector<int> &degreeList)
{
    nbEdge++;

    adjMat[u] ^= (1<<v);
    adjMat[v] ^= (1<<u);

    //TODO bitwise xor vs bitwise or
    /*
    const vector<int> &adjListU = get_neighb(u);
    const vector<int> &adjListV = get_neighb(v);
    int nbNeighbU = adjListU.size();
    int nbNeighbV = adjListV.size();


    int n1 = min(nbNeighbU, nbNeighbV);
    int n2 = max(nbNeighbU, nbNeighbV);
    */

    /*
    int i = 0;
    while (i < nbVert && degreeList[i] <= n1)
        i++;
    degreeList[i-1]++;
    i--;
    while(i < nbVert && degreeList[i] <= n2)
        i++;
    degreeList[i-1]++;
    */
}

void Graph::remove_last_edge(int u, int v, vector<int> &degreeList)
{
    nbEdge--;
    const vector<int> &adjListU = get_neighb(u);
    const vector<int> &adjListV = get_neighb(v);
    int nbNeighbU = adjListU.size();
    int nbNeighbV = adjListV.size();

    adjMat[u] ^= (1<<v);
    adjMat[v] ^= (1<<u);

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
        for (int y : get_neighb(i))
            if (i > y)
                f << "\t" << i << ";" << y;
    }
    f << endl;
}

void Graph::print(void) const
{
    cout << nbVert << " vertices and " << nbEdge << " edges.\n";
    //for (int x : degreeeList)
        //cout << x << " ";
    //cout << endl;
    for (int i = 0; i < nbVert; i++)
    {
        for (int y : get_neighb(i))
            if (i > y)
                cout << i << ";" << y << "\t";
    }
    cout << endl;
}


inline int my_hash2(int colours[], const vector<int> &adjList)
{
    int tmpVals[NBMAXVERT];
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
            newCol ^= (curCol << 4);
        else
            newCol ^= curCol;

    }
    return newCol;
}

void Graph::compute_hashes(vector<char> &degreeList)
{
    int cols[NBMAXVERT], prevCols[NBMAXVERT];
    for (int u = 0; u < nbVert; u++)
        cols[u] = get_neighb(u).size();

    for (int i = 0; i < 2; i++)
    {
        swap(cols, prevCols);
        for (int u = 0; u < nbVert; u++)
            cols[u] = my_hash2(prevCols, get_neighb(u));

    }
    if (vertsCol == NULL)
        vertsCol = (char*) malloc(nbVert*sizeof(char));
    int xorAll = 0;
    for (int u = 0; u < nbVert; u++)
    {
        vertsCol[u] = cols[u];
        xorAll ^= cols[u];
    }

    //degreeList.back() = xorAll;
    degreeList[nbVert] = xorAll;
    degreeList[nbVert+1] = xorAll >> 8;
    degreeList[nbVert+2] = xorAll >> 16;
    degreeList[nbVert+3] = xorAll >> 24;
}

void init_adjListGlobal(int n)
{
    adjListGlobal = (vector<int>*) malloc(sizeof(*adjListGlobal)*((1<<n)));

    for (int i = 0; i < (1<<n); i++)
    {
        vector<int> cur;
        for (int j = 0; j < n; j++)
        {
            if (i & (1<<j))
                cur.push_back(j);
        }
        swap(cur, adjListGlobal[i]);
    }
}

void free_adjListGlobal(void)
{
    free(adjListGlobal);
}


