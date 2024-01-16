#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <sstream>

#include "sparsepp/spp.h"
#include "gzstream/gzstream.h"
#include "Graph.hh"
#include "problemArray.hh"
#include "test-properties.hh"
#include "gen-graph.hh"


using namespace std;

inline string intToSetName (int x)
{
    string s;
    s.push_back((char)('A'+x));
    return s;
}

Graph ProblemArray::add_vertices_to_base_graph(const ProblemArraySet* adjVertsToAdd[], int nbSet) const
{
    int n0 = baseGraph.nbVert;
    Graph gRet;
    gRet.init(n0+nbSet, baseGraph.nbEdge);
    for (int u = 0; u < n0; u++)
        gRet.adjMat[u] = baseGraph.adjMat[u];

    for (int uNew = 0; uNew < nbSet; uNew++)
    {
        const vector<int> &curAdj = adjVertsToAdd[uNew]->neighbInBaseGraph;
        for (int x : curAdj)
            gRet.add_edge(n0+uNew, x);
    }

    return gRet;
}

bool ProblemArray::can_3sets_be_possible(const ProblemArraySet &setA, const ProblemArraySet &setB, const ProblemArraySet &setC) const
{
    assert(!setA.is_advanced() && !setB.is_advanced() && !setC.is_advanced());

    int n0 = baseGraph.nbVert;
    /*Graph gABC;
      gABC.init(n0+3, baseGraph.nbEdge);
      for (int u = 0; u < baseGraph.nbVert; u++)
      gABC.adjMat[u] = baseGraph.adjMat[u];

      const ProblemArraySet *adjPtrs[3] = {&setA, &setB, &setB};

      for (int uNew = 0; uNew < 3; uNew++)
      {
      const vector<int> &curAdj = adjPtrs[uNew]->neighbInBaseGraph;
      for (int x : curAdj)
      gABC.add_edge(n0+uNew, x);
      }*/

    const ProblemArraySet* sets[3] = {&setA, &setB, &setC};
    Graph gABC = add_vertices_to_base_graph(sets, 3);

    int uA = n0, uB = n0+1, uC=  n0+2;

    vector<Graph> realGABList;
    Graph fooG = gABC;
    get_possible_free_neighbourhoods(uB, {uA, uC}, fooG, 0, realGABList);


    vector<Graph> realGABCList;
    for (Graph &g : realGABList)
    {
        Graph fooGG = gABC;
        get_possible_free_neighbourhoods(uB, {uA, uC}, fooGG, 0, realGABCList);
        if (!realGABCList.empty())
            return true;
    }

    return false;
}


bool ProblemArray::is_true_N_between_two(const ProblemArraySet &setA, const ProblemArraySet &setB) const
{
    assert(!setA.is_advanced() && !setB.is_advanced());
    int n0 = baseGraph.nbVert;
    const ProblemArraySet* sets[3] = {&setB, &setB, &setA};
    Graph gAB = add_vertices_to_base_graph(sets, 3);
    int uB1 = n0, uB2 = n0+1, uA = n0+2;
    gAB.add_edge(uB1, uB2);

    gAB.add_edge(uA, uB1);
    return is_graph_ok(gAB, false);
}

bool ProblemArray::can_NN_be_solved_method1(const ProblemArraySet &setA, const ProblemArraySet &setB, const ProblemArraySet &setC) const
{
  //TODO même si inclus dans method2 ?


    return false;
}


