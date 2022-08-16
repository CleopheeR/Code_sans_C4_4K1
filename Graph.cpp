#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "sparsepp/spp.h"
#include "gzstream/gzstream.h"
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


Graph Graph::subgraph_removing_vertex(int idToRemove) const
{
    Graph ret;
    int n = nbVert;
    ret.init(nbVert-1, 0);

    for (int u = 0; u < n; u++)
    {
        if (u == idToRemove)
            continue;
        for (int v : get_neighb(u))
        {
            if (v > u && v != idToRemove)
            {
                int u2 = u, v2 = v;
                if (u > idToRemove)
                    u2--;
                if (v2 > idToRemove)
                    v2--;
                ret.add_edge(u2, v2);
            }
        }
    }

    return ret;
}


void Graph::add_edge(int u, int v)
{
    nbEdge++;

    adjMat[u] ^= (1<<v);
    adjMat[v] ^= (1<<u);
}

void Graph::delete_edge(int u, int v)
{
    nbEdge--;

    adjMat[u] ^= (1<<v);
    adjMat[v] ^= (1<<u);
}


void Graph::print_in_file(ogzstream &f) const
{
    f << nbVert << " " << nbEdge;
    for (int i = 0; i < nbVert; i++)
    {
        for (int y : get_neighb(i))
            if (i > y)
                f << " " << i << "," << y;
    }
    f << "\n";
}

/*
 * void Graph::print_in_binfile(ofstream &f) const
{
    //TODO coder pour de vrai !
    f << nbVert << " " << nbEdge;
    for (int i = 0; i < nbVert; i++)
    {
        for (int y : get_neighb(i))
            if (i > y)
                f << " " << i << "," << y;
    }
    f << "\n";
}
*/

void Graph::print(void) const
{
    cerr << nbVert << " vertices and " << nbEdge << " edges.\n";
    for (int i = 0; i < nbVert; i++)
    {
        for (int y : get_neighb(i))
            if (i > y)
                cerr << i << ";" << y << "\t";
    }
    cerr << endl;
}


//J'avais testé, c'est plus rapide overall si on hashe en triant
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

//TTAADDAA : documenter ce qu'il y a dans degreeList dans README global
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

    degreeList[nbVert] = xorAll;
    degreeList[nbVert+1] = xorAll >> 8;
    degreeList[nbVert+2] = xorAll >> 16;
    degreeList[nbVert+3] = xorAll >> 24;
}


//TTAADDAA definir dans misc, comme le hash2 ?
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

//TTAADDAA idem
void free_adjListGlobal(void)
{
    free(adjListGlobal);
}

void read_prefixeurs_compute_hash(const string &fName, int nbVert,sparse_hash_map<vector<char>, vector<Graph>> &deglist2Graphs)
{
    vector<char> degreeList(nbVert+4);
    long long nbGraph;
    igzstream file(fName.c_str());
    if (file.peek() != EOF)
    {
        Graph gLu;
        file >> nbGraph;
        string toto;
        getline(file, toto);
        cerr << "I want to see " << nbGraph << " plus one prefixeurs" << endl;

        for (long long i = 0; i < nbGraph; i++)
        {
            gLu = Graph(file);
            for (int u = 0; u < gLu.nbVert; u++)
                degreeList[u] = gLu.get_neighb(u).size();
            sort(degreeList.begin(), degreeList.begin()+gLu.nbVert);
            gLu.compute_hashes(degreeList);
            deglist2Graphs[degreeList].push_back(gLu);
            //gLu.print();
        }
    }

    file.close();
}

