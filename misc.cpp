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
Graph isPreOrFixeurGWithEdges[NBMAXPROC];

void compare_two_fixeurs_sets(const sparse_hash_map<vector<char>, vector<Graph>> &list1,
        const sparse_hash_map<vector<char>, vector<Graph>> &list2)
{
    int nbIncluded = 0, nbExcluded = 0, nbInvalid = 0; // invalid = C4 or 4K1

    for (const auto &sublist1 : list1)
    {
        const vector<char> &hashVector1 = sublist1.first;
        const auto &it2  = list2.find(hashVector1);
        const vector<Graph> emptyVect;
        const vector<Graph> &sameHashList2 = it2 != list2.cend() ? it2->second : emptyVect; // The hash is present in the second list.

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

void get_minimal_fixeurs(const vector<Graph> &prefixeurMinusList, sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict)
{
    for (const Graph &g : prefixeurMinusList)
        remove_nonminimal_fixeurs(g, prefixeurPlusDict, NULL, 0);


    int nbMinimal = 0;
    for (const auto& inDict : prefixeurPlusDict)
        nbMinimal += inDict.second.size();

    cout << "Il y a " << nbMinimal << " prefixeurs minimaux.\n";
}


void remove_nonminimal_fixeurs(const Graph &g, sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat, int idThread)
{
    int nbVert = g.nbVert+1;
    const int puissNewVert = (1<<(nbVert-1));
    const int nbEdgeCombi = (1<<(nbVert-1));

    if (bigDegreeList[idThread].empty())
        bigDegreeList[idThread].resize(g.nbVert+5);
    Graph &gWithEdges = isPreOrFixeurGWithEdges[idThread];
    if (gWithEdges.adjMat == NULL)
        gWithEdges.init(g.nbVert+1, -1);
    bool printDebug = false;

    //TTEEDDEE : refaire benchmark trop long ?
    //vector<long long> twinLists2;
    //twinLists2.reserve(NBMAXVERT*NBMAXVERT);
    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);

    if (g.nbVert == 0)
        exit(78);
    gen_P2_list(g, pathLength2, nbVert);



    for (int code = 0; code < nbEdgeCombi; code++)
    {
        /*bool refuseBecauseTwins = can_discard_edgelist(twinLists2, isTwinCompat[code], nbVert);
        if (refuseBecauseTwins)
        {
            //cerr << "lol YEAH\n";
            continue;
        }*/


        bool hasC4 = detect_C4(pathLength2, code);
        if (hasC4)
            continue;
        gWithEdges.copy_and_add_new_vertex_noalloc(g, puissNewVert, code);

        if (printDebug)
        {
            gWithEdges.pretty_print();
            cout << "-----------------";
        }

        //TTAADDAA : pourquoi on reteste C4 ???!!!
        if (!free_C4(gWithEdges, gWithEdges.nbVert) || !free_O4(gWithEdges, gWithEdges.nbVert))
        //if (!free_O4(gWithEdges, gWithEdges.nbVert))
        {
            if (printDebug)
                cerr << "cond1\n";
            continue;
        }

        //TTAADDAA : y'a des trucs en commun avec la fonction de is_pre_of_fixeur
        vector<char> &curBigDegreeList = bigDegreeList[idThread];

        gWithEdges.compute_hashes(curBigDegreeList);
        sort(curBigDegreeList.begin(), curBigDegreeList.begin()+gWithEdges.nbVert);

        const auto &itPrefixeursPlus1ToTest = prefixeurPlusDict.find(curBigDegreeList);
        if (itPrefixeursPlus1ToTest == prefixeurPlusDict.cend())
            continue;

        vector<Graph> &prefixeursPlus1ToTest = itPrefixeursPlus1ToTest->second;
        int iG = 0;
        for (;iG < prefixeursPlus1ToTest.size(); iG++)
        {
            const Graph& gSeen = prefixeursPlus1ToTest[iG];
            if (are_isomorphic(gWithEdges, gSeen, idThread))
                break;
        }

        if (iG < prefixeursPlus1ToTest.size())
            prefixeursPlus1ToTest.erase(prefixeursPlus1ToTest.begin()+iG);
    }
}
