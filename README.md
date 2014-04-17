Hierarchical-topic-detection
============================

Group documents in a hierarchy of clusters and extract topics from those
clusters.

### Compile and Run  
_______________________________  
Go to the bin directory and execute,  
`make`
  _______________________________
  
### External libraries

- [**NLTK**](http://www.nltk.org/) - You would need to download some nltk packages such as *maxent_treebank_pos_tagger* and *punkt* for the code to work. To download package you could execute nltk.download() in the python interpreter
- [**D3**](d3js.org)
- [**rapidjson**](https://code.google.com/p/rapidjson/)

_______________________________

The executables print the correct usage in case of an error.

- **parser.cpp** - Parses the documents. Generates document vector (term frequencies) and document frequency for all words.  
  
- **parse_and_pos_tag.py** - This is a slower but a better parser. It cleans the html, non-printable characters etc. It finds parts of speech (POS) and makes document vector of only specified POS.
  
- **calc_doc_freq.cpp** - Calculates document frequency for all documents in the corpus from the document vector. Works with the output of *parse_and_pos_tag.py*.  
  
- **porter_stemmer_thread_safe.hpp** - Stem words using the Porter Stemming algorithm.
  
- **similarity.cpp** - Generates the all pair document similarity matrix.  
  
- **cluster.cpp** - Performs agglomerative hierarchical clustering and finds topics. This code generates the binary tree, mappings of each node in tree to its topics and the json file used by *output/collapse.html*.  
  
- **topics.hpp** - Finds topics from a cluster of document. Used by *cluster.cpp*  
  
