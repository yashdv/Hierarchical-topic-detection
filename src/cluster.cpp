#include<iostream>
#include<cstdio>

using namespace std;

//cluster to store the details
class Cluster
{
	public:	
		vector<int> doc_ids;		
};


class Similarity
{
	public:
		char* unigram_dpath;	
		vector<vector<double> > sim;

		Similarity(char* unigram_dp);
};

Similarity::Similarity(char* unigram_dp)
{
	unigram_dpath=unigram_dp;
}

void Similarity::FindSim()
{
	DIR* dir=opendir(unigram_dpath);
	if(dir==NULL)
	{
		puts("Error:can't open directory");
		exit(-1);
	}

	struct dirent* file;
	char fpath[MAX_PATH_LEN];
	int unigram_dpath_len = strlen(unigram_dpath);

	strcpy(fpath, unigram_dpath);
	fpath[unigram_dpath_len] = '/';

	while((file = readdir(dir)) != NULL)
	{
		if(file->d_name[0] == '.')
			continue;

		strcpy(fpath + unigram_dpath_len + 1, file->d_name);
		DIR* dir2=opendir(unigram_dpath);
		struct dirent* file2;
		char fpath2[MAX_PATH_LEN];
		int unigram_dpath_len2 = strlen(unigram_dpath);
		
		strcpy(fpath2, unigram_dpath);
		fpath2[unigram_dpath_len2] = '/';

		while((file2 = readdir(dir2)) != NULL)
		{
			if(file2->d_name[0] == '.')
				continue;

			strcpy(fpath2 + unigram_dpath_len2 + 1, file2->d_name);
			

		}
		closedir(dir);
	}
}
int main()



