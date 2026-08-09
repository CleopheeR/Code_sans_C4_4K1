#include <vector>

#include "sparsepp/spp.h"
#include "Graph.hh"
#include "test-properties.hh"
#include "gen-graph.hh"
#include "misc.hh"


using namespace std;

//TYDY: see if very useful
// To avoid reallocating. One vector/graph per thread.
vector<char> bigDegreeList[NBMAXPROC];
Graph tempGraphs[NBMAXPROC];

void compare_two_graphs_sets(const sparse_hash_map<vector<char>, vector<Graph>> &list1,
        const sparse_hash_map<vector<char>, vector<Graph>> &list2)
{
    int nbIncluded = 0, nbExcluded = 0, nbInvalid = 0; // invalid = C4 or 4K1

    for (const auto &sublist1 : list1)
    {
        const vector<char> &hashVector1 = sublist1.first;
        const auto &it2  = list2.find(hashVector1);
        const vector<Graph> emptyVect;
        // Assigns the list of graphs with the same fingerprint, or the empty vector if none are found.
        const vector<Graph> &sameHashList2 = it2 != list2.cend() ? it2->second : emptyVect;

        for (const Graph &g1 : sublist1.second)
        {
            if (!free_C4_O4(g1, g1.nbVert))
            {
                nbInvalid++;
                continue;
            }

            bool found = false;
            for (const Graph& g2 : sameHashList2)
            {
                if (are_isomorphic(g1, g2, 0))
                {
                    found = true;
                    break;
                }
            }

            if (found)
                nbIncluded++;
            else
            {
                g1.pretty_print();
                nbExcluded++;
            }
        }
    }

    int nbTotal = nbIncluded+nbExcluded+nbInvalid;

    cout << nbTotal << " graphs among which " << nbInvalid << " contains a C4 or 4K1\n";
    cout << "\t" << nbIncluded << " are included in the other list\n";
    cout << "\t" << nbExcluded << " are uniquely in this list" << endl;
}

void get_minimal_graphs(const vector<Graph> &smallGraphs, sparse_hash_map<vector<char>, vector<Graph>> &biggerGraphs)
{
    for (const Graph &g : smallGraphs)
        remove_nonminimal_graphs(g, biggerGraphs, 0);


    int nbMinimal = 0;
    for (const auto& inDict : biggerGraphs)
        nbMinimal += inDict.second.size();

    cout << "Il y a " << nbMinimal << " prefixeurs minimaux.\n";
}


void remove_nonminimal_graphs(const Graph &g, sparse_hash_map<vector<char>, vector<Graph>> &biggerGraphs, int idThread)
{
    assert(g.nbVert != 0);
    int nbVert = g.nbVert+1;
    const int puissNewVert = (1<<(nbVert-1));
    const int nbEdgeCombi = (1<<(nbVert-1));

    bigDegreeList[idThread].resize(g.nbVert+5);
    Graph &gWithEdges = tempGraphs[idThread];

    if (gWithEdges.adjMat == NULL)
        gWithEdges.init(g.nbVert+1, -1);

    bool printDebug = false;

    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);
    gen_P2_list(g, pathLength2, nbVert);

    //TYDY : also try to check for twins

    // Adding a new vertex with all possible neighbourhood, and removing it from the bigger graphs.
    for (int code = 1; code < nbEdgeCombi; code++)
    {
        // It is useless to test graphs containing a C4 or a 4K1.
        // But maybe faster to test them nonetheless?
        bool hasC4 = detect_C4(pathLength2, code);
        if (hasC4)
            continue;
        gWithEdges.copy_and_add_new_vertex_noalloc(g, puissNewVert, code);

        if (printDebug)
        {
            gWithEdges.pretty_print();
            cout << "-----------------";
        }

        if (!free_O4(gWithEdges, gWithEdges.nbVert))
        {
            if (printDebug)
                cerr << "cond1\n";
            continue;
        }

        vector<char> &curBigDegreeList = bigDegreeList[idThread];

        gWithEdges.compute_hashes(curBigDegreeList);
        sort(curBigDegreeList.begin(), curBigDegreeList.begin()+gWithEdges.nbVert);

        const auto &itBiggerGraphsToTest = biggerGraphs.find(curBigDegreeList);
        if (itBiggerGraphsToTest == biggerGraphs.cend())
            continue;

        vector<Graph> &biggerGraphsToTest = itBiggerGraphsToTest->second;
        int iG = 0;
        for (;iG < biggerGraphsToTest.size(); iG++)
        {
            const Graph& gSeen = biggerGraphsToTest[iG];
            if (are_isomorphic(gWithEdges, gSeen, idThread)) // We found the (unique) isomorphic copy.
                break;
        }

        if (iG < biggerGraphsToTest.size()) // We remove this graph: it is not minimal.
            biggerGraphsToTest.erase(biggerGraphsToTest.begin()+iG);
    }
}
