#!/usr/bin/env python

import sys
import pysnugglefish
import multiprocessing

from optparse import OptionParser

def main():
    cpu_count = multiprocessing.cpu_count()

    parser = OptionParser()
    parser.add_option("-i", "--index", action="store_true", dest="index",
                      default=None, help="Index operation.")
    parser.add_option("-s", "--search", action="store_true", dest="search",
                      default=None, help="Search operation.")
    parser.add_option("-I", "--indexfile", action="store", dest="indexfile",
                      default="", help="Index file to search or create.")
    parser.add_option("-t", "--threads", action="store", dest="threads",
                      default=cpu_count,
                      help="Number of threads to spawn (default is #cpus).")

    (opts, searchstring) = parser.parse_args()

    if (not opts.index and not opts.search) or (opts.index and opts.search):
        print "[!] Must specify one of index or search."
        return

    if not opts.indexfile:
        print "[!] Must specify output location"
        return

    s = pysnugglefish.init(opts.indexfile)

    try:
        threads = int(opts.threads)
    except Exception as e:
        print "[!] Invalid threads: %s" % e.message
        return

    if threads <= 0:
        print "[!] Invalid threads. Defaulting to %i." % cpu_count
        threads = cpu_count

    if opts.index:
        s.file_list = [ line.rstrip('\n') for line in sys.stdin.readlines() ]
        msg = "[+] Indexing %i files with %i threads." % (len(s.file_list),
                                                          threads)
        print "[+] This might take a while... ;)"
        try:
            s.make_index(threads)
        except Exception as e:
            print "[!] Exception while indexing: %s" % e.message
    elif opts.search:
        searchstring = ' '.join(searchstring)
        if not searchstring:
            searchstring = raw_input("Search string: ")

        if not searchstring:
            print "[!] Must enter a search string."
            return

        print "[+] Searching for %s with %i threads" % (searchstring, threads)
        try:
            results = s.search(searchstring, threads)
            for result in results:
                print result
        except Exception as e:
            print "[!] Exception while searching: %s" % e.message

if __name__ == '__main__':
    main()
