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
#include <sstream>

#include <bitset>

#include "gzstream/gzstream.h"
#include "sparsepp/spp.h"
#include "Graph.hh"
#include "gen-graph.hh"
#include "test-properties.hh"

//#define STATS_GEN

using namespace std;
using spp::sparse_hash_map;


//TODO idée : stocker aussi somme des degrés des voisins ? (bof, peu portable sauf si double indirection...)
long long nbTotalGraphsWritten = 0;


//bool check_if_seen_and_add(const Graph& g, unordered_map<vector<int>, vector<Graph>, vector_hash> &dico)
bool check_if_seen_and_add(Graph& g, const vector<char> &degreeList, sparse_hash_map<vector<char>, vector<Graph>> &dico, int idThread)
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
    if (nbVert == 1)
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

#ifdef STATS_GEN
    int nbGraphPerComp[5] = {0,0,0,0,0};
    int nbFreeGraphPerComp[5] = {0,0,0,0,0};
    int nbGraphFree = 0;
    int nbPassedIso = 0;
    int nbGraphTried = 0;
#endif
    vector<Graph> res;
    vector<char> degreeList;
    degreeList.resize(nbVert+4);

    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);

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
        /* TODO see if working
        vector<Graph> dummy;
        listMinus = gen_graphs(nbVert-1, dummy);
        */
    }

    cout << "j'ai généré/trouvé les graphes à " << nbVert-1 << " somets : il y en a " << listMinus.size() << endl;


    long long nbMinus = listMinus.size();
    long long step = (nbMinus/STEP_RATIO+1);
    long long nbBatch = nbMinus/max(step,1ll);
    vector<pair<long long, long long>> indicesToDo(nbBatch+1);
    for (long long i = 0; i <= nbBatch; i++)
        indicesToDo[i] = {i*step, min(nbMinus,(i+1)*step)};

    stringstream fileName;
    fileName << "Alexgraphedelataille";
    fileName << nbVert << ".txt.gz";

    ogzstream outFile(fileName.str().c_str());

    mutex threadMutexBatches;
    vector<mutex> threadMutexes(256*256);
    vector<thread> threads(nbProc-1);

    vector<sparse_hash_map<vector<char>, vector<Graph>>> graphsForIsomCheck(256*256);
    sparse_hash_map<vector<char>, vector<Graph>> *ptrFooNULL = &graphsForIsomCheck[0];
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc] = thread(&gen_graphs_thread, std::cref(listMinus), std::ref(startingGraphs), std::ref(indicesToDo), std::ref(outFile), iProc, std::ref(threadMutexes), std::ref(threadMutexBatches), ptrFooNULL, false);

    thread lastThread(&gen_graphs_thread, std::cref(listMinus), std::ref(startingGraphs), std::ref(indicesToDo), std::ref(outFile), nbProc-1, std::ref(threadMutexes), std::ref(threadMutexBatches), ptrFooNULL, false);
    lastThread.join();
    for (int i = 0; i < nbProc-1; i++)
        threads[i].join();

    unsigned long long nbWritten = nbTotalGraphsWritten;
    graphsForIsomCheck.clear();
    outFile.close();

    stringstream fileSizeName;
    fileSizeName << "Alexsizegraphedelataille" << nbVert << ".txt";
    ofstream fileSize(fileSizeName.str());
    fileSize << nbWritten << "\n";
    cerr << "Generated " << nbWritten << " graphs for size " << nbVert << endl;
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
        res[i] = Graph(file);//.fill_from_file(file);
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
vector<Graph> gen_graphs_thread(const vector<Graph> &listMinus, vector<Graph> &startingGraphs, vector<pair<long long, long long>> &indicesToDo, ogzstream &outFile, int idThread, vector<mutex> &locksTests, mutex &lockToDo, sparse_hash_map<vector<char>, vector<Graph>> *deglists2GraphsToAdd, bool keepTwins)
{
    const int nbVert = listMinus[0].nbVert+1;
    const int puissNewVert = (1<<(nbVert-1));

    vector<char> degreeList;
    degreeList.resize(nbVert+4);

    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);

    const int nbEdgeCombi = (1<<(nbVert-1));

    long long cptGraph = 0;
    Graph gWithEdges;
    gWithEdges.init(nbVert, -1);
    long long curNbTotalGraphsWritten = 0;

    stringstream strAllGenGraphs;
    if (idThread == 0)
    {
        for (Graph& gStart : startingGraphs)
        {
            for (int i = 0; i < gStart.nbVert; i++)
                degreeList[i] = gStart.get_neighb(i).size();
            gStart.compute_hashes(degreeList);
            sort(degreeList.begin(), degreeList.begin()+gStart.nbVert);
            int truc = (unsigned char)degreeList[gStart.nbVert]+256*(unsigned char) degreeList[gStart.nbVert+1];
            locksTests[truc].lock();
            if (check_if_seen_and_add(gStart, degreeList, deglists2GraphsToAdd[truc], idThread))
            { //TODO separate the printing out of the lock?
                curNbTotalGraphsWritten++;
                gStart.print_in_string(strAllGenGraphs);
            }
            locksTests[truc].unlock();
        }
    }


    bool *graphNeighbsToBool = NULL;
    graphNeighbsToBool = (bool*) calloc(nbEdgeCombi, sizeof(bool));
    while (true)
    {
        long long nbSizesLeft;
        lockToDo.lock();
        if (indicesToDo.empty())
        {
            lockToDo.unlock();
            break;
        }

        pair<long long, long long> toDo = indicesToDo.back();
        indicesToDo.pop_back();
        nbSizesLeft = indicesToDo.size();
        lockToDo.unlock();

        long long nbGraph = 0, nbGraphMinus = listMinus.size();
        long long nbSizesTotal = nbGraphMinus/(max(toDo.second-toDo.first,1ll));
        string newName = "Gen: " + to_string(100-nbSizesLeft*100/nbSizesTotal)+"%";
        pthread_setname_np(pthread_self(), newName.c_str());
        for (long iG = toDo.first; iG < toDo.second; iG++)
        {
            const Graph &g = listMinus[iG];

            // Marking true the neighbourhood (code) that generates twins
            for (int iV = 0; iV < g.nbVert; iV++)
            {
                graphNeighbsToBool[g.adjMat[iV]+(long long)(1<<iV)] = !keepTwins;
            }

            nbGraph++;
            cptGraph++;

            gen_P2_list(g, pathLength2, nbVert);

            for (int code = 1; code < nbEdgeCombi; code++)
            {
                //TYDY hint to compiler will always be false almost
                if (graphNeighbsToBool[code])
                {
                    continue;
                }

                bool refuseBecauseC4 = detect_C4(pathLength2, code);
                if (refuseBecauseC4)
                    continue;

                gWithEdges.copy_and_add_new_vertex_noalloc(g, puissNewVert, code);

                if (free_O4(gWithEdges, nbVert))
                {
                    gWithEdges.compute_hashes(degreeList);
                    sort(degreeList.begin(), degreeList.begin()+gWithEdges.nbVert);
                    int codeLock = (unsigned char)degreeList[gWithEdges.nbVert]+256*(unsigned char) degreeList[gWithEdges.nbVert+1];
                    locksTests[codeLock].lock();
                    if (check_if_seen_and_add(gWithEdges, degreeList, deglists2GraphsToAdd[codeLock], idThread))
                    {
                        curNbTotalGraphsWritten++;
                        gWithEdges.print_in_string(strAllGenGraphs); //TODO also here separate printing from lock?
                    }
                    locksTests[codeLock].unlock();
                }

            }

            for (int iV = 0; iV < g.nbVert; iV++)
                graphNeighbsToBool[g.adjMat[iV]+(long long)(1<<iV)] = false;
        }
    }
    free(graphNeighbsToBool);

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
    lockToDo.lock();
    nbTotalGraphsWritten+=curNbTotalGraphsWritten;
    outFile << strAllGenGraphs.str();
    lockToDo.unlock();
    return {}; //TODO enlever ça en transformer en void
}