/*
//TODO faire les fusions, puis voir si souci ou pas souci
bool ProblemArray::can_NN_be_solved_method2(void) const
{
    string merging1Log = "MergingPhaseOne:\n", merging2Log = "mergingPhaseTwo:\n";
    int nbError = 0;
    int nbSet = partitionSets.size();
    vector<set<int>> badNeighbs(nbSet);
    for (int i1 = 0; i1 < nbSet; i1++)
    {
        for (int i2 = 0; i2 < nbSet; i2++)
        {
            if (partitionArray[i1][i2] == 'N')
                badNeighbs[i1].insert(i2);
        }
    }


    for (int i = 0; i < nbSet; i++)
    {
        string curMsg;
        for (int i1 : badNeighbs[i]) //TODO ii1 indice, et ii2 >= ii1 ?
        {
            if (!is_true_N_between_two(partitionSets[i], partitionSets[i1]))
                continue;
            for (int i2 : badNeighbs[i])
            {
                if (i1 == i2)
                    continue;
                curMsg = intToSetName(i1)+ "," +intToSetName(i2) + "  ";
                if (partitionArray[i1][i2] == '1' || partitionArray[i1][i2] == '-')
                    continue;
                if (!is_true_N_between_two(partitionSets[i], partitionSets[i2]))
                    continue;

                if (!can_3sets_be_possible(partitionSets[i], partitionSets[i1], partitionSets[i2]))
                    continue;

                curMsg = "";

                //TODO étudier le cas false N between i1 et i2 ! a priori pas souci !

                //
                //cerr << " CANNOT MERGEFIRST " << (char)('A'+i1) << " AND " << (char)('A'+i2) << ": " << partitionArray[i1][i2] << endl;
                //return false;
                nbError++;
            }
            merging1Log += curMsg+"\n";
        }
    }
    cerr << " Second phase of second method.\n";

    set<int> seen, seenAdvanced; // seenAdvanced also contains the centers of the triplets
    for (int i = 0; i < nbSet; i++)
    {
        if (badNeighbs[i].size() <= 1)
            continue;
        merging2Log += "\tSet " + intToSetName(i) + ": merging ";
        seenAdvanced.insert(i);
        for (int x : badNeighbs[i])
        {
            if (seen.find(x) != seen.end())
            {
                //cerr << " CANNOT MERGESECOND " << (char)('A'+i) << " AND " << (char)('A'+x) << ": " << partitionArray[i][x] << endl;
                nbError++;
                continue;
                //return false;
            }
            if (seenAdvanced.find(x) != seenAdvanced.end())
                merging2Log += "WARN:";
            merging2Log += intToSetName(x) + ", ";
            seenAdvanced.insert(x);
            seen.insert(x);
        }
        merging2Log += "\n";
    }

    cerr << "uuu\n" <<merging1Log << "\n" << merging2Log << "\n";

    //cerr << "trobi1\n";
    cerr << "Soucis réels: " << nbError << endl;
    //TODO
    return nbError == 0;
}*/


int find(int x, std::vector<int> &uf)
{
    if (uf[x] != x)
        uf[x] = find(uf[x], uf);
    return uf[x];
}


// New version
bool ProblemArray::can_NN_be_solved_method2(void) const
{
    string merging1Log = "MergingPhaseOne:\n", merging2Log = "mergingPhaseTwo:\n";
    int nbError = 0;
    int nbSet = partitionSets.size();
    vector<pair<int, int>> badTriplet(nbSet);
    std::vector<std::vector<int>> toMerge(nbSet);

    std::vector<int> unionfind(nbSet);
    for (int i = 0; i < nbSet; i++)
      unionfind[i] = i;
    std::vector<std::set<int>> ufSets(nbSet);
    for (int i = 0; i < nbSet; i++)
      ufSets[i].insert(i);
    for (int i = 0; i < nbSet; i++)
    {
        toMerge[i].push_back(i);
        for (int i1 = 0; i1 < nbSet; i1++)
        {
            if (partitionArray[i][i1] != 'N')
                continue;
            if (!is_true_N_between_two(partitionSets[i], partitionSets[i1]))
                continue;
            for (int i2 = i1+1; i2 < nbSet; i2++)
            {
                if (partitionArray[i1][i2] == '-')
                  continue;
                if (partitionArray[i][i2] != 'N')
                    continue;
                if (!is_true_N_between_two(partitionSets[i], partitionSets[i2]))
                    continue;
                if (!can_3sets_be_possible(partitionSets[i], partitionSets[i1], partitionSets[i2]))
                    continue;
                if (partitionArray[i1][i2] != '1')
                    return false;
                {
                    int repr1 = find(i1, unionfind);
                    int repr2 = find(i2, unionfind);
                    if (repr1 == repr2)
                      continue;
                    unionfind[repr1] = repr2;
                    set<int> &set1 = ufSets[repr1];
                    set<int> &set2 = ufSets[repr2];

                    set1.clear();
                    toMerge[i1].push_back(i2);
                    toMerge[i2].push_back(i1);
                    //cout << "Triplet bad: " << (char)('A'+i) << (char)('A'+i1) << (char)('A'+i2) << endl;
                }
            }
        }
    }


/*
    cout << endl << endl;
    for (const auto& set : ufSets)
    {
      if (set.size() == 1)
        continue;
      cout << "Il faut fusionner : ";
      for (int x : set)
        cout << (char)('A'+x);
      cout << endl;
    }
*/
    string mergingLog;
    for (int i = 0; i < nbSet; i++)
    {
        for (int i1 : toMerge[i])
            for (int i2 : toMerge[i])
                if (partitionArray[i1][i2] != '1' && partitionArray[i1][i2] != '-')
                    return false;
        /*mergingLog += "Merging : ";
        for (int x : toMerge[i])
            mergingLog = mergingLog + to_string('A'+i) + " ";
        mergingLog += "\n";*/
    }

    //cout << mergingLog << endl;

    return true;
}


