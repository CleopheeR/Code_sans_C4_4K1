#include <vector>
#include "test-properties.hh"
#include "sparsepp/spp.h"
#include "Graph.hh"


using namespace std;


void compare_two_fixeurs_sets(const sparse_hash_map<vector<char>, vector<Graph>> &list1,
        const sparse_hash_map<vector<char>, vector<Graph>> &list2)
{
    int nbIncluded = 0, nbExcluded = 0, nbInvalid = 0; // invalid = C4 or 4K1

    for (const auto &sublist1 : list1)
    {
        const vector<char> &hashVector1 = sublist1.first;
        const auto &it2  = list2.find(hashVector1);
        const vector<Graph> emptyVect;
        const vector<Graph> &sameHashList2 = it2 != list2.cend() ? it2->second : emptyVect; // The hash is present in the second list.

        for (const Graph &g1 : sublist1.second)
        {
            if (!free_C4_O4(g1, g1.nbVert))
            {
                nbInvalid++;
                continue;
            }
            bool found = false;
            for (const Graph& g2 : sameHashList2)
            {
                if (are_isomorphic(g1, g2, 0))
                {
                    found = true;
                    break;
                }
            }

            if (found)
                nbIncluded++;
            else
                nbExcluded++;
        }
    }

    int nbTotal = nbIncluded+nbExcluded+nbInvalid;

    cout << nbTotal << " graphs among which " << nbInvalid << " contains a C4 or 4K1\n";
    cout << "\t" << nbIncluded << " are included in the other list\n";
    cout << "\t" << nbExcluded << " are uniquely in this list" << endl;
}
