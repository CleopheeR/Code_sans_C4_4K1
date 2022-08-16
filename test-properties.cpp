#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <cassert>
#include <cstring>

#include "Graph.hh"
#include "test-properties.hh"


//DEBUG
vector<int> swapsSubgraphList; // Only to print which vertices we used

//TTAADDAA documenter variables
bool isMatched[NBMAXPROC][NBMAXVERT];
bool checkeVoisins[NBMAXPROC][NBMAXVERT];
vector<int> v1ToV2PossibleMatches[NBMAXPROC][NBMAXVERT];//(NBMAXVERT);
int v1ToV2Matches[NBMAXPROC][NBMAXVERT];//(NBMAXVERT);
bool notFirstTime[NBMAXPROC] = {false};

int vertIsoOrderToExplore[NBMAXPROC][NBMAXVERT];
int indexInIsoOrder[NBMAXPROC][NBMAXVERT];

//vector<int> listColours1(NBMAXVERT), listColours2(NBMAXVERT);

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
                    //cerr << "O4 found: " << v1 << " " << v2 << " " << v4 << " " << v3 << endl;
                    return false;
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
        //if (v1 == v2) //TODO facultatif si graphe simple
            //continue;

        for (int i3 = i2+1; i3 < nbNeighb1; i3++)
        {
            const int v3 = neighb1[i3];
            if (are_neighb(g, v3, v2))
                continue;

           for (const int v4 : g.get_neighb(v3))
            {
                if (v4 == v1)// || v4 == v2)// || v4 == v3)
                    continue;
                if (are_neighb(g, v4, v2) && !are_neighb(g, v4, v1))
                {
                    //cerr << "cycle found: " << v1 << " " << v2 << " " << v4 << " " << v3 << endl;
                    return false;
                }
            }

        }
    }

    return true;
}


bool are_isomorphic(const Graph &g1, const Graph &g2, int idThread)
{
    static long long nbTimesAborted = 0;
    static long long nbTotalBucketSize = 0;
    static long long nbTimesCalled = 0;

    //On first run (per thread) we reserve some space to gain time
    //TTAADDAA => this space is never recovered?
    if (!notFirstTime[idThread])
    {
        notFirstTime[idThread] = true;
        cout << "YYYYYYYYYY\n";
        for (int i = 0; i < NBMAXVERT; i++)
            v1ToV2PossibleMatches[idThread][i].reserve(NBMAXVERT);
    }
    /*
       cout << " Comparing a pair of graphs\n";
       g1.print();
       g2.print();
       for (int x : g1.degreeList)
       cout << x  << "; ";
       cout << " ===== ";
       for (int x : g2.degreeList)
       cout << x  << "; ";
       cout << endl;
       */
    /*
       if (g1.nbVert != g2.nbVert || g1.nbEdge != g2.nbEdge || g1.degreeList != g2.degreeList)
       {
       assert(false);
       cout << "\t" << g1.nbVert << "," << g1.nbEdge << " => " << g2.nbVert << "," << g2.nbEdge << endl;
       return false;
       }
       */

    memset(isMatched[idThread], 0, g1.nbVert);// TODO ou bien dans la fonction gen_iso et dessus blah

    /*if (v1ToV2Matches.size() != g1.nbVert)
    {
        v1ToV2Matches.resize(g1.nbVert);
        v1ToV2PossibleMatches.resize(g1.nbVert);
    }*/

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
        curVertIsoOrderToExplore[v1] = 1000*g1.get_neighb(v1).size()+v1;
        curV1ToV2PossibleMatches[v1].resize(0);

	// We create the set of vertices from g2 that could be matched to v1.
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
            //assert(false);
            return false;
        }

        if (curV1ToV2PossibleMatches[v1].size() == 1) // Unique possible match, we assign v1 to it
        {
            curV1ToV2Matches[v1] = curV1ToV2PossibleMatches[v1][0];
            if (curIsMatched[curV1ToV2Matches[v1]])
                return false; // Two vertices must be matched to the same one in g2.
            curIsMatched[curV1ToV2Matches[v1]] = true;
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
	    // We test the isomorphism for the vertices of g1 that are uniquely matched.
            if (!are_neighb(g1, u1, u2) ^ !are_neighb(g2, v1, v2))
            {
                nbTimesAborted++;
                return false;
            }
        }
    }
    nbTotalBucketSize += curNbBucketSize;
    nbTimesCalled++;

    int *curIndexInIsoOrder = indexInIsoOrder[idThread];
    sort(curVertIsoOrderToExplore, curVertIsoOrderToExplore+g1.nbVert); // We will explore first the vertices with few possible matches.
    for (int i = 0; i < g1.nbVert; i++)
        curIndexInIsoOrder[curVertIsoOrderToExplore[i]%1000] = i;

    bool toto = gen_iso_matching(g1, g2, 0, idThread); // We look for a full matching.

    if (nbTimesCalled % 100000 == 0)
    {
        double avg = nbTotalBucketSize/(double)nbTimesCalled;
        cerr << nbTimesCalled << "  " << nbTimesAborted << " " << avg << "\n";
    }


    //To print the subgraph we found... ?
    if (false && toto)
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
        gbis.print();
        cout << "olololo\n";
        g2.print();
        cout << "oeoureoiuezouroizeurioezuoizuoiezruo\n";
    }
    return toto;
}


bool has_twin(const Graph &g, int v)
{
    assert(v == g.nbVert-1);
    for (int u = 0; u < g.nbVert-1; u++)
        if ((g.adjMat[u]^g.adjMat[v]) == ((1<<u) ^ (1<<v)))
            return true;
    return false;
}


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

