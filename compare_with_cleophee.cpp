#include <vector>
#include <fstream>

#include "Graph.hh"


vector<Graph> read_from_cleophee(const string &filename)
{
    vector<Graph> res;

    int n;
    ifstream file(filename);
    while (file >> n)
    {
        file.get();
        Graph g;
        g.init(n, 0);

        while (file.peek() != '\n')
        {
            char virgule;
            int u, v;
            file >> u >> virgule >> v;
            file.get();

            g.add_edge(u,v);
        }
        res.push_back(g);
    }

    return res;
}