//Standard generation algorithm found on the internet.
void gen_subsets(int k, int n, vector<vector<int>> &listRes)
{
    vector<int> l(k);
    for (int i = 0; i < k; i++)
        l[i] = i+1;

    bool end = false;
    assert(k != 0 and k <= n);

    while (!end)
    {
        int i = k-1;
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


/*void gen_O3_list(const Graph &g, vector<int> &indepList, int nbVert)
{
    //g.pretty_print();
    indepList.clear();
    int maskAllVerts = (1 << nbVert) -1;
    //cout << "mask = " << maskAllVerts << endl;

    int puissV1 = 1, puissV2, puissV3;
    for (int v1 = 0; v1 < nbVert-3; v1++)
    {
        int maskLowerThanV1 = maskAllVerts ^((puissV1-1)^puissV1);
        int nonAdj1 = (maskAllVerts ^ g.adjMat[v1]);
        nonAdj1 &= maskLowerThanV1; //To avoid c1,c2 and c2,c1
        for (int v2 : adjListGlobal[nonAdj1])
        {
            //assert(v2 > v1);
            //assert(are_neighb(g,v1, v2) == 0);
            puissV2 = 1 << v2;
            int lowerThanV2 = puissV2-1;
            int maskLowerThanV2 = maskAllVerts ^((puissV2-1)^puissV2);
            int nonAdj2 = (maskAllVerts ^ g.adjMat[v2]);
            nonAdj2 &= maskLowerThanV2;
            for (int v3 : adjListGlobal[nonAdj1 & nonAdj2])
            {
                //cout << "\t" << v1 << "-" << v2 << "-" << v3 << endl;
                //assert(v3 > v2);
                //assert(are_neighb(g, v1, v3) == 0 && are_neighb(g, v2, v3) == 0);
                puissV3 = 1 << v3;
                indepList.push_back(puissV1^puissV2^puissV3);
            }
        }
        puissV1 *= 2;
    }

     * cout << "TADA: ";
    for (int x : indepList)
        cout << x << " ";
    cout << endl << endl;

}*/
void gen_O3_list(const Graph &g, vector<int> &indepList, int nbVert)
{
    //g.pretty_print();
    indepList.clear();
    //cout << "mask = " << maskAllVerts << endl;

    for (int v1 = 0; v1 < nbVert-3; v1++)
    {
        int puissV1 = 1<<v1;
        for (int v2  = v1+1; v2 < nbVert-2; v2++)
        {
                if (are_neighb(g,v1,v2))
                    continue;
            int puissV2 = 1<<v2;
            for (int v3 = v2+1; v3 < nbVert; v3++)
            {
                if (!are_neighb(g, v1,v3) && !are_neighb(g, v2, v3))
                    indepList.push_back((1<<v3) ^ puissV2 ^ puissV1);
            }
        }
    }
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

            // Checking that the possible P_2 is not a C_3: v1 and v2 are not neighbours
            if ((adj1 & puissV2) == 0 && (adj2 & puissV1) == 0) //TYDY second check useless
            {
                // commonAdj's i-th bit is 1 iff v1-i-v2 is an induced P_2
                int commonAdj = adj1 & adj2;
                if (commonAdj)
                {
                    // We encode u and v, and their common adjacency (the middle of the P_2)
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

        // Cchecking if the new neighbourhood code contains u,v and at least one
        // vertex not in the common adjacency (it would make a C_3).
        if (((code & uv) == uv) && ((code&commonAdj) != commonAdj))
            return true;
    }
    return false;
}

//Idem new_O4
bool detect_O4(const vector<int> &indepList, int code)
{
    for (int o2 : indepList)
    {
        if ((code & o2) == 0)
            return true;
    }
    return false;
}

