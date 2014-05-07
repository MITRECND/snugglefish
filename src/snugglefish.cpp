/*
Approved for Public Release; Distribution Unlimited: 13-1937

Copyright (c) 2014 The MITRE Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

*/


// Snugglefish.cpp
// Sample NGram Fast Indexer and Search (SNGFIS)
// Allows for the indexing and searching of a large amount of samples in a short 
// period of time.

#include <nGramSearch.h>
#include <nGramIndex.h>
#include <fileIndexer.h>
#include <snugglefish.h>
#include <iostream>
#include <sstream>
#include <getopt.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

#include <cerrno>
#include <unistd.h>
#include <cstring>

#include <utility>
#include <pthread.h>

#include "common.h"

using namespace std;
using namespace snugglefish;

#define NGRAM_SIZE_OPTION_LONG "ngramsize"
#define NGRAM_SIZE_OPTION_SHORT 'n'
#define HELP_OPTION_LONG "help"
#define HELP_OPTION_SHORT 'h'
#define INDEX_OPTION_SHORT 'i'
#define INDEX_OPTION_LONG "index"
#define SEARCH_OPTION_SHORT 's'
#define SEARCH_OPTION_LONG "search"
#define OUTPUT_OPTION_SHORT 'o'
#define OUTPUT_OPTION_LONG "output"
#define FILE_OPTION_SHORT 'f'
#define FILE_OPTION_LONG "file"
#define NODE_BUFFER_OPTION_LONG "node_bound"
#define NODE_BUFFER_OPTION_SHORT 'b'
#define THREADS_OPTION_LONG "threads"
#define THREADS_OPTION_SHORT 't'
#define SHORT_OPTIONS_STRING "n:hsio:f:b:t:"


uint32_t cpu_count(){
    long procs = -1;

    //This only works on some *nixes TODO figure out which systems don't support this call
    procs = sysconf(_SC_NPROCESSORS_ONLN);
    if (procs < 1){
        cerr << "Unable to get CPU count, defaulting to 1 -- Error: " 
             << strerror(errno) << endl;
        return 1;
    }
    return (uint32_t) procs;
}



void handler(int sig) {
  void *array[10];
  int size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, 2);
  exit(1);
}

void printHelp();


