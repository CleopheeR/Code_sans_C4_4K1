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

#include <bitset>


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
}


//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico)
bool check_if_seen_and_add(Graph& g, vector<char> &degreeList, sparse_hash_map<vector<char>, vector<Graph>> &dico)
{
    for (const Graph& gSeen : dico[degreeList])
    {
        if (are_isomorphic(g, gSeen))
            return false;
    }

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
void save_to_file(const string &filename, const sparse_hash_map<vector<char>, vector<Graph>> &graphList, int nbGraph)
{
    vector<Graph> res;
    ofstream file(filename);

    file << nbGraph << endl;

    for (const auto& inDict : graphList)
        for (const Graph &g : inDict.second)
            g.print_in_file(file);
}


void gen_twin_list(const Graph &g, vector<long long> &twinLists, int nbVert)
{
   for (int v1 = 0; v1 < nbVert-2; v1++)
    {
        int puiss1 = 1<<v1;
        long long curCompat = v1;
        long long newCompat = 0;
        int puiss2 = (1<<v1);
        for (int v2 = v1+1; v2 < nbVert-1; v2++)
        {
            puiss2 *= 2;
            int xorage = g.adjMat[v1] ^ g.adjMat[v2];
            if (xorage == 0 || xorage == (puiss1 ^ puiss2))
                newCompat ^= puiss2;
        }
        if (newCompat)
            twinLists.push_back(curCompat+(newCompat*256));
    }

}


void gen_twin_list2(const Graph &g, vector<long long> &twinLists, int nbVert)
{
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
                twinLists.push_back(newCompat);
                break;
            }
        }
    }
}

bool can_discard_edgelist(const vector<long long> &twinLists, int *isTwinCompat, int nbVert)
{
    for (const long long &curTwins : twinLists)
    {
        const int twin1 = curTwins%256;
        const int twin2 = curTwins/256;
        if (isTwinCompat[twin1] & twin2)
            return true;
    }

    return false;
}


void gen_P2_list(const Graph &g, vector<long long> &pathList, int nbVert)
{
    pathList.clear();

    int puissV1 = 1, puissV2;
    for (int v1 = 0; v1 < nbVert-2; v1++)
    {
        puissV2 = puissV1;
        int adj1 = g.adjMat[v1];
        for (int v2 = v1+1; v2 < nbVert-1; v2++)
        {
            puissV2 *= 2;
            int adj2 = g.adjMat[v2];

            if ((adj1 & puissV2) == 0 && (adj2 & puissV1) == 0)
            {
                int commonAdj = adj1 & adj2;
                if (commonAdj)
                {
                    long long truc = puissV1^puissV2;
                    truc = (truc << 32) + commonAdj;
                    pathList.push_back(truc);
                }
            }
        }
        puissV1 *= 2;
    }
}

bool detect_C4(const vector<long long> &pathList, int code)
{
    for (long long p2 : pathList)
    {
        int uv = p2 >> 32;
        int commonAdj = p2;

        if (((code & uv) == uv) && ((code&commonAdj) != commonAdj))
            return true;
    }
    return false;
}

vector<Graph> gen_graphs(int nbVert)
{
    const int puissNewVert = (1<<(nbVert-1));

    int nbGraphPerComp[5] = {0,0,0,0,0};
    int nbFreeGraphPerComp[5] = {0,0,0,0,0};
    int nbGraphFree = 0;
    int nbPassedIso = 0;
    int nbGraphTried = 0;
    vector<Graph> res;
    vector<char> degreeList;
    degreeList.resize(nbVert+1);
    sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs;

    long long sizeTotalTwinVector = 0;
    vector<long long> twinLists;
    twinLists.reserve(NBMAXVERT*NBMAXVERT);
    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);

    const int nbEdgeCombi = (1<<(nbVert-1));

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
        vector<char> vv(1, 0);
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

            twinLists.clear();
            gen_twin_list(g, twinLists, nbVert);
            sizeTotalTwinVector += twinLists.size();

            gen_P2_list(g, pathLength2, nbVert);

            for (int code = 0; code < nbEdgeCombi; code++)
            {
                bool refuseBecauseTwins = can_discard_edgelist(twinLists, isTwinCompat[code], nbVert);
                if (refuseBecauseTwins)
                {
                   //cerr << "lol YEAH\n";
                    continue;
                }


                bool refuseBecauseC4 = detect_C4(pathLength2, code);
                if (refuseBecauseC4)
                    continue;
                const vector<int> &newEdgesList = adjListGlobal[code];
                gWithEdges.copy_and_add_new_vertex_bis(g, newEdgesList, puissNewVert, code);

                nbGraphTried++;
                if (free_O4(gWithEdges, nbVert))
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
    cout << sizeTotalTwinVector << " elements in twin vector, so average " << (double)sizeTotalTwinVector/(double)listMinus.size() << endl;
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

