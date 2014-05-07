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


#include "nGramSearch.h"
#include <iostream>
#include <queue>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <utility>
#include <pthread.h>
#include "indexSet.h"

using namespace snugglefish;
using namespace std;


nGramSearch::nGramSearch( uint32_t ngramLength, string indexFileName)
    :nGramBase(ngramLength, indexFileName),numThreads(1) {

    masterFile = new smFile(baseFileName, ngramLength);

    if (!masterFile->exists()){
        //some error
    }else{
        masterFile->open('r');
    }


    numIndexFiles = masterFile->getNumIndexFiles();
    numFiles = masterFile->getNumFiles();
}

nGramSearch::nGramSearch( uint32_t ngramLength, string indexFileName, uint32_t threads)
    :nGramBase(ngramLength, indexFileName),numThreads(threads) {

    masterFile = new smFile(baseFileName, ngramLength);

    if (!masterFile->exists()){
        //some error
    }else{
        masterFile->open('r');
    }


    numIndexFiles = masterFile->getNumIndexFiles();
    numFiles = masterFile->getNumFiles();
    
}

nGramSearch::~nGramSearch(){
   delete masterFile; 
}


vector<uint64_t>* nGramSearch::stringToNGrams(string searchString){
    uint64_t nGram;
    vector <uint64_t>* ngrams = new vector<uint64_t>;


    for(size_t i = 0; i + ngramLength - 1 < searchString.length(); i++){
        nGram = 0;
        for(size_t j = 0; j < ngramLength; j++){
	    // (1 << (8*j)) is equivalent to pow(256,j)
            nGram += (unsigned char)searchString[i+j] * (1 << (8*j));
        }
        ngrams->push_back(nGram);
    }

    return ngrams;
}


vector<string>* nGramSearch::searchNGrams(vector<uint64_t> nGramQuery){
    vector<string>* matchedFiles = new vector<string>;
    pthread_t * threads;
    thread_data* tdata;
    void* status;

    tdata = new thread_data();

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    pthread_mutex_init(&(tdata->queueMutex), NULL);
    pthread_mutex_init(&(tdata->smFileMutex), NULL);
    pthread_mutex_init(&(tdata->mfMutex), NULL);

    tdata->ngramLength = ngramLength;
    tdata->queue = 0;
    tdata->maximumIndex = numIndexFiles;

    tdata->masterFile = (this->masterFile);
    tdata->matchedFiles = matchedFiles;
    tdata->baseFileName = &baseFileName;
    tdata->nGramQuery = &(nGramQuery);

    threads = (pthread_t*) malloc(numThreads * sizeof(pthread_t));

    //Create the threads
    for(uint32_t i = 0; i < numThreads; i++){
        pthread_create(& threads[i], &attr, searchNGramThread, (void*) (tdata));
    }
    
    //Join on them
    for(uint32_t i = 0; i < numThreads; i++){
        pthread_join(threads[i], &status);
    }

    pthread_mutex_destroy(&(tdata->queueMutex));
    pthread_mutex_destroy(&(tdata->smFileMutex));
    pthread_mutex_destroy(&(tdata->mfMutex));
    pthread_attr_destroy(&attr);

    free(threads);
    delete tdata;
    masterFile->close();
    return matchedFiles;

}

void* nGramSearch::searchNGramThread(void* input){


    thread_data* tdata = (thread_data*) input;

    while(1){
        pthread_mutex_lock(& tdata->queueMutex);
        if (tdata->queue >= tdata->maximumIndex){
            pthread_mutex_unlock(& tdata->queueMutex);
            break;
        }

        uint32_t i = tdata->queue++;
        pthread_mutex_unlock(& tdata->queueMutex);

        indexSet* tIndex = new indexSet(tdata->baseFileName->c_str(), i, tdata->ngramLength);
        tIndex->open();

        //Get ordered list of NGrams
        // In ascending order by number of files that contain that ngram
        //list<index_entry> queryList = orderNGrams(nGramQuery);
        list< pair<uint64_t,size_t> > queryList = orderNGrams(tIndex, *(tdata->nGramQuery));

        //Get list of File Ids that match NGrams
        list<ngram_t_fidtype> matchedIds = searchAlpha((indexSet*) tIndex, queryList);

        //Convert File IDs to filenames
        list<ngram_t_fidtype>::iterator ft = matchedIds.begin();
        while(ft != matchedIds.end()){
            pthread_mutex_lock(& tdata->smFileMutex);
            string matched_filename = tdata->masterFile->getFilebyId(*ft);
            pthread_mutex_unlock(& tdata->smFileMutex);

            pthread_mutex_lock(& tdata->mfMutex);
            tdata->matchedFiles->push_back(matched_filename);
            pthread_mutex_unlock(& tdata->mfMutex);
            ft++;
        }

        tIndex->close();
        delete tIndex;
    }

    return NULL;
}

