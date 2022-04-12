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
#include <thread>
#include <mutex>

#include <bitset>

#include "gzstream/gzstream.h"
#include "sparsepp/spp.h"
#include "Graph.hh"
#include "gen-graph.hh"
#include "test-properties.hh"

using namespace std;
using spp::sparse_hash_map;

vector<int> subsetsBySize[NBMAXVERT];

//TODO idée : stocker aussi somme des degrés des voisins ? (bof, peu portable sauf si double indirection...)
long long nbTotalGraphsWritten = 0;


//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico)
bool check_if_seen_and_add(Graph& g, vector<char> &degreeList, sparse_hash_map<vector<char>, vector<Graph>> &dico, int idThread)
{
    for (const Graph& gSeen : dico[degreeList])
    {
        if (are_isomorphic(g, gSeen, idThread))
            return false;
    }

    dico[degreeList].push_back(g);
    return true;
}

//TTAADDAA mieux gérer les variables et print débug/info
//TTAADDAA changer le type, là on écrit dans un fichier
//TTAADDAA faire sous-fonction ?
vector<Graph> gen_graphs(int nbVert, vector<Graph> &startingGraphs)
{
    if (nbVert == 1) //TODO: déplacer dans le main ? Mettre dans une fonction ? (oui)
    {
        ofstream fSize("Alexsizegraphedelataille1.txt");
        fSize << "1\n";
        fSize.close();

        ogzstream fGraph("Alexgraphedelataille1.txt.gz");
        fGraph << "1 0\n";
        fGraph.close();
        return {};
    }

    nbTotalGraphsWritten = 0;

    const int puissNewVert = (1<<(nbVert-1));

    int nbGraphPerComp[5] = {0,0,0,0,0};
    int nbFreeGraphPerComp[5] = {0,0,0,0,0};
    int nbGraphFree = 0;
    int nbPassedIso = 0;
    int nbGraphTried = 0;
    vector<Graph> res;
    vector<char> degreeList;
    degreeList.resize(nbVert+4);
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


    for (int i = 0; i < nbVert-1; i++)
    {
        subsetsBySize[i].clear();
        subsetsBySize[i].reserve(1<<i);
    }

    //TODO attention pas symmétrique là.
    for (int code = 0; code < nbEdgeCombi; code++)
    {
        subsetsBySize[adjListGlobal[code].size()].push_back(code);
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



    stringstream fileMinusName, fileSizeMinusName;
    fileMinusName << "Alexgraphedelataille";
    fileMinusName << nbVert-1 << ".txt.gz";
    fileSizeMinusName << "Alexsizegraphedelataille" << nbVert-1 << ".txt";
    ifstream fSize(fileSizeMinusName.str());
    if (fSize.peek() == EOF)
    {
        cerr << "Lancer avant la taille -1 size \n";
        cerr << fileSizeMinusName.str() << endl;
        exit(3);
    }
    int nbGMinus = -1;
    fSize >> nbGMinus;
    fSize.close();
    vector<Graph> listMinus = load_from_file(fileMinusName.str(), nbGMinus);

    if (listMinus.empty())
    {
        cerr << "Lancer avant la taille -1 \n";
        exit(3);
        vector<Graph> dummy;
        listMinus = gen_graphs(nbVert-1, dummy);
    }

    cout << "j'ai généré/trouvé les graphes à " << nbVert-1 << " somets : il y en a " << listMinus.size() << endl;


    int degMin = 1000000, degMax = 0;
    for (const Graph &g : listMinus)
    {
        degMin = min(degMin, g.nbEdge);
        degMax = max(degMax, g.nbEdge);
    } //TODO in load_from_file...

    vector<int> degreesToDo;
    degreesToDo.reserve(degMax-degMin+1+nbVert);
    int moy = (degMax+degMin+nbVert)/2;
    degreesToDo.push_back(moy);
    for (int i = 1; ; i++)
    {
        int d1 = moy-i;
        int d2 = moy+i;

        if (d1 >= degMin)
            degreesToDo.push_back(d1);
        if (d2 <= degMax+nbVert-1)
            degreesToDo.push_back(d2);
        //degreesToDo.push_back(i); TTAADDAA => what ?!
        if (d1 < degMin && d2 > degMax+nbVert-1)
            break;
    }

    stringstream fileName;
    fileName << "Alexgraphedelataille";
    fileName << nbVert << ".txt.gz";

    ogzstream outFile(fileName.str().c_str());

    mutex threadMutex;
    vector<thread> threads(nbProc-1);
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc] = thread(&gen_graphs_thread, std::ref(listMinus), std::ref(startingGraphs), isTwinCompat, std::ref(degreesToDo), std::ref(outFile), iProc, std::ref(threadMutex));

    thread lastThread(&gen_graphs_thread, std::ref(listMinus), std::ref(startingGraphs), isTwinCompat, std::ref(degreesToDo), std::ref(outFile), nbProc-1, std::ref(threadMutex));
    lastThread.join();
    for (int i = 0; i < nbProc-1; i++)
        threads[i].join();
    outFile.close();

    stringstream fileSizeName;
    fileSizeName << "Alexsizegraphedelataille" << nbVert << ".txt";
    ofstream fileSize(fileSizeName.str());
    fileSize << nbTotalGraphsWritten << "\n";
    cerr << "Generated " << nbTotalGraphsWritten << " graphs for size " << nbVert << endl;
    fileSize.close();


    return res;
}


