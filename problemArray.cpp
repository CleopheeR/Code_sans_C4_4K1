#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>

#include "sparsepp/spp.h"
#include "gzstream/gzstream.h"
#include "Graph.hh"
#include "problemArray.hh"
#include "test-properties.hh"

using namespace std;

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
    int n0 = baseGraph.nbVert;
    const ProblemArraySet* sets[3] = {&setB, &setB, &setA};
    Graph gAB = add_vertices_to_base_graph(sets, 3);
    int uB1 = n0, uB2 = n0+1, uA = n0+2;
    gAB.add_edge(uB1, uB2);

    gAB.add_edge(uA, uB1);
    return is_graph_ok(gAB, false);
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

    for (int size = 5; sizeMax; size++)
    {
        sparse_hash_map<vector<char>, vector<Graph>> &curObstructions = obstructions[size];
        if (curObstructions.empty())
            continue;

        for (const auto& pairObstr : curObstructions)
        {
            for (const Graph &gObstr : pairObstr.second)
            {
                if (size == g.nbVert)
                {
                    if (are_isomorphic(g, gObstr, 0))//TODO idthread
                        return false;
                }

                else // size < g.nbVert
                {
                    Graph gObstrCopy = gObstr;
                    if (is_supergraph_of(g, gObstrCopy, 0)) //TODO attention en parallèle les hash, le target graph pas const...
                                                            //TODO idthread
                        return false;
                }
            }
        }
    }

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
        return 'N';
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

bool ProblemArray::check_that_set_is_clique(const ProblemArraySet &set) const
{
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

bool ProblemArray::solve_array_problems(void) const
{
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
                    cout << "xD\n";
                    continue;
                }

                if (!is_true_N_between_two(set1, set3))
                {
                    cout << " LOL13\n";
                    continue;
                }


                if (!can_3sets_be_possible(set1, set2, set3))
                {
                    cout << " mdr \n";
                    continue;
                }

                // We did not save this bad triplet...
                return false;
            }
        }
    }

    return true;
}



//TODO free vertices for base graph
bool is_magic_graph(const Graph &g)
{
    ProblemArray pbArray;
    pbArray.baseGraph = g;


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

    if (!pbArray.solve_array_problems())
        return false;

    pbArray.print_array();
    cout << endl << endl;
    return true;
}


