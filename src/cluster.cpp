#include<cstdio>
#include<iostream>
#include<vector>
#include<algorithm>
#include<utility>
#include<stack>

#include "topics.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

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
        string tags;

        Node* left;
        Node* right;
};

class Jsonx
{
    public:

        char* json_file_name;
        Document newdoc;
        Document::AllocatorType& allocator;

        Jsonx(char* json_file_name);
        void WriteJsonStrToFile();
};

Jsonx::Jsonx(char* _json_file_name) :
    allocator(newdoc.GetAllocator())
{
    json_file_name = _json_file_name;
    newdoc.SetObject();
}

void Jsonx::WriteJsonStrToFile()
{
    StringBuffer buf;
    Writer<rapidjson::StringBuffer> writer(buf);
    newdoc.Accept(writer);

    FILE* fp = fopen(json_file_name, "w");
    fputs(buf.GetString(), fp);
    fclose(fp);
}

class HC
{
    public:
        Jsonx& jsonhandler;
        ClusterTopics& clt;
        vector<vector<double> > sim;
        vector<Cluster> clusters;
        vector<Node*> tree;
        int ID_CNT;
        int n;
        FILE* topics_map_file;

        HC(char* simf,
           char* topics_map_fpath,
           ClusterTopics& _clt,
           Jsonx& _jsonhandler);

        void LoadSim(char* simf);
        double ClusterSim(Cluster& c1, Cluster& c2);
        void WriteTopics(int cluster_id, string& v);
        void Run();
        void PrintTree();
        void PrintRec(Node* p, int level, Value& obj_c);
        void Print(Node* p);
        ~HC();
};

HC::HC(char* simf,
       char* topics_map_fpath,
       ClusterTopics& _clt,
       Jsonx& _jsonhandler) :
    clt(_clt),
    jsonhandler(_jsonhandler)
{
    topics_map_file = fopen(topics_map_fpath, "w");
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

void HC::LoadSim(char* simf)
{
    FILE* fp = fopen(simf, "r");

    int _ = fscanf(fp, "%d", &n);
    sim = vector<vector<double> >(n+1, vector<double>(n+1));

    for(int i=1; i<=n; ++i)
        for(int j=1; j<=n; ++j)
            _ = fscanf(fp, "%lf", &sim[i][j]);
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

void HC::WriteTopics(int cluster_id, string& v)
{
    fprintf(topics_map_file, "%d %s\n", cluster_id, v.c_str());
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

        //clt.GetTopics(clusters[idx1].did, topics);
        //WriteTopics(clusters[idx1].id, topics);
        clt.GetTopics(clusters[idx1].did, node->tags);
        WriteTopics(clusters[idx1].id, node->tags);

        clusters.erase(clusters.begin() + idx2);
    }
}

void HC::PrintTree()
{
    jsonhandler.newdoc.AddMember("name", "All News", jsonhandler.allocator);
    Value array(kArrayType);

    for(int i=1; i<tree.size(); ++i)
    {
        if(!tree[i]->is_root)
            continue;

        Value obj(kObjectType);
        PrintRec(tree[i], 0, obj);
        printf("\n\n************\n\n");

        if(tree[i]->id <= 1000)
            continue;

        array.PushBack(obj, jsonhandler.allocator);
    }

    jsonhandler.newdoc.AddMember("children", array, jsonhandler.allocator);
    jsonhandler.WriteJsonStrToFile();
}


void HC::PrintRec(Node* p, int level, Value& obj_c)
{
    if(p == NULL)
        return;

    for(int i=1; i<level; ++i)
        printf("|    ");
    if(level)
        printf("|----");
    printf("%d\n", p->id);

    Value obj_r(kObjectType);
    Value obj_l(kObjectType);

    PrintRec(p->right, level + 1, obj_r);
    PrintRec(p->left, level + 1, obj_l);

    Value array_c(kArrayType);
    array_c.PushBack(obj_r, jsonhandler.allocator);
    array_c.PushBack(obj_l, jsonhandler.allocator);

    if((int)p->tags.size() > 0)
        obj_c.AddMember("name", p->tags.c_str(), jsonhandler.allocator);
    else
        obj_c.AddMember("name", p->id, jsonhandler.allocator);
    obj_c.AddMember("children", array_c, jsonhandler.allocator);
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

HC::~HC()
{
    fclose(topics_map_file);
    for(int i=0; i<tree.size(); ++i)
        delete tree[i];
}

int main(int argc, char* argv[])
{
    if(argc != 5)
    {
        puts("Usage: ./a.out <sim_mat_fpath> <topics_map_fpath> <json_fpath>"
             " <topic_tf_dir> > output_tree_fpath");
        return -1;
    }

    Jsonx jsonhandler(argv[3]);
    ClusterTopics clt(argv[4]);

    HC handler(argv[1], argv[2], clt, jsonhandler);
    handler.Run();
    handler.PrintTree();
    return 0;
}
