#include<cstdio>
#include<iostream>
#include<dirent.h>
#include<map>
#include "porter_stemmer_thread_safe.hpp"

using namespace std;

#define ALPHABET_SZ 26
#define MAX_SW_SZ 1024
#define MAX_PATH_LEN 1024
#define CHUNK_SZ 100000000
#define MAX_WORD_SZ 100

// STOP WORDS TRIE NODE

class StopWordsTrieNode
{
    public:

        bool is_word;
        StopWordsTrieNode* children[ALPHABET_SZ];

        StopWordsTrieNode();
};

StopWordsTrieNode::StopWordsTrieNode()
{
    is_word = false;
    for(int i=0; i<ALPHABET_SZ; ++i)
        children[i] = NULL;
}

// STOP WORDS TRIE

class StopWordsTrie
{
    public:

        StopWordsTrieNode* root;

        StopWordsTrie();
        void Insert(char* word);
        bool Search(char* word);
        void Clear(StopWordsTrieNode* p);
        ~StopWordsTrie();
};

StopWordsTrie::StopWordsTrie()
{
    root = new StopWordsTrieNode;
}

inline void StopWordsTrie::Insert(char* word)
{
    StopWordsTrieNode* p = root;
    int child_idx;

    for(; *word!='\0'; word++)
    {
        child_idx = *word - 'a';
        if(p->children[child_idx] == NULL)
            p->children[child_idx] = new StopWordsTrieNode;
        p = p->children[child_idx];
    }
    p->is_word = true;
}

inline bool StopWordsTrie::Search(char* word)
{
    StopWordsTrieNode* p = root;
    int child_idx;

    for(; *word!='\0'; word++)
    {
        if(!isalpha(*word))
            return false;

        child_idx = *word - 'a';
        if(p->children[child_idx] == NULL)
            return false;

        p = p->children[child_idx];
    }
    return p->is_word;
}

void StopWordsTrie::Clear(StopWordsTrieNode* p)
{
    for(int i=0; i<ALPHABET_SZ; ++i)
        if(p->children[i] != NULL)
            Clear(p->children[i]);
    delete p;
}

StopWordsTrie::~StopWordsTrie()
{
    Clear(root);
}

// NEWS CORPUS PARSER

class Parser
{
    public:
        StopWordsTrie sw_trie;
        int FILE_NUM;
        int bufp;
        int buflen;
        char* buf;
        char* corpus_dir;
        char* out_dir;
        struct stemmer* z;
        map<string, int> freq;

        Parser(char* stop_words_fpath, char* corpus_dpath, char* outd);
        bool Read(FILE* fp);
        bool BufpInc(FILE* fp);
        void WriteFreq(char* outfpath);
        void RecordWord(char* word, int word_len);
        void ParseFile(char* fname);
        void IterateDir();
        ~Parser();
};

Parser::Parser(char* stop_words_fpath, char* corpus_dpath, char* outd) :
    corpus_dir(corpus_dpath), FILE_NUM(0), out_dir(outd), bufp(0), buflen(0)
{
    char stop_word[MAX_SW_SZ];
    FILE* fp = fopen(stop_words_fpath, "r");

    while(fscanf(fp, "%s", stop_word) != EOF)
        sw_trie.Insert(stop_word);
    fclose(fp);

    buf = new char[CHUNK_SZ + 4];
    z = create_stemmer();
}

inline bool Parser::Read(FILE* fp)
{
    bufp = 0;
    buflen = fread(buf, 1, CHUNK_SZ, fp);
    return (buflen != 0);
}

inline bool Parser::BufpInc(FILE* fp)
{
    ++bufp;
    if(bufp < buflen)
        return true;
    return Read(fp);
}

void Parser::WriteFreq(char* outfpath)
{
    FILE* fp = fopen(outfpath, "w");
    map<string, int>::iterator it = freq.begin();

    for(; it != freq.end(); it++)
        fprintf(fp, "%s %d\n", it->first.c_str(), it->second);
    fclose(fp);
}

void Parser::RecordWord(char* word, int word_len)
{
    if(word_len < 3 || word_len > MAX_WORD_SZ)
        return;

    if(!sw_trie.Search(word))
    {
        word_len = stem(z, word, word_len - 1) + 1;
        word[word_len] = '\0';

        if(!sw_trie.Search(word))
            freq[string(word)]++;
    }
}

void Parser::ParseFile(char* fname)
{
    FILE* fp = fopen(fname, "r");
    int word_len;
    char word[MAX_WORD_SZ];

    freq.clear();

    while(BufpInc(fp))
    {
        word_len = 0;

        while(isalnum(buf[bufp]))
        {
            if(word_len < MAX_WORD_SZ)
                word[word_len++] = tolower(buf[bufp]);
            BufpInc(fp);
        }

        if(word_len < MAX_WORD_SZ)
            word[word_len] = '\0';

        RecordWord(word, word_len);
    }

    if(!freq.empty())
    {
        ++FILE_NUM;
        char outfpath[MAX_PATH_LEN];
        sprintf(outfpath, "%s/%d", out_dir, FILE_NUM);
        WriteFreq(outfpath);
    }

    fclose(fp);
}

void Parser::IterateDir()
{
    DIR* dir = opendir(corpus_dir);

    if(dir == NULL)
    {
        puts("Error: Cant open directory");
        exit(-1);
    }

    struct dirent* file;
    char fpath[MAX_PATH_LEN];
    int corpus_dir_len = strlen(corpus_dir);

    strcpy(fpath, corpus_dir);
    fpath[corpus_dir_len] = '/';

    while((file = readdir(dir)) != NULL)
    {
        if(file->d_name[0] == '.')
            continue;

        strcpy(fpath + corpus_dir_len + 1, file->d_name);
        ParseFile(fpath);
    }

    closedir(dir);
}

Parser::~Parser()
{
    delete[] buf;
    free_stemmer(z);
}

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        puts("Usage: ./a.out <StopWordsFile> <CorpusDir> <OutputDir>");
        return -1;
    }

    Parser par(argv[1], argv[2], argv[3]);
    par.IterateDir();

    return 0;
}
