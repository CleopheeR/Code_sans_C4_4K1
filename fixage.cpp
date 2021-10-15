#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>
#include <algorithm>

#include "fixage.hh"
#include "Graph.hh"
#include "test-properties.hh"
#include "gen-graph.hh"


vector<int> bigDegreeList;

bool is_pre_or_fixeur(Graph &g, vector<int> &degreeList, bool prefixeurTest, sparse_hash_map<vector<int>, vector<Graph>> &prefixeurPlusDict, int **isTwinCompat)
{
    int nbVert = g.nbVert+1;
    const int puissNewVert = (1<<(nbVert-1));
    const int nbEdgeCombi = (1<<(nbVert-1));

    if (bigDegreeList.empty())
        bigDegreeList.resize(g.nbVert+2);
    static Graph gWithEdges;
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

        for (int i = 0; i < gWithEdges.nbVert; i++)
            bigDegreeList[i] = gWithEdges.get_neighb(i).size();
        sort(bigDegreeList.begin(), bigDegreeList.begin()+gWithEdges.nbVert);
        gWithEdges.compute_hashes(bigDegreeList);
        for (const Graph& gSeen : prefixeurPlusDict[bigDegreeList])
        {
            if (are_isomorphic(gWithEdges, gSeen))
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


sparse_hash_map<vector<int>, vector<Graph>> gen_fixeurs(int nbVert)
{
    sparse_hash_map<vector<int>, vector<Graph>> deglist2Fixeurs;

    sparse_hash_map<vector<int>, vector<Graph>> deglist2PrefixeursPlus;

    vector<int> degreeList(nbVert+1);
    vector<int> degreeListPlus(nbVert+2);
    int nbEdgeCombi = 1<<nbVert;

    stringstream fileName;
    fileName << "Alexgraphedelataille";
    fileName << nbVert << ".txt";
    vector<Graph> listGraphs = load_from_file(fileName.str());
    if (listGraphs.empty())
    {
        cerr << "Erreur : lancer avant la génération de la même taille \n";
        exit(3);
    }

    stringstream fileNamePlus;
    fileNamePlus << "Alexfixeursdelataille";
    fileNamePlus << nbVert+1 << ".txt";

    ifstream filePlus(fileNamePlus.str());
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
    /*
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
    }*/



    int cptGraph = 0;
    for (Graph& g : listGraphs)
    {
        cptGraph++;
        if (cptGraph%1000 == 0)
            cerr << "Nous sommes sur le " << cptGraph << "-ème graphe sur " << listGraphs.size() << endl;

        if (is_pre_or_fixeur(g, degreeList, true, deglist2PrefixeursPlus, isTwinCompat))
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
