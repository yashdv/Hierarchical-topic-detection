#! /usr/bin/python

import sys
import os
import nltk
import string

MAX_WORD_SZ = 100

stop_words_dict = {}
doc_freq = {}
allowed_tags = ['V', 'N']
printable = {}

stemmer = nltk.stem.PorterStemmer()

def LoadPrintable():
    global printable

    for c in string.printable:
        if c is not '?':
            printable[c] = 1

def LoadStopWords(fname):
    global stop_words_dict

    f = open(fname, 'r')
    for word in f:
        stop_words_dict[word.strip()] = 1
    f.close()

def UpdateDocFreq(word_freq):
    global doc_freq

    for key in word_freq:
        if key in doc_freq:
            doc_freq[key] += 1
        else:
            doc_freq[key] = 1

def WriteFreq(word_freq, out_fname):
    f = open(out_fname, 'w')
    l = sorted(word_freq.items())
    for key, val in l:
        f.write(key[0] + ' ' + key[1] + ' ' + str(val) + '\n')
    f.close()

def RecordTag(word_freq, word, tag):
    global stemmer
    global MAX_WORD_SZ
    global stop_words_dict

    word_len = len(word)
    while not word[word_len - 1].isalnum():
        word_len -= 1
        if word_len == 0:
            return

    word = word[:word_len]

    word = word.lower()
    if word_len < 3 or word_len > MAX_WORD_SZ or (word in stop_words_dict):
        return

    word = stemmer.stem(word)
    if word in stop_words_dict:
        return

    key = (word, tag)
    if key in word_freq:
        word_freq[key] += 1
    else:
        word_freq[key] = 1

def ParseFile(fname, out_fname):
    global allowed_tags

    print fname
    f = open(fname, 'r')
    html = f.read()
    f.close()

    word_freq = {}

    raw = nltk.clean_html(html)
    if len(raw) < 10:
        raw = html
    raw = ''.join([c if c in printable else ' ' for c in raw])

    sentences = nltk.sent_tokenize(raw)

    for sent in sentences:
        tokens = nltk.word_tokenize(sent)
        pos_tags = nltk.pos_tag(tokens)

        for word, tag in pos_tags:
            if tag[0] in allowed_tags:
                RecordTag(word_freq, word, tag[0])

    WriteFreq(word_freq, out_fname)
#    UpdateDocFreq(word_freq)

def IterateCorpusDir(dir_path, output_dir):
    files = os.listdir(dir_path)
    files.sort(key = lambda x: int(x))

    for file_ in files:
        fpath = os.path.join(dir_path, file_)
        out_fpath = os.path.join(output_dir, file_)
        ParseFile(fpath, out_fpath)

def main():
    global doc_freq

    if len(sys.argv) != 5:
        print ('Usage: ./a.out <StopWordsFile> <CorpusDir> <OutputDir>'
               ' <df-fpath>')
        print 'Note: df-fpath means filename/filepath and not just dir.'
        return

    LoadPrintable()
    LoadStopWords(sys.argv[1])
    IterateCorpusDir(sys.argv[2], sys.argv[3])
    
    doc_freq_out_path = os.path.join(sys.argv[4], "docfreq_pos_1K")
#    WriteFreq(doc_freq, doc_freq_out_path)

if __name__ == '__main__':
    main()