void ProblemArray::get_possible_free_neighbourhoods(int newVert, const vector<int> &freeVerts, Graph &curG, int pos, vector<Graph> &ret) const
{
    if (pos == freeVerts.size())
    {
        if (is_graph_ok(curG, false))//verbose == 2))
            ret.push_back(curG);
    }
    else
    {
        get_possible_free_neighbourhoods(newVert, freeVerts, curG, pos+1, ret);
        curG.add_edge(newVert, freeVerts[pos]);
        get_possible_free_neighbourhoods(newVert, freeVerts, curG, pos+1, ret);
        curG.delete_edge(newVert, freeVerts[pos]);
    }
}


void ProblemArray::gen_default_partition(void)
{
    int nbVert = baseGraph.nbVert+1;
    int puissNewVert = (1<< (nbVert-1));
    sparse_hash_map<vector<char>, vector<Graph>> fooObstructions;

    assert(!has_twin(baseGraph));

    for (int code = 0; code < puissNewVert; code++)
    {
        const vector<int> &newEdgesList = adjListGlobal[code];
        Graph gWithEdges;
        gWithEdges.copy_and_add_new_vertex_bis(baseGraph, newEdgesList, puissNewVert, code);

        if (is_graph_ok(gWithEdges, false))
        {
            ProblemArraySet newSet;
            newSet.id = partitionSets.size();
            newSet.neighbInBaseGraph = adjListGlobal[code];
            partitionSets.push_back(newSet);
        }
    }
}


bool ProblemArray::is_graph_ok(const Graph &g, bool print) const
{
    if (!free_C4_O4(g, g.nbVert))
        return false;

    if (deglist2ObstructionsBySize == NULL)
        return true;
    int n = g.nbVert;
    vector<sparse_hash_map<vector<char>, vector<Graph>>> &obstructions = *deglist2ObstructionsBySize;
    int sizeMax = min((int)obstructions.size(), n);

    vector<char> hashVect(n+4);
    Graph gg = g;
    gg.compute_hashes(hashVect);

    sparse_hash_map<vector<char>, vector<Graph>> &curObstructions = obstructions[n];
    for (const Graph &gObstr : curObstructions[hashVect])
        if (are_isomorphic(gg, gObstr, 0))//TODO idthread
            return false;

    return true;
}


//TODO compute given constraints on forced neighbours and so on in the sets
void ProblemArray::compute_partition_array(void)
{
    int n = baseGraph.nbVert;
    int nbSet = partitionSets.size();

    partitionArray.resize(nbSet);
    for (int i = 0; i < nbSet; i++)
        partitionArray[i].resize(nbSet);

    for (int i1 = 0; i1 < nbSet; i1++)
    {
        assert(!partitionSets[i1].is_advanced());
        partitionArray[i1][i1] = '1';
        for (int i2 = i1+1; i2 < nbSet; i2++)
        {
            char compat = get_sets_compatibility(i1, i2);

            partitionArray[i1][i2] = compat;
            partitionArray[i2][i1] = compat;
        }
    }
}

