/*
Approved for Public Release; Distribution Unlimited: 13-1937

Copyright (c) 2013 The MITRE Corporation. All rights reserved.

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


#include "../include/fileIndexer.h"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include <vector>
#include <map>


using namespace snugglefish;
using namespace std;

fileIndexer::fileIndexer(uint32_t ngramLength):
filesProcessed(0), verbose(true)
{

    if(ngramLength == 3  || ngramLength == 4){
        this->ngramLength = ngramLength;
        this->maxNgram = (1 << (8*ngramLength));
        this->pagesize = getpagesize();
    }
    else {
        throw std::runtime_error("Ngram len must be either 3 or 4");
    }

}

// Takes in a file name, mmaps it, and creates a vector of ngrams
// return: pointer to vector of ngrams
vector<uint32_t>* fileIndexer::processFile(const char* fileName){
    uint64_t bytesRead, number=0, fileSize;
    unsigned char *buf;
    int fd;
    struct stat st;

    fd = open(fileName, O_RDONLY);
    if(fd == -1){
        cout << "error opening " << fileName << " with errno: " << errno << std::endl;
        return 0;
    }

    if (stat(fileName, &st) == 0)
        fileSize = st.st_size;
    else{
        std::cout << "File size for file: " << fileName << " not found" << std::endl;
	return 0;
    }

    uint8_t* filemmap = (uint8_t *) mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fd, 0);
    if(filemmap == MAP_FAILED){
        cout << "error mapping " << fileName << " with errno: " << errno << endl;
    	return 0;
    }

    close(fd);

    vector<uint32_t>* ngramList = 0;

    if (this->verbose){
        cout << "Adding file #" << this->filesProcessed << ": " << fileName << endl;
    }
    this->filesProcessed++;

    //It is much faster to just use an array that holds a boolean
    //for every element, this requires only ~64MB per file for 3-byte Ngrams
    //but 16GB for 4 byte ngrams, so only do this for 3-byte ngrams
    //and do a map instead for 4-byte
    if(this->ngramLength == 3){
        ngramList = new vector<uint32_t>;
        bool* bngramList = new bool[this->maxNgram]();

        try{
            processNgrams(filemmap,fileSize,bngramList);

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
            ngramList = processNgrams(filemmap, fileSize);
        } catch(exception &e){
            cout << "Error processing ngrams: " << e.what() << endl;
        }
    }

    munmap(filemmap, fileSize);
    return ngramList;
}


//Creates vector of ngrams in a given file
//Uses an STL map which should use a RED/BLACK tree
//Insertion/Search should be O(log(n)) time where n is the 
//number of nodes already in the tree
vector<uint32_t>* fileIndexer::processNgrams(unsigned char* buf, uint64_t fileSize){
    map<uint32_t, bool> ngram_map;

    uint32_t nGram = 0;
    uint32_t i, j, k;
    for(i = 0; ((i + this->ngramLength) - 1) < fileSize; i++){
        nGram = 0;
        for(j = 0; j < this->ngramLength; j++){
            nGram += (unsigned char)buf[i+j] * (1 << (8*j));
        }
        if(nGram >= this->maxNgram){
            throw std::runtime_error("Ngram greater than mMaxNgram");
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


//TODO Remove this function after veriying it's no longer useful

// Takes in a file name, mmaps it, and creates a array of booleans to containing
// the file's Ngrams
// return: array containing whether or not a ngram is contained in the file
bool* fileIndexer::processFile(const char* fileName, int flag){
    uint64_t bytesRead, number=0, fileSize;
    unsigned char *buf;
    int fd;
    struct stat st;

    fd = open(fileName, O_RDONLY);
    if(fd == -1){
        cout << "error opening " << fileName << " with errno: " << errno << std::endl;
        return 0;
    }

    if (stat(fileName, &st) == 0)
        fileSize = st.st_size;
    else{
        std::cout << "File size for file: " << fileName << " not found" << std::endl;
	return 0;
    }

    uint8_t* filemmap = (uint8_t *) mmap(NULL, fileSize, PROT_READ, MAP_SHARED, fd, 0);
    if(filemmap == MAP_FAILED){
        cout << "error mapping " << fileName << " with errno: " << errno << endl;
    	return 0;
    }

    close(fd);
    bool* ngramList = new bool [this->maxNgram];
    memset(ngramList, 0, this->maxNgram);

    try{
        if (this->verbose){
            cout << "Adding file #" << this->filesProcessed << ": " << fileName << endl;
        }

        this->filesProcessed++;
        processNgrams(filemmap, fileSize, ngramList);

    } catch(std::exception &e){
        cout << "Error processing ngrams: "<< e.what() << endl;
    }

    munmap(filemmap, fileSize);
    return ngramList;
}


// Takes the file and creates ngrams from it
bool* fileIndexer::processNgrams(unsigned char* buf, uint64_t fileSize, bool ngramList[]){
    //TODO update with byte array
    uint64_t nGram = 0;
    uint64_t i, j, k;
    for(i = 0; ((i + this->ngramLength) - 1) < fileSize; i++){
        nGram = 0;
	// creates an ngram using the formula buf[i] + buf[i+1]*256 
	// + buf[i+2]*256*256
        for(j = 0; j < this->ngramLength; j++){
            // (1 <<(8*j)) is equivalent to pow(256,j)
            nGram += (unsigned char)buf[i+j] * (1 << (8*j));
        }
        if(nGram >= this->maxNgram){
            throw std::runtime_error("Ngram greater than mMaxNgram");
        }
        ngramList[nGram] = 1;
    }

}
