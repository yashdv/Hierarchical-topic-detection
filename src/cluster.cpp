#include<cstdio>
#include<iostream>
#include<vector>
#include<algorithm>

using namespace std;

class Cluster
{
    public:

        int id;
        vector<int> did;
};

class Node
{
    public:
        int id;
        Node* left;
        Node* right;
};

class HC
{
    public:
        vector<vector<double> > sim;
        vector<Cluster> clusters;
        vector<Node> tree;
        int ID_CNT;

        HC(char* simf);
        void LoadSim(char* simf);
        void Run();
}

HC::HC(char* simf)
{
    LoadSim(simf);

    clusters = vector<Cluster>(n);
    for(int i=0; i<n; ++i)
    {
        Cluster c;
        c.id = i+1;
        c.did.push_back(i+1);
        clusters[i] = c;
    }

    tree = vector<Node>(n+1);
    for(int i=1; i<=n; ++i)
    {
        Node n;
        n.id = i;
        n.left = NULL;
        n.right = NULL;
        tree[i] = n;
    }

    ID_CNT = n;
}

void HC::LoadSim(char* simf)
{
    FILE* fp = fopen(simf, "r");
    int n;

    fscanf(fp, "%d", &n);
    sim = vector<vector<double> >(n+1, vector<double>(n+1));

    for(int i=1; i<=n; ++i)
        for(int j=1; j<=n; ++j)
            fscanf(fp, "%lf", &sim[i][j]);
    fclose(fp);
}

double HC::ClusterSim(Cluster& c1, Cluster& c2)
{
    double avg_sim = 0;
    for(int i=0; i<c1.did.size(); i++)
        for(int j=0; j<c2.did.size(); j++)
            avg_sim += sim[c1.did[i]][c2.did[j]];
    
    avg_sim /= (c1.did.size() * c2.did.size());
    return avg_sim;
}

void HC::Run()
{
    while(true)
    {
        double max_clus_sim = -1;
        int c1;
        int c2;
        int idx1;
        int idx2;
        for(int i=0; i<clusters.size(); ++i)
        {
            for(int j=0; j<i; ++j)
            {
                double clus_sim = ClusterSim(clusters[i], clusters[j]);
                if(clus_sim > max_clus_sim)
                {
                    max_clus_sim = clus_sim;
                    c1 = clusters[i].id;
                    c2 = clusters[j].id;
                    idx1 = i;
                    idx2 = j;
                }
            }
        }

        ++ID_CNT;
        Node n;
        n.id = ID_CNT;
        n.left = tree[c1];
        n.right = tree[c2];
        tree.push_back(n);

        clusters[idx1].id = ID_CNT;
        clusters[idx1].did.insert(clusters[idx1].did.end(),
                                  clusters[idx2].did.begin(),
                                  clusters[idx2].did.end());

        clusters.erase(clusters.begin() + idx2);
    }
}

int main(int argc, char* argv[])
{
    return 0;
}
