#include <vector>
#include <string>

#include "Graph.hh"
#include "test-properties.hh"
#include "compute_arrays_compat.hh"


using namespace std;
const int nbObstruction = 7;
vector<Graph> obstructions(nbObstruction);


void getPossibleFreeNeighourhoods(int nbVert, const vector<int> &freeVerts, vector<Graph> &ret, Graph &curG, int pos)
{
    if (pos == freeVerts.size())
    {
        if (!free_C4_O4(curG, nbVert))
            return;
        for (int i = 0; i < nbObstruction; i++)
        {
            if (is_supergraph_of(curG, obstructions[i], 0)) //0 = idThread parameter
            {
                //cerr << "Identifying graph " << i << endl;
                return;
            }
        }
        ret.push_back(curG);
    }

    else
    {
        int newNeighb = freeVerts[pos];

        curG.add_edge(nbVert-1, newNeighb);
        getPossibleFreeNeighourhoods(nbVert, freeVerts, ret, curG, pos+1);

        curG.delete_edge(nbVert-1, newNeighb);
        getPossibleFreeNeighourhoods(nbVert, freeVerts, ret, curG, pos+1);
    }
}


void compute_cleophee_arrays(const Graph &g, const vector<vector<int>> &adjSets, const vector<vector<int>> &antiCompleteSets, const vector<string> &setsNames, const vector<int> &freeVerts)
{
    for (int x : freeVerts)
        cerr << x << " ";
    cerr << " <---- these are the free vertices\n";

    igzstream fObstructions("obstructions-c3.txt");
    for (int i = 0; i < nbObstruction; i++)
    {
        obstructions[i] = Graph(fObstructions);
        obstructions[i].print();
    }
    fObstructions.close();


    int n = g.nbVert;
    int nbSets = adjSets.size();
    vector<vector<Graph>> graphsSetsPerId(adjSets.size());

    for (int i = 0; i < nbSets; i++)
    {
        const vector<int> &curAdj = adjSets[i];

        Graph gNew;
        gNew.init(n+1, g.nbEdge);
        for (int u = 0; u < n; u++)
            gNew.adjMat[u] =  g.adjMat[u];

        for (int x : curAdj)
            gNew.add_edge(x, n);

        vector<int> realFreeVerts;
        for (int x : freeVerts)
        {
            if (find(antiCompleteSets[i].begin(), antiCompleteSets[i].end(), x) != antiCompleteSets[i].end())
                continue;

            if (find(curAdj.begin(), curAdj.end(), x) != curAdj.end())
                continue;

            realFreeVerts.push_back(x);
        }
        if (realFreeVerts.size() != freeVerts.size())
        {
            cerr << "\t\t" << setsNames[i] << " free verts are: ";
            for (int x : realFreeVerts)
                cerr << x << " ";
            cerr << endl;
        }


        getPossibleFreeNeighourhoods(n+1, realFreeVerts, graphsSetsPerId[i], gNew, 0);

        if (graphsSetsPerId[i].empty())
        {
            cerr << "set " << setsNames[i] << " cannot exist " << endl;
            /*
               if (!free_C4(gNew, n+1))
               cerr << "\t pas de C4 possible\n";
               if (!free_O4(gNew, n+1))
               cerr << "\t pas de O4 possible\n";
               */
        }
    }

    cout << "\t";
    for (const string& s : setsNames)
        cout << s << "\t";
    cout << endl;

    for (int i1 = 0; i1 < nbSets; i1++)
    {
        cout << setsNames[i1] << "\t";
        //for (int ii = 0; ii < i1+1; ii++)
        //    cout << "\t";
        const vector<int> &curAdj1 = adjSets[i1];

        for (int i2 = 0; i2 < nbSets; i2++)
        {
            const vector<int> &curAdj2 = adjSets[i2];
            bool isOkNoEdge = false;
            bool isOkWithEdge = false;


            for (const Graph &g1 : graphsSetsPerId[i1])
            {
                for (const Graph &g2 : graphsSetsPerId[i2])
                {
                    Graph g12;
                    g12.init(g1.nbVert+1, g1.nbEdge);
                    for (int u = 0; u < g1.nbVert; u++)
                        g12.adjMat[u] = g1.adjMat[u];

                    for (int x : adjListGlobal[g2.adjMat[g2.nbVert-1]])
                        g12.add_edge(g1.nbVert, x);

                    // No edge
                    if (free_C4_O4(g12, g12.nbVert))
                    {
                        bool containsObstruction = false;
                        for (int i = 0; i < nbObstruction; i++)
                        {
                            if (is_supergraph_of(g12, obstructions[i], 0)) //0 = idThread
                            {
                                cerr << endl;
                                cerr << "No edge: Identifying graph " << i << endl;
                                cerr << "\t => was " << setsNames[i1] << " VS " << setsNames[i2] << endl;
                                containsObstruction = true;
                            }
                        }
                        if (!containsObstruction)
                            isOkNoEdge = true;
                    }

                    // An edge
                    g12.add_edge(g12.nbVert-1, g12.nbVert-2);
                    if (free_C4_O4(g12, g12.nbVert))
                    {
                        bool containsObstruction = false;
                        for (int i = 0; i < nbObstruction; i++)
                        {
                            if (is_supergraph_of(g12, obstructions[i], 0)) //0 = idThread
                            {
                                cerr << endl;
                                cerr << "Edge: Identifying graph " << i << endl;
                                cerr << "\t => was " << setsNames[i1] << " VS " << setsNames[i2] << endl;

                                containsObstruction = true;
                            }
                        }
                        if (!containsObstruction)
                            isOkWithEdge = true;

                    }
                }
            }

            if (!isOkNoEdge && !isOkWithEdge)
                cout << "-1\t";
            //cerr << setsNames[i1] << " and " << setsNames[i2] << " cannot bot exist" << endl;
            else if (!isOkNoEdge)
                cout << "1\t";
            //cerr << setsNames[i1] << " and " << setsNames[i2] << ": 1" << endl;

            else if (!isOkWithEdge)
                cout << "0\t";
            //cerr << setsNames[i1] << " and " << setsNames[i2] << ": 0" << endl;
            else
                cout << "N\t";
            //cerr << setsNames[i1] << " and " << setsNames[i2] << ": can say nothing :(" << endl;
        }
        cout << endl;
    }
}

/*
   void compute_N_melted_graph(const Graph &g, int **tabCompat)
   {
   int n = g.nbVert;
   vector<int> oldVertId2NewId(n, -1);
   int lastId = 0;

   for (int x : listNVerts)
   {
   oldVertId2NewId[x] = lastId;
   lastId++;
   }

   Graph gOnlyNVerts;
   gOnlyNVerts.init(listNVerts.size(), 0);
   for (int x : listNVerts)
   {
   int newId = oldVertId2NewId[x];
   const vector<int> neighbs = g.get_neighb(x);

   for (int y : neighbs)
   {
   int neighbNewId = oldVertId2NewId[y];
   if (neighbNewId != -1)
   gOnlyNVerts.add_edge(newId, neighbNewId);
   }




   }






   }

*/
