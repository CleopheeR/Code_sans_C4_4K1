#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
//#include <map>

#include "Graph.hh"
#include "gen-graph.hh"
#include "fixage.hh"
#include "test-properties.hh"
#include "compare_with_cleophee.hh"
//#include <cstdlib>

using namespace std;

int main(int argc, char* argv[])
{
    int nbVert = atoi(argv[1]);
    char testOrGen = argv[2][0];
    int nbBlowup = 0;

    if (testOrGen == 'G')
    {
        init_adjListGlobal(nbVert);
        vector<Graph> graphList;
        graphList = gen_graphs(nbVert);


        /*
        stringstream fileName;
        fileName << "graphedelataille";
        fileName << nbVert << ".txt";

        vector<Graph> graphListCleophee = read_from_cleophee(fileName.str());
        cout << "Cléophée a généré " << graphListCleophee.size() << " graphes\n";

        map<vector<int>, vector<Graph>> deglist2Graphs;
        for (const Graph &g : graphListCleophee)
        {
            if (!free_O4(g, nbVert))
            {
                cout << "Horreur, un O4 !\n";
                g.print();
            }
            if (!free_C4(g, nbVert))
            {
                cout << "Sapristi, un C4 !\n";
                g.print();
            }
            if (!check_if_seen_and_add(g, deglist2Graphs))
                cout << "Oh no, two are isomorphic\n";
        }

        for (const Graph & gg : graphList)
        {
            if (check_if_seen_and_add(gg, deglist2Graphs))
            {
                cout << "Ohlalalala, mon graphe superflu !\n";
                gg.print();
            }
        }
        */

    }


    else if (testOrGen == 'F')
    {
        init_adjListGlobal(nbVert+1);
         gen_fixeurs(nbVert);




    }

    else if (testOrGen == 'S') //statistics
    {
        stringstream fileName;
        fileName << "Alexgraphedelataille";
        fileName << nbVert << ".txt";
        vector<Graph> list = load_from_file(fileName.str());

        vector<int> cpt((nbVert*nbVert)/2+1, 0);
        for (const Graph &g : list)
            cpt[g.nbEdge]++;

        for (int i = 0; i < cpt.size(); i++)
            if (cpt[i] != 0)
                cout << i << ": " << cpt[i] << endl;


    }

    free_adjListGlobal();


    return 0;
}
