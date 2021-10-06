#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cassert>
#include <map>
#include <cstdlib>
#include <string>
#include <cstdio>
#include <cstring>
#include <algorithm>


#include "sparsepp/spp.h"
#include "Graph.hh"
#include "gen-graph.hh"
#include "test-properties.hh"

using namespace std;
using spp::sparse_hash_map;

//TODO idée : stocker aussi somme des degrés des voisins ? (bof, peu portable sauf si double indirection...)

void gen_subsets(int k, int n, vector<vector<int>> &listRes)
{
    vector<int> l(k);
    for (int i = 0; i < k; i++)
        l[i] = i+1;

    bool end = false;
    assert(k != 0 and k <= n);

    int i;
    while (!end)
    {
        i = k-1;
        listRes.push_back(l);

        while (i != -1 && l[i] == n-k+i+1)
            i = i-1;
        if (i == -1)
            end = true;
        else
        {
            l[i] = l[i]+1;
            for (int z = i+1; z < k; z++)
                l[z] = l[z-1]+1;
        }
    }
/*
    for (const auto &combi : listRes)
    {
        for (int x : combi)
            cout << x << " ";
        cout << endl;
    }
*/
}


//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico)
bool check_if_seen_and_add(Graph& g, vector<int> &degreeList, sparse_hash_map<vector<int>, vector<Graph>> &dico)
{
    /*
    cout << "checking...\n";
    for (int x : g.degreeList)
        cout << x << " ";
    cout << endl;
    */
    for (const Graph& gSeen : dico[degreeList])
    {
        if (are_isomorphic(g, gSeen))
            return false;
    }

    //cout << "   accepting :)\n";

    dico[degreeList].push_back(g);
    return true;
}

vector<Graph> load_from_file(const string &filename)
{
    vector<Graph> res;
    ifstream file(filename);
    int nbGraph;
    if (file.peek() == EOF)
        return res;
    file >> nbGraph;

    res.resize(nbGraph);
    for (int i = 0; i < nbGraph; i++)
    {
        res[i] = Graph(file);
    }

    /*
    long long nbTwinTotal = 0;
    long long  nbGraphWithTwin = 0;
    for (const Graph& g : res)
    {
        int curNbTwin = 0;
        for (int i = 0; i < g.nbVert; i++)
        {
            curNbTwin += nb_twin(g, i);
        }
        nbTwinTotal += curNbTwin/2;
        if (curNbTwin)
            nbGraphWithTwin++;
    }

    cerr << (long double)nbGraphWithTwin*100 / res.size() << "\% graphs with twins and on average " << (long double) nbTwinTotal / res.size() << " twins per graph and " << (long double) nbTwinTotal << " twins per twin-containing graph\n";
    cerr << res[0].nbVert << " " <<  (long double)nbGraphWithTwin*100 / res.size() << " " << (long double) nbTwinTotal / res.size() << " " << (long double) nbTwinTotal << "\n";
*/


    return res;
}

//void save_to_file(const string &filename, const unordered_map<vector<int>, vector<Graph>, vector_hash> &graphList, int nbGraph)
void save_to_file(const string &filename, const sparse_hash_map<vector<int>, vector<Graph>> &graphList, int nbGraph)
{
    vector<Graph> res;
    ofstream file(filename);
    //int nbGraph = graphList.size();

    file << nbGraph << endl;

    for (const auto& inDict : graphList)
        for (const Graph &g : inDict.second)
            g.print_in_file(file);
}


inline void gen_twin_list(const Graph &g, vector<long long> &twinLists2, int nbVert)
{
    /*
    twinLists.clear();
    memset(isTwin, 0, nbVert-1);

    int lastTwin = -1;
    for (int v1 = 0; v1 < nbVert-1; v1++)
    {
        if (isTwin[v1])
            continue;
        for (int v2 : g.get_neighb(v1))
        {
            if (v2 > v1 && (g.adjMat[v1] ^ g.adjMat[v2]) == ((1<<v1)^(1<<v2)))
            {
                if (lastTwin != v1)
                {
                    lastTwin = v1;
                    twinLists.push_back({v1, v2});
                }
                else
                {
                    twinLists.back().push_back(v2);
                }
                isTwin[v2] = true;
                lastTwin = v1;
            }
        }
    }
    */

    for (int v1 = 0; v1 < nbVert-2; v1++)
    {
        //for (int v2 : g.get_neighb(v1))
        int puiss1 = 1<<v1;
        long long curCompat = v1;
        long long newCompat = 0;
        int puiss2 = (1<<v1);
        for (int v2 = v1+1; v2 < nbVert-1; v2++)
        {
            //newCompat*=2;
            //int puiss2 = 1<<v2;
            puiss2 *= 2;
            int xorage = g.adjMat[v1] ^ g.adjMat[v2];
            if (xorage == 0 || xorage == (puiss1 ^ puiss2))
            ///if (v2 > v1 && (g.adjMat[v1] ^ g.adjMat[v2]) == ((1<<v1)^(1<<v2)))
            {
                //long long newCompat = (1<<(32+v2));
                newCompat ^= puiss2;
                //newCompat <<= 32;
                //newCompat ^= v1;
                //curCompat ^= newCompat;
                //bitset<64> y(newCompat);
                //cout << "\t" << y << endl;
                //twinLists2.push_back(newCompat);
                //break;

            }
        }
        if (newCompat)
            twinLists2.push_back(curCompat+(newCompat*256));
    }

}


