#include <sstream>
#include <fstream>
#include <cassert>
#include <algorithm>

#include "fixage.hh"
#include "Graph.hh"
#include "test-properties.hh"
#include "gen-graph.hh"


/*
bool is_fixeur(const Graph &g, const vector<vector<int>> &listSubsetsEdges)
{
    for (const vector<int> &newEdgesList : listSubsetsEdges)
    {
        if (listSubsetsEdges.size() == g.nbVert)
            continue;
        bool isUniversal = false;
        for (int u : newEdgesList)
            if (g.adjList[u-1].size() == g.nbVert-1)
                isUniversal = true;
        if (isUniversal)
            continue;

        Graph gWithEdges;
        gWithEdges.copy_and_add_new_vertex(g); //TODO garder le retour, un sommet ?

        for (int newNeighb : newEdgesList)
            gWithEdges.add_edge(gWithEdges.nbVert-1, newNeighb-1);

        if (!free_C4_O4(gWithEdges, gWithEdges.nbVert-1))
            continue;

        if (has_twin(gWithEdges, gWithEdges.nbVert-1))
            continue;

        return false;
    }

    return true;
}
*/





bool is_pre_or_fixeur(Graph &g, vector<int> &degreeList, const vector<vector<int>> &listSubsetsEdges, bool prefixeurTest, map<vector<int>, vector<Graph>> &fixeurDict, map<vector<int>, vector<Graph>> &prefixeurDict)
{
    bool printDebug = false;
    for (const vector<int> &newEdgesList : listSubsetsEdges)
    {
        if (newEdgesList.size() == g.nbVert)
            continue;

        //for (int i = 0; i < g.nbVert; i++)
        //    curDegreeList[i] = degreeList[i];
        Graph gWithEdges;
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
        for (const Graph& gSeen : prefixeurDict[degreeList])
        {
            if (are_isomorphic(gWithEdges, gSeen))
            {
                isGBigPreFixeur = true;
                break;
            }
        }

        if (isGBigPreFixeur)
            continue;

        bool isGBigFixeur = false;
        for (const Graph& gSeen : fixeurDict[degreeList])
        {
            if (are_isomorphic(gWithEdges, gSeen))
            {
                isGBigFixeur = true;
                break;
            }
        }
        if (isGBigFixeur)
            continue;

        return false;
    }

    return true;
}


map<vector<int>, vector<Graph>> gen_fixeurs(int nbVert)
{
    map<vector<int>, vector<Graph>> deglist2Fixeurs;

    map<vector<int>, vector<Graph>> fixeursPlus, prefixeursPlus;

    vector<int> degreeList(nbVert+1);

    stringstream fileName;
    fileName << "Alexgraphedelataille";
    fileName << nbVert << ".txt";
    vector<Graph> listGraphs = load_from_file(fileName.str());
    if (listGraphs.empty())
    {
        cerr << "Erreur : lancer avant la génération de la même taille \n";
        exit(3);
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

        if (is_pre_or_fixeur(g, degreeList, listSubsetsEdges, false, fixeursPlus, prefixeursPlus))
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
