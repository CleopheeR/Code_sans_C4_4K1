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

bool is_pre_or_fixeur(Graph &g, vector<int> &degreeList, const vector<vector<int>> &listSubsetsEdges, bool prefixeurTest, sparse_hash_map<vector<int>, vector<Graph>> &fixeurDict, sparse_hash_map<vector<int>, vector<Graph>> &prefixeurPlusDict)
{
    if (bigDegreeList.empty())
        bigDegreeList.resize(g.nbVert+2);
    static Graph gWithEdges;
    if (gWithEdges.adjMat == NULL)
        gWithEdges.init(g.nbVert+1, -1);
    bool printDebug = false;
    for (const vector<int> &newEdgesList : listSubsetsEdges)
    {
        if (newEdgesList.size() == g.nbVert)
            continue;

        //for (int i = 0; i < g.nbVert; i++)
        //    curDegreeList[i] = degreeList[i];
        gWithEdges.copy_and_add_new_vertex(g); //TODO garder le retour, un sommet ?
        for (int u : newEdgesList)
            gWithEdges.add_edge(gWithEdges.nbVert-1, u-1);
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
        if (!free_C4(gWithEdges, gWithEdges.nbVert) || !free_O4(gWithEdges, gWithEdges.nbVert))
        {
            if (printDebug)
                cerr << "cond1\n";
            continue;
        }


        if (!prefixeurTest)
            return false;

        bool isGBigPreFixeur = false;

        //TODO ici calculer degreelist et hash

        //TODO compute hash!
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

        /*
        bool isGBigFixeur = false;
        for (const Graph& gSeen : prefixeurPlusDict[degreeList])
        {
            if (are_isomorphic(gWithEdges, gSeen))
            {
                isGBigFixeur = true;
                break;
            }
        }
        if (isGBigFixeur)
            continue;

        */
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

    vector<vector<int>> listSubsetsEdges; //TODO compléter
    listSubsetsEdges.push_back({}); // Liste vide
    for (int m = 1; m <= nbVert; m++)
        gen_subsets(m, nbVert, listSubsetsEdges);


    int cptGraph = 0;
    for (Graph& g : listGraphs)
    {
        //degreeList.assign(g.nbVert, 0);
        /*
           for (int u = 0; u < g.nbVert; u++)
           {
           for (int v = u+1; v < g.nbVert; v++)
           {
           if (are_neighb(g, u,v))
           {
           degreeList[u]++;
           degreeList[v]++;
           }
           }

           }
           sort(degreeList.begin(), degreeList.begin()+g.nbVert);
           */


        cptGraph++;
        if (cptGraph%1000 == 0)
            cerr << "Nous sommes sur le " << cptGraph << "-ème graphe sur " << listGraphs.size() << endl;

        if (is_pre_or_fixeur(g, degreeList, listSubsetsEdges, true, deglist2PrefixeursPlus, deglist2PrefixeursPlus))
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
    //TODO change

    return deglist2Fixeurs;
}
