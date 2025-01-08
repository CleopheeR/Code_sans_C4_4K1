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

// Faire fusion d'abord tous ceux avec A, puis tous ceux avec B
// Programme qui regarde et qui fait le tableau, avec ou sans les N, sans fusionner. Mais sans, sans C4/4K1


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

bool ProblemArray::is_true_badTriple_4sets(const ProblemArraySet &setA, const ProblemArraySet &setB, const ProblemArraySet &setNA, const ProblemArraySet &setNB) const
{
    assert(!setA.is_advanced() && !setB.is_advanced() && !setNA.is_advanced() && !setNB.is_advanced());

    int n0 = baseGraph.nbVert;

    int uA = n0, uB = n0+1, uNA = n0+2, uNB = n0+3;
    const ProblemArraySet* sets[3] = {&setA, &setB, &setNA};
    Graph gAB = add_vertices_to_graph(baseGraph, sets, 3);
    // Mandatory edge here because A and B were merged, so they are complete to each other.
    gAB.add_edge(uA, uB);

    vector<Graph> realGAB_NAList;
    Graph fooG = gAB;
    get_possible_free_neighbourhoods(uNA, {uA, uB}, fooG, 0, realGAB_NAList);

    //cout << "öhhhhhh"<<endl;

    const ProblemArraySet* setBis[1] = {&setNB};
    vector<Graph> realGAB_NA_NBList;
    for (Graph &g : realGAB_NAList)
    {
        Graph fooGG = add_vertices_to_graph(g, setBis, 1);
        //g.print();
        get_possible_free_neighbourhoods(uNB, {uA,uB, uNA}, fooGG, 0, realGAB_NA_NBList);
    }

    int nbLinkNANB0 = 0, nbLinkNANB1 = 0; // number of valid extensions when there is (/no) link between NA and NB.
    for (const Graph &g : realGAB_NA_NBList)
    {
        if (are_neighb(g, uNA, uNB))
            nbLinkNANB1++;
        else
            nbLinkNANB0++;
    }

    // TODO vérifier que vraiment ça marche
    // TODO si 3 et 3 et les mêmes...
    //if ((calcLinkBC > 1 && calcLinkBC % 2 == 1) || realGABCList.size() > 2)
    if ((nbLinkNANB0 > 2 && nbLinkNANB1 > 2) || nbLinkNANB0 == 4 || nbLinkNANB1 == 4)
    {
        cout << "Nb links = " << nbLinkNANB0 << "," << nbLinkNANB1 << endl;
        //cout << "end" <<endl;
        //cerr << "#"<< endl;
        return true;
    }



    //realGABCList[0].print();

    //cerr << "@\n";
    return false;
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

    //cout << "öhhhhhh"<<endl;

    vector<Graph> realGABCList;
    for (Graph &g : realGABList)
    {
        const ProblemArraySet* setBis[1] = {&setC};
        Graph fooGG = add_vertices_to_graph(g, setBis, 1);
        //g.print();
        get_possible_free_neighbourhoods(uC, {uA,uB}, fooGG, 0, realGABCList);
    }

    int nbA1B = 0, nbA0B = 0, nbA1C = 0, nbA0C = 0;
    int nbB1C_A0B = 0, nbB1C_A1B = 0, nbB1C_A0C = 0, nbB1C_A1C = 0;
    int nbB0C_A0B = 0, nbB0C_A1B = 0, nbB0C_A0C = 0, nbB0C_A1C = 0;
    int nbLinkBC0 = 0, nbLinkBC1 = 0; // number of valid extensions when there is (/no) link between B and C.
    for (const Graph &g : realGABCList)
    {/*
        if (are_neighb(g, uB, uC))
        {
            if (are_neighb(g, uA, uB))
                nbB1C_A1B++;
            else
                nbB1C_A0B++;

            if (are_neighb(g, uA, uC))
                nbB1C_A1C++;
            else
                nbB1C_A0C++;
        }

        else // uB 0 uC
        {
            if (are_neighb(g, uA, uB))
                nbB0C_A1B++;
            else
                nbB0C_A0B++;

            if (are_neighb(g, uA, uC))
                nbB0C_A1C++;
            else
                nbB0C_A0C++;
        }*/
        if (are_neighb(g, uB, uC))
            nbLinkBC1++;
        else
            nbLinkBC0++;
    }

    // TODO maybe possible to have more good cases
    //if ((calcLinkBC > 1 && calcLinkBC % 2 == 1) || realGABCList.size() > 2)
    if (nbLinkBC0 > 2 || nbLinkBC1 > 2)
    {
        //cout << "end" <<endl;
        //cerr << "#"<< endl;
        return true;
    }



    //realGABCList[0].print();

    //cerr << "@\n";
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
bool ProblemArray::can_NN_be_solved_method2(const vector<array<int, 3>> &badTriplets) const
{
    string merging1Log = "MergingPhaseOne:\n", merging2Log = "mergingPhaseTwo:\n";
    int nbError = 0;
    int nbSet = partitionSets.size();
    vector<pair<int, int>> badTriplet(nbSet);
    std::vector<std::vector<int>> toMerge(nbSet);

    vector<vector<int>> setsToNs(nbSet);

    std::vector<int> unionfind(nbSet);
    for (int i = 0; i < nbSet; i++)
      unionfind[i] = i;
    std::vector<std::vector<int>> ufSets(nbSet);
    for (int i = 0; i < nbSet; i++)
      ufSets[i].push_back(i);

    // TODO take as argument to avoid again double loop
    for (int i = 0; i < nbSet; i++)
    {
        for (int j = 0; j < nbSet; j++)
        {
            if (partitionArray[i][j] == 'N')
                setsToNs[i].push_back(j);
        }
    }

    for (int i = 0; i < badTriplets.size(); i++)
    {
        int i1 = badTriplets[i][0], i2 = badTriplets[i][1], i3 = badTriplets[i][2];
        if (partitionArray[i1][i2] != '1')
            return false;
        {
            int repr1 = find(i1, unionfind);
            int repr2 = find(i2, unionfind);
            if (repr1 == repr2)
                continue;
            unionfind[repr1] = repr2;
            vector<int> &set1 = ufSets[repr1];
            vector<int> &set2 = ufSets[repr2];
            set2.insert(set2.end(), set1.begin(), set1.end());
            set1.clear();
            toMerge[i1].push_back(i2);
            toMerge[i2].push_back(i1);
            //cout << "Triplet bad: " << (char)('A'+i) << (char)('A'+i1) << (char)('A'+i2) << endl;
        }
    }

    string mergingLog;
    for (int i = 0; i < nbSet; i++)
    {
        if (ufSets[i].size() <= 1)
            continue;
        cerr << "Merging: ";
        for (int i1 = 0; i1 < ufSets[i].size(); i1++)
        {
            int x1 = ufSets[i][i1];
            cerr << 'A'+x1 << ", ";
            for (int i2 = i1+1; i2 < ufSets[i].size(); i2++)
            {
                int x2 = ufSets[i][i2];
                if (partitionArray[i1][i2] != '-' && partitionArray[i1][i2] != '1')
                {
                    cerr << "Problem merging: I want to merge " << 'A'+x1 << " and " << 'A'+x2 << " but their relation is " << partitionArray[i1][i2] << endl;
                    return false;
                }
            }
        /*mergingLog += "Merging : ";
        for (int x : toMerge[i])
            mergingLog = mergingLog + to_string('A'+i) + " ";
        mergingLog += "\n";*/
        }
        cerr << "\n";
    }

    // Checking no new bad pairs... (useless?)
    for (int i = 0; i < nbSet; i++)
    {
        if (ufSets[i].size() <= 1)
            continue;
        int idRepr = -1;
        for (int idSet : ufSets[i])
        {
            for (int idN : setsToNs[idSet])
            {
                assert(idRepr == -1 || idRepr ==  find(idN, unionfind));
                idRepr = find(idN, unionfind);
            }
        }
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
    //g.print();
    //cout << "\n\n";
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

vector<array<int, 3>> ProblemArray::find_bad_triplets(void) const
{
    vector<array<int, 3>> badTriplets;

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
                //partitionArray[i1][i2] = 'S';
                cerr << " LOL12\n";
                continue;
            } //Cannot happen :(
            for (int i3 = i2+1; i3 < nbSet; i3++)
            {
               if (partitionArray[i1][i3] != 'N')
                    continue;

                if (partitionArray[i2][i3] == '-')
                    continue;

                const ProblemArraySet &set3 = partitionSets[i3];
                if (!is_true_N_between_two(set1, set3))
                {
                    cout << " ptdr\n";
                    continue;
                } //Cannot happen

                if (!can_3sets_be_possible(set1, set2, set3))
                {
                    if (baseGraph.nbVert <= 13)
                        cerr << "false bad 3 sets = "<< (char)('A'+i1) << ","<< (char)('A'+i2)  << "," << (char)('A'+i3) << endl;
                    continue;
                }

                badTriplets.push_back({i1, i2, i3});

            }
        }
    }
    return badTriplets;
}

