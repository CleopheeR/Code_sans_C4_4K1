#include <vector>
#include <string>

#include "Graph.hh"
#include "test-properties.hh"
#include "compute_arrays_compat.hh"


using namespace std;

void compute_cleophee_arrays(const Graph &g, const vector<vector<int>> &adjSets, const vector<string> &setsNames)
{
    int n = g.nbVert;
    int nbSets = adjSets.size();
    vector<bool> isAdjCompat(nbSets, true);

    for (int i = 0; i < nbSets; i++)
    {
        const vector<int> &curAdj = adjSets[i];

        Graph gNew;
        gNew.init(n+1, g.nbEdge);
        for (int u = 0; u < n; u++)
            gNew.adjMat[u] =  g.adjMat[u];

        for (int x : curAdj)
            gNew.add_edge(x, n);

        if (!free_C4_O4(gNew, n+1))
        {
            cerr << "set " << setsNames[i] << " cannot exist " << endl;
            if (!free_C4(gNew, n+1))
                cerr << "\t pas de C4 possible\n";
            if (!free_O4(gNew, n+1))
                cerr << "\t pas de O4 possible\n";
            isAdjCompat[i] = false;
        }
    }

    cout << "\t";
    for (const string& s : setsNames)
        cout << s << "\t";
    cout << endl;

    for (int i1 = 0; i1 < nbSets; i1++)
    {
        cout << setsNames[i1] << "\t";
        for (int ii = 0; ii < i1+1; ii++)
            cout << "\t";
        const vector<int> &curAdj1 = adjSets[i1];

        Graph g1Plus;
        g1Plus.init(n+2, g.nbEdge);
        for (int u = 0; u < n; u++)
            g1Plus.adjMat[u] =  g.adjMat[u];

        for (int x : curAdj1)
            g1Plus.add_edge(x, n);



        for (int i2 = i1+1; i2 < nbSets; i2++)
        {
            const vector<int> &curAdj2 = adjSets[i2];

            Graph g12 = g1Plus;
            for (int x : curAdj2)
                g12.add_edge(x, n+1);

            bool isOkNoEdge = free_C4_O4(g12, n+2);
            g12.add_edge(n, n+1);
            bool isOkWithEdge = free_C4_O4(g12, n+2);

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
