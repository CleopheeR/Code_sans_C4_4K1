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

//TTAADDAA documenter ça ici ou ailleurs
vector<char> bigDegreeList[NBMAXPROC];
Graph isPreOrFixeurGWithEdges[NBMAXPROC];


sparse_hash_map<vector<char>, vector<Graph>> gen_fixeurs(int nbVert)
{
    //TTAADDAA documenter variables, plus parce que taille au dessus
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
    long long nbGToRead;
    fSize >> nbGToRead;
    vector<Graph> listGraphs = load_from_file(fileName.str(), nbGToRead);
    if (listGraphs.empty())
    {
        cerr << "Erreur : lancer avant la génération de la même taille \n";
        exit(3);
    }

    stringstream fileNamePlus;
    fileNamePlus << "Alexfixeursdelataille";
    fileNamePlus << nbVert+1 << ".txt.gz";

    read_prefixeurs_compute_hash(fileNamePlus.str(), nbVert+1 ,deglist2PrefixeursPlus);

    cout << "j'ai généré/trouvé les graphes à " << nbVert << " somets : il y en a " << listGraphs.size() << endl;

    int **isTwinCompat = NULL;

    isTwinCompat = (int**) malloc(sizeof(*isTwinCompat)*nbEdgeCombi);
    for (int i = 0; i < nbEdgeCombi; i++)
        isTwinCompat[i] = (int*) malloc(sizeof(*isTwinCompat)*NBMAXVERT);

    //TTAADDAA : code identique dans gen-graphs : faire une fonction ?
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

    //TTAADDAA : une fonction pour lancer puis join des fonctions avec liste d'arguments fixée ?
    long long nbPerProc = listGraphs.size()/nbProc;
    mutex threadMutex;
    vector<thread> threads(nbProc-1);
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc] = thread(&gen_fixeurs_thread, nbVert, std::cref(listGraphs), isTwinCompat, std::ref(fixeursList), std::cref(deglist2PrefixeursPlus), std::ref(threadMutex), iProc);

    thread lastProc = thread(&gen_fixeurs_thread, nbVert, std::cref(listGraphs), isTwinCompat, std::ref(fixeursList), std::cref(deglist2PrefixeursPlus), std::ref(threadMutex), nbProc-1);

    lastProc.join();
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc].join();

    cout  << "Il y a " << fixeursList.size() << " fixeurs à " << nbVert << " sommets.\n";
    stringstream fixeursFileNamee;
    fixeursFileNamee << "Alexfixeursdelataille";
    fixeursFileNamee << nbVert << ".txt.gz";

    ogzstream outFile(fixeursFileNamee.str().c_str());
    outFile << fixeursList.size() << endl;
    for (const Graph &g : fixeursList)
        g.print_in_file(outFile);
    outFile.close();

    return deglist2Fixeurs;

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


bool is_pre_or_fixeur(const Graph &g, bool prefixeurTest, const sparse_hash_map<vector<char>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat, int idThread)
{
    int nbVert = g.nbVert+1;
    const int puissNewVert = (1<<(nbVert-1));
    const int nbEdgeCombi = (1<<(nbVert-1));

    //TTAADDAA : documenter les variables, fonctionnement
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

    //TTEEDDEE : refaire benchs avec twins et voir
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
        if (!free_O4(gWithEdges, gWithEdges.nbVert))
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
        gWithEdges.compute_hashes(curBigDegreeList);

        const auto &itPrefixeursPlus1ToTest = prefixeurPlusDict.find(curBigDegreeList);
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


/** Internal functions **/

void gen_fixeurs_thread(int nbVert, const vector<Graph> &graphList, int** isTwinCompat, vector<Graph> &fixeursList, const sparse_hash_map<vector<char>, vector<Graph>> &deglist2PrefixeursPlus, mutex &mutInsert, int idThread)
{
    vector<char> degreeListPlus(nbVert+5);
    int nbEdgeCombi = 1<<nbVert;
    vector<Graph> ourFixeurs;


    long long cptGraph = 0;
    long long nbGToDo = graphList.size();

    for (long long iG = idThread; iG < nbGToDo; iG += nbProc)
    {
        //cerr << " coucou " << iG << " (" << idThread << endl;
        const Graph &g = graphList[iG];
        cptGraph++;
        if (cptGraph%10000 == 0)
            cerr << "Nous sommes sur le " << cptGraph << "-ème graphe sur " << nbGToDo/nbProc << "(" << idThread << ")" << endl; //iEnd-iBeg << "(" << iBeg << "," << iEnd << ")" << endl;

        if (is_pre_or_fixeur(g, true, deglist2PrefixeursPlus, isTwinCompat, idThread))
        {
            cerr << "YYYYYYYYYYYYYYYYEEEEEEEEESSSSSSSSSS\n";
            mutInsert.lock();
            fixeursList.push_back(g);
            mutInsert.unlock();
        }
    }
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
    bool isTwin[NBMAXVERT];
    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);

    if (g.nbVert == 0)
        exit(78);
    gen_P2_list(g, pathLength2, nbVert);



    for (int code = 0; code < nbEdgeCombi; code++)
    {
        const vector<int> &newEdgesList = adjListGlobal[code];

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

        for (int i = 0; i < gWithEdges.nbVert; i++)
            curBigDegreeList[i] = gWithEdges.get_neighb(i).size();
        sort(curBigDegreeList.begin(), curBigDegreeList.begin()+gWithEdges.nbVert);
        gWithEdges.compute_hashes(curBigDegreeList);

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


void save_to_file(const string &filename, const sparse_hash_map<vector<char>, vector<Graph>> &graphList, long long nbGraph)
{
    vector<Graph> res;
    ogzstream file(filename.c_str());

    file << nbGraph << endl;

    for (const auto& inDict : graphList)
        for (const Graph &g : inDict.second)
            g.print_in_file(file);
}
