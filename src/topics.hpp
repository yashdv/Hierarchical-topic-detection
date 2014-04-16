#ifndef __HTD_SRC_TOPICS_HPP
#define __HTD_SRC_TOPICS_HPP

#include<cstdio>
#include<cstring>
#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<string>
#include<utility>

using namespace std;

#define CHUNK_SZ 100000000
#define MAX_PATH_LEN 1024
#define MAX_WORD_SZ 100

class ClusterTopics
{
    public:
        char* tf_dir;
        char* buf;
        int bufp;
        int buflen;
        int sum_doc_freqs;
        int doc_word_cnt;
        int topk_val;
        map<pair<string, string>, int> term_freq;
        map<pair<string, string>, int> doc_freq;
        map<pair<string, string>, double> score_per_doc;
        map<string, double> nnps;
        map<string, double> verbs;
        vector<pair<double, string> >sorted_nnps;
        vector<pair<double, string> >sorted_verbs;
        vector<string> topics;

        ClusterTopics(char* tf_dpath, int _topkval);
        bool BufpInc(FILE* fp);
        bool Read(FILE* fp);
        void GetDocFreq(vector<int>& doc_ids);
        void GetTermFreq(char* fpath);
        void NormalizeScore(double sum);
        void RecordScores();
        void CalculateScore();
        void ParseDocs(vector<int>& doc_ids);
        void SortByScore();
        void TopBigrams(vector<string>& topics);
        void GetTopics(vector<int> &doc_ids, vector<string> &topics);
        ~ClusterTopics();
};

ClusterTopics::ClusterTopics(char* tf_dpath, int _topkval = 3)
{
    topk_val = _topkval;
    tf_dir = tf_dpath;
    buf = new char[CHUNK_SZ + 4];
}

inline bool ClusterTopics::Read(FILE* fp)
{
    bufp = 0;
    buflen = fread(buf, 1, CHUNK_SZ, fp);
    return (buflen != 0);
}

inline bool ClusterTopics::BufpInc(FILE* fp)
{
    ++bufp;
    if(bufp < buflen)
        return true;
    return Read(fp);
}

void ClusterTopics::GetDocFreq(vector<int>& doc_ids)
{
    FILE* fp;
    char  fpath[MAX_PATH_LEN];
    char  word[MAX_WORD_SZ];
    int   word_len;
    int   tf_dir_len = strlen(tf_dir);
    pair<string, string>  temp;
    vector<int>::iterator it = doc_ids.begin();

    doc_freq.clear();
    strcpy(fpath, tf_dir);
    fpath[tf_dir_len] = '/';

    for(; it != doc_ids.end(); it++)
    {
        sprintf(fpath + tf_dir_len + 1, "%d", *it);
        fp = fopen(fpath, "r");
        bufp = 0;
        buflen = 0;

        while(BufpInc(fp))
        {
            word_len = 0;
            while(buf[bufp] != ' ')
            {
                word[word_len++] = buf[bufp];
                BufpInc(fp);
            }
            word[word_len] = '\0';
            temp.first = string(word);

            BufpInc(fp);

            word_len = 0;
            while(isalnum(buf[bufp]))
            {
                word[word_len++] = buf[bufp];
                BufpInc(fp);
            }
            word[word_len] = '\0';
            temp.second = string(word);

            BufpInc(fp);

            while(isdigit(buf[bufp]))
                BufpInc(fp);

            doc_freq[temp]++;
        }
        fclose(fp);
    }
}

void ClusterTopics::GetTermFreq(char* fpath)
{
    FILE* fp;
    char  word[MAX_WORD_SZ];
    int   word_len;
    int   frequency;
    pair<string, string>  temp;

    fp = fopen(fpath, "r");
    bufp = 0;
    buflen = 0;
    doc_word_cnt = 0;
    term_freq.clear();

    while(BufpInc(fp))
    {
        word_len = 0;
        while(buf[bufp] != ' ')
        {
            word[word_len++] = buf[bufp];
            BufpInc(fp);
        }
        word[word_len] = '\0';
        temp.first = string(word);

        BufpInc(fp);

        word_len = 0;
        while(isalnum(buf[bufp]))
        {
            word[word_len++] = buf[bufp];
            BufpInc(fp);
        }
        word[word_len] = '\0';
        temp.second = string(word);
        BufpInc(fp);

        frequency = 0;
        while(isdigit(buf[bufp]))
        {
            frequency = frequency*10 + buf[bufp] - '0';
            BufpInc(fp);
        }

        term_freq[temp] += frequency;
        doc_word_cnt += frequency;
    }
    fclose(fp);
}