//TODO free vertices...
char ProblemArray::get_sets_compatibility(int i1, int i2) const
{
    const ProblemArraySet &set1 = partitionSets[i1], &set2 = partitionSets[i2];
    assert(!set1.is_advanced() && !set2.is_advanced());
    int n0 = baseGraph.nbVert;

    const ProblemArraySet* sets[2] = {&set1, &set2};
    Graph gNew = add_vertices_to_base_graph(sets, 2);

    bool edgeOk = true, noEdgeOk = true;

    if (!is_graph_ok(gNew, false))
        noEdgeOk = false;
    gNew.add_edge(n0, n0+1);
    if (!is_graph_ok(gNew, false))
        edgeOk = false;

    if (noEdgeOk && edgeOk)
    {
        if (!is_true_N_between_two(set1, set2))
        {
            cerr << "yyyyyyyyyyyyyyyyy\n";
            return 'B';
        }
        return 'N';
    }
    else if (noEdgeOk)
        return '0';
    else if (edgeOk)
        return '1';
    else
        return '-';
}

void ProblemArray::print_array(void) const
{
    int nbSet = partitionSets.size();
    cout << "\t";
    for (int i = 0; i < nbSet; i++)
        cout << (char)('A'+i) << "\t";
    cout << endl;

    //printGlobal = true;
    for (int i1 = 0; i1 < nbSet; i1++)
    {
        cout << (char)('A'+i1) << "\t";
        for (int i2 = 0; i2 < nbSet; i2++)
            cout << partitionArray[i1][i2] << "\t";
        cout << endl;
    }
    cout << endl << endl;
}

void ProblemArray::print_array_latex(void) const
{
    map<char, string> corres;
    corres['0'] = "$\\ominus$&";
    corres['1'] = "$\\oplus$&";
    corres['N'] = "$\\otimes$&";
    corres['-'] = "$\\oslash$&";
    int nbSet = partitionSets.size();
    cout << "\t";
    for (int i = 0; i < nbSet; i++)
        cout << (char)('A'+i) << "&\t";
    cout << "\\\\" <<  endl;

    //printGlobal = true;
    for (int i1 = 0; i1 < nbSet; i1++)
    {
        cout << (char)('A'+i1) << "&\t";
        for (int i2 = 0; i2 < nbSet; i2++)
            cout << corres[partitionArray[i1][i2]] << "\t";
        cout << "\\\\" << endl;
    }
    cout << endl << endl;
}

bool ProblemArray::check_that_set_is_clique(const ProblemArraySet &set) const
{
    assert(!set.is_advanced());
    int n = baseGraph.nbVert;
    const ProblemArraySet* sets[2] = {&set, &set};
    Graph gPlus2 = add_vertices_to_base_graph(sets, 2);

    vector<Graph> possibleGraphs;
    get_possible_free_neighbourhoods(n+1, {}, gPlus2, 0, possibleGraphs);
    for (const Graph &g : possibleGraphs)
        if (!are_neighb(g, n, (n+1)))
            return false;

    return true;
}

vector<string> ProblemArray::solve_array_problems(void) const
{
    vector<string> badTriplets;
    int nbError = 0;
    bool error = false;
    set<int> badTripletElts;
    int nbSet = partitionSets.size();
    for (int i1 = 0; i1 < nbSet; i1++)
    {
        const ProblemArraySet &set1 = partitionSets[i1];
        for (int i2 = 0; i2 < nbSet; i2++)
        {
            const ProblemArraySet &set2 = partitionSets[i2];
            if (partitionArray[i1][i2] != 'N')
                continue;
            if (!is_true_N_between_two(set1, set2))
            {
                cout << " LOL12\n";
                continue;
            }
            for (int i3 = i2+1; i3 < nbSet; i3++)
            {
                const ProblemArraySet &set3 = partitionSets[i3];
                if (partitionArray[i1][i3] != 'N')
                    continue;

                if (partitionArray[i2][i3] == '-')
                {
                    //cout << "xD\n";
                    continue;
                }

                if (!is_true_N_between_two(set1, set3))
                {
                    cout << " ptdr\n";
                    continue;
                }


                if (!can_3sets_be_possible(set1, set2, set3))
                {
                    cout << " mdr \n";
                    continue;
                }


                string tripletName;
                for (int x : {i1,i2,i3})
                    tripletName.push_back((char)('A'+x));
                if (partitionArray[i2][i3] == '1')
                {
                    auto itEnd = badTripletElts.end();
                    if (badTripletElts.find(i1) != itEnd || badTripletElts.find(i2) != itEnd || badTripletElts.find(i3) != itEnd)
                    {
                        nbError++;
                        badTriplets.push_back(tripletName);
                        continue;
                    }
                    //cout << "FUSION" << i1 << "," <<i2 << "," << i3 <<"\n";
                    //cout << "FUSION" << (char)(i1+'A') << "," <<(char)(i2+'A') << "," << (char)(i3+'A') <<"\n";
                    badTripletElts.insert(i1);
                    badTripletElts.insert(i2);
                    badTripletElts.insert(i3);
                    continue; // TODO WARNING EXPERIMENTAL!!!
                }

                nbError++;
                badTriplets.push_back(tripletName);
                // We did not save this bad triplet...
                // TODO REMETTRE
                //return false;
            }
        }
    }

    return badTriplets;
}