void gen_twin_list2(const Graph &g, vector<long long> &twinLists2, int nbVert)
{
    /*
    for (int i = 0; i < nbVert; i++)
        isTwin[i] = false;

    int lastTwin = -1;
    for (int v1 = 0; v1 < nbVert-2; v1++)
    {
        if (isTwin[v1])
            continue;
        for (int v2 =v1+1; v2 < nbVert-1; v2++)
        {
            if (g.adjMat[v1] == g.adjMat[v2])// == ((1<<v1)^(1<<v2)))
            {
                if (lastTwin != v1)
                {
                    lastTwin = v1;
                    twinLists.push_back({v1, v2});
                }
                else
                {
                    twinLists.back().push_back(v2);
                }
                isTwin[v2] = true;
                lastTwin = v1;
            }
        }
    }
    */

    //TODO modifier si jamais utilisé à nouveau
    for (int v1 = 0; v1 < nbVert-2; v1++)
    {
        for (int v2 = v1+1; v2 < nbVert-1; v2++)
        {
            if (v2 > v1 && (g.adjMat[v1] == g.adjMat[v2]))
            {
                long long newCompat = (1<<v2);
                newCompat <<= 32;
                newCompat ^= v1;
                twinLists2.push_back(newCompat);
                break;
            }
        }
    }



}

inline bool can_discard_edgelist(const vector<long long> &twinLists2, int *isTwinCompat, int nbVert)
{
    /*
    memset(isInList, 0, nbVert-1);
    for (int newNeighb : newEdgesList)
        isInList[newNeighb] = true;

    for (const vector<int> &curTwins : twinLists)
    {
        for (int i = 1; i < curTwins.size(); i++)
        {
            if (!isInList[curTwins[i-1]] && isInList[curTwins[i]])
                return true;
        }
    }
    */
    for (const long long &curTwins : twinLists2)
    {
        const int twin1 = curTwins%256;
        const int twin2 = curTwins/256;
        if (isTwinCompat[twin1] & twin2)
            return true;
    }


    return false;
}