// Tries to find a full matching. Recursive function, we are here at index i.
bool gen_iso_matching(const Graph &g1, const Graph &g2, int i, int idThread)
{
    vector<int> *curV1ToV2PossibleMatches = v1ToV2PossibleMatches[idThread];
    int *curVertIsoOrderToExplore = vertIsoOrderToExplore[idThread];
    /*cerr << "---------------\n";
      g1.print();
      g2.print();
    */
    //TODO vertIsoOrder, on avait un autre truc sans besoin du %1000 défini dans la fin de are_isomorphic
    // We pass the vertices which were matched because unique possible candidate.
    while (i < g1.nbVert && curV1ToV2PossibleMatches[curVertIsoOrderToExplore[i]%1000].size() == 1)
        i++;

    // All vertices have been matched with no errors!
    if (i == g1.nbVert)
        return true;

    int v1 = curVertIsoOrderToExplore[i]%1000;
    int u1 = v1; // TODO: useful ?!
    int iU1 = i;//indexInIsoOrder[u1];
    bool *curIsMatched = isMatched[idThread];
    int *curV1ToV2Matches = v1ToV2Matches[idThread];
    bool *curCheckeVoisins = checkeVoisins[idThread]; // To check in linear time that we match the neighbours to the neighbours of the match
    int *curIndexInIsoOrder = indexInIsoOrder[idThread]; //This guy should be used instead of the other one and %1000
    // We try, for each candidate, to match it to v1 and recurse.
    for (int match : curV1ToV2PossibleMatches[v1])
    {
        if (curIsMatched[match]) //This vertex is already matched.
            continue;
        curV1ToV2Matches[v1] = match;
        curIsMatched[match] = true;

        int u2 = match; //TODO useful?!
        int nbNeighb = g1.get_neighb(u1).size();
        memset(curCheckeVoisins, 0, g1.nbVert);

        int nbGreater = 0; // We count how many neighbours we haven't matched yet
        for (int x : g1.get_neighb(u1))
        {
	    //if x is already matched, we mark it as a neighbour of v1 we have seen
            if (curIndexInIsoOrder[x] <= iU1 || curV1ToV2PossibleMatches[x].size() == 1)
            {
		// We ensure that we see the match only once. //TODO => c'est vraiment ça ?
                assert(!curCheckeVoisins[curV1ToV2Matches[x]]);
                curCheckeVoisins[curV1ToV2Matches[x]] = true;
            }
            else
                nbGreater++;
        }


        for (int v2 : g2.get_neighb(u2))
        {
            if (!curCheckeVoisins[v2]) // A neighbour of u2 which was not marked
            {
                if (curIsMatched[v2]) // If it is already matched, this is bad, it breaks the isomorphism
                    nbGreater = -17;
                nbGreater--; // Otherwise we make sure we see as many unknown vertices as when we counted the neighbours of u1
            }
        }
        //cerr << "Tring to match " << u1 << " with " << match << " and greater = " << nbGreater << endl;

        if (nbGreater == 0 && gen_iso_matching(g1, g2, i+1, idThread))
            return true;
        curIsMatched[match] = false;
    }

    return false;
}

bool is_supergraph_of_aux(Graph &g, const Graph &targetGraph, const vector<char> &targetHash, vector<char> &tmpHash, int pos, int idThread)
{
    if (g.nbVert == targetGraph.nbVert)
    {
        if (g.nbEdge != targetGraph.nbEdge)
            return false;

        g.compute_hashes(tmpHash);

        if (tmpHash == targetHash && are_isomorphic(g, targetGraph, idThread))
        {
            if (false) // Print which vertices we identified for the subgraph
            {
                cerr << "Here is the list of the removed vertices :";
                for (int x : swapsSubgraphList)
                    cerr << x << " ";
                cerr << endl;
                g.print();
                targetGraph.print();
                cerr << "YEAH\n\n";
            }
            return true;
        }

        return false;
    }





    Graph smallGraph;
    int n = g.nbVert;

    if (n - (n-pos) > targetGraph.nbVert)
        return false;
    if (n < targetGraph.nbVert || g.nbEdge < targetGraph.nbEdge)
        return false;


    vector<char> smallDegreeList(g.nbVert+3);
    for (int i = pos; i < n; i++)
    {
        if (g.nbEdge - g.get_neighb(i).size() < targetGraph.nbEdge)
            continue;

        smallGraph = g.subgraph_removing_vertex(i);

        //cerr << "trying to remove vertex " << i << "=> now " << g.nbVert-1 << " <-> " << i << endl;
        swapsSubgraphList.push_back(i);
        if (is_supergraph_of_aux(smallGraph, targetGraph, targetHash, tmpHash, i, idThread))
            return true;
        //cerr << "end of i=" << i << " removed" << endl;
        swapsSubgraphList.pop_back();
    }

    if (n - (n-pos)+1 < targetGraph.nbVert)
        return is_supergraph_of_aux(g, targetGraph, targetHash, tmpHash, pos+1, idThread);

    return false;
}



bool is_supergraph_of(const Graph &g, Graph &targetGraph, int idThread)
{
    int n = targetGraph.nbVert;

    vector<char> tmpHash(n+4), targetHash(n+4);

    targetGraph.compute_hashes(targetHash);

    Graph gCopy = g;

    swapsSubgraphList.clear();

    if (is_supergraph_of_aux(gCopy, targetGraph, targetHash, tmpHash, 0, idThread))
    {
        if (false)
        {
            g.print();
            cerr << " ^^^^ was the graph tested against \n";
            targetGraph.print();
            cerr << "\n\n";
        }
        return true;
    }

    return false;
}
