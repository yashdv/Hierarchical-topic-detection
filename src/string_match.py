#! /usr/bin/python

import sys
import os

def Searchx(fpath, ql):
    f = open(fpath)
    raw = f.read()
    raw = raw.lower()
    for q in ql:
        if raw.count(q) < 6:
            return;
    print fpath
    f.close()

def main():
    num_args = len(sys.argv)
    dir_name = sys.argv[1]
    ql = sys.argv[2:]

    files = os.listdir(dir_name)
    files.sort(key = lambda x: int(x))
    files = files[:1000]
    for file_ in files:
        fpath = os.path.join(dir_name, file_)
        Searchx(fpath, ql)

    

if __name__ == '__main__':
    main()