int main(int argc, char *argv[]){
    int c, option_index = 0;
    uint32_t ngramSize = 3;
    uint32_t max_files = 0;
    uint64_t max_buffer = 0;
    uint32_t threads = 0;

    signal(SIGSEGV, handler); 

    string commandString, indexFileName = "", searchString;
    vector <string> fileList;
    static struct option long_options[] ={
        // Ngramsize defaults to 3 if not present
        {INDEX_OPTION_LONG, no_argument, 0, INDEX_OPTION_SHORT},
        {SEARCH_OPTION_LONG, no_argument, 0, SEARCH_OPTION_SHORT},
        {FILE_OPTION_LONG, required_argument, 0, FILE_OPTION_SHORT},
        {OUTPUT_OPTION_LONG, required_argument, 0, OUTPUT_OPTION_SHORT},
        {NGRAM_SIZE_OPTION_LONG, required_argument, 0, NGRAM_SIZE_OPTION_SHORT},
        {HELP_OPTION_LONG, no_argument, 0, HELP_OPTION_SHORT},
        {THREADS_OPTION_LONG, required_argument, 0, THREADS_OPTION_SHORT},
        {0,0,0,0}
    };
    bool indexFlag = false, searchFlag = false;

    // Default threads to the number of cpus
    threads = cpu_count();

    // loop over all of the options
    while ((c = getopt_long(argc, argv, SHORT_OPTIONS_STRING, long_options, &option_index)) != -1){
        // check to see if a single character or long option came through
        switch (c){
            // short option 't'
            case THREADS_OPTION_SHORT:
                {
                    istringstream tss(optarg);
                    uint32_t thr = 0;

                    if(!(tss >> thr)){
                        cout << "Invalid number of threads, please enter an integer" << endl;
                        return 0;
                    }else{
                        threads = thr;
                    }

                    break;
                }
            case SEARCH_OPTION_SHORT:
                searchFlag = true;
                break;
            case INDEX_OPTION_SHORT:
                indexFlag = true;
                break;
                // Outpur and file options are the same
            case OUTPUT_OPTION_SHORT:
            case FILE_OPTION_SHORT:
                indexFileName = optarg;
                break;
            case HELP_OPTION_SHORT:
                printHelp();
                return 0;
                // short option 'a'
            case NGRAM_SIZE_OPTION_SHORT:
                {
                    //string ngramSizeString(optarg);
                    istringstream ss(optarg);
                    //stringstream ss;
                    //ss << ngramSizeString;
                    if(!(ss >> ngramSize)){
                        cout << "Invalid ngram size, please enter an integer" << endl;
                        return 0;
                    }
                    if(ngramSize < 3 || ngramSize > 4){
                        cout << "Ngram size must be 3 or 4. Ngram size is " << ngramSize << endl;
                        return 0;
                    }
                    break;
                }

            case NODE_BUFFER_OPTION_SHORT:
                {
                    istringstream ss(optarg);
                    uint64_t tmax_buffer = 0;
                    if(!(ss >> tmax_buffer)){
                        cout << "Invalid maxmimum node buffer, please enter an integer" << endl;
                        return 0;
                    }

                    if(tmax_buffer < 4){
                        cout << "Node buffer size must be at least 4GB in size" << endl;
                        return 0;
                    }
            
                    max_buffer = tmax_buffer * 1073741824; //in Gigabytes 
                    break;
                }
                    
        }
    }
    /* the rest of the command line arguments 
     * each option needs at least an aditional parameter*/
    if((!indexFlag && !searchFlag)){
        cout << "Must specify -i for index or -s for search" << endl;
        return 0;
    }

    if((indexFlag && searchFlag)){
        cout << "Must specify only one of -i or -s" << endl;
        return 0;

    }
    if(indexFileName.empty()){
        cout << "Index file necessary" << endl;
        return 0;
    }


    if(indexFlag){
        // get the list of files to add

        for(; optind < argc; optind++){
            fileList.push_back(argv[optind]);
        }
        // if fileList is empty, it means we need to get the
        // filenames from stdin
        if(fileList.empty()){
            string fileName;
            while(cin){
                getline(cin,fileName);
                if(fileName.empty())
                    break;
                fileList.push_back(fileName);
            }
        }
        //Eventually options should be sent as a structure
        make_index(indexFileName, fileList, ngramSize, max_files, max_buffer, threads);
    } else if(searchFlag) {
        // Get the string to search for
		if (optind < argc) {
			searchString = argv[optind];
		} else {
			cin >> searchString;
		}
	    if (searchString.size() < ngramSize){
	        cout << "Search string size is smaller than Ngram size, the search string must be greater than or equal to the ngram size" << endl;
	    } else {
			vector<string>* found = search(indexFileName, searchString, ngramSize, threads);
			for(uint32_t i = 0; i < found->size(); i++){
       			cout << (*found)[i] << endl;
   			}
            delete found;
		}
    } else{
        printHelp();
    }

    return 0;
}

void printHelp(){
    cout << "Usage: snugglefish [OPTIONS]" << endl;
    cout << "Index files based on Ngrams then Search for a string" << endl;
    cout << "-i, --index            Index Operation, requires -o, and files to index" << endl;
    cout << "-s, --search           Search Operation, requires -f, and search string" << endl;
    cout << "-o, --output           Specifies the output file for indexing, equivalent to -f" << endl;
    cout << "-f, --file             Index file to search, equivalent to -o" << endl;
    cout << "-b, --node_bound       Maximum node buffer memory size before flushing" << endl;
    cout << "-n, --ngramsize        The size of Ngram to use (default is 3)" << endl;
    cout << "-h, --help             This help screen" << endl;
    cout << "-t, --threads          Number of search threads to spawn (default is #cpus)" << endl;
    cout << "Examples:" << endl;
    cout << "Index: snugglefish [-n ngramsize]  -i -o <index filename> <files to index>" << endl;
    cout << "If no file names are given on the commandline, the stdin will be used" << endl;
    cout << "Search: snugglefish [-n ngramsize] -s -f <index filename> <search string>" << endl;
    cout << "If no string is given, it can be entered on the command line" << endl;


}


