#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <cassert>
#include <cstring>

#include "Graph.hh"
#include "test-properties.hh"


//DEBUG
vector<int> removedVertsInIsSupergraphOf; // Only to print which vertices we used.

//TTAADDAA documenter variables
// Variables used by are_isomorphic. There is one copy per thread so that they can work independently.
// We declare them here to avoid multiple allocations and save time.
bool isMatched[NBMAXPROC][NBMAXVERT]; // isMatched[v] = true if we've currently assigned a match for v.
bool checkNeighbs[NBMAXPROC][NBMAXVERT]; // TYDY ???
// Lists the vertices in g2 with same colour as the one in g1.
vector<int> v1ToV2PossibleMatches[NBMAXPROC][NBMAXVERT];
int v1ToV2Matches[NBMAXPROC][NBMAXVERT]; // tab[v1] is the vertex in g2 that was assigned to v1 (from g1).

// tab[i] contains the vertex that should be explored at the i-th step (heuristics).
int vertIsoOrderToExplore[NBMAXPROC][NBMAXVERT];
int indexInIsoOrder[NBMAXPROC][NBMAXVERT]; // Associates to v its position in the array just above.


bool free_C4_O4(const Graph& g, int n)
{
    return free_C4(g, n) && free_O4(g, n);
}


bool free_O4(const Graph& g, int n)
{
    int v1 = n-1;
    for (int v2 = 0; v2 < n-1; v2++)
    {
        if (are_neighb(g,v2,v1))
            continue;

        for (int v3 = v2+1; v3 < n-1; v3++)
        {
            if (are_neighb(g, v3, v1) || are_neighb(g, v3, v2))
                continue;

            for (int v4 = v3+1; v4 < n-1; v4++)
            {
                if (!are_neighb(g, v4, v3) && !are_neighb(g, v4, v2) && !are_neighb(g, v4, v1))
                {
                    return false; // v1, v2, v3 and v4 form an induce O4.
                }
            }
        }
    }

    return true;
}


bool free_C4(const Graph& g, int n)
{
    int v1 = n-1;

    const vector<int> &neighb1 = g.get_neighb(v1);
    int nbNeighb1 = neighb1.size();
    for (int i2 = 0; i2 < nbNeighb1; i2++)
    {
        const int v2 = neighb1[i2];

        for (int i3 = i2+1; i3 < nbNeighb1; i3++)
        {
            const int v3 = neighb1[i3];
            if (are_neighb(g, v3, v2))
                continue;

           for (const int v4 : g.get_neighb(v3))
            {
                if (v4 == v1)
                    continue;
                if (are_neighb(g, v4, v2) && !are_neighb(g, v4, v1))
                {
                    return false; // v1, v2, v3 and v4 form an induced C4.
                }
            }

        }
    }

    return true;
}