vector<Graph> gen_graphs(int nbVert)
{
    const int puissNewVert = (1<<nbVert-1);

    int nbGraphPerComp[5] = {0,0,0,0,0};
    int nbFreeGraphPerComp[5] = {0,0,0,0,0};
    int nbGraphFree = 0;
    int nbPassedIso = 0;
    int nbGraphTried = 0;
    vector<Graph> res;
    vector<int> degreeList;
    degreeList.resize(nbVert+1);
    sparse_hash_map<vector<int>, vector<Graph>> deglist2Graphs;

    vector<vector<int>> twinLists;
    vector<long long> twinLists2;
    twinLists2.reserve(NBMAXVERT*NBMAXVERT);

    const int nbEdgeCombi = (1<<nbVert-1);

    bool isTwin[NBMAXVERT];
    bool isInList[NBMAXVERT];
    int **isTwinCompat = NULL;
    isTwinCompat = (int**) malloc(sizeof(*isTwinCompat)*nbEdgeCombi);
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



    if (nbVert == 1)
    {
        Graph g;
        g.init(1, 0);
        vector<int> vv(1, 0);
        deglist2Graphs[vv].push_back(g);
    }

    else
    {
        stringstream fileMinusName;
        fileMinusName << "Alexgraphedelataille";
        fileMinusName << nbVert-1 << ".txt";
        vector<Graph> listMinus = load_from_file(fileMinusName.str());
        if (listMinus.empty())
        {
            cerr << "Lancer avant la taille -1 \n";
            exit(3);
            listMinus = gen_graphs(nbVert-1);
        }

        cout << "j'ai généré/trouvé les graphes à " << nbVert-1 << " somets : il y en a " << listMinus.size() << endl;

        vector<vector<int>> listSubsetsEdges = {{}};
        for (int m = 1; m < nbVert; m++)
            gen_subsets(m, nbVert-1, listSubsetsEdges);

        int cptGraph = 0;
        Graph gNew, gWithEdges;
        gNew.init(nbVert, -1);
        gWithEdges.init(nbVert, -1);
        int nbComp = 1;//+0*nb_connected_comp(gNew);
        for (const Graph& g : listMinus)
        {
            cptGraph++;
            if (cptGraph%100 == 0)
                cout << "Nous sommes sur le " << cptGraph << "-ème graphe sur " << listMinus.size() << endl;
            /*gNew.copy_and_add_new_vertex(g);//TODO garder le retour, un sommet ?

            int nbComp = 1;//+0*nb_connected_comp(gNew);
            //nbGraphPerComp[nbComp]++;

            nbGraphTried++;
            if (nbComp && free_C4_O4(gNew, nbVert))
            {
                for (int i = 0; i < gNew.nbVert; i++)
                    degreeList[i] = gNew.get_neighb(i).size();
                sort(degreeList.begin(), degreeList.begin()+(gNew.nbVert));


                //nbGraphFree++;
                //nbFreeGraphPerComp[nbComp]++;
                gNew.compute_hashes(degreeList);
                nbGraphFree++;
                nbFreeGraphPerComp[nbComp]++;
                if (check_if_seen_and_add(gNew, degreeList, deglist2Graphs))
                    nbPassedIso++;
            }

            */


            twinLists2.clear();
            gen_twin_list(g, twinLists2, nbVert);
            //gen_twin_list2(g, twinLists2, nbVert);
            //for (int i = 0; i < nbVert; i++)
            //    isTwin[i] = false;



            //for (const vector<int> &newEdgesList : listSubsetsEdges)
            for (int code = 0; code < nbEdgeCombi; code++)
            {
                const vector<int> &newEdgesList = adjListGlobal[code];
                //for (int i = 0; i < nbVert; i++)
                    //isInList[i] = false;
                    //
                    //
                bool refuseBecauseTwins = can_discard_edgelist(twinLists2, isTwinCompat[code], nbVert);
                if (refuseBecauseTwins)
                {
                   //cerr << "lol YEAH\n";
                    continue;
                }
                /*
                gWithEdges.copy_and_add_new_vertex(g);
                for (int newNeighb : newEdgesList)
                {
                    gWithEdges.adjMat[newNeighb]^= puissNewVert;
                }
                gWithEdges.nbEdge += newEdgesList.size();
                gWithEdges.adjMat[nbVert-1] = code;*/
                gWithEdges.copy_and_add_new_vertex_bis(g, newEdgesList, puissNewVert, code);
                    //gWithEdges.add_edge(nbVert-1, newNeighb);






                /*
                 * nbComp = nb_connected_comp(gWithEdges);
                nbGraphPerComp[nbComp]++;*/
                nbGraphTried++;
                if (nbComp && free_C4_O4(gWithEdges, nbVert))
                {
                    for (int i = 0; i < gWithEdges.nbVert; i++)
                        degreeList[i] = gWithEdges.get_neighb(i).size();
                    sort(degreeList.begin(), degreeList.begin()+gWithEdges.nbVert);


                    nbGraphFree++;
                    nbFreeGraphPerComp[nbComp]++;
                    gWithEdges.compute_hashes(degreeList);
                    if (check_if_seen_and_add(gWithEdges, degreeList, deglist2Graphs))
                        nbPassedIso++;
                    //TODO refléchir : si déjà vu, alors il existe des combinaisons avec plus d'arêtes qui sont possibles ?!

                }

            }
        }
    }

    int nbGraph = 0;
    for (const auto& inDict : deglist2Graphs)
        nbGraph += inDict.second.size();

    cout  << "Il y a " << nbGraph << " graphes à " << nbVert << " sommets.\n";

    stringstream fileName;
    fileName << "Alexgraphedelataille";
    fileName << nbVert << ".txt";

    save_to_file(fileName.str(), deglist2Graphs, nbGraph);
    /*
    for (int i = 1; i < 5; i++)
        cerr << "nb graphes avec " << i << " composantes connexes : " << nbGraphPerComp[i] << endl;
    for (int i = 1; i < 4; i++)
        cerr << "nb free graphes avec " << i << " composantes connexes : " << nbFreeGraphPerComp[i] << endl;
    cerr << nbPassedIso << " out of " << nbGraphFree << " were not isomorphic\n";

    cerr << nbVert << " " << nbGraphTried << " " << nbGraphFree << " " << nbPassedIso << " ";
    for (int i = 1; i < 5; i++)
        cerr << nbGraphPerComp[i] << " ";
    for (int i = 1; i < 4; i++)
        cerr << nbFreeGraphPerComp[i] << " ";
    cerr << endl;
    */
    return res;

}

