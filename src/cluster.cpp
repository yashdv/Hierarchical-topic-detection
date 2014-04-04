#include<cstdio>
#include<iostream>
#include<vector>
#include<algorithm>
#include<utility>
#include<stack>

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
        vector<Node*> tree;
        int ID_CNT;
        int n;

        HC(char* simf);
        void LoadSim(char* simf);
        double ClusterSim(Cluster& c1, Cluster& c2);
        void Run();
        void PrintTree();
        void Print(Node* p);
        ~HC();
};

HC::HC(char* simf)
{
    LoadSim(simf);

    clusters = vector<Cluster>(n);
    for(int i=0; i<n; ++i)
    {
        clusters[i].id = i+1;
        clusters[i].did.push_back(i+1);
    }

    tree = vector<Node*>(n+1);
    for(int i=1; i<=n; ++i)
    {
        tree[i] = new Node;
        tree[i]->id = i;
        tree[i]->left = NULL;
        tree[i]->right = NULL;
        tree[i]->is_root = true;
    }

    ID_CNT = n;
}

HC::~HC()
{
    for(int i=0; i<tree.size(); ++i)
        delete tree[i];
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
    while(clusters.size() > 400)
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

        ++ID_CNT;

        Node* node = new Node;
        node->id = ID_CNT;
        node->left  = tree[clusters[idx1].id];
        node->right = tree[clusters[idx2].id];
        node->is_root = true;

        node->left->is_root = false;
        node->right->is_root = false;

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
    for(int i=1; i<tree.size(); ++i)
    {
        if(!tree[i]->is_root)
            continue;

        Print(tree[i]);
        printf("\n\n");
        printf("************");
        printf("\n\n");
    }
}

void HC::Print(Node* p)
{
    stack< pair<Node*, int> > rec;
    rec.push(make_pair(p, 0));

    while(!rec.empty())
    {

        Node* n = rec.top().first;
        int level = rec.top().second;
        rec.pop();

        for(int i=1; i<level; ++i)
            printf("|    ");
        if(level)
            printf("|----");
        printf("%d\n", n->id);

        if(n->left != NULL)
            rec.push(make_pair(n->left, level + 1));
        if(n->right != NULL)
            rec.push(make_pair(n->right, level + 1));
    }
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