bool are_isomorphic(const Graph &g1, const Graph &g2, int idThread)
{
#ifdef DEBUG
    assert(g1.vertsCol != NULL && g2.vertsCol != NULL);
    assert(g1.nbVert == g2.nbVert && g1.nbEdge == g2.nbEdge && g1.degreeList == g2.degreeList);

#endif

    static long long nbTimesAborted = 0;
    static long long nbTotalBucketSize = 0;
    static long long nbTimesCalled = 0;
    static long long nbRetFalse = 0;

    memset(isMatched[idThread], 0, g1.nbVert);// TODO ou bien dans la fonction gen_iso et dessus blah

    vector<int> degreeNeighb1(g1.nbVert), degreeNeighb2(g1.nbVert);
    vector<int> uniqueMatchVertices;
    uniqueMatchVertices.reserve(g1.nbVert);

    int curNbBucketSize = 0;
    int *curVertIsoOrderToExplore = vertIsoOrderToExplore[idThread];
    int *curV1ToV2Matches = v1ToV2Matches[idThread];
    vector<int> *curV1ToV2PossibleMatches = v1ToV2PossibleMatches[idThread];
    bool* curIsMatched = isMatched[idThread];

    for (int v1 = 0; v1 < g1.nbVert; v1++)
    {
        // Assigning a priority to be explored to each vertex.
        curVertIsoOrderToExplore[v1] = 1000*g1.get_neighb(v1).size()+v1;
        // Resetting the vector.
        curV1ToV2PossibleMatches[v1].resize(0);

        // We create the set of vertices from g2 that are candidates to being matched to v1.
        // They need to have the same degree, and the same colour (fingerprint).
        for (int v2 = 0; v2 < g2.nbVert; v2++)
        {
            if (g1.vertsCol[v1] == g2.vertsCol[v2] && g1.get_neighb(v1).size() == g2.get_neighb(v2).size())
            {
                curV1ToV2PossibleMatches[v1].push_back(v2);
            }
        }

        curNbBucketSize += curV1ToV2PossibleMatches[v1].size();
        if (curV1ToV2PossibleMatches[v1].empty()) // One vertex has no possible match, hence no matching!
        {
            nbTimesAborted++;
            return false;
        }

        // v1 has only one candidate, hence we match it right away, before recursion.
        if (curV1ToV2PossibleMatches[v1].size() == 1)
        {
            curV1ToV2Matches[v1] = curV1ToV2PossibleMatches[v1][0];
            // If this candidate was already matched (to another vertex), then no isomorphism is possible.
            if (curIsMatched[curV1ToV2Matches[v1]])
                return false;

            curIsMatched[curV1ToV2Matches[v1]] = true; // This "v2" must not be rematched later.
            uniqueMatchVertices.push_back(v1);
        }
    }


    int nbUnique = uniqueMatchVertices.size();
    for (int i1 = 0; i1 < nbUnique; i1++)
    {
        int u1 = uniqueMatchVertices[i1];
        int v1 = curV1ToV2Matches[u1];
        for (int i2 = i1+1; i2 < nbUnique; i2++)
        {
            int u2 = uniqueMatchVertices[i2];
            int v2 = curV1ToV2Matches[u2];
	    // We test the isomorphism for the subgraph induced by the vertices of g1 that are uniquely matched.
            if ((!are_neighb(g1, u1, u2)) ^ (!are_neighb(g2, v1, v2)))
            {
                nbTimesAborted++;
                // Two vertices in g2 are neighbours but not the corresponding vertices in g1 (or the contrary).;
                return false;
            }
        }
    }

    // Statistics
    nbTotalBucketSize += curNbBucketSize;
    nbTimesCalled++;

    int *curIndexInIsoOrder = indexInIsoOrder[idThread];
    sort(curVertIsoOrderToExplore, curVertIsoOrderToExplore+g1.nbVert); // We will explore first the vertices with few possible matches.
    for (int i = 0; i < g1.nbVert; i++)
        curIndexInIsoOrder[curVertIsoOrderToExplore[i]%1000] = i; // The score was 1000*degree(v)+v.

    bool matchingFound  = gen_iso_matching(g1, g2, 0, idThread); // We try all the possibilities to get a full matching.
    if (!matchingFound)
        nbRetFalse++;

    // Statistics.
    if (nbTimesCalled % 100000 == 0)
    {
        double avg = nbTotalBucketSize/(double)nbTimesCalled;
        double avgFalse = nbRetFalse/(double)nbTimesCalled;
        cerr << nbTimesCalled << "  " << nbTimesAborted << " " << avg  << " " << avgFalse << "\n";
    }


    //To print the subgraph we found... ?
    if (false && matchingFound)
    {
        for (int i = 0; i < g1.nbVert; i++)
            cout << i << " => " << v1ToV2Matches[idThread][i] << endl;

        Graph gbis;
        gbis.init(g1.nbVert, 0);
        for (int u1 = 0; u1 < g1.nbVert; u1++)
        {
            int u2 = v1ToV2Matches[idThread][u1];
            for (int v1 : g1.get_neighb(u1))
            {
                if (v1 > u1)
                    continue;
                int v2 = v1ToV2Matches[idThread][v1];
                gbis.add_edge(u2, v2);
            }
        }

        // Normally, the two graphs should be identical.
        gbis.pretty_print();
        g2.pretty_print();
        cout << "Are they identical?\n";
    }
    return matchingFound;
}


