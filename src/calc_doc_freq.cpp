#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<dirent.h>
#include<map>
#include<string>
#include<utility>

using namespace std;

#define ALPHABET_SZ 26
#define MAX_SW_SZ 1024
#define MAX_PATH_LEN 1024
#define CHUNK_SZ 100000000
#define MAX_WORD_SZ 100

int bufp;
int buflen;
char* buf;
char* tf_dir;
char* df_fpath;
map< pair<string, string>, int> doc_freq;

void initx(char* tf_dpath, char* dffpath);
bool Read(FILE* fp);
bool BufpInc(FILE* fp);
void WriteFreq(char* outfpath, map< pair<string, string>, int>& mp);
void ParseFile(char* fpath);
void IterateDir();

void initx(char* tf_dpath, char* dffpath)
{
    tf_dir = tf_dpath;
    df_fpath = dffpath;
    bufp = 0;
    buflen = 0;
    buf = new char[CHUNK_SZ + 4];
}

inline bool Read(FILE* fp)
{
    bufp = 0;
    buflen = fread(buf, 1, CHUNK_SZ, fp);
    return (buflen != 0);
}

inline bool BufpInc(FILE* fp)
{
    ++bufp;
    if(bufp < buflen)
        return true;
    return Read(fp);
}

void WriteFreq(char* outfpath, map< pair<string, string>, int>& mp)
{
    FILE* fp = fopen(df_fpath, "w");
    map<pair<string, string>, int>::iterator it = mp.begin();

    for(; it != mp.end(); it++)
        fprintf(fp, "%s %s %d\n", it->first.first.c_str(),
                                  it->first.second.c_str(),
                                  it->second);
    fclose(fp);
}

void ParseFile(char* fpath)
{
    FILE* fp = fopen(fpath, "r");
    int word_len;
    char word[MAX_WORD_SZ];
    pair<string, string> temp;

    puts(fpath);
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

void IterateDir()
{
    DIR* dir = opendir(tf_dir);

    if(dir == NULL)
    {
        puts("Error: Cant open directory");
        exit(-1);
    }

    struct dirent* file;
    char fpath[MAX_PATH_LEN];
    int tf_dir_len = strlen(tf_dir);

    strcpy(fpath, tf_dir);
    fpath[tf_dir_len] = '/';

    while((file = readdir(dir)) != NULL)
    {
        if(file->d_name[0] == '.')
            continue;

        strcpy(fpath + tf_dir_len + 1, file->d_name);
        ParseFile(fpath);
    }
    closedir(dir);

    if(!doc_freq.empty())
        WriteFreq(df_fpath, doc_freq);
}

void Endx()
{
    delete[] buf;
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        puts("Usage: ./a.out <term_freq_dir> <doc_freq_fpath>");
        return -1;
    }

    initx(argv[1], argv[2]);
    IterateDir();
    Endx();

    return 0;
}
