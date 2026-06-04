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

//TODO:
//   1. Écrire dans un fichier inflated-size.tgz les magiques et inflatés.
//      => Quand on calcule des graphes magiques. Voir, pour chaque taille tq il existait le fichier inflated, si on en génère des nouveaux. Si oui, les écrire à la fin. Partir de ceux-ci pour ajouter à chaque taille supérieure existante (supposer que les inflated sont synchros).
//
//   2. Paralléliser la création des inflated :
//      => ajouter paramètres à gen_graphs_threads : rajouter deglisttotrucs en param, un pointeur. Si pointeur pas null, à la place d'écrire dans ogzstream, locker mutex et écrire dans la map. Aussi, ajouter params pour twins autorisés ou non, maybe pour connexe ou non


using namespace std;

inline string intToSetName (int x)
{
    string s;
    s.push_back((char)('A'+x));
    return s;
}


Graph ProblemArray::add_vertices_to_graph(const Graph &g, const ProblemArraySet* adjVertsToAdd[], int nbSet) const
{
    int n0 = g.nbVert;
    Graph gRet;
    gRet.init(n0+nbSet, g.nbEdge);
    for (int u = 0; u < n0; u++)
        gRet.adjMat[u] = g.adjMat[u];

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

    const ProblemArraySet* sets[2] = {&setA, &setB};
    Graph gAB = add_vertices_to_graph(baseGraph, sets, 2);

    int uA = n0, uB = n0+1, uC=  n0+2;
    vector<Graph> realGABList;
    Graph fooG = gAB;
    get_possible_free_neighbourhoods(uB, {uA}, fooG, 0, realGABList);


    vector<Graph> realGABCList;
    for (Graph &g : realGABList)
    {
        const ProblemArraySet* setBis[1] = {&setC};
        Graph fooGG = add_vertices_to_graph(g, setBis, 1);
        //g.print();
        get_possible_free_neighbourhoods(uC, {uA,uB}, fooGG, 0, realGABCList);
    }
    if (realGABCList.size() > 1)
    {
        return true;
    }

    return false;
}