bool has_twin(const Graph& g)
{
    for (int u = 0; u < g.nbVert; u++)
    {
        for (int v = u+1; v < g.nbVert; v++)
        {
            if ((g.adjMat[u]^g.adjMat[v]) == ((1<<u) ^ (1<<v)))
                return true;
        }
    }

    return false;
}

// Assuming v is the last vertex
bool has_twin(const Graph &g, int v)
{
#ifdef DEBUG
    assert(v == g.nbVert-1);
#endif
    for (int u = 0; u < g.nbVert-1; u++)
        if ((g.adjMat[u]^g.adjMat[v]) == ((1<<u) ^ (1<<v)))
            return true;
    return false;
}


// Performing a simple BFS.
int nb_connected_comp(const Graph& g)
{
    int nbComp = 0;
    vector<bool> isSeen(g.nbVert, false);
    queue<int> toDo;
    for (int u = 0; u < g.nbVert; u++)
    {
        if (isSeen[u])
            continue;
        nbComp++;
        toDo.push(u);
        isSeen[u] = true;
        while (!toDo.empty())
        {
            int v = toDo.front();
            toDo.pop();
            for (int w : g.get_neighb(v))
            {
                if (!isSeen[w])
                {
                    toDo.push(w);
                    isSeen[w] = true;
                }
            }
        }
    }
    return nbComp;
}



/** Internal functions **/

// Tries to find a full matching. Recursive function, we are here exploring the vertex at index i (in
// vertIsoOrderToExplore).
bool gen_iso_matching(const Graph &g1, const Graph &g2, int i, int idThread)
{
    vector<int> *curV1ToV2PossibleMatches = v1ToV2PossibleMatches[idThread];
    const int *curVertIsoOrderToExplore = vertIsoOrderToExplore[idThread];

    //TODO vertIsoOrder, on avait un autre truc sans besoin du %1000 défini dans la fin de are_isomorphic
    // We pass the vertices which were matched in the main function, because only one possible candidate.
    while (i < g1.nbVert && curV1ToV2PossibleMatches[curVertIsoOrderToExplore[i]%1000].size() == 1)
        i++;

    // All vertices have been matched with no errors!
    if (i == g1.nbVert)
        return true;

    int u1 = curVertIsoOrderToExplore[i]%1000;
    int iU1 = i;//indexInIsoOrder[u1];
    bool *curIsMatched = isMatched[idThread];
    int *curV1ToV2Matches = v1ToV2Matches[idThread];
    // To check in linear time that we match the neighbours to the neighbours of the match.
    bool *curCheckNeighbs = checkNeighbs[idThread];

    const int *curIndexInIsoOrder = indexInIsoOrder[idThread];
    // We try, for each candidate, to match it to u1 and recurse. u2 is u1's match in g2.
    for (int u2 : curV1ToV2PossibleMatches[u1])
    {
        if (curIsMatched[u2]) // This vertex is already matched, it cannot be matched twice.
            continue;
        curV1ToV2Matches[u1] = u2;
        curIsMatched[u2] = true;

        // Resetting this working array.
        memset(curCheckNeighbs, 0, g1.nbVert);

        int nbUnmatchedYet = 0; // We count how many neighbours we haven't matched yet.
        for (int v1 : g1.get_neighb(u1))
        {
	        // If v1 is already matched, we mark it as a neighbour of v1 we have seen.
            if (curIndexInIsoOrder[v1] <= iU1 || curV1ToV2PossibleMatches[v1].size() == 1)
            {
		        // We ensure that we see the match only once. //TODO => c'est vraiment ça ?
#ifdef DEBUG
                assert(!curCheckNeighbs[curV1ToV2Matches[v1]]);
#endif
                curCheckNeighbs[curV1ToV2Matches[v1]] = true;
            }
            else
                nbUnmatchedYet++;
        }


        // A bit complex. But it amounts to a counting argument.
        // We go through the neighbours of u2. There are as many as neighbours of u1.
            // 1. They are not vertices in g2 already corresponding to u1's neighbours (matched)
            // 2. They are vertices in g2 already marked as correponding to one of u1's neighbours.
            // We count the number of unmatched, and check that there are as many as the number we counted
            // for the true neighbours of u1 (unmatched in g2 for the moment).
            // If there are the same number, then there are the same number of already matched neighbours.
        // Also, if some neighbour of u2 was matched but to a vertex not corresponding to u1's neighbours,
        // then the two graphs cannot be isomorphic.
        for (int v2 : g2.get_neighb(u2))
        {
            if (!curCheckNeighbs[v2]) // A neighbour of u2 which was not marked
            {
                if (curIsMatched[v2]) // If it is already matched, this is bad, it breaks the isomorphism
                    nbUnmatchedYet = -17;
                nbUnmatchedYet--; // Otherwise we make sure we see as many unknown vertices as when we counted the neighbours of u1
            }
        }

        // All the already matched neighbours of u1 are matched to matched neighbours of u2.
        // For the others, we will check the edge with u2 when they are matched themselfes later (check that
        // u1 and u2 are neighbours of the vertex in g1 which will be matched, and its match in g2).
        if (nbUnmatchedYet == 0 && gen_iso_matching(g1, g2, i+1, idThread))
            return true;

        curIsMatched[u2] = false; // Forgetting we tried this vertex, to try a new one.
    }

    return false;
}