void ClusterTopics::NormalizeScore(double sum)
{
    map< pair<string, string>, double>::iterator it = score_per_doc.begin();

    for(; it != score_per_doc.end(); it++)
        it->second /= sum;
}

void ClusterTopics::RecordScores()
{
    map< pair<string, string>, double>::iterator it = score_per_doc.begin();

    for(; it != score_per_doc.end(); it++)
    {
        if(it->first.second[0] == 'N')
            nnps[it->first.first] += it->second;
        else if(it->first.second[0] == 'V')
            verbs[it->first.first] += it->second;
    }
}

void ClusterTopics::CalculateScore()
{
    map< pair<string, string>, int>::iterator it = term_freq.begin();
    double a;
    double b;
    double l = 0.7;
    double c;
    double exp;
    double e;
    double per_doc_score_sum = 0;

    score_per_doc.clear();

    for(; it != term_freq.end(); it++)
    {
        a = doc_freq[it->first] / (double)sum_doc_freqs;
        b = it->second / (double)doc_word_cnt;
        c = (1-l)*a + l*b;
        exp = (l*b) / c;
        e = doc_word_cnt * exp;

        score_per_doc[it->first] = e;
        per_doc_score_sum += e;
    }

    NormalizeScore(per_doc_score_sum);
    RecordScores();
}

void ClusterTopics::ParseDocs(vector<int>& doc_ids)
{
    char  fpath[MAX_PATH_LEN];
    int   tf_dir_len = strlen(tf_dir);
    vector<int>::iterator it = doc_ids.begin();

    strcpy(fpath, tf_dir);
    fpath[tf_dir_len] = '/';
    nnps.clear();
    verbs.clear();

    for(; it != doc_ids.end(); it++)
    {
        sprintf(fpath + tf_dir_len + 1, "%d", *it);
        GetTermFreq(fpath);
        CalculateScore();
    }
}

void ClusterTopics::SortByScore()
{
    map<string, double>::iterator it;

    sorted_nnps.clear();
    sorted_verbs.clear();
    
    for(it = nnps.begin(); it!=nnps.end(); it++)
        sorted_nnps.push_back(make_pair(it->second, it->first));

    sort(sorted_nnps.rbegin(), sorted_nnps.rend());

    for(it = verbs.begin(); it!=verbs.end(); it++)
        sorted_verbs.push_back(make_pair(it->second, it->first));

    sort(sorted_verbs.rbegin(), sorted_verbs.rend());
}

void ClusterTopics::TopBigrams(vector<string>& topics)
{
    int l1 = min(topk_val, (int)sorted_nnps.size());
    int l2 = min(topk_val, (int)sorted_verbs.size());

    topics.clear();

    for(int i=0; i<l1; ++i)
    {
        topics.push_back(sorted_nnps[i].second);
    }

    for(int i=0; i<l2; ++i)
    {
        topics.push_back(sorted_verbs[i].second);
    }

    /*
    for(int i=0; i<l1; ++i)
    {
        for(int j=0; j<l2; ++j)
        {
            topics.push_back(sorted_nnps[i].second +
                             " " +
                             sorted_verbs[j].second);
        }
    }
    */
}

void ClusterTopics::GetTopics(vector<int>& doc_ids, vector<string>& topics)
{
    bufp = 0;
    buflen = 0;

    GetDocFreq(doc_ids);
    
    sum_doc_freqs = 0;
    map< pair<string, string>, int>::iterator it = doc_freq.begin();
    for(; it != doc_freq.end(); it++)
        sum_doc_freqs += it->second;

    ParseDocs(doc_ids);
    SortByScore();
    TopBigrams(topics);
}

ClusterTopics::~ClusterTopics()
{
    delete[] buf;
}

/*
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        puts("Usage: ./a.out <term_freq_dir>");
        return -1;
    }

    vector<int> v;
    for(int i=1; i<=4; ++i)
        v.push_back(i);

    ClusterTopics clt(argv[1]);
    clt.GetTopics(v);

	return 0;
}
*/
#endif