bool ProblemArray::is_true_N_between_two(const ProblemArraySet &setA, const ProblemArraySet &setB) const
{
    assert(!setA.is_advanced() && !setB.is_advanced());
    int n0 = baseGraph.nbVert;
    const ProblemArraySet* sets[3] = {&setB, &setB, &setA};
    Graph gAB = add_vertices_to_graph(baseGraph, sets, 3);
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


int find(int x, std::vector<int> &uf)
{
    if (uf[x] != x)
        uf[x] = find(uf[x], uf);
    return uf[x];
}


// New version
bool ProblemArray::can_NN_be_solved_method2(void)
{
    string merging1Log = "MergingPhaseOne:\n", merging2Log = "mergingPhaseTwo:\n";
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
            {
                partitionArray[i][i1] = 'F';
                partitionArray[i1][i] = 'F';
                continue;
            }
            for (int i2 = i1+1; i2 < nbSet; i2++)
            {
                if (partitionArray[i1][i2] == '-')
                  continue;
                if (partitionArray[i][i2] != 'N')
                    continue;
                if (!is_true_N_between_two(partitionSets[i], partitionSets[i2]))
                {
                    partitionArray[i][i2] = 'F';
                    partitionArray[i2][i] = 'F';
                    continue;
                }
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

                    for (int x : set1)
                        set2.insert(x);
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
    int cptBad = 0;
    for (int i = 0; i < nbSet; i++)
    {
        const set<int> &curSet = ufSets[i];
        if (curSet.size() <= 1)
            continue;
        vector<int> v(curSet.begin(), curSet.end());
        for (int i1 = 0; i1 < v.size(); i1++)
        {
            int x1 = v[i1];
            for (int i2 = i1+1; i2 < v.size(); i2++)
            {
                int x2 = v[i2];
                if (partitionArray[x1][x2] != '1' && partitionArray[x1][x2] != '-')
                {
                    /*
                    mergingLog+= "\tCannot merge ";
                    mergingLog += (char)('A'+x1);
                    mergingLog += " and ";
                    mergingLog += (char) ('A'+x2);
                    mergingLog += "\n";
                    */
                    cptBad++;
                    //if (cptBad > 20)
                    //    return false;
                }
            }
        }
        mergingLog += "Merging : ";
        for (int x : v)
            mergingLog = mergingLog + (char)('A'+x) + " ";
        mergingLog += "\n";
    }

    /*if (cptBad)
    {
        cout << cptBad << " non-mergeable bad triplets\n";
        print_array();
    }*/
    //cout << mergingLog << endl;

    if (cptBad == 0)
        cout << mergingLog;
    return cptBad == 0;
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

//TODO testing 2^n sets but in fact only 100 or less are valid
void ProblemArray::gen_default_partition(void)
{
    int nbVert = baseGraph.nbVert+1;
    int puissNewVert = (1<< (nbVert-1));
    sparse_hash_map<vector<char>, vector<Graph>> fooObstructions;

    assert(!has_twin(baseGraph));
    Graph gWithEdges;
    gWithEdges.init(baseGraph.nbVert+1, baseGraph.nbEdge);

    vector<long long> pathLength2;
    pathLength2.reserve(NBMAXVERT);
    gen_P2_list(baseGraph, pathLength2, nbVert);

    vector<int> indepSize3;
    indepSize3.reserve(NBMAXVERT);
    gen_O3_list(baseGraph, indepSize3, nbVert-1);

    for (int code = 0; code < puissNewVert; code++)
    {
        bool refuseBecauseC4O4 = detect_C4(pathLength2, code) || detect_O4(indepSize3, code);
        if (refuseBecauseC4O4)
            continue;

        const vector<int> &newEdgesList = adjListGlobal[code];
        //gWithEdges.copy_and_add_new_vertex_noalloc(baseGraph, newEdgesList, puissNewVert, code);
        //Graph gWithEdges;
        gWithEdges.copy_and_add_new_vertex_bis(baseGraph, newEdgesList, puissNewVert, code);

        //TODO precompute stuff? for Graph C4
        if (is_graph_ok_notestC4O4(gWithEdges, false))
        {
            ProblemArraySet newSet;
            newSet.id = partitionSets.size();
            newSet.neighbInBaseGraph = adjListGlobal[code];
            partitionSets.push_back(newSet);
        }
    }
}

bool ProblemArray::is_graph_ok_notestC4O4(const Graph &g, bool print) const
{
    if (!free_O4(g, g.nbVert))
        return false;
    //if (!free_O4(g, g.nbVert))
    //    return false;

    if (deglist2ObstructionsBySize == NULL)
        return true;
    int n = g.nbVert;
    const vector<sparse_hash_map<vector<char>, vector<Graph>>> &obstructions = *deglist2ObstructionsBySize;

    //TODO disable if no inflate
    vector<char> hashVect(n+4);
    Graph gg = g;
    gg.compute_hashes(hashVect);
    sort(hashVect.begin(), hashVect.begin()+n);


    const sparse_hash_map<vector<char>, vector<Graph>> &curObstructions = obstructions[n];

    const sparse_hash_map<vector<char>, vector<Graph> >::const_iterator graphsIter = curObstructions.find(hashVect);

    if (graphsIter == curObstructions.cend())
        return true;

    for (const Graph &gObstr : graphsIter->second)
        if (are_isomorphic(gg, gObstr, idThread))//TODO idthread
            return false;

    return true;
}

bool ProblemArray::is_graph_ok(const Graph &g, bool print) const
{
    if (!free_C4_O4(g, g.nbVert))
        return false;

    if (deglist2ObstructionsBySize == NULL)
        return true;
    int n = g.nbVert;
    vector<sparse_hash_map<vector<char>, vector<Graph>>> &obstructions = *deglist2ObstructionsBySize;

    //TODO disable if no inflate
    vector<char> hashVect(n+4);
    Graph gg = g;
    gg.compute_hashes(hashVect);
    sort(hashVect.begin(), hashVect.begin()+n); //TODO and noC4O4 problem when not sorted, find that yyyyyyyyyyyy

    sparse_hash_map<vector<char>, vector<Graph>> &curObstructions = obstructions[n];
    if (curObstructions.find(hashVect) == curObstructions.end())
        return true;

    for (const Graph &gObstr : curObstructions[hashVect])
        if (are_isomorphic(gg, gObstr, idThread))//TODO idthread
            return false;

    return true;
}


//TODO compute given constraints on forced neighbours and so on in the sets
void ProblemArray::compute_partition_array(void)
{
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
    Graph gNew = add_vertices_to_graph(baseGraph, sets, 2);

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
    Graph gPlus2 = add_vertices_to_graph(baseGraph, sets, 2);

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
            /*if (!is_true_N_between_two(set1, set2))
            {
                partitionArray[i1][i2] = 'S';
                cerr << " LOL12\n";
                continue;
            }*/ //Cannot happen :(
            for (int i3 = i2+1; i3 < nbSet; i3++)
            {
               if (partitionArray[i1][i3] != 'N')
                    continue;

                if (partitionArray[i2][i3] == '-')
                {
                  //cout << "No coexisting triplet: " << tripletName << endl;
                    //cout << "xD\n";
                    continue;
                }

                /*if (!is_true_N_between_two(set1, set3))
                {
                    cout << " ptdr\n";
                    continue;
                }*/ //Cannot happen


                const ProblemArraySet &set3 = partitionSets[i3];
                if (!can_3sets_be_possible(set1, set2, set3))
                {
                    cout << "false bad 3 sets = "<< (char)('A'+i1) << ","<< (char)('A'+i2)  << "," << (char)('A'+i3) << endl;
                    //cout << " mdr \n";
                    continue;
                } //cannot happen

                string tripletName;
                for (int x : {i1,i2,i3})
                    tripletName.push_back((char)('A'+x));


                if (partitionArray[i2][i3] == '1')
                {
                    //cout << " Mergeable triple: " << tripletName << endl;
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
                    //disabled continue; // TODO WARNING EXPERIMENTAL!!!
                }

                //cout << "Non Mergeable triple: " << tripletName << endl;
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
bool is_magic_graph(const Graph &g, bool special, mutex &lock, vector<sparse_hash_map<vector<char>, vector<Graph>>> *deglist2ObstructionsBySize, int idThread)
{
    //std::cout << "-----\n\n";
    ProblemArray pbArray;
    pbArray.idThread = idThread;
    pbArray.baseGraph = g;
    //pbArray.deglist2ObstructionsBySize = deglist2ObstructionsBySize; //XXX


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
    /*
    const vector<vector<char>> &tableau = pbArray.partitionArray;
    int nbSet = pbArray.partitionSets.size();
    pbArray.print_array();

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

    //TODO remove since can_NN_be_solves_method2 is more powerful (+make it return bad triplets)
    //vector<string> errorTriplets = pbArray.solve_array_problems();

    //cout << "------------------------------\n";
    //cout << "il y a " << errorTriplets.size() << " vrais soucis\n";

    bool isOk2 = pbArray.can_NN_be_solved_method2();
    //if (isOk2 || errorTriplets.size() <= 20)
    if (isOk2)
    {
        lock.lock();
        cerr << "printing graph:\n";
        g.print();
        cerr << "printing neighbourfood of vertices:\n";
        int n = pbArray.partitionSets.size();
        std::cerr << "il y a " << n << " sets dans la partition.\n";
        for (int i = 0; i < n; i++)
        {
            char name = i+'A';
            cerr << "set " << name << ": ";
            for (int x : pbArray.partitionSets[i].neighbInBaseGraph)
                cerr << x << ", ";
            cerr << endl;
        }
        pbArray.print_array();
        if (false && isOk2 && g.nbVert <= 13)
          pbArray.print_array_latex();


        /*cerr << "Printing bad triplets :";
        for (string &x : errorTriplets)
            cerr << x << ", ";
        cout << endl << endl;
        cout << "Il y a " << errorTriplets.size() << " bad triplets\n";
        */
        lock.unlock();
    }
    if (isOk2)
        return true;

    return false;
}

sparse_hash_map<vector<char>, vector<Graph>> gen_magic_graphs(int nbVert)
{
    //TTAADDAA documenter variables, plus parce que taille au dessus
    vector<sparse_hash_map<vector<char>, vector<Graph>>> deglists2MagicGraphs(NBMAXVERT);
    vector<vector<char>> degreeLists(NBMAXVERT);
    for (int i = 0; i < nbVert; i++)
        degreeLists[i].resize(i+4);



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

    for (int i = 1; i < nbVert+4; i++)
    {
        //if (i == nbVert)
        //    continue;
        string fileNamee = "Alexmagicdelataille"+to_string(i)+".txt.gz";
        cerr << fileNamee << " is my file " << endl;
        read_prefixeurs_compute_hash(fileNamee, i ,deglists2MagicGraphs[i]);
    }

    //vector<sparse_hash_map<vector<char>, vector<Graph>>> deglists2MagicGraphs(NBMAXVERT);
    int cptInflatingTotal = 0;
    vector<long long> pathLength2(NBMAXVERT);
    std::mutex threadMutex;
    vector<mutex> threadMutexes(256*256);

    ogzstream fGraph("Alextestmagicisom.txt.gz");
    for (int i = 100; i < nbVert+4; i++)
    //for (int i = 1; i < nbVert+4; i++)
    {
        int cptInflating = 0;
        cerr << "Trying to inflate size " << i << endl;
        Graph gBigger;
        int puissNewVert = (1<< i);
        vector<char> hashVect(i+5); //TODO parallelise?
        vector<Graph> listMinus;
        for (const pair<const vector<char>, vector<Graph>>& dToGraphs : deglists2MagicGraphs[i])
            for (const Graph &g : dToGraphs.second)
                listMinus.push_back(g);
        if (listMinus.empty())
            continue;

        vector<int> degreesToDo;
        //degreesToDo.reserve(degMax-degMin+1+nbVert);

        vector<Graph> fooEmpty;
        ogzstream outFileBis("/tmp/toto");

        cout << "Found " << listMinus.size() << " smaller graphs" << endl;
        cout << "We have " << nbProc << " threads yeah" << endl;
        vector<thread> threads(nbProc);

        long long nbMinus = listMinus.size();
        long long step = (nbMinus/STEP_RATIO+1);
        long long nbBatch = nbMinus/max(step, 1ll);
        vector<pair<long long, long long>> indicesToDo(nbBatch+1);
        for (long long i = 0; i <= nbBatch; i++)
        {
            indicesToDo[i] = {i*step, min(nbMinus,(i+1)*step)};
        }

    /*Temporaire pour test...
    bool isTwin[NBMAXVERT];
    bool isInList[NBMAXVERT];*/
    int **isTwinCompat = NULL;
    isTwinCompat = (int**) malloc(sizeof(*isTwinCompat)*puissNewVert);
    for (int i = 0; i < puissNewVert; i++)
        isTwinCompat[i] = (int*) malloc(sizeof(*isTwinCompat)*NBMAXVERT);



    //TODO attention pas symmétrique là.
    for (int code = 0; code < puissNewVert; code++)
    {
        for (int v1 = 0; v1 < nbVert-2; v1++)
        {
            int curCompat = 0;
            if (code & (1<<v1))
            {
                isTwinCompat[code][v1] = 0;
                continue;
            }

            for (int v2 = v1+1; v2 < nbVert-1; v2++)
            {
                if (code & (1<<v2))
                    curCompat ^= (1<<v2);
            }
            isTwinCompat[code][v1] = curCompat;
        }
    }
    //Fin temporaire




        vector<sparse_hash_map<vector<char>, vector<Graph>>> curDeglist2Magic(max(256*256,i*(i+1)/2));
        sparse_hash_map<vector<char>, vector<Graph>> *ptrThreadCall = &curDeglist2Magic[0];
        for (int iProc = 0; iProc < nbProc; iProc++)
            threads[iProc] = thread(&gen_graphs_thread, std::ref(listMinus), std::ref(fooEmpty), isTwinCompat, std::ref(indicesToDo), std::ref(outFileBis), iProc, std::ref(threadMutexes), std::ref(threadMutex), ptrThreadCall, false&&true);
        for (int iProc = 0; iProc < nbProc; iProc++)
            threads[iProc].join();

        /* For tests if something goes wrong.
        vector<pair<Graph, vector<char>>> seenCur;
        vector<char> degreeListFoo(70);
        for (const auto & dToGraphs:deglists2MagicGraphs[i+1])
        {
            for (const Graph& ggg : dToGraphs.second)
            {
                Graph totoG = ggg;
                totoG.print();
                totoG.compute_hashes(degreeListFoo);
                seenCur.push_back({totoG, dToGraphs.first});
            }
        }
        for (int i1 = 0; i1 < seenCur.size(); i1++)
        {
            seenCur[i1].first.print_in_file(fGraph);

            if (!free_C4_O4(seenCur[i1].first, seenCur[i1].first.nbVert))
                cout << "HAS C4 or O4\n" << endl;
            for (int i2 = i1+1; i2 < seenCur.size(); i2++)
                if (are_isomorphic(seenCur[i1].first, seenCur[i2].first, 0))
                {
                    cout << "ERROR ISOM\n";
                    cout << "\t";
                    for (int x : seenCur[i1].second)
                        cout << x << " ";
                    cout << "\n\t";
                    for (int x : seenCur[i2].second)
                        cout << x << " ";
                    cout << endl;
                }
        }
        cout << "Real size = " << seenCur.size() << "\n";
        */
        for (auto &toto : curDeglist2Magic)
        {
            for (auto &x : toto)
            {
                swap(x.second, deglists2MagicGraphs[i+1][x.first]);
            }
        }

        for (const auto & dToGraphs:deglists2MagicGraphs[i+1])
            cptInflating += dToGraphs.second.size();
        cerr << "inflated in total " << cptInflating << " graphs of size" << i+1 << "\n";
        cptInflatingTotal += cptInflating;

        for (int i = 0; i < puissNewVert; i++)
            free(isTwinCompat[i]);
        free(isTwinCompat);
    }
    cerr << "inflated in TOTAL " << cptInflatingTotal << " graphs\n";
    fGraph.close();

    vector<Graph> magicList;
    magicList.reserve(1000);

    mutex threadMutexMagic;
    vector<thread> threads(nbProc);
    for (int iProc = 0; iProc < nbProc; iProc++)
        threads[iProc] = thread(&is_magic_graph_thread, std::ref(magicList), std::ref(threadMutexMagic), std::cref(listGraphs), false, &deglists2MagicGraphs, iProc);


    for (int iProc = 0; iProc < nbProc; iProc++)
        threads[iProc].join();

    cout  << "Il y a " << magicList.size() << " fixeurs à " << nbVert << " sommets.\n";
    string magicGenFileName = "Alexmagicdelataille"+to_string(nbVert)+".txt.gz";

    ogzstream outFile(magicGenFileName.c_str());
    outFile << magicList.size() << endl;
    for (const Graph &g : magicList)
        g.print_in_file(outFile);
    outFile.close();

    return deglists2MagicGraphs[nbVert];
}

  //TODO join all
        //
        //
/*
        //#pragma omp parallel
        for (const pair<const vector<char>, vector<Graph>>& dToGraphs : deglists2MagicGraphs[i])
        {
            //int tid = omp_get_thread_num();
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
    }*/


void is_magic_graph_thread(vector<Graph> &magicListToFill, mutex &lock, const vector<Graph> &graphList, bool special, vector<sparse_hash_map<vector<char>, vector<Graph>>> *deglist2Obstruction, int idThread)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(idThread+1, &cpuset);
    assert(sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0);


    long long nbGToDo = graphList.size();
    int pctDone = -1, cpt = 0;
    int onePercent = max(1ll, nbGToDo/(nbProc*100));
    for (long long iG = idThread; iG < nbGToDo; iG += nbProc)
    {
        if (cpt % onePercent == 0)
        {
            pctDone++;
            string newName = "Magic: " + to_string(pctDone)+"%";
            pthread_setname_np(pthread_self(), newName.c_str());
        }
        cpt++;
        const Graph &g = graphList[iG];
        if (is_magic_graph(g, special, lock, deglist2Obstruction, idThread))
        {
            lock.lock();
            magicListToFill.push_back(g);
            lock.unlock();
        }
    }
}
