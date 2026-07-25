#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>
#include <set>

#include "sparsepp/spp.h"
#include "Graph.hh"
#include "gen-graph.hh"
#include "fixage.hh"
#include "test-properties.hh"
#include "compare_with_cleophee.hh"
#include "misc.hh"
#include "compute_arrays_compat.hh"
#include "problemArray.hh"

using namespace std;

int nbProc = 2;

int main(int argc, char* argv[])
{
    int nbVert = atoi(argv[1]);
    char testOrGen = argv[2][0];
    if (argc > 3)
        nbProc = atoi(argv[3]);

    // Initialising the big adjListGlobal array : t[x] contains the sets of ids of bits equals to 1 in x.
    init_adjListGlobal(max(14,nbVert+5));

    // Generates graphs, standard way (attention: problem if beginning from 1 and no twins).
    if (testOrGen == 'G')
    {
        vector<Graph> dummy;
        gen_graphs(nbVert, dummy);
    }


    // Generates graphs, but only the ones which are supergraphs of the graphs in the file
    // startingGraphs.txt. Generates all graphs up to sive nbVert.
    else if (testOrGen == 'X') //Gen graphs including some specific graphs.
    {
        string graphsFname("startingGraphs.txt");
        igzstream graphsFile(graphsFname.c_str());
        int nbStartingGraphs;
        graphsFile >> nbStartingGraphs;

        map<int, vector<Graph>> startingGraphsBySize;
        int minDegStarting = 128;
        string toto;
        getline(graphsFile, toto);
        cerr << " seen " << nbStartingGraphs << " starting graphs." << endl;

        for (int i = 0; i < nbStartingGraphs; i++)
        {
            Graph gRead = Graph(graphsFile);
            startingGraphsBySize[gRead.nbVert].push_back(gRead);
            minDegStarting = min(gRead.nbVert, minDegStarting);
        }
        graphsFile.close();

        string fNameFirst = "Alexgraphedelataille"+to_string(minDegStarting)+".txt.gz";
        string fNameSizeFirst = "Alexsizegraphedelataille"+to_string(minDegStarting)+".txt";

        ifstream fTest(fNameSizeFirst);
        if (!fTest.fail())
        {
            cerr << "Error: the file " << fNameSizeFirst << " already exists." << endl;
            exit(7);
        }
        fTest.close();

        // Writes the smallest graphs to include.
        ofstream fSize(fNameSizeFirst);
        fSize << startingGraphsBySize[minDegStarting].size() << "\n";
        fSize.close();

        ogzstream fGraphs(fNameFirst.c_str());
        for (const Graph &g : startingGraphsBySize[minDegStarting])
            g.print_in_file(fGraphs);
        fGraphs.close();

        // Generates the bigger graphs.
        for (int i = minDegStarting+1; i <= nbVert; i++)
        {
            string fNameSize = "Alexsizegraphedelataille"+to_string(i)+".txt";

            ifstream fTest2(fNameSize);
            if (!fTest2.fail())
            {
                cerr << "Error: the file " << fNameSizeFirst << " already exists." << endl;
                exit(7);
            }
            fTest2.close();


            gen_graphs(i, startingGraphsBySize[i]);
        }
    }

    else if (testOrGen == 'Z') // Checks the graphs and writes down the one which are magic.
    {
        gen_magic_graphs(nbVert);
    }


    // Reads the magic graphs of size nbVert, and the ones with one fewer vertex. Lists the graphs
    // of size nbVert which do not contain smaller magic graphs, i.e. computes the list of the
    // minimal magic graphs.
    else if (testOrGen == 'M')
    {
        stringstream fileNameMinus;
        fileNameMinus << "Alexmagicdelataille";
        fileNameMinus << nbVert-1 << ".txt.gz";
        cerr << "Read magic graphs of size " << nbVert-1 << " in file " << fileNameMinus.str() << endl;
        vector<Graph> listGraphsMinus = load_from_file(fileNameMinus.str());

        stringstream fileName;
        fileName << "Alexmagicdelataille";
        fileName << nbVert << ".txt.gz";

        sparse_hash_map<vector<char>, vector<Graph>> magicGraphs;
        read_magic_graphs_compute_hash(fileName.str(), nbVert, magicGraphs);

        cerr << listGraphsMinus.size() << " magical graphs minus seen\n";

        get_minimal_fixeurs(listGraphsMinus, magicGraphs);
        int nbMinimal = 0;

        stringstream fileNameMinimal;
        fileNameMinimal << "Alexminimalmagiquedelataille";
        fileNameMinimal << nbVert << ".txt.gz";
        ogzstream fileMinimal(fileNameMinimal.str().c_str());

        for (const auto &pairDegG : magicGraphs)
        {
            for (const Graph &g : pairDegG.second)
            {
                nbMinimal++;
                g.pretty_print();
                g.print_in_file(fileMinimal);
            }
        }
        cerr << nbMinimal << endl;
        fileMinimal.close();
    }

    // Printing the number of graphs with x edges, for every value of x.
    else if (testOrGen == 'S') // Statistics
    {

        stringstream fileName;
        fileName << "Alexgraphedelataille";
        fileName << nbVert << ".txt.gz";
        cerr << fileName.str() << endl;
        vector<Graph> list = load_from_file(fileName.str());

        vector<int> cpt((nbVert*nbVert)/2+1, 0);

        igzstream file(fileName.str().c_str());
        stringstream fileSizeName;
        fileSizeName << "Alexsizegraphedelataille" << nbVert << ".txt";

        ifstream fSize(fileSizeName.str());
        if (fSize.peek() == EOF)
        {
            cerr << "You need to run the generation of graphs of size " << nbVert << " before." << endl;
            exit(3);
        }
        int nbG = -1;
        fSize >> nbG;

        for (const Graph &g : list)
        {
            // Only checks twins involving the last vertex.
            for (int u = 0; u < g.nbVert; u++)
                assert(!has_twin(g, u));
        }

        string toto;
        for (int i = 0; i < nbG; i++)
        {
            if (i%500000 == 0)
                cerr << "examining graph n°" << i << " out of " << nbG << endl;
            getline(file, toto);
            stringstream totoSs(toto);
            int n,m;
            totoSs >> n >> m;
            cpt[m]++;
        }

        for (int i = 0; i < cpt.size(); i++)
            if (cpt[i] != 0)
                cout << "m = " << i << ": " << cpt[i] << endl;
    }

    // Compares two list of graphs of same size.
    else if (testOrGen == 'C')
    {
        cerr << "Comparing for size " << nbVert << "." << endl;
        string fname1(argv[3]), fname2(argv[4]);
        cerr << "Comparing files = " << fname1 << " and " << fname2 << "." << endl;

        int nbG1 = -1, nbG2 = -1;
        string strNbVert = to_string(nbVert);
        if (argc > 5) // In case it is not magic graphs, we need to read the number of graphs.
        {
            ifstream fSize1(argv[5]);
            fSize1 >> nbG1;
            fSize1.close();
        }
        if (argc > 6)
        {
            ifstream fSize2(argv[6]);
            fSize2 >> nbG2;
            fSize2.close();
        }

        vector<Graph> listGraphs1 = load_from_file(fname1, nbG1);
        vector<Graph> listGraphs2 = load_from_file(fname2, nbG2);

        sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs1;
        sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs2;

        vector<char> degreeList(nbVert+4);

        for (Graph& g : listGraphs1)
        {
            g.compute_hashes(degreeList);
            sort(degreeList.begin(), degreeList.begin()+nbVert);
            if (!check_if_seen_and_add(g, degreeList, deglist2Graphs1))
                cerr << "ERROR : seen a copy of another graph in the first file" << endl;
        }
        for (Graph& g : listGraphs2)
        {
            g.compute_hashes(degreeList);
            sort(degreeList.begin(), degreeList.begin()+nbVert);
            if (!check_if_seen_and_add(g, degreeList, deglist2Graphs2))
                cerr << "ERROR : seen a copy of another graph in the second file" << endl;
        }
        cerr << listGraphs1.size() << " graphs up to isomorphism in the first file " << endl;
        cerr << listGraphs1.size() << " graphs up to isomorphism in the second file " << endl;

        cout << "This list = " << fname1 << " and other list = " << fname2 << endl;
        compare_two_graphs_sets(deglist2Graphs1, deglist2Graphs2);
        cout << "\n------------------------\n\n";
        cout << "This list = " << fname2 << " and other list = " << fname1 << endl;
        compare_two_graphs_sets(deglist2Graphs2, deglist2Graphs1);
    }

    else if (testOrGen == 'T') // Tests if some graphs are not twin-free.
    {
        cerr << nbVert << " zut \n";
        string fname(argv[3]);
        cerr << "file = " << fname << endl;
        int nbG = -1;
        string strNbVert = to_string(nbVert);
        if (argc > 4) // In case it is not magic graphs, we need to read the number of graphs.
        {
            ifstream fSize(argv[4]);
            fSize >> nbG;
            fSize.close();
        }

        vector<Graph> listGraphs = load_from_file(fname, nbG);

        for (const Graph& g : listGraphs)
        {
            if (has_twin(g))
            {
                g.pretty_print();
                cerr << "This (^) graph has twins." << endl;
            }
        }
    }

    else if (testOrGen == 'A') // Generates sets and compatibility array for some graphs.
    {
        vector<Graph> listG;
        string fname(argv[3]);
        cerr << "Generating arrays from file " << fname << endl;
        igzstream fTableau(fname.c_str());
        string tmpStr;
        while (fTableau.peek() != EOF)
        {
            Graph gRead(fTableau);
            listG.push_back(fTableau);
            getline(fTableau, tmpStr); // Removing end of line
        }

        for (const Graph &g : listG)
        {
            ProblemArray pb;
            pb.baseGraph = g;
            pb.gen_default_partition();
            //TODO try to load known magic graphs as obstructions.

            cout << "The graph:" << endl;
            g.pretty_print();
            cout << "Its array:" << endl;
            pb.print_array();
            cout << "And its array, in latex:" << endl;
            pb.print_array_latex();
            cout << endl << "---------------------------" << endl;
        }
    }

    /* TODO study later
    else if (testOrGen == 'B') // Generates sets and compatibility array for some graphs.
    {
        vector<vector<int>> adjSets, antiCompleteSets;
        vector<string> setsNames;
        //TODO
        cerr << nbVert << " zut \n";
        string fname(argv[3]);
        cerr << "file = " << fname << endl;
        igzstream fTableau(fname.c_str());

        Graph g = Graph(fTableau);
        nbVert = g.nbVert;

        string line;
        vector<int> freeVerts;
        vector<bool> wasSeen(g.nbVert);

        map<string, int> setsNames2Id;
        int cptSet = 0;

        vector<vector<string>> forcedNeighoursSetsNames;

        if (fTableau.peek() == 'F') // Free vertices list
        {
            string foo;
            getline(fTableau, line);
            stringstream lineSs(line);
            lineSs >> foo;

            int u;
            while (lineSs >> u)
            {
                cerr << u << " is Free!\n";
                freeVerts.push_back(u);
            }
        }

        while (getline(fTableau, line))
        {
            stringstream lineSs(line);
            string nameRead;
            if (lineSs.peek() == ';')
            {
                getline(lineSs, nameRead);
                continue;
            }
            lineSs >> nameRead;

            while (nameRead[0] == ';') // Comment, not to process
            {
                string foo;
                getline(lineSs, foo);
                continue;
            }

            setsNames.push_back(nameRead);
            setsNames2Id[nameRead] = cptSet;
            cptSet++;

            vector<string> curForcedNeighbNames;

            int x;
            vector<int> curAdj, curAntiAdj;
            while (lineSs.peek() != '.' && lineSs.peek() != ';' && lineSs >> x)
            {
                curAdj.push_back(x);
                wasSeen[x] = true;
                lineSs.get();
            }

            if (lineSs.peek() == '.')
            {
                lineSs.get();
                while (lineSs.peek() != ';' && lineSs >> x)
                {
                    cerr << x << " is anticomplete for " << nameRead << endl;
                    curAntiAdj.push_back(x);
                }

            }
            if (lineSs.peek() == ';')
            {
                cerr << " YEAH22";
                string curName;
                lineSs.get();

                while (lineSs >> curName)
                {
                    cerr << "  " << curName;
                    curForcedNeighbNames.push_back(curName);
                }
                cerr << endl;
            }

             //Inutiles là : on spécifie les sommets libres
            //if (lineSs.peek() == ':')
            //{
            //    wasSeen[x] = false;
            //    freeVerts.push_back(x); //TODO s'assurer que y'a pas des en double...
            //}


            adjSets.push_back(curAdj);
            antiCompleteSets.push_back(curAntiAdj);

            forcedNeighoursSetsNames.push_back(curForcedNeighbNames);
        }

        g.pretty_print();
        cerr << adjSets.size() << " sets and " << setsNames.size() << " names\n";
        for (int i = 0; i < adjSets.size(); i++)
        {
            cout << setsNames[i] << " ";
            for (int u : adjSets[i])
                cout << u << " ";
            cout << endl;
        }

        vector<Graph> obstructions;
        int nbObstruction;
        igzstream fObstructions("obstructions-c3.txt");
        fObstructions >> nbObstruction;
        string foo;
        getline(fObstructions, foo);
        obstructions.resize(nbObstruction);
        for (int i = 0; i < nbObstruction; i++)
        {
            obstructions[i] = Graph(fObstructions);
            obstructions[i].pretty_print();
        }
        fObstructions.close();


        sparse_hash_map<vector<char>, vector<Graph>> prefixeurs;

        vector<vector<int>> forcedNeighoursSetsIds;

        for (const vector<string> &curForcedNeighbs : forcedNeighoursSetsNames)
        {
            vector<int> curIds;

            for (const string &str :curForcedNeighbs)
                curIds.push_back(setsNames2Id[str]);

            forcedNeighoursSetsIds.push_back(curIds);
        }

        //return 1;


        compute_cleophee_arrays(g, adjSets, antiCompleteSets, setsNames, freeVerts, obstructions, prefixeurs, prefixeurs, 0, true); //0 = idthread

        fTableau.close();
    }*/

    // Tests if a list of graphs contain multiple isomorphic copies of some graph.
    else if (testOrGen == 'I')
    {
        string fname(argv[3]);
        int nbG = -1;
        if (argc > 4) // In case it is not magic graphs, we need to read the number of graphs.
        {
            ifstream fSize(argv[4]);
            fSize >> nbG;
            fSize.close();
        }
        vector<Graph> listGraphs = load_from_file(fname, nbG);

        vector<char> degreeList(nbVert+4);
        for (Graph &g : listGraphs)
            g.compute_hashes(degreeList);

        // Possible to do it faster by adding the graphs wifh check_if_seen_and_add and when a graph
        // is already present, check with which it is isomorphic.
        int cptIso = 0;
        for (int i1 = 0; i1 < listGraphs.size(); i1++)
        {
            for (int i2 = i1+1; i2 < listGraphs.size(); i2++)
            {
                if (are_isomorphic(listGraphs[i1], listGraphs[i2], 0))
                {
                    cout << "ERROR: graphs " << i1 << " and " << i2 << " ARE ISOM (" << ++cptIso  << "problems so far)." << endl;
                    listGraphs[i1].pretty_print();
                    listGraphs[i2].pretty_print();
                    cout << endl << endl;
                }
            }
        }
    }


    free_adjListGlobal();

    return 0;
}