vector<Graph> load_from_file(const string &filename, long long nbGraphToRead)
{
    vector<Graph> res;
    igzstream file(filename.c_str());
    long long nbGraph;
    if (file.peek() == EOF)
        return res;
    if (nbGraphToRead == -1) // C'est a priori des fixeurs, leur nombre est la première
                             // ligne du fichier.
    {
        string tata;
        file >> nbGraph;
        getline(file, tata);
    }

    else
        nbGraph = nbGraphToRead;
    res.resize(nbGraph);
    for (long long i = 0; i < nbGraph; i++)
    {
        res[i].fill_from_file(file);
    }
    file.close();

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

//TTAADDAA à voir avec gen_graphs_thread
//void save_to_file(const string &filename, const sparse_hash_map<vector<char>, vector<Graph>> &graphList, long long nbGraph)


/** Internal functions **/
//TTAADDAA peut-être un peu long, splitter en sous-fonctions ?
vector<Graph> gen_graphs_thread(vector<Graph> &listMinus, vector<Graph> &startingGraphs, int **isTwinCompat, vector<int> &sizesToDo, ogzstream &outFile, int idThread, mutex &lock)
{
    const int nbVert = listMinus[0].nbVert+1;
    const int puissNewVert = (1<<(nbVert-1));

    vector<char> degreeList;
    degreeList.resize(nbVert+4);
    sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs;

    long long sizeTotalTwinVector = 0;
    vector<long long> twinLists;
    twinLists.reserve(NBMAXVERT*NBMAXVERT);
    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);

    const int nbEdgeCombi = (1<<(nbVert-1));

    long long cptGraph = 0;
    Graph gWithEdges;
    gWithEdges.init(nbVert, -1);

    while (true)
    {
        lock.lock();
        if (sizesToDo.empty())
        {
            lock.unlock();
            return {};
        }

        int m = sizesToDo.back();
        //cerr << "doing size " << m << endl;
        sizesToDo.pop_back();
        lock.unlock();

        for (Graph &gStart : startingGraphs)
        {
            if (gStart.nbEdge == m)
            {
                for (int i = 0; i < gStart.nbVert; i++)
                    degreeList[i] = gStart.get_neighb(i).size();
                sort(degreeList.begin(), degreeList.begin()+gStart.nbVert);
                gStart.compute_hashes(degreeList);
                check_if_seen_and_add(gStart, degreeList, deglist2Graphs, idThread);
            }
        }

        long long nbGraph = 0;
        for (const Graph& g : listMinus)
        {
            if (g.nbEdge > m || g.nbEdge + g.nbVert < m)
                continue;
            //cerr << "\t" << g.nbEdge << endl;
            nbGraph++;
            cptGraph++;
            if (cptGraph%50000 == 0)
                cout << "Nous sommes sur leeeee " << cptGraph << "-ème graphe sur " << listMinus.size() << " (nbEdges: " << m << ')' << endl;

            bool isPref13Taken[8192];
            memset(isPref13Taken, false, 8192);
            for (int iNode = 0; iNode < nbVert; iNode++)
            {
                int prefAdj = g.adjMat[iNode]%8192;
                isPref13Taken[prefAdj] = true;
            }

            twinLists.clear();
            gen_twin_list(g, twinLists, nbVert);
            sizeTotalTwinVector += twinLists.size();

            gen_P2_list(g, pathLength2, nbVert);

            const vector<int> &ourSubsets = subsetsBySize[m-g.nbEdge];
            for (int code : ourSubsets)
            {
                if (isPref13Taken[code%8192])
                {
                    static int cptBlah = 0;
                    //cerr << ++cptBlah << ": discarded subset because same adjacency in F13\n";
                    continue;
                }
                /*bool hasTwin = false;
                for (int x : adjListGlobal[code])
                {
                    if ((g.adjMat[x] ^ code) == (1<<x))
                    {
                        hasTwin = true;
                        break;
                    }
                }
                if (hasTwin)
                    continue;
                */

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

                if (free_O4(gWithEdges, nbVert))
                {
                    for (int i = 0; i < gWithEdges.nbVert; i++)
                        degreeList[i] = gWithEdges.get_neighb(i).size();
                    sort(degreeList.begin(), degreeList.begin()+gWithEdges.nbVert);
                    gWithEdges.compute_hashes(degreeList);
                    check_if_seen_and_add(gWithEdges, degreeList, deglist2Graphs, idThread);
                }

            }

            //cout  << "Il y a " << nbGraph << " graphes à " << nbVert << " sommets et " << m << " arêtes .\n";
        }
        lock.lock(); //TTAADDAA ça dans un équivalent de save_to_file
        cerr << "seen " << nbGraph << " graphs for size " << m << endl;

        long long curNbWritten = 0;
        for (const auto& inDict : deglist2Graphs)
        {
            curNbWritten += inDict.second.size();
            for (const Graph &g : inDict.second)
            {
                g.print_in_file(outFile);
            }
        }
	    cerr << "finished writing for nbEdges = " << m << endl;
        nbTotalGraphsWritten += curNbWritten;
        lock.unlock();
        deglist2Graphs.clear();
    }

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
    return {}; //TODO enlever ça en transformer en void
}

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

//TTAADDAA renomer en gen_falsetwins_list, gen_truetwins_list
// Faux jumeaux
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

//Vrais jumeaux
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

//TTAADDAA renomer en detect_newC4_from_P2list
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

