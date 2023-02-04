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

    init_adjListGlobal(nbVert+3);
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
        int nbMinimal = 0;

        stringstream fileNameMinimal;
        fileNameMinimal << "Alexminimalprefixeursdelataille";
        fileNameMinimal << nbVert << ".txt.gz";
        ogzstream fileMinimal(fileNameMinimal.str().c_str());

        for (const auto &pairDegG : prefixeurs)
        {


            for (const Graph &g : pairDegG.second)
            {
                nbMinimal++;
                g.print();
                g.print_in_file(fileMinimal);
            }
        }
        cerr << nbMinimal << endl;
        fileMinimal.close();
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

    else if (testOrGen == 'Z') // Gen the extension sets for each graph, compute its array, check for quasi-fixers^^
    {
        stringstream fileName;
        fileName << "Alexgraphedelataille";
        fileName << nbVert << ".txt.gz";
        cerr << fileName.str() << endl;
        int nbGraph = -1;
        stringstream sizeFileName;
        sizeFileName << "Alexsizegraphedelataille";
        sizeFileName << nbVert << ".txt";
        cerr << sizeFileName.str() << endl;

        ifstream sizeFile(sizeFileName.str());
        sizeFile >> nbGraph;
        assert(nbGraph > 0);
        cerr << "Je veux voir " << nbGraph << " graphes\n";


        vector<Graph> listGraphs = load_from_file(fileName.str(), nbGraph);

         stringstream fileNamePlus;
        fileNamePlus << "Alexfixeursdelataille";
        fileNamePlus << nbVert+1 << ".txt.gz";

        sparse_hash_map<vector<char>, vector<Graph>> prefixeurs;
        read_prefixeurs_compute_hash(fileNamePlus.str(), nbVert+1, prefixeurs);


        stringstream fileNamePlusPlus;
        fileNamePlusPlus << "Alexfixeursdelataille";
        fileNamePlusPlus << nbVert+2 << ".txt.gz";

        sparse_hash_map<vector<char>, vector<Graph>> prefixeursPlus2;
        read_prefixeurs_compute_hash(fileNamePlusPlus.str(), nbVert+2, prefixeursPlus2);


        cout << " FINI lire préfixeurs" << endl;



        int cptGood = 0;
        int cpt = 0;
        int pct = (max(1,nbGraph/100));
        for (Graph &g : listGraphs)
        {
            if (argv[2][1] == 'o' && is_quasi_fixer(g, prefixeurs, prefixeursPlus2, 0))
            {
                cout << "\tYEAH "<< ++cptGood << endl;
                g.print();
            }
            if (argv[2][1] == 'n' && is_magic_graph(g))
            {
                cout << "\tYEAH "<< ++cptGood << endl;
                g.print();
            }
            cpt++;
            if (cpt%pct == 0)
                cout << round(100*cpt/(double)nbGraph)  << endl;
            continue;
            assert(false);
            vector<vector<int>> truc;
            gen_klmpartition_default_sets(g, truc, prefixeurs, 0);

            for (const vector<int> &t : truc)
            {
                cerr << "V_i ";
                for (const int x : t)
                    if (x != 0 && x != 7)
                        cerr << x << " ";
                cerr << endl;
            }
            g.print();
        }

        cerr << cptGood << "/" << listGraphs.size() << " are good\n";
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

        cerr << "HAHAHA YEAH\n";
        for (const Graph &g : list)
        {
            for (int u = 0; u < g.nbVert; u++)
                assert(!has_twin(g, u));
        }
        cerr << "HIHIHI YEAH\n";


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
        string fname1(argv[3]), fname2(argv[4]);
        cerr << "files = " << fname1 << " and " << fname2 << endl;
        int nbG1 = -1, nbG2 = -1;
        string strNbVert = to_string(nbVert);
        if (fname1.back() == '/') // In case it is not fixers, we need to read the nb of graphs.
        {
            ifstream fSize1(fname1+"Alexsizegraphedelataille"+strNbVert+".txt");
            fSize1 >> nbG1;
            fSize1.close();
            fname1 += "Alexgraphedelataille"+strNbVert+".txt.gz";
        }
        if (fname2.back() == '/')
        {
            ifstream fSize2(fname2+"Alexsizegraphedelataille"+strNbVert+".txt");
            fSize2 >> nbG2;
            fSize2.close();
            fname2 += "Alexgraphedelataille"+strNbVert+".txt.gz";
        }
        cerr << "files = " << fname1 << " and " << fname2 << endl;


        vector<Graph> listGraphs1 = load_from_file(fname1, nbG1);
        vector<Graph> listGraphs2 = load_from_file(fname2, nbG2);

        sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs1;
        sparse_hash_map<vector<char>, vector<Graph>> deglist2Graphs2;

        vector<char> degreeList(nbVert+4);

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

        cout << "This list = " << fname1 << " and other list = " << fname2 << endl;
        compare_two_fixeurs_sets(deglist2Graphs1, deglist2Graphs2);
        cout << "\n------------------------\n\n";
        cout << "This list = " << fname2 << " and other list = " << fname1 << endl;
        compare_two_fixeurs_sets(deglist2Graphs2, deglist2Graphs1);

    }

    else if (testOrGen == 'T') //Test if there are twins
    {
        cerr << nbVert << " zut \n";
        string fname(argv[3]);
        cerr << "file = " << fname << endl;
        int nbG = -1;
        string strNbVert = to_string(nbVert);
        if (fname.back() == '/') // In case it is not fixers, we need to read the nb of graphs.
        {
            ifstream fSize(fname+"Alexsizegraphedelataille"+strNbVert+".txt");
            fSize >> nbG;
            fSize.close();
            fname += "Alexgraphedelataille"+strNbVert+".txt.gz";
        }

        vector<Graph> listGraphs = load_from_file(fname, nbG);

        for (Graph& g : listGraphs)
        {
            bool stop = false;
            for (int u = 0; u < nbVert && !stop; u++)
            {
                for (int v = u+1; v < nbVert; v++)
                {
                    if ((g.adjMat[u]^g.adjMat[v]) == ((1<<u) ^ (1<< v)))
                    {
                        stop = true;
                        g.print();
                        cerr << "ERROR " << u << "," << v << endl;
                        break;
                    }
                }
            }
        }
        cerr << listGraphs.size() << " mdr " << endl;

    }

    else if (testOrGen == 'A') //Generate arrays
    {
        vector<vector<int>> adjSets, antiCompleteSets;
        vector<string> setsNames;
        //TODO
        cerr << nbVert << " zut \n";
        string fname(argv[3]);
        cerr << "file = " << fname << endl;
        int nbEdge;
        string strNbVert = to_string(nbVert);
        igzstream fTableau(fname.c_str());

        Graph g = Graph(fTableau);
        nbVert = g.nbVert;
        nbEdge = g.nbEdge;

        /*
        vector<vector<int>> setsPartition;
        set<vector<int>> toto;
        gen_klmpartition_default_sets(g, setsPartition);
        sort(setsPartition.begin(), setsPartition.end(), [](const vector<int> & a, const vector<int> & b){ return a.size() < b.size(); });
        int cpt = 0;
        for (const auto &x : setsPartition)
        {
            if (toto.insert(x).second == false)
                continue;
            char car = 'A'+cpt;
            if (car > 'Z')
                car = 'a'+cpt;
            cpt++;
            cerr << car << "\t";
            for (int u : x)
                cerr << u << " ";
            cerr << endl;
        }
        return 0;
        */

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

            if (nameRead[0] == ';') // Comment, not to process
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

            /* //Inutiles là : on spécifie les sommets libres
            if (lineSs.peek() == ':')
            {
                wasSeen[x] = false;
                freeVerts.push_back(x); //TODO s'assurer que y'a pas des en double...
            }*/


            adjSets.push_back(curAdj);
            antiCompleteSets.push_back(curAntiAdj);

            forcedNeighoursSetsNames.push_back(curForcedNeighbNames);
        }

        g.print();
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
            obstructions[i].print();
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
    }

    else if (testOrGen == 'I') //Test bug isomorphisme
    {
        cerr << nbVert << " zut \n";
        string fname(argv[3]);
        int nbG = -1;
        string strNbVert = to_string(nbVert);

        vector<Graph> listGraphs = load_from_file(fname, nbG);

        vector<char> degreeList1(nbVert+4), degreeList2(nbVert+4);
        Graph &g1=listGraphs[0], &g2= listGraphs[1];
        g1.compute_hashes(degreeList1);
        g2.compute_hashes(degreeList2);
        g1.print();
        g2.print();

        cerr << are_isomorphic(g1, g2, 0);

    }


    free_adjListGlobal();

    return 0;
}
