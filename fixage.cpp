#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>

#include "gzstream/gzstream.h"
#include "fixage.hh"
#include "Graph.hh"
#include "test-properties.hh"
#include "gen-graph.hh"



vector<char> bigDegreeList[NBMAXPROC];
Graph isPreOrFixeurGWithEdges[NBMAXPROC];

bool is_pre_or_fixeur(const Graph &g, bool prefixeurTest, const sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat, int idThread)
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

    vector<long long> twinLists2;
    twinLists2.reserve(NBMAXVERT*NBMAXVERT);
    bool isTwin[NBMAXVERT];
    bool isInList[NBMAXVERT];
    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);



    //twinLists2.clear();
    //gen_twin_list(g, twinLists2, nbVert);

    if (g.nbVert == 0)
        exit(78);
    gen_P2_list(g, pathLength2, nbVert);




    for (int code = 0; code < nbEdgeCombi; code++)
    {
        const vector<int> &newEdgesList = adjListGlobal[code];
        if (newEdgesList.size() == g.nbVert)
            continue;

        /*bool refuseBecauseTwins = can_discard_edgelist(twinLists2, isTwinCompat[code], nbVert);
        if (refuseBecauseTwins)
        {
            //cerr << "lol YEAH\n";
            continue;
        }*/


        bool hasC4 = detect_C4(pathLength2, code);
        if (hasC4)
            continue;
        gWithEdges.copy_and_add_new_vertex_bis(g, newEdgesList, puissNewVert, code);

        if (printDebug)
        {
            gWithEdges.print();
            cout << "-----------------";
        }

        if (has_twin(gWithEdges, gWithEdges.nbVert-1))
        {
            if (printDebug)
                cerr << "cond2\n";
            continue;
        }
        if (/*!free_C4(gWithEdges, gWithEdges.nbVert) ||*/ !free_O4(gWithEdges, gWithEdges.nbVert))
        {
            if (printDebug)
                cerr << "cond1\n";
            continue;
        }


        if (!prefixeurTest)
            return false;

        bool isGBigPreFixeur = false;
        vector<char> &curBigDegreeList = bigDegreeList[idThread];

        for (int i = 0; i < gWithEdges.nbVert; i++)
            curBigDegreeList[i] = gWithEdges.get_neighb(i).size();
        sort(curBigDegreeList.begin(), curBigDegreeList.begin()+gWithEdges.nbVert);
        //gWithEdges.compute_hashes(bigDegreeList);
        const auto &itPrefixeursPlus1ToTest = prefixeurPlusDict.find(curBigDegreeList);
        //for (const Graph& gSeen : prefixeurPlusDict[bigDegreeList])
        if (itPrefixeursPlus1ToTest == prefixeurPlusDict.cend())
            return false;
        const vector<Graph> &prefixeursPlus1ToTest = itPrefixeursPlus1ToTest->second;
        for (const Graph& gSeen : prefixeursPlus1ToTest)
        {
            if (are_isomorphic(gWithEdges, gSeen, idThread))
            {
                isGBigPreFixeur = true;
                break;
            }
        }

        if (isGBigPreFixeur)
            continue;

        return false;
    }

    return true;
}


