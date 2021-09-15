#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstring>

#include "Graph.hh"

bool isMatched[1000];
bool checkeVoisins[1000];
vector<vector<int>> v1ToV2PossibleMatches(1000);
vector<int> v1ToV2Matches(1000);
bool firstTime = true;

bool free_O4(const Graph& g, int n)
{
    int v1 = n-1;
    for (int v2 = 0; v2 < n-1; v2++)
    {
        if (g.adjMat[v2][v1])
            continue;

        for (int v3 = v2+1; v3 < n-1; v3++)
        {
            if (g.adjMat[v3][v1] || g.adjMat[v3][v2])
                continue;

            for (int v4 = v3+1; v4 < n-1; v4++)
            {
                if (!g.adjMat[v4][v3] && !g.adjMat[v4][v2] && !g.adjMat[v4][v1])
                    return false;
            }
        }
    }

    return true;
}


bool free_C4(const Graph& g, int n)
{
    int v1 = n-1;

    for (const int v2 : g.adjList[v1])
    {
        if (v1 == v2) //TODO facultatif si graphe simple
            continue;


        for (const int v3 : g.adjList[v1])
        {
            //if (v3 == v2)
            //    break;
            if (v3 == v2 || g.adjMat[v3][v2])
                continue;

            for (const int v4 : g.adjList[v3])
            {
                if (v4 == v1 || v4 == v2 || v4 == v3)
                    continue;
                if (g.adjMat[v4][v2] && !g.adjMat[v4][v1])
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


bool aunjumeau(const Graph& g, int v)
{

    return false;
}//TODO coder


bool gen_iso_matching(const Graph &g1, const LightGraph &g2, /*const vector<vector<int>> &v1ToV2PossibleMatches, vector<int> &v1ToV2Matches, bool* isMatched,*/ int v1)
{
    while (v1 < g1.nbVert && v1ToV2PossibleMatches[v1].size() == 1)
        v1++;

    if (v1 == g1.nbVert)
    {
        //cout << "-------------------\n";
        for (int u1 = 0; u1 < g1.nbVert; u1++) //TODO un tour de moins ?
        {
            int u2 = v1ToV2Matches[u1];
            int doublons = 0; // On xore, si ça fait 0 à la fin, c'est good !
            int nbNeighb = g1.adjList[u1].size();
            //bool* checkeVoisins = (bool*) calloc(g1.nbVert, sizeof(bool));
            memset(checkeVoisins, 0, g1.nbVert);
            //cout << u1 << " VS " << u2 << " ===> " << g1.adjList[u1].size() << " VS " << g2.adjList[u2].size() << endl;
            assert(g1.adjList[u1].size() == g2.adjList[u2].size());

            for (int i = 0; i < nbNeighb; i++)
            {
                //cout << "\t"<<g1.adjList[u1][i] << "->" << v1ToV2Matches[g1.adjList[u1][i]] <<  " = " << g2.adjList[u2][i] << "\n";
                doublons = doublons ^ v1ToV2Matches[g1.adjList[u1][i]];
                doublons = doublons ^ g2.adjList[u2][i];
                checkeVoisins[v1ToV2Matches[g1.adjList[u1][i]]] = true;

            }
            //cout << " doublons = " << doublons << endl;

            if (doublons != 0)
            {
                //free(checkeVoisins);
                //memset(checkeVoisins, 0, g1.nbVert);
                return false;
            }

            for (int v2 : g2.adjList[u2])
            {
                if (!checkeVoisins[v2])
                {
                    //free(checkeVoisins);
                    //memset(checkeVoisins, 0, g1.nbVert);
                    return false;
                }
                checkeVoisins[v2] = false;//TODO inutile si voisins uniques
            }

            //free(checkeVoisins);
        }

        return true;
    }

    for (int match : v1ToV2PossibleMatches[v1])
    {
        if (isMatched[match])
            continue;
        v1ToV2Matches[v1] = match;
        isMatched[match] = true;
        if (gen_iso_matching(g1, g2,/* v1ToV2PossibleMatches, v1ToV2Matches, isMatched,*/ v1+1))
            return true;
        isMatched[match] = false;
    }

    return false;
}

bool are_isomorphic(const Graph &g1, const LightGraph &g2)
{
    if (firstTime)
    {
        firstTime = false;
        cout << "YYYYYYYYYY\n";
        for (int i = 0; i < 1000; i++)
            v1ToV2PossibleMatches[i].reserve(1000);
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
    //vector<vector<int>> v1ToV2PossibleMatches(g1.nbVert);
    //vector<int> v1ToV2Matches(g1.nbVert);
    //bool* isMatched = (bool*) calloc(g2.nbVert, sizeof(bool));
    //TODO maybe matrice de match possible, oui/non

    memset(isMatched, 0, g1.nbVert);// TODO ou bien dans la fonction gen_iso et dessus blah

    if (v1ToV2Matches.size() != g1.nbVert)
    {
        v1ToV2Matches.resize(g1.nbVert);
        v1ToV2PossibleMatches.resize(g1.nbVert);
    }

    vector<int> degreeNeighb1(g1.nbVert), degreeNeighb2(g1.nbVert);
    for (int v1 = 0; v1 < g1.nbVert; v1++)
    {
        //v1ToV2PossibleMatches[v1].reserve(g1.nbVert); //TODO reserver au tout début une fois pour toute
        v1ToV2PossibleMatches[v1].resize(0);
        for (int v2 = 0; v2 < g2.nbVert; v2++)
        {
            if (g1.adjList[v1].size() != g2.adjList[v2].size())
            {
                //cerr << "will not allow " << v1 << " = " << v2 << endl;
                continue;
            }
            //cerr << "\t\t\t" << v1 << " = " << v2 << " is ok because " << g1.adjList[v1].size()  << " = " << g2.adjList[v2].size() << endl;
            int nbNeighb = g1.adjList[v1].size();
            degreeNeighb1.resize(nbNeighb);
            degreeNeighb2.resize(nbNeighb);

            for (int i = 0; i < nbNeighb; i++)
            {
                int w1 = g1.adjList[v1][i];
                int w2 = g2.adjList[v2][i];
                degreeNeighb1[i] = g1.adjList[w1].size();
                degreeNeighb2[i] = g2.adjList[w2].size();
            }

            sort(degreeNeighb1.begin(), degreeNeighb1.end());
            sort(degreeNeighb2.begin(), degreeNeighb2.end());

            if (degreeNeighb1 == degreeNeighb2)
            {
                //cerr << "\t\t\tTTTTTT" << v1 << " = " << v2 << " is ok because " << g1.adjList[v1].size()  << " = " << g2.adjList[v2].size() << endl;
                v1ToV2PossibleMatches[v1].push_back(v2);
            }
        }
        if (v1ToV2PossibleMatches[v1].empty())
        {
            //free(isMatched);
            return false;
        }

        if (v1ToV2PossibleMatches[v1].size() == 1)
        {
            v1ToV2Matches[v1] = v1ToV2PossibleMatches[v1][0];
            isMatched[v1ToV2Matches[v1]] = true;
        }
    }

    bool toto = gen_iso_matching(g1, g2,/* v1ToV2PossibleMatches, v1ToV2Matches, isMatched,*/ 0);

    if (false && toto)
    {
        cout << "REFUSING3\n";
        //g1.print();
        //g2.print();
        cout << "FOUND ISOMORPHISM :D" << endl;

        for (int i = 0; i < g1.nbVert; i++)
            cout << i << " -> " << v1ToV2Matches[i] << endl;
    }
    //free(isMatched);
    return toto;

}