vector<array<int, 3>> ProblemArray::solve_bad_triplets(const vector<array<int, 3>> &badTriplets) const
{
    //TODO important si je fusionne BC pour A, mais B avait N U et C avait N V alors BC N U et N V souci
    if (baseGraph.nbVert <= 13)
        cerr << "--------\n";
    //TODO si jamais on doit fusionner BCDE pour A, peut-être que si BC N, ABC pas BC N.
    //string merging1Log = "MergingPhaseOne:\n", merging2Log = "mergingPhaseTwo:\n";
    int nbError = 0;
    vector<array<int, 3>> stillBadTriplets;
    int nbSet = partitionSets.size();
    vector<pair<int, int>> badTriplet(nbSet);
    vector<int> setToUniqueNSetId(nbSet,-1);

    vector<vector<int>> setsToNs(nbSet);
    for (int i = 0; i < nbSet; i++)
    {
        for (int i2 = 0; i2 < nbSet; i2++)
            if (partitionArray[i][i2] == 'N')
                setsToNs[i].push_back(i2);
    }

    std::vector<int> unionfind(nbSet);
    for (int i = 0; i < nbSet; i++)
      unionfind[i] = i;
    std::vector<std::set<int>> ufSets(nbSet);
    for (int i = 0; i < nbSet; i++)
      ufSets[i].insert(i);


    vector<pair<int, int>> problematicDoubleNs;

    for (const auto &triplet : badTriplets)
    {
        cerr << "Cur triplet: "<< intToSetName(triplet[0]) << intToSetName(triplet[1]) << intToSetName(triplet[2])<<endl;
        int curNbError = 0;
        int i1 = triplet[0], i2 = triplet[1], i3 = triplet[2];
        int repr2 = find(i2, unionfind);
        int repr3 = find(i3, unionfind);
        if (repr2 == repr3)
            continue;
        set<int> &toMerge2 = ufSets[repr2];
        set<int> &toMerge3 = ufSets[repr3];
        cerr << "Trying to merge:\n";
        if (baseGraph.nbVert <= 13)
        {
            for (int x : toMerge2)
                cerr << intToSetName(x) << ",";
            cerr << "  AND  ";
            for (int x : toMerge3)
                cerr << intToSetName(x) << ", ";
            cerr << endl;
        }

        for (int ii2 : toMerge2)
            for (int ii3 : toMerge3)
                if (partitionArray[ii2][ii3] != '1' && partitionArray[ii2][ii3] != '-')
                    curNbError++;


        //if (nbTimesGood)
        //    std::cout << "nbTimesGood = " << nbTimesGood << endl;
        if (curNbError == 0)
        {
            if (baseGraph.nbVert <= 13)
            {
                cerr << "Merging: ";
                for (int x : toMerge2)
                    cerr << intToSetName(x) << ",";
                cerr << "  AND  ";
                for (int x : toMerge3)
                    cerr << intToSetName(x) << ", ";
                cerr << endl;
            }
            setToUniqueNSetId[i2] = i1;
            setToUniqueNSetId[i3] = i1;
            setToUniqueNSetId[i1] = i3;
            unionfind[repr2] = repr3;
            toMerge3.merge(toMerge2);
            toMerge2.clear();
        }
        else
        {
            stillBadTriplets.push_back(triplet);
            nbError++;
        }


   // Checking that non-checked N do not pose problems. For instance, if A is N with B and C but this is a bad triplet, we never check problem related to B and C when merging. The same goes if A is N with only B: when merging A with E, we may create bad triplets.
        int nbTimesGood = 0;
        for (int i2 : toMerge2)
        {
            if (curNbError)
                break;
            for (int i3 : toMerge3)
            {
                if (curNbError)
                    break;
                for (int iN2 : setsToNs[i2])
                {
                    if (curNbError)
                        break;
                    for (int iN3 : setsToNs[i3])
                    {
                        if (is_true_badTriple_4sets(partitionSets[i2], partitionSets[i3], partitionSets[iN2], partitionSets[iN3]))
                        {
                            problematicDoubleNs.push_back({iN2, iN3});
                            //curNbError++;
                            //std::cerr << "Error advanced merged for: " << intToSetName(iN2) <<"," << intToSetName(iN3) << ".\n";
                            //break;
                        }
                        else
                            nbTimesGood++;
                    }
                }
            }
        }
    }

    for (const pair<int, int> &pbPair : problematicDoubleNs)
    {
        int i1 = pbPair.first, i2 = pbPair.second;
        if (find(i1, unionfind) != find(i2, unionfind))
            stillBadTriplets.push_back({-1, i1, i2});
    }
    return stillBadTriplets;
}


