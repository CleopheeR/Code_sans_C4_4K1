#include <vector>
#include <string>
#include "sparsepp/spp.h"

#include "Graph.hh"
#include "test-properties.hh"
#include "compute_arrays_compat.hh"
#include "gen-graph.hh"

const int verbose = 0;

using namespace std;

bool is_quasi_fixer(const Graph &g)
{
    vector<vector<int>> partitionSets;
    gen_klmpartition_default_sets(g, partitionSets);

    vector<int> foo;
    vector<string> foo2;
    vector<vector<int>> foo3;
    vector<vector<char>> tableau = compute_cleophee_arrays(g, partitionSets, foo3, foo2, foo);

    int n = tableau.size();
    for (int i1 = 0; i1 < n; i1++)
    {
        for (int i2 = 0; i2 < n; i2++)
        {
            if (tableau[i1][i2] != 'N')
                continue;
            for (int i3 = 0; i3 < n; i3++)
            {
                if (tableau[i1][i3] != 'N')
                    continue;

                if (tableau[i2][i3] == '0' || tableau[i2][i3] == 'N')
                    return false;
            }
        }
    }



    return true;
}


// Generates the possible neighbourhoods for each vertex and puts it in TODO les bons bails
void gen_klmpartition_default_sets(const Graph &g, vector<vector<int>> &listPossibleNeighbs)
{
    int nbVert = g.nbVert+1;
    vector<long long> twinLists;
    twinLists.reserve(NBMAXVERT*NBMAXVERT);
    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);

    const int nbEdgeCombi = (1<<(nbVert-1));

    bool isTwin[NBMAXVERT];
    bool isInList[NBMAXVERT];
    int **isTwinCompat = NULL;

    /*
     * isTwinCompat = (int**) malloc(sizeof(*isTwinCompat)*nbEdgeCombi);
     for (int i = 0; i < nbEdgeCombi; i++)
     isTwinCompat[i] = (int*) malloc(sizeof(*isTwinCompat)*NBMAXVERT);



    //TODO attention pas symmétrique là.
    for (int code = 0; code < nbEdgeCombi; code++)
    {
    for (int v1 = 0; v1 < nbVert-2; v1++)
    {
    int curCompat = 0;
    if (code & (1<<v1))
    {
    isTwinCompat[code][v1] = 0;
    continue;
    }

    for (int v2 = v1+1; v2 < nbVert-1; v2++)
    {
    if (code & (1<<v2))
    curCompat ^= (1<<v2);
    }
    isTwinCompat[code][v1] = curCompat;
    }
    }
    */

    twinLists.clear();
    gen_twin_list(g, twinLists, nbVert);

    gen_P2_list(g, pathLength2, nbVert);

    const int puissNewVert = 1<<(nbVert-1);
    for (int code = 0; code < puissNewVert; code++)
    {
        bool hasTwin = false;
        for (int x : adjListGlobal[code])
        {
            if ((g.adjMat[x] ^ code) == (1<<x))
            {
                hasTwin = true;
                break;
            }
        }
        //if (hasTwin) //TODO peut-être pas...
        //    continue;

        bool refuseBecauseTwins = false;//can_discard_edgelist(twinLists, isTwinCompat[code], nbVert+1);
        if (false && refuseBecauseTwins) //TODO peut-être pas
        {
            //cerr << "lol YEAH\n";
            continue;
        }

        const vector<int> &newEdgesList = adjListGlobal[code];

        //if (newEdgesList.size() == 6 && newEdgesList[0] == 0 && newEdgesList[1] == 1 && newEdgesList[2] == 2 && newEdgesList[3] == 3 && newEdgesList[4] == 4 && newEdgesList[5] == 7)
        //if (newEdgesList.size() == 5 && newEdgesList[0] == 1 && newEdgesList[1] == 2 && newEdgesList[2] == 3 && newEdgesList[3] == 4 && newEdgesList[4] == 7)
        //cerr << "lol " << detect_C4(pathLength2, code) << endl;
        bool refuseBecauseC4 = detect_C4(pathLength2, code);
        if (refuseBecauseC4)
            continue;
        Graph gWithEdges;
        gWithEdges.copy_and_add_new_vertex_bis(g, newEdgesList, puissNewVert, code);

        //if (newEdgesList.size() == 6 && newEdgesList[0] == 0 && newEdgesList[1] == 1 && newEdgesList[2] == 2 && newEdgesList[3] == 3 && newEdgesList[4] == 4 && newEdgesList[5] == 7)
        //    gWithEdges.print();
        if (free_O4(gWithEdges, nbVert))
            listPossibleNeighbs.push_back(newEdgesList);

    }
}

