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


#include "fileIndexer.h"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include <vector>
#include <map>


#include "file.h"

using namespace snugglefish;
using namespace std;

fileIndexer::fileIndexer(uint8_t ngramLength):
filesProcessed(0)
{

    if(ngramLength == 3  || ngramLength == 4){
        this->ngramLength = ngramLength;
        this->maxNgram = ((uint64_t) (1) << (8*ngramLength));
        this->pagesize = getpagesize();
    }
    else {
        throw std::runtime_error("Ngram len must be either 3 or 4");
    }

}

// Takes in a file name, mmaps it, and creates a vector of ngrams
// return: pointer to vector of ngrams
vector<uint32_t>* fileIndexer::processFile(const char* fileName){

    vector<uint32_t>* ngramList = 0;
    file* inputFile = new file(fileName);
    size_t fileSize = inputFile->get_size();
    uint8_t* inputFileMap = inputFile->mmap();

    if (!fileSize)
        return NULL; 


    this->filesProcessed++;

    //It is much faster to just use an array that holds a boolean
    //for every element, this requires only ~64MB per file for 3-byte Ngrams
    //but 16GB for 4 byte ngrams, so only do this for 3-byte ngrams
    //and do a map instead for 4-byte
    if(this->ngramLength == 3){
        ngramList = new vector<uint32_t>;
        bool* bngramList = new bool[this->maxNgram]();

        try{
            processNgrams(inputFileMap, fileSize, bngramList);

            for(uint32_t k = 0; k < this->maxNgram; k++){
                if(bngramList[k]){
                    ngramList->push_back(k);
                }
            }
            
            delete[] bngramList;

        } catch(exception &e){
            cout << "Error processing ngrams: "<< e.what() << endl;
        }

    }else{ //ngramLength == 4
        try {
            ngramList = processNgrams(inputFileMap, fileSize);
        } catch(exception &e){
            cout << "Error processing ngrams: " << e.what() << endl;
        }
    }

    inputFile->close();
    delete inputFile;

    return ngramList;
}


//Creates vector of ngrams in a given file
//Uses an STL map which should use a RED/BLACK tree
//Insertion/Search should be O(log(n)) time where n is the 
//number of nodes already in the tree
vector<uint32_t>* fileIndexer::processNgrams(unsigned char* buf, uint64_t fileSize){
    map<uint32_t, bool> ngram_map;

    uint32_t nGram = 0;
    uint32_t i, j;

    for(i = 0; ((i + this->ngramLength) - 1) < fileSize; i++){
        nGram = 0;
        for(j = 0; j < this->ngramLength; j++){
            nGram += (unsigned char)buf[i+j] * (1 << (8*j));
        }
        if(nGram >= this->maxNgram){
            throw std::runtime_error("Ngram greater than maxNgram");
        }
        ngram_map.insert(pair<uint32_t,bool>(nGram, true));
    }

    //We should now have a map of ngrams, turn into a sorted vector
    //TODO pre-allocate vector to do this faster
    vector<uint32_t>* ngramVector = new vector<uint32_t>;
    
    for(map<uint32_t,bool>::iterator it = ngram_map.begin(); it != ngram_map.end() ; it++){
        ngramVector->push_back(it->first);
    }

    return ngramVector;
}

// Takes the file and creates ngrams from it
void fileIndexer::processNgrams(unsigned char* buf, uint64_t fileSize, bool ngramList[]){
    //TODO update with byte array
    uint64_t nGram = 0;
    uint64_t i, j;
    for(i = 0; ((i + this->ngramLength) - 1) < fileSize; i++){
        nGram = 0;
       // creates an ngram using the formula buf[i] + buf[i+1]*256 
       // + buf[i+2]*256*256
        for(j = 0; j < this->ngramLength; j++){
            // (1 <<(8*j)) is equivalent to pow(256,j)
           nGram += (unsigned char)buf[i+j] * (1 << (8*j));
        }
        if(nGram >= this->maxNgram){
            throw std::runtime_error("Ngram greater than maxNgram");
        }
        ngramList[nGram] = 1;
    }

    return;
}