// Tries to remove vertices from g (one at a time), such that there are as many as in targetGraph,
// and the two are isomorphic.
bool is_supergraph_of_aux(Graph &g, const Graph &targetGraph, const vector<char> &targetHash, vector<char> &tmpHash, int pos, int idThread)
{
    // We removed the right number of vertices. Are g isomorphic to targetGraph?
    if (g.nbVert == targetGraph.nbVert)
    {
        if (g.nbEdge != targetGraph.nbEdge)
            return false;

        g.compute_hashes(tmpHash);
        if (tmpHash == targetHash && are_isomorphic(g, targetGraph, idThread))
        {
            if (false) // Prints which vertices we identified for the subgraph.
            {
                cerr << "Here is the list of the removed vertices :";
                for (int x : removedVertsInIsSupergraphOf)
                    cerr << x << " ";
                cerr << endl;
                g.pretty_print();
                targetGraph.pretty_print();
                cerr << "YEAH\n\n";
            }
            return true;
        }

        return false;
    }


    // Otherwise, we need to remove at least one vertex.
    Graph smallGraph;
    int n = g.nbVert;

    if (pos > targetGraph.nbVert) // There remains too few vertices to remove.
        return false;
    //TYDY useful because done below?
    if (n < targetGraph.nbVert || g.nbEdge < targetGraph.nbEdge) // We removed too many edges.
        return false;


    vector<char> smallDegreeList(g.nbVert+3);
    for (int i = pos; i < n; i++)
    {
        if (g.nbEdge - g.get_neighb(i).size() < targetGraph.nbEdge) // Too many edges to remove.
            continue;

        smallGraph = g.subgraph_removing_vertex(i); // We remove vertex i.

        removedVertsInIsSupergraphOf.push_back(i); //TYDY bettername
        if (is_supergraph_of_aux(smallGraph, targetGraph, targetHash, tmpHash, i, idThread))
            return true;
        removedVertsInIsSupergraphOf.pop_back();
    }

    if (pos-1 < targetGraph.nbVert)
        return is_supergraph_of_aux(g, targetGraph, targetHash, tmpHash, pos+1, idThread);

    return false;
}



bool is_supergraph_of(const Graph &g, Graph &targetGraph, int idThread)
{
    int n = targetGraph.nbVert;

    vector<char> tmpHash(n+4), targetHash(n+4);

    targetGraph.compute_hashes(targetHash);

    Graph gCopy = g;

    removedVertsInIsSupergraphOf.clear();

    if (is_supergraph_of_aux(gCopy, targetGraph, targetHash, tmpHash, 0, idThread))
    {
        if (false)
        {
            g.pretty_print();
            cerr << " ^^^^ was the graph tested against \n";
            targetGraph.pretty_print();
            cerr << "\n\n";
        }
        return true;
    }

    return false;
}