void getPossibleFreeNeighourhoods(int nbVert, const vector<int> &freeVerts, vector<Graph> &ret, Graph &curG, int pos, vector<Graph> &obstructions, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread)
{
    if (pos == freeVerts.size())
    {
        if (is_graph_ok(curG, obstructions, deglist2PrefixeursPlus, verbose == 2))
            ret.push_back(curG);
        else if (verbose == 2)
        {
            cerr << " was graph testing for sets, printing the graph:\n";
            curG.print();
        }
    }

    else
    {
        int newNeighb = freeVerts[pos];

        curG.add_edge(nbVert-1, newNeighb);
        getPossibleFreeNeighourhoods(nbVert, freeVerts, ret, curG, pos+1, obstructions, deglist2PrefixeursPlus, idThread);

        curG.delete_edge(nbVert-1, newNeighb);
        getPossibleFreeNeighourhoods(nbVert, freeVerts, ret, curG, pos+1, obstructions, deglist2PrefixeursPlus, idThread);
    }
}


bool is_graph_ok(Graph& g, vector<Graph> &obstructions, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread, bool print)
{
    int nbObstruction = obstructions.size();
    if (!free_C4_O4(g, g.nbVert))
        return false;
    bool containsObstruction = false;
    for (int i = 0; i < nbObstruction; i++)
    {
        if (is_supergraph_of(g, obstructions[i], idThread))
        {
            if (print)
            {
                cerr << "Identifying graph " << i << endl;
                cerr << endl;
            }
            return false;
        }
    }

    thread_local vector<char> deglist;
    deglist.resize(g.nbVert+4);
    for (int i = 0; i < g.nbVert; i++)
        deglist[i] = g.get_neighb(i).size();
    sort(deglist.begin(), deglist.begin()+g.nbVert);
    g.compute_hashes(deglist);

    const auto &itFindDeglist = deglist2PrefixeursPlus.find(deglist);
    if (itFindDeglist != deglist2PrefixeursPlus.end())
    {
        const vector<Graph> &prefixeursPlusToTest = itFindDeglist->second;
        for (const Graph& gSeen : prefixeursPlusToTest)
        {
            if (are_isomorphic(g, gSeen, idThread))
            {
                if (print)
                    cerr << "Isomorphic to a prefixeur !\n";
                return false;
            }
        }
    }


    return true;
}