//TODO free vertices for base graph
bool is_magic_graph(const Graph &g, bool special, mutex &lock, vector<sparse_hash_map<vector<char>, vector<Graph>>> *deglist2ObstructionsBySize, int idThread)
{
    //std::cout << "-----\n\n";
    ProblemArray pbArray;
    pbArray.idThread = idThread;
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
    //pbArray.print_array();
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

    vector<std::array<int, 3>> errorTriplets = pbArray.find_bad_triplets();

    //vector<std::array<int, 3>> finalErrors = pbArray.solve_bad_triplets(errorTriplets);
    //cout << "------------------------------\n";
    //cout << "il y a " << errorTriplets.size() << " vrais soucis\n";

    bool isOk2 = pbArray.can_NN_be_solved_method2(errorTriplets);

    //if (isOk2)
    //    errorTriplets.clear();
    if (false || errorTriplets.size() <=30 || isOk2)
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
        /*if (false && finalErrors.empty() && g.nbVert <= 13)
          pbArray.print_array_latex();


        cerr << "Printing bad triplets :";
        for (const auto &x : finalErrors)
            cerr << intToSetName(x[0]) << intToSetName(x[1]) << intToSetName(x[2]) << ", ";
        cout << endl << endl;
        cout << "Il y a " << finalErrors.size() << " bad triplets\n";
        */
        lock.unlock();
    }
    //if (finalErrors.empty())
    if (isOk2)
        return true;
    //if (errorTriplets.size() != 0)
    //    return false;


    return false;
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

    for (int i = 1; i < nbVert+4; i++)
    {
        int cptInflating = 0;
        cerr << "Trying to inflate size " << i << endl;
        Graph gBigger;
        int puissNewVert = (1<< i);
        vector<char> hashVect(i+5); //TODO parallelise?
        vector<Graph> listMinus;
        int degMin = 1000000000, degMax = 0;
        for (const pair<const vector<char>, vector<Graph>>& dToGraphs : deglists2MagicGraphs[i])
        {
            for (const Graph &g : dToGraphs.second)
            {
                listMinus.push_back(g);

                degMin = min(degMin, g.nbEdge);
                degMax = max(degMax, g.nbEdge);
            }
        }
        if (listMinus.empty())
            continue;

        vector<int> degreesToDo;
        degreesToDo.reserve(degMax-degMin+1+nbVert);
        int moy = (degMax+degMin+nbVert)/2;
        degreesToDo.push_back(moy);
        for (int j = 1; ; j++)
        {
            int d1 = moy-j;
            int d2 = moy+j;

            if (d1 >= degMin)
                degreesToDo.push_back(d1);
            if (d2 <= degMax+nbVert-1)
                degreesToDo.push_back(d2);
            //degreesToDo.push_back(j); TTAADDAA => what ?!
            if (d1 < degMin && d2 > degMax+nbVert-1)
                break;
        }

        vector<Graph> fooEmpty;
        ogzstream outFileBis("/tmp/toto");

        initialise_subsetBySize(i+1);
        cout << "Found " << listMinus.size() << " smaller graphs" << endl;
        cout << "We have " << nbProc << " threads yeah" << endl;
        vector<thread> threads(nbProc);


        sparse_hash_map<vector<char>, vector<Graph>> *ptrThreadCall = &deglists2MagicGraphs[i+1];
        for (int iProc = 0; iProc < nbProc; iProc++)
            threads[iProc] = thread(&gen_graphs_thread, std::ref(listMinus), std::ref(fooEmpty), nullptr, std::ref(degreesToDo), std::ref(outFileBis), iProc, std::ref(threadMutex), ptrThreadCall, true);
        for (int iProc = 0; iProc < nbProc; iProc++)
            threads[iProc].join();


        for (const auto & dToGraphs:deglists2MagicGraphs[i+1])
            cptInflating += dToGraphs.second.size();
        cerr << "inflated in total " << cptInflating << " graphs of size" << i+1 << "\n";
        cptInflatingTotal += cptInflating;
    }
    cerr << "inflated in total " << cptInflatingTotal << " graphs\n";
    vector<Graph> magicList;
    magicList.reserve(1000);

    long long nbPerProc = listGraphs.size()/nbProc;
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