//TODO free vertices for base graph
bool is_magic_graph(const Graph &g, bool special, vector<sparse_hash_map<vector<char>, vector<Graph>>> *deglist2ObstructionsBySize)
{
    ProblemArray pbArray;
    pbArray.baseGraph = g;
    pbArray.deglist2ObstructionsBySize = deglist2ObstructionsBySize;


    pbArray.gen_default_partition();
    for (int i = 0; i < pbArray.partitionSets.size(); i++)
    {
        if (!pbArray.check_that_set_is_clique(pbArray.partitionSets[i]))
        {
            cout << "WARNING: sets of the partition do not induce cliques :(" << endl;
            exit(3);

            return false;
        }
    }
    pbArray.compute_partition_array();
    const vector<vector<char>> &tableau = pbArray.partitionArray;
    int nbSet = pbArray.partitionSets.size();
    /*
    for (int i1 = 0; i1 < nbSet; i1++)
    {
        for (int i2 = 0; i2 < nbSet; i2++)
        {
            if (tableau[i1][i2] != 'N')
                continue;
            for (int i3 = i2+1; i3 < nbSet; i3++)
            {
                if (tableau[i1][i3] == 'N' && tableau[i2][i3] != '-')
                    return false;
            }
        }
    }*/

    vector<string> errorTriplets = pbArray.solve_array_problems();

    cout << "------------------------------\n";
    cout << "il y a " << errorTriplets.size() << " vrais soucis\n";

    bool isOk2 = pbArray.can_NN_be_solved_method2();
    if (isOk2)
        errorTriplets.clear();
    if (true || errorTriplets.size() <= 40 || isOk2)
    {
        cerr << "printing graph:\n";
        g.print();
        cerr << "printing neighbourfood of vertices:\n";
        int n = pbArray.partitionSets.size();
        for (int i = 0; i < n; i++)
        {
            char name = i+'A';
            cerr << "set " << name << ": ";
            for (int x : pbArray.partitionSets[i].neighbInBaseGraph)
                cerr << x << ", ";
            cerr << endl;
        }
        pbArray.print_array();
        if (isOk2)
          pbArray.print_array_latex();

        /*
        cerr << "Printing bad triplets :";
        for (string &x : errorTriplets)
            cerr << x << ", ";
        cout << endl << endl; */
        cout << "Il y a " << errorTriplets.size() << " bad triplets\n";
    }
    if (isOk2)
        return true;
    if (errorTriplets.size() != 0)
        return false;


    return true;
}

