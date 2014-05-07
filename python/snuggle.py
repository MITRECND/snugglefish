#!/usr/bin/env python

import sys
import pysnugglefish

from optparse import OptionParser

def main():
    parser = OptionParser()
    parser.add_option("-i", "--index", action="store_true", dest="index",
                      default=None, help="Index operation.")
    parser.add_option("-s", "--search", action="store_true", dest="search",
                      default=None, help="Search operation.")
    parser.add_option("-I", "--indexfile", action="store", dest="indexfile",
                      default="", help="Index file to search or create.")
    parser.add_option("-t", "--threads", action="store", dest="threads",
                      default="", help="Number of threads to spawn (default is #cpus).")

    (opts, searchstring) = parser.parse_args()

    if (not opts.index and not opts.search) or (opts.index and opts.search):
        print "[!] Must specify one of index or search."
        return

    if not opts.indexfile:
        print "[!] Must specify output location"
        return

    s = pysnugglefish.init(opts.indexfile)

    if opts.index:
        s.file_list = [ line.rstrip('\n') for line in sys.stdin.readlines() ]
        if opts.threads:
            try:
                threads = int(opts.threads)
            except Exception as e:
                print "[!] %s" % e
                return
            print "[+] Indexing %i files with %i threads." % (len(s.file_list),
                                                              threads)
            print "[+] This might take a while... ;)"
            s.make_index(threads)
        else:
            print "[+] Indexing %i files with default threads." % len(s.file_list)
            print "[+] This might take a while... ;)"
            s.make_index()
    elif opts.search:
        if searchstring:
            print "[+] Searching for %s" % searchstring
        else:
            searchstring = raw_input("Search string: ")

        if not searchstring:
            print "[!] Must enter a search string."
            return

        results = s.search(searchstring)
        for result in results:
            print result

if __name__ == '__main__':
    main()