vector<vector<char>> compute_cleophee_arrays(const Graph &g, const vector<vector<int>> &adjSets, const vector<vector<int>> &antiCompleteSets, const vector<string> &setsNames, const vector<int> &freeVerts, vector<Graph> &obstructions, sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int idThread, bool print)
{
    if (print)
    {
        for (int x : freeVerts)
            cerr << x << " ";
        cerr << " <---- these are the free vertices\n";
    }


    int n = g.nbVert;
    int nbSets = adjSets.size();
    vector<vector<Graph>> graphsSetsPerId(adjSets.size());

    for (int i = 0; i < nbSets; i++)
    {
        const vector<int> &curAdj = adjSets[i];

        Graph gNew;
        gNew.init(n+1, g.nbEdge);
        for (int u = 0; u < n; u++)
            gNew.adjMat[u] =  g.adjMat[u];

        for (int x : curAdj)
            gNew.add_edge(x, n);

        vector<int> realFreeVerts;
        for (int x : freeVerts)
        {
            if (find(antiCompleteSets[i].begin(), antiCompleteSets[i].end(), x) != antiCompleteSets[i].end())
                continue;

            if (find(curAdj.begin(), curAdj.end(), x) != curAdj.end())
                continue;

            realFreeVerts.push_back(x);
        }
        if (print && realFreeVerts.size() != freeVerts.size())
        {
            cerr << "\t\t" << setsNames[i] << " free verts are: ";
            for (int x : realFreeVerts)
                cerr << x << " ";
            cerr << endl;
        }


        getPossibleFreeNeighourhoods(n+1, realFreeVerts, graphsSetsPerId[i], gNew, 0, obstructions, deglist2PrefixeursPlus, idThread);

        if (print)
        {
            if (graphsSetsPerId[i].empty())
            {
                cerr << "set " << setsNames[i] << " cannot exist " << endl;

                if (!free_C4(gNew, n+1))
                    cerr << "\t pas de C4 possible\n";
                if (!free_O4(gNew, n+1))
                    cerr << "\t pas de O4 possible\n";

            }
            else if (print)
                cerr << "set " << setsNames[i] << " can exist " << endl;
        }
        if (realFreeVerts.empty())
            assert(graphsSetsPerId[i].size() <= 1);
    }

    if (print)
    {
        cout << "\t";
        for (const string& s : setsNames)
            cout << s << "\t";
        cout << endl;
    }

    vector<vector<char>> tableau(nbSets);
    vector<vector<vector<Graph>>> possibleG1EdgeG2s(nbSets);
    vector<vector<vector<Graph>>> possibleG1NoEdgeG2s(nbSets);

    for (int i = 0; i < nbSets; i++)
    {
        tableau[i].resize(nbSets);
        possibleG1EdgeG2s[i].resize(nbSets);
        possibleG1NoEdgeG2s[i].resize(nbSets);
    }


    for (int i1 = 0; i1 < nbSets; i1++)
    {
        if (print)
            cout << setsNames[i1] << "\t";
        //for (int ii = 0; ii < i1+1; ii++)
        //    cout << "\t";
        const vector<int> &curAdj1 = adjSets[i1];


        for (int i2 = 0; i2 < nbSets; i2++)
        {
            const vector<int> &curAdj2 = adjSets[i2];
            bool isOkNoEdge = false;
            bool isOkWithEdge = false;


            for (const Graph &g1 : graphsSetsPerId[i1])
            {
                for (const Graph &g2 : graphsSetsPerId[i2])
                {
                    Graph g12;
                    g12.init(g1.nbVert+1, g1.nbEdge);
                    for (int u = 0; u < g1.nbVert; u++)
                        g12.adjMat[u] = g1.adjMat[u];

                    for (int x : adjListGlobal[g2.adjMat[g2.nbVert-1]])
                        g12.add_edge(g1.nbVert, x);

                    // No edge
                    isOkNoEdge = is_graph_ok(g12, obstructions, deglist2PrefixeursPlus, print);
                    if (isOkNoEdge)
                        possibleG1NoEdgeG2s[i1][i2].push_back(g12);
                    else if (print)
                        cerr << "\n => For NoEdge was " << setsNames[i1] << " VS " << setsNames[i2] << endl;

                    // An edge
                    g12.add_edge(g12.nbVert-1, g12.nbVert-2);
                    isOkWithEdge = is_graph_ok(g12, obstructions, deglist2PrefixeursPlus, print);

                    if (isOkWithEdge)
                        possibleG1EdgeG2s[i1][i2].push_back(g12);
                    else if (print)
                        cerr << "\n => For with edge was " << setsNames[i1] << " VS " << setsNames[i2] << endl;

                }
            }

            char charToPrint = 'N';

            if (!isOkNoEdge && !isOkWithEdge)
                charToPrint = '-';
            else if (!isOkNoEdge)
                charToPrint = '1';
            else if (!isOkWithEdge)
                charToPrint = '0';

            if (print)
                cout << charToPrint << "\t";
            tableau[i1][i2] = charToPrint;

        }
        if (print)
            cout << endl;
    }
    if (print)
        cerr << endl << endl;

    return tableau;
}


/*
   void compute_N_melted_graph(const Graph &g, int **tabCompat)
   {
   int n = g.nbVert;
   vector<int> oldVertId2NewId(n, -1);
   int lastId = 0;

   for (int x : listNVerts)
   {
   oldVertId2NewId[x] = lastId;
   lastId++;
   }

   Graph gOnlyNVerts;
   gOnlyNVerts.init(listNVerts.size(), 0);
   for (int x : listNVerts)
   {
   int newId = oldVertId2NewId[x];
   const vector<int> neighbs = g.get_neighb(x);

   for (int y : neighbs)
   {
   int neighbNewId = oldVertId2NewId[y];
   if (neighbNewId != -1)
   gOnlyNVerts.add_edge(newId, neighbNewId);
   }




   }






   }

*/
