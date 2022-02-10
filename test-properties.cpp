#include <iostream>
#include <algorithm>
#include <queue>
#include <vector>
#include <cassert>
#include <cstring>

#include "Graph.hh"



bool isMatched[NBMAXPROC][NBMAXVERT];
bool checkeVoisins[NBMAXPROC][NBMAXVERT];
vector<int> v1ToV2PossibleMatches[NBMAXPROC][NBMAXVERT];//(NBMAXVERT);
int v1ToV2Matches[NBMAXPROC][NBMAXVERT];//(NBMAXVERT);
bool notFirstTime[NBMAXPROC] = {false};

int vertIsoOrderToExplore[NBMAXPROC][NBMAXVERT];
int indexInIsoOrder[NBMAXPROC][NBMAXVERT];

//vector<int> listColours1(NBMAXVERT), listColours2(NBMAXVERT);


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

bool has_twin(const Graph &g, int v)
{
    assert(v == g.nbVert-1);
    for (int u = 0; u < g.nbVert-1; u++)
        if ((g.adjMat[u]^g.adjMat[v]) == ((1<<u) ^ (1<<v)))
            return true;
    return false;
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
                    return false;
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
                    return false;
            }

        }
    }

    return true;
}


bool free_C4_O4(const Graph& g, int n)
{
    return free_C4(g, n) && free_O4(g, n);
}



bool gen_iso_matching(const Graph &g1, const Graph &g2, int i, int idThread)
{
    vector<int> *curV1ToV2PossibleMatches = v1ToV2PossibleMatches[idThread];
    int *curVertIsoOrderToExplore = vertIsoOrderToExplore[idThread];
    /*cerr << "---------------\n";
      g1.print();
      g2.print();
      */
    while (i < g1.nbVert && curV1ToV2PossibleMatches[curVertIsoOrderToExplore[i]%1000].size() == 1)
        i++;

    if (i == g1.nbVert)
        return true;

    int v1 = curVertIsoOrderToExplore[i]%1000;
    int u1 = v1;
    int iU1 = i;//indexInIsoOrder[u1];
    bool *curIsMatched = isMatched[idThread];
    int *curV1ToV2Matches = v1ToV2Matches[idThread];
    bool *curCheckeVoisins = checkeVoisins[idThread];
    int *curIndexInIsoOrder = indexInIsoOrder[idThread];
    for (int match : curV1ToV2PossibleMatches[v1])
    {
        if (curIsMatched[match])
            continue;
        curV1ToV2Matches[v1] = match;
        curIsMatched[match] = true;

        int u2 = match;
        int nbNeighb = g1.get_neighb(u1).size();
        memset(curCheckeVoisins, 0, g1.nbVert);


        int nbGreater = 0;
        for (int x : g1.get_neighb(u1))
        {
            if (curIndexInIsoOrder[x] <= iU1 || curV1ToV2PossibleMatches[x].size() == 1)
            {
                assert(!curCheckeVoisins[curV1ToV2Matches[x]]);
                curCheckeVoisins[curV1ToV2Matches[x]] = true;
            }
            else
                nbGreater++;

        }


        for (int v2 : g2.get_neighb(u2))
        {
            //if (!checkeVoisins[v2])

           if (!curCheckeVoisins[v2])
            {
                if (curIsMatched[v2])
                    nbGreater = -17;
                nbGreater--;
            }
        }
        //cerr << "Tring to match " << u1 << " with " << match << " and greater = " << nbGreater << endl;



        if (nbGreater == 0 && gen_iso_matching(g1, g2, i+1, idThread))
            return true;
        curIsMatched[match] = false;
    }

    return false;
}

bool are_isomorphic(const Graph &g1, const Graph &g2, int idThread)
{
    static long long nbTimesAborted = 0;
    static long long nbTotalBucketSize = 0;
    static long long nbTimesCalled = 0;
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

    /*
    listColours1.resize(g1.nbVert);
    listColours2.resize(g1.nbVert);

    for (int u = 0; u < g1.nbVert; u++)
    {
        listColours1[u] = g1.vertsCol[u];
        listColours2[u] = g2.vertsCol[u];
    }
    sort(listColours1.begin(), listColours1.end());
    sort(listColours2.begin(), listColours2.end());

    if (listColours1 != listColours2)
    {
        nbTimesAborted++;
        return false;
    }
    */

    int curNbBucketSize = 0;
    int *curVertIsoOrderToExplore = vertIsoOrderToExplore[idThread];
    int *curV1ToV2Matches = v1ToV2Matches[idThread];
    vector<int> *curV1ToV2PossibleMatches = v1ToV2PossibleMatches[idThread];
    bool* curIsMatched = isMatched[idThread];
    for (int v1 = 0; v1 < g1.nbVert; v1++)
    {
        curVertIsoOrderToExplore[v1] = 1000*g1.get_neighb(v1).size()+v1;
        curV1ToV2PossibleMatches[v1].resize(0);
        for (int v2 = 0; v2 < g2.nbVert; v2++)
        {
            if (g1.vertsCol[v1] == g2.vertsCol[v2] && g1.get_neighb(v1).size() == g2.get_neighb(v2).size())
            {

                curV1ToV2PossibleMatches[v1].push_back(v2);
            }
        }

        curNbBucketSize += curV1ToV2PossibleMatches[v1].size();
        if (curV1ToV2PossibleMatches[v1].empty())
        {
            nbTimesAborted++;
            //assert(false);
            return false;
        }

        if (curV1ToV2PossibleMatches[v1].size() == 1)
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
    sort(curVertIsoOrderToExplore, curVertIsoOrderToExplore+g1.nbVert);
    for (int i = 0; i < g1.nbVert; i++)
        curIndexInIsoOrder[curVertIsoOrderToExplore[i]%1000] = i;



    bool toto = gen_iso_matching(g1, g2, 0, idThread);

    if (nbTimesCalled % 100000 == 0)
    {
        double avg = nbTotalBucketSize/(double)nbTimesCalled;
        cerr << nbTimesCalled << "  " << nbTimesAborted << " " << avg << "\n";
    }
    return toto;

}