sparse_hash_map<vector<char>, vector<Graph>> gen_fixeurs(int nbVert)
{
     sparse_hash_map<vector<char>, vector<Graph>> deglist2Fixeurs;
    sparse_hash_map<vector<char>, vector<Graph>> deglist2PrefixeursPlus;
    vector<char> degreeList(nbVert+4);
    vector<char> degreeListPlus(nbVert+5);


    int nbEdgeCombi = 1<<nbVert;

    stringstream fileName, fileSizeName;
    fileName << "Alexgraphedelataille" << nbVert << ".txt.gz";
    fileSizeName << "Alexsizegraphedelataille" << nbVert << ".txt";
    ifstream fSize(fileSizeName.str());
    if (fSize.peek() == EOF)
    {
        cerr << "Lancer avant la taille -1 size \n";
        cerr << fileSizeName.str() << endl;
        exit(3);
    }
    int nbGToRead;
    fSize >> nbGToRead;
    vector<Graph> listGraphs = load_from_file(fileName.str(), nbGToRead);
    if (listGraphs.empty())
    {
        cerr << "Erreur : lancer avant la génération de la même taille \n";
        exit(3);
    }

    stringstream fileNamePlus;
    fileNamePlus << "Alexfixeursdelataille";
    fileNamePlus << nbVert+1 << ".txt";

    igzstream filePlus(fileNamePlus.str().c_str());
    int nbPlus;
    if (filePlus.peek() != EOF)
    {
        Graph gLu;
        filePlus >> nbPlus;

        cerr << "ohlalal : " << nbPlus << endl;
        for (int i = 0; i < nbPlus; i++)
        {
            gLu = Graph(filePlus);
            for (int u = 0; u < gLu.nbVert; u++)
                degreeListPlus[u] = gLu.get_neighb(u).size();
            sort(degreeListPlus.begin(), degreeListPlus.begin()+gLu.nbVert);
            gLu.compute_hashes(degreeListPlus);
            deglist2PrefixeursPlus[degreeListPlus].push_back(gLu);
        }
    }



    cout << "j'ai généré/trouvé les graphes à " << nbVert << " somets : il y en a " << listGraphs.size() << endl;


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



    // Nouvelle version avec la parallélisation :-)
    vector<Graph> fixeursList;
    fixeursList.reserve(1000000);

    int nbPerProc = listGraphs.size()/nbProc;
    mutex threadMutex;
    vector<thread> threads(nbProc-1);
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc] = thread(&gen_fixeurs_thread, nbVert, std::cref(listGraphs), isTwinCompat, std::ref(fixeursList), std::cref(deglist2PrefixeursPlus), iProc*nbPerProc, (iProc+1)*nbPerProc, std::ref(threadMutex), iProc);

    thread lastProc = thread(&gen_fixeurs_thread, nbVert, std::cref(listGraphs), isTwinCompat, std::ref(fixeursList), std::cref(deglist2PrefixeursPlus), (nbProc-1)*nbPerProc, listGraphs.size(), std::ref(threadMutex), nbProc-1);

    lastProc.join();
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc].join();

    cout  << "Il y a " << fixeursList.size() << " fixeurs à " << nbVert << " sommets.\n";
    stringstream fixeursFileNamee;
    fixeursFileNamee << "Alexfixeursdelataille";
    fixeursFileNamee << nbVert << ".txt";

    //save_to_file(fixeursFileName.str(), deglist2Fixeurs, nbGraph);
    //TODO maybe refaire fonction ?
    ogzstream outFile(fixeursFileNamee.str().c_str());
    outFile << fixeursList.size() << endl;
    for (const Graph &g : fixeursList)
        g.print_in_file(outFile);
    outFile.close();




    return deglist2Fixeurs;



    int cptGraph = 0;
    for (Graph& g : listGraphs)
    {
        cptGraph++;
        if (cptGraph%1000 == 0)
            cerr << "Nous sommes sur le " << cptGraph << "-ème graphe sur " << listGraphs.size() << endl;

        if (is_pre_or_fixeur(g, true, deglist2PrefixeursPlus, isTwinCompat, 0))
        {
            for (int i = 0; i < g.nbVert; i++)
                degreeList[i] = g.get_neighb(i).size();
            g.compute_hashes(degreeList);
            cerr << "YYYYYYYYYYYYYYYYEEEEEEEEESSSSSSSSSS\n";
            deglist2Fixeurs[degreeList].push_back(g);
        }
    }

    int nbGraph = 0;
    for (const auto& inDict : deglist2Fixeurs)
        nbGraph += inDict.second.size();

    cout  << "Il y a " << nbGraph << " fixeurs à " << nbVert << " sommets.\n";
    stringstream fixeursFileName;
    fixeursFileName << "Alexfixeursdelataille";
    fixeursFileName << nbVert << ".txt";

    save_to_file(fixeursFileName.str(), deglist2Fixeurs, nbGraph);

    return deglist2Fixeurs;
}


void gen_fixeurs_thread(int nbVert, const vector<Graph> &graphList, int** isTwinCompat, vector<Graph> &fixeursList, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, int iBeg, int iEnd, mutex &mutInsert, int idThread)
{
    vector<char> degreeListPlus(nbVert+5);
    int nbEdgeCombi = 1<<nbVert;


    int cptGraph = 0;

    for (int iG = iBeg; iG < iEnd; iG++)
    {
        //cerr << " coucou " << iG << " (" << idThread << endl;
        const Graph &g = graphList[iG];
        cptGraph++;
        if (cptGraph%10000 == 0)
            cerr << "Nous sommes sur le " << cptGraph << "-ème graphe sur " << iEnd-iBeg << "(" << iBeg << "," << iEnd << ")" << endl;

        if (is_pre_or_fixeur(g, true, deglist2PrefixeursPlus, isTwinCompat, idThread))
        {
            cerr << "YYYYYYYYYYYYYYYYEEEEEEEEESSSSSSSSSS\n";
            mutInsert.lock();
            fixeursList.push_back(g);
            mutInsert.unlock();
        }
    }
}

