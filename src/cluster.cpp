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
        bool is_root;
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
        int n;

        HC(char* simf);
        void LoadSim(char* simf);
        double ClusterSim(Cluster& c1, Cluster& c2);
        void Run();
        void PrintTree();
        void Print(Node* p, int level);
};

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
        Node node;
        node.id = i;
        node.left = NULL;
        node.right = NULL;
        node.is_root = true;
        tree[i] = node;
    }

    ID_CNT = n;
}

void HC::LoadSim(char* simf)
{
    FILE* fp = fopen(simf, "r");

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
    for(int i=0; i<c1.did.size(); ++i)
        for(int j=0; j<c2.did.size(); ++j)
            avg_sim += sim[c1.did[i]][c2.did[j]];
    
    avg_sim /= (c1.did.size() * c2.did.size());
    return avg_sim;
}

void HC::Run()
{
    while(clusters.size() > 650)
    {
        double max_clus_sim = -1;
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
                    idx1 = i;
                    idx2 = j;
                }
            }
        }

        //printf("max_clus_sim = %lf, %d %d\n", max_clus_sim, clusters[idx1].id,
        //        clusters[idx2].id);

        ++ID_CNT;

        Node node;
        node.id = ID_CNT;
        node.left  = &tree[clusters[idx1].id];
        node.right = &tree[clusters[idx2].id];
        node.is_root = true;

        node.left->is_root = false;
        node.right->is_root = false;

        tree.push_back(node);

        clusters[idx1].id = ID_CNT;
        clusters[idx1].did.insert(clusters[idx1].did.end(),
                                  clusters[idx2].did.begin(),
                                  clusters[idx2].did.end());

        clusters.erase(clusters.begin() + idx2);
    }
}

void HC::PrintTree()
{
    for(int i=0; i<tree.size(); ++i)
    {
        if(!tree[i].is_root)
            continue;

        Node* root = &tree[i];
        Print(root, 0);
        printf("\n\n");
        printf("************");
        printf("\n\n");
    }
}

void HC::Print(Node* p, int level)
{
    if(p == NULL)
        return;
    for(int i=1; i<level; ++i)
    {
        printf("|    ");
    }
    if(level)
        printf("|----");
    printf("%d\n", p->id);
    Print(p->left,  level + 1);
    Print(p->right, level + 1);
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        puts("Usage: ./a.out sim_mat_fpath");
        return -1;
    }

    HC handler(argv[1]);
    handler.Run();
    handler.PrintTree();
    return 0;
}