list< pair<uint64_t, size_t> > nGramSearch::orderNGrams(indexSet* index, const vector<uint64_t> & nGramQuery){
    bool nomatch = false;
    list< pair<uint64_t, size_t> > queryList;
    for(uint32_t j = 0; j < nGramQuery.size(); j++){
        size_t numfiles = index->getNGramCount(nGramQuery[j]);
        //index_entry* index_table = (index_entry*) (indexEntries + (nGramQuery[j] * (INDEX_ENTRY_SIZE)));

        if(numfiles == 0){
            nomatch = true;
            break;
            //No Files Match
        }

        if(queryList.empty()){
            queryList.push_back(pair<uint64_t,size_t>(nGramQuery[j], numfiles));
        }else{
            bool placed = false;
            for (list< pair<uint64_t, size_t> >::iterator it = queryList.begin(); 
              it != queryList.end(); it++ ){
                if((*it).second > numfiles){
                    queryList.insert(it, pair<uint64_t, size_t>(nGramQuery[j], numfiles));
                    placed = true;
                    break;
                }
            }
            if (!placed){
                queryList.push_back(pair<uint64_t,size_t>(nGramQuery[j], numfiles));
            }
        }
    }

    if (nomatch){
        queryList.clear();
    }

    return queryList;

}


// Takes the list of sorted fids, and puts the common fids into matchedIds
list<ngram_t_fidtype> nGramSearch::searchAlpha(indexSet* index, list< pair<uint64_t,size_t> > &queryList){
    list<ngram_t_fidtype> matchedIds;

    if(queryList.size() == 0){
        return matchedIds;
    }
    // this gets the ngram list for the first file, and places them 
    // into the matchedIds list
    ngram_t_numfiles count = 0;
    ngram_t_fidtype* ngrams = index->getNGrams((uint64_t) queryList.front().first, (size_t*) &count);
    for (ngram_t_numfiles j = 0; j < count; j++){
        matchedIds.push_back(ngrams[j]);
    }
        
    //Dont need the front element anymore so get rid of it
    queryList.pop_front();

    //For every id see if it's in the remaining Ngrams
    //Now for every subsequent NGram whittle down the list
    for(list< pair<uint64_t, size_t> >::iterator it = queryList.begin();
            it != queryList.end(); it++){

        ngram_t_numfiles ngram_elements = 0;
        ngram_t_fidtype* ngrams = index->getNGrams((uint64_t) (*it).first, (size_t*) &ngram_elements);
        uint32_t ngram_index = 0;

        list<ngram_t_fidtype>::iterator ft = matchedIds.begin();
        while(ft != matchedIds.end()){
            bool found = false;
            // Checks each element of the next fid array for
            // the current fid in matchedIds
            while(ngram_index < ngram_elements){
                if (ngrams[ngram_index] == (*ft)){
                    found = true;
                    break;
                }else if(ngrams[ngram_index] > (*ft)){
                    //update ngram location
                    // If the fid is too large, it is
                    // necessary to recheck the same index on the
                    // next pass, as to make sure it isn't missed
                    break;
                }
                ngram_index++;
            }

            if(!found){
                ft = matchedIds.erase(ft);
            }else{
                // ft++ is only needed if erase isn't called as 
                // erase moves the iterator ahead
                ft++;
            }
        }
    }

    return matchedIds;
}
