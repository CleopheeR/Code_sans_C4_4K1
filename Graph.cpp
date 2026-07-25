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

void Graph::copy_and_add_new_vertex_noalloc(const Graph& g, int puissNew, int code)
{
    assert (adjMat != NULL && nbVert == g.nbVert+1);

    // Getting the set of neighbours represented by code.
    const vector<int> &newEdges = adjListGlobal[code];
    nbEdge = g.nbEdge+newEdges.size();

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
        // Skipping the vertex to remove.
        if (u == idToRemove)
            continue;

        // Otherwise, adding an edge to the graph, and decreasing the id of the vertices by 1 if they are >= idToRemove
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


void Graph::add_new_edges(int u, const vector<int> &adj)
{
    for (int x : adj)
        add_edge(u, x);
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

void Graph::print_in_string(stringstream& str) const
{
    str << nbVert << " " << nbEdge;
    for (int i = 0; i < nbVert; i++)
        for (int y : get_neighb(i))
            if (i > y)
                str << " " << i << "," << y;

    str << "\n";
}

void Graph::pretty_print(void) const
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


// After trying, the program is faster if we sort the values and do a non-associative hashing.
inline int my_hash2(const int colours[], const vector<int> &adjList)
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
        //try this: ret ^= x + 0x9e3779b9 + (ret << 6) + (ret >> 2) ???
        //newCol ^= std::hash<int>()(tmpVals[i]);
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

    //TYDY remove xorall...
    // Xor of all colours, consisting of a 32-bits fingerprint, we add to the array.
    int xorAll = 0;
    for (int u = 0; u < nbVert; u++)
    {
        vertsCol[u] = cols[u];
        degreeList[u] = cols[u];
        xorAll ^= cols[u];
    }
    // Copying the four bytes in two four cells of this char array.
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

void read_magic_graphs_compute_hash(const string &fName, int nbVert,sparse_hash_map<vector<char>, vector<Graph>> &deglist2Graphs)
{
    igzstream file(fName.c_str());
    if (file.peek() != EOF)
    {
        vector<char> degreeList(nbVert+4);
        long long nbGraph;

        Graph gRead;
        file >> nbGraph;
        string toto;
        getline(file, toto);
        cerr << "I want to see " << nbGraph << " plus one magic graphs" << endl;

        for (long long i = 0; i < nbGraph; i++)
        {
            gRead = Graph(file);
            gRead.compute_hashes(degreeList);
            sort(degreeList.begin(), degreeList.begin()+gRead.nbVert);
            deglist2Graphs[degreeList].push_back(gRead);
        }
    }

    file.close();
}