sparse_hash_map<vector<char>, vector<Graph>> gen_magic_graphs(int nbVert)
{
    //TTAADDAA documenter variables, plus parce que taille au dessus
    vector<sparse_hash_map<vector<char>, vector<Graph>>> deglists2MagicGraphs(NBMAXVERT);
    vector<vector<char>> degreeLists(NBMAXVERT);
    for (int i = 0; i < nbVert; i++)
        degreeLists[i].resize(i+4);


    int nbEdgeCombi = 1<<nbVert;

    stringstream fileName, fileSizeName;
    fileName << "Alexgraphedelataille" << nbVert << ".txt.gz";
    fileSizeName << "Alexsizegraphedelataille" << nbVert << ".txt";
    ifstream fSize(fileSizeName.str());

    if (fSize.peek() == EOF)
    {
        cerr << "Lancer avant la taille -1 size \n";
        cerr << fileSizeName.str() << endl;
        exit(3);
    }
    long long nbGToRead;
    fSize >> nbGToRead;
    vector<Graph> listGraphs = load_from_file(fileName.str(), nbGToRead);
    if (listGraphs.empty())
    {
        cerr << "Erreur : lancer avant la génération de la même taille \n";
        exit(3);
    }
    cout << "j'ai généré/trouvé les graphes à " << nbVert << " somets : il y en a " << listGraphs.size() << endl;

    for (int i = 1; i < nbVert; i++)
    {
        //if (i == nbVert)
        //    continue;
        string fileNamee = "Alexmagicdelataille"+to_string(i)+".txt.gz";
        cerr << fileNamee << " is my file " << endl;
        read_prefixeurs_compute_hash(fileNamee, i ,deglists2MagicGraphs[i]);
    }

    //vector<sparse_hash_map<vector<char>, vector<Graph>>> deglists2MagicGraphs(NBMAXVERT);
    int cptInflating = 0;
    vector<long long> pathLength2(NBMAXVERT);

    for (int i = 1; i < nbVert+4; i++)
    {
        cerr << "Trying to inflate size " << i << endl;
        Graph gBigger;
        int puissNewVert = (1<< i);
        vector<char> hashVect(i+5);
        for (const pair<const vector<char>, vector<Graph>>& dToGraphs : deglists2MagicGraphs[i])
        {
            for (const Graph &gMagic : dToGraphs.second)
            {
                gen_P2_list(gMagic, pathLength2, i+1);
                //connected graphs. We generate all graphs with one more vertex containing gMagic
                for (int idNewEdges = 1; idNewEdges < puissNewVert; idNewEdges++)
                {
                    gBigger.copy_and_add_new_vertex_bis(gMagic, adjListGlobal[idNewEdges], puissNewVert, idNewEdges);

                    if (detect_C4(pathLength2, idNewEdges) || !free_O4(gBigger, i+1))
                        continue;

                    gBigger.compute_hashes(hashVect);
                    if (check_if_seen_and_add(gBigger, hashVect, deglists2MagicGraphs[i+1], 0))// 0 idthread
                        cptInflating++;
                    //cerr << "Inflating one more graph " << ++cptInflating << endl;
                }
            }
        }
    }
    cerr << "inflated in total " << cptInflating << " graphs\n";
    vector<Graph> magicList;
    magicList.reserve(1000);
    for (const Graph& g : listGraphs)
    {
        if (is_magic_graph(g, false, &deglists2MagicGraphs))
        {
            magicList.push_back(g);
        }
    }

    /*
    //TTAADDAA : une fonction pour lancer puis join des fonctions avec liste d'arguments fixée ?
    long long nbPerProc = listGraphs.size()/nbProc;
    mutex threadMutex;
    vector<thread> threads(nbProc-1);
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc] = thread(&gen_fixeurs_thread, nbVert, std::cref(listGraphs), isTwinCompat, std::ref(fixeursList), std::cref(deglist2PrefixeursPlus), std::ref(threadMutex), iProc);

    thread lastProc = thread(&gen_fixeurs_thread, nbVert, std::cref(listGraphs), isTwinCompat, std::ref(fixeursList), std::cref(deglist2PrefixeursPlus), std::ref(threadMutex), nbProc-1);

    lastProc.join();
    for (int iProc = 0; iProc < nbProc-1; iProc++)
        threads[iProc].join();
    */

    cout  << "Il y a " << magicList.size() << " fixeurs à " << nbVert << " sommets.\n";
    string magicGenFileName = "Alexmagicdelataille"+to_string(nbVert)+".txt.gz";

    ogzstream outFile(magicGenFileName.c_str());
    outFile << magicList.size() << endl;
    for (const Graph &g : magicList)
        g.print_in_file(outFile);
    outFile.close();

    return deglists2MagicGraphs[nbVert];

}


