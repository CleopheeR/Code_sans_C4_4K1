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


#include "Graph.hh"
#include "gen-graph.hh"
#include "test-properties.hh"

using namespace std;


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
bool check_if_seen_and_add(Graph& g, vector<int> &degreeList, map<vector<int>, vector<Graph>> &dico)
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
void save_to_file(const string &filename, const map<vector<int>, vector<Graph>> &graphList, int nbGraph)
{
    vector<Graph> res;
    ofstream file(filename);
    //int nbGraph = graphList.size();

    file << nbGraph << endl;

    for (const auto& inDict : graphList)
        for (const Graph &g : inDict.second)
            g.print_in_file(file);
}


vector<Graph> gen_graphs(int nbVert)
{
    int nbGraphPerComp[5] = {0,0,0,0,0};
    int nbFreeGraphPerComp[5] = {0,0,0,0,0};
    int nbGraphFree = 0;
    int nbPassedIso = 0;
    int nbGraphTried = 0;
    vector<Graph> res;
    vector<int> degreeList, curDegreeList;
    curDegreeList.resize(nbVert+1);
    degreeList.resize(nbVert+1);
    //unordered_map<vector<int>, vector<Graph>, vector_hash> deglist2Graphs;
    map<vector<int>, vector<Graph>> deglist2Graphs;

    if (nbVert == 1)
    {
        Graph g;
        g.init(1, 0);
        deglist2Graphs[{0}].push_back(g);
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

        vector<vector<int>> listSubsetsEdges; //TODO compléter
        for (int m = 1; m < nbVert; m++)
            gen_subsets(m, nbVert-1, listSubsetsEdges);

        /*
        cout << " DEBUT\n";
        for (auto &x : listSubsetsEdges)
        {
            for (auto& y : x)
            {
                cout << y << " ";
            }
            cout << endl;
        }
        cout << " FIN\n";
        */

        int cptGraph = 0;
        for (const Graph& g : listMinus)
        {
            degreeList.assign(nbVert+1, 0);
            for (int u = 0; u < nbVert-1; u++)
            {
                for (int v = u+1; v < nbVert-1; v++)
                {
                    if (are_neighb(g, u,v))
                    {
                        degreeList[u]++;
                        degreeList[v]++;
                    }
                }
            }
            sort(degreeList.begin(), degreeList.begin()+nbVert-1);
            */

            cptGraph++;
            if (cptGraph%100 == 0)
                cout << "Nous sommes sur le " << cptGraph << "-ème graphe sur " << listMinus.size() << endl;
            Graph gNew;
            gNew.copy_and_add_new_vertex(g, degreeList); //TODO garder le retour, un sommet ?


            int nbComp = 1;//+0*nb_connected_comp(gNew);
            //nbGraphPerComp[nbComp]++;

            nbGraphTried++;
            if (nbComp && free_C4_O4(gNew, nbVert))
            {
                //nbGraphFree++;
                //nbFreeGraphPerComp[nbComp]++;
                gNew.compute_hashes(degreeList);
                nbGraphFree++;
                nbFreeGraphPerComp[nbComp]++;
                if (check_if_seen_and_add(gNew, degreeList, deglist2Graphs))
                    nbPassedIso++;
            }
            //TODO ajouter à la liste et dans un dico


            for (const vector<int> &newEdgesList : listSubsetsEdges)
            {
                for (int i = 0; i < nbVert; i++)
                    curDegreeList[i] = degreeList[i];
                Graph gWithEdges = gNew;
                for (int newNeighb : newEdgesList)
                    gWithEdges.add_edge(nbVert-1, newNeighb-1, curDegreeList);



                /*
                 * nbComp = nb_connected_comp(gWithEdges);
                nbGraphPerComp[nbComp]++;*/
                nbGraphTried++;
                if (nbComp && free_C4_O4(gWithEdges, nbVert))
                {
                    nbGraphFree++;
                    nbFreeGraphPerComp[nbComp]++;
                    gWithEdges.compute_hashes(curDegreeList);
                    if (check_if_seen_and_add(gWithEdges, curDegreeList, deglist2Graphs))
                        nbPassedIso++;
                    //TODO refléchir : si déjà vu, alors il existe des combinaisons avec plus d'arêtes qui sont possibles ?!

                }

            }
        }
        /*
        for (const auto &inDict : deglist2Graphs)
            for (const Graph& g : inDict.second)
                res.push_back(g);
        */

    }

    int nbGraph = 0;
    for (const auto& inDict : deglist2Graphs)
        nbGraph += inDict.second.size();

    cout  << "Il y a " << nbGraph << " graphes à " << nbVert << " sommets.\n";
    /*
    for (auto & g : res)
    {
        cout << "hihi\n";
        g.print();
    }
    */
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