void printStats(nGramIndex* indexer, uint64_t processed, uint64_t listsize){
    uint64_t total;
    uint64_t session;
    uint64_t indexes;
    bool flushing;
    indexer->getStats(total, session, indexes, flushing);

    uint64_t percent = ((double) processed / listsize) * 100;

    cerr << "\r";
    cerr << "Processed: " << percent << "% -- " << processed << "/" << listsize;

    if(flushing){
        cerr << " (Flushing ... )"; 
    }else{
        cerr << "                "; 
    }
}

void* indexerThread(void* input){
    mi_data* midata = (mi_data*) input;
    nGramIndex* ngramindex = (nGramIndex*) midata->ngramindex;
    fileIndexer indexer(midata->ngramSize);

    while(1){
        pthread_mutex_lock(& midata->filesMutex);
        if (midata->queue >= midata->fileList->size()){
            pthread_mutex_unlock(& midata->filesMutex);
            break;
        }
        uint32_t i = midata->queue++; 
        pthread_mutex_unlock(& midata->filesMutex); 

        try{
            vector<uint32_t>* processedFile = indexer.processFile((*(midata->fileList))[i].c_str());
            if(processedFile != 0){
                pthread_mutex_lock(& midata->nGramIndexMutex);
                ngramindex->addNGrams(processedFile, (*(midata->fileList))[i]);
                pthread_mutex_unlock(& midata->nGramIndexMutex);
            }
        }catch(exception& e){
            cout << "Error in thread:" << e.what() << endl;
            handler(SIGSEGV);
        }
    }
    return 0;
}

void make_index(string indexFileName, vector <string> fileNames, uint32_t ngramSize, uint32_t max_files, uint64_t max_buffer, uint32_t threads){

    pthread_t* indexers;
    mi_data* midata;
    void* status;


    midata = new mi_data;
    indexers = (pthread_t*) malloc(threads * sizeof(pthread_t));


    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_mutex_init(& (midata->filesMutex), NULL);
    pthread_mutex_init(& (midata->nGramIndexMutex), NULL);

    midata->fileList = & fileNames;
    midata->ngramSize = ngramSize;
    midata->queue = 0;

    try{
        nGramIndex ngramindex(ngramSize, indexFileName);
        if (max_buffer > 0){
            ngramindex.setmaxBufferSize(max_buffer);
        }

        midata->ngramindex = &ngramindex;

        for(uint32_t i = 0; i < threads; i++){
            pthread_create(& indexers[i], & attr, indexerThread, (void*) midata);
        }

        while(1){
            //Usage of mutex shouldn't matter
            if (midata->queue >= fileNames.size()){
                break;
            }
            
            printStats(&ngramindex, midata->queue, fileNames.size());
            sleep(1);
        }


        for(uint32_t i = 0; i < threads; i++){
            pthread_join(indexers[i], &status);
        }

        //Print some final stats
        printStats(&ngramindex, midata->queue, fileNames.size());
        cerr << endl;

    } catch (exception& e){
        cout << "Error:" << e.what() << endl;
        handler(SIGSEGV);
    }


    pthread_mutex_destroy(&(midata->filesMutex));
    pthread_mutex_destroy(&(midata->nGramIndexMutex));

    delete midata;
    free(indexers);
}


vector<string>* search(string indexFileName, string searchString, uint32_t ngramSize, uint32_t threads){
	vector<string>* ret;
	nGramSearch searcher(ngramSize, indexFileName, threads);
    vector<uint64_t>* ngrams = searcher.stringToNGrams(searchString);
	ret = searcher.searchNGrams(*ngrams);

    delete ngrams;
    return ret;
}
