#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>

#include "sparsepp/spp.h"
#include "Graph.hh"
#include "gen-graph.hh"
#include "fixage.hh"
#include "test-properties.hh"
#include "compare_with_cleophee.hh"
#include "misc.hh"

using namespace std;

int nbProc = 2;

int main(int argc, char* argv[])
{
    int nbVert = atoi(argv[1]);
    char testOrGen = argv[2][0];
    if (argc > 3)
        nbProc = atoi(argv[3]);

    init_adjListGlobal(nbVert+1);
    if (testOrGen == 'G')
    {
        vector<Graph> graphList;
        vector<Graph> dummy;
        graphList = gen_graphs(nbVert, dummy);

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
        gen_fixeurs(nbVert);
    }

    else if (testOrGen == 'M') // Find minimal prefixers
    {
        stringstream fileNameMinus;
        fileNameMinus << "Alexfixeursdelataille";
        fileNameMinus << nbVert-1 << ".txt.gz";
        cerr << fileNameMinus.str() << endl;

        vector<Graph> listGraphsMinus = load_from_file(fileNameMinus.str());


        stringstream fileName;
        fileName << "Alexfixeursdelataille";
        fileName << nbVert << ".txt.gz";

        sparse_hash_map<vector<char>, vector<Graph>> prefixeurs;
        read_prefixeurs_compute_hash(fileName.str(), nbVert, prefixeurs);

        cerr << listGraphsMinus.size() << " fixeurs minus seen\n";


        get_minimal_fixeurs(listGraphsMinus, prefixeurs);

        for (const auto &pairDegG : prefixeurs)
        {
            for (const Graph &g : pairDegG.second)
                g.print();
        }

    }

    else if (testOrGen == 'X') //Gen graphs including some specific graphs.
    {
        cerr << "coucou " << endl;
        string graphsFname("startingGraphs.txt");
        igzstream graphsFile(graphsFname.c_str());

        int nbStartingGraphs;
        graphsFile >> nbStartingGraphs;
        map<int, vector<Graph>> startingGraphsBySize;
        int minDegStarting = 128;
        string toto;
        getline(graphsFile, toto);
        cerr << " vu " << nbStartingGraphs << " graphes de départ " << endl;

        for (int i = 0; i < nbStartingGraphs; i++)
        {
            Graph gLu = Graph(graphsFile);
            gLu.print();
            startingGraphsBySize[gLu.nbVert].push_back(gLu);
            minDegStarting = min(gLu.nbVert, minDegStarting);
        }
        graphsFile.close();

        string fNameFirst = "Alexgraphedelataille"+to_string(minDegStarting)+".txt.gz";
        string fNameSizeFirst = "Alexsizegraphedelataille"+to_string(minDegStarting)+".txt";

        ifstream fTest(fNameSizeFirst);
        if (!fTest.fail())
        {
            cerr << "Attention, il y a des fichiers de graphes déjà générés :(\n";
            exit(7);
        }
        fTest.close();

        ofstream fSize(fNameSizeFirst);
        fSize << startingGraphsBySize[minDegStarting].size() << "\n";
        fSize.close();

        ogzstream fGraphs(fNameFirst.c_str());
        for (const Graph &g : startingGraphsBySize[minDegStarting])
            g.print_in_file(fGraphs);
        fGraphs.close();

        for (int i = minDegStarting+1; i <= nbVert; i++)
        {
            string fNameSize = "Alexsizegraphedelataille"+to_string(i)+".txt";

            ifstream fTest2(fNameSize);
            if (!fTest2.fail())
            {
                cerr << "Attention, il y a des fichiers de graphes déjà générés pour la taille " << i << ":(\n";
                exit(7);
            }
            fTest2.close();


            vector<Graph> graphList;
            graphList = gen_graphs(i, startingGraphsBySize[i]); //TODO récupérer les graphes vraiment
        }



    }

    else if (testOrGen == 'S') //statistics
    {

        stringstream fileName;
        fileName << "Alexgraphedelataille";
        fileName << nbVert << ".txt.gz";
        cerr << fileName.str() << endl;
        vector<Graph> list = load_from_file(fileName.str());

        vector<int> cpt((nbVert*nbVert)/2+1, 0);

        igzstream file(fileName.str().c_str());
        stringstream fileSizeMinusName;
        fileSizeMinusName << "Alexsizegraphedelataille" << nbVert << ".txt";
        ifstream fSize(fileSizeMinusName.str());
        if (fSize.peek() == EOF)
        {
            cerr << "Lancer avant la taille -1 size \n";
            cerr << fileSizeMinusName.str() << endl;
            exit(3);
        }
        int nbGMinus = -1;
        fSize >> nbGMinus;

        string toto;

        for (int i = 0; i < nbGMinus; i++)
        {
            if (i%500000 == 0)
                cerr << "on est à " << i << " sur " << nbGMinus << endl;
            getline(file, toto);
            stringstream totoSs(toto);
            int n,m;
            totoSs >> n >> m;
            cpt[m]++;
        }

        for (int i = 0; i < cpt.size(); i++)
            if (cpt[i] != 0)
                cout << i << ": " << cpt[i] << endl;
    }

    //TTAADDAA WHAT?! real compare please!
    else if (testOrGen == 'C') //Compare fixeurs files
    {
        cerr << nbVert << " zut \n";
        cerr << "file = " << argv[3] << endl;
        vector<Graph> listGraphs1 = load_from_file(argv[3]);
        vector<Graph> listGraphs2 = load_from_file(argv[4]);

        sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs1;
        sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs2;

        vector<char> degreeList(nbVert+5);

        for (Graph& g : listGraphs1)
        {
            g.compute_hashes(degreeList);
            if (!check_if_seen_and_add(g, degreeList, deglist2Graphs1))
                cerr << "ERROR" << endl;
        }
        cerr << listGraphs1.size() << " mdr1 " << endl;

        for (Graph& g : listGraphs2)
        {
            g.compute_hashes(degreeList);
            if (!check_if_seen_and_add(g, degreeList, deglist2Graphs2))
                cerr << "ERROR" << endl;
        }
        cerr << listGraphs2.size() << " mdr2 " << endl;

        cout << "This list = " << argv[3] << " and other list = " << argv[4] << endl;
        compare_two_fixeurs_sets(deglist2Graphs1, deglist2Graphs2);
        cout << "\n------------------------\n\n";
        cout << "This list = " << argv[4] << " and other list = " << argv[3] << endl;
        compare_two_fixeurs_sets(deglist2Graphs2, deglist2Graphs1);

    }

    free_adjListGlobal();

    return 0;
}
