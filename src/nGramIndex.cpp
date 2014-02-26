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


#include "nGramIndex.h"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

using namespace snugglefish;
using namespace std;


nGramIndex::nGramIndex( uint32_t ngramLength, string indexFileName)
    :nGramBase(ngramLength, indexFileName), bufferMax(MAX_BUFFER_SIZE),
    buffer_memory_usage(0), flush(false), flushing(false) {


    this->maxFileNameLength = DEFAULT_MAX_FILENAME_SIZE;
    
	//allocate output buffer
	this->output_buffer = new buffer_element[maxNgram];
	for(uint64_t i = 0; i < maxNgram; i++){
		this->output_buffer[i].elements_size = 0;
		this->output_buffer[i].elements = new list<ngram_t_fidtype>;
	}

    masterFile = new smFile(baseFileName, ngramLength);

    if (masterFile->exists()){
        masterFile->open('w');
    }else{ //Must create it
        masterFile->create(maxFileNameLength);
    }

    numFilesProcessed = masterFile->getNumFiles();
    numSessionFilesProcessed = 0;
}

nGramIndex::~nGramIndex(){
    
	this->flushAll();
	for(uint64_t i = 0; i < maxNgram; i++){
		delete this->output_buffer[i].elements;
	}
	delete[] output_buffer;    

    delete masterFile;

}


/*  NGram Related Functions */
void nGramIndex::addNGrams(vector<uint32_t>* nGramList, string filename){
    //POSIX basename may modify argument so create a copy
    char* temp_filename = new char[filename.length() + 1];
    strncpy(temp_filename, filename.c_str(), filename.length() + 1);
    filename = basename(temp_filename);
    delete[] temp_filename;

    ngram_t_fidtype file_id = numFilesProcessed++;
    fileNameList.push_back(filename);

    // Insert ngram into the list, add the new node to the memory usage variable,
    // and check if the maximum memory has been used, and if so, indicate that the
    // nGrams should be flushed to disk
    for(uint32_t i = 0; i < nGramList->size(); i++){
        uint32_t nGram = (*nGramList)[i];
        output_buffer[nGram].elements_size++;
        output_buffer[nGram].elements->push_back(file_id);

        buffer_memory_usage += BUFFER_NODE_SIZE; //Add size of node

        if(buffer_memory_usage >= bufferMax){ 
            flush = true;
        }
    }

    //We cleanup the memory
    delete nGramList;

    if (flush){
        flushAll();
        flush = false;
    }


}

/*
//Takes an array of bools that is of size 2^(ngram bits)
void nGramIndex::addNGrams(bool nGramList[], string filename, int flag){
    //POSIX basename may modify argument so create a copy
    char* temp_filename = new char[filename.length() + 1];
    strncpy(temp_filename, filename.c_str(), filename.length() + 1);
    filename = basename(temp_filename);
    delete[] temp_filename;


    ngram_t_fidtype file_id = numFilesProcessed++;
    fileNameList.push_back(filename);

    for(uint64_t i = 0; i < maxNgram; i++){
        if (nGramList[i]){
            addNGram(i, file_id);
        }
    }

    //We cleanup the memory
    delete[] nGramList;


    //Maximum number of files per index
    if(fileNameList.size() > maxFiles){
        flush = true;
    }

    if (flush){
        flushAll();
        flush = false;
    }


}
*/


void nGramIndex::flushAll(){
    //cout<<"Flushing ... " << endl 
    //    <<"\tCurrent Buffer Usage: " << buffer_memory_usage<< endl;

    flushing = true;
    if(fileNameList.size()){
        ngram_t_indexfcount num_files = fileNameList.size();
        //By updating the master file last, this set can be queried
        //while creating a new index set
        flushIndex(num_files);
        flushMaster();

        numSessionFilesProcessed += num_files;
    }
    flushing = false;
}

// Flush the file names and update the number of index files
void nGramIndex::flushMaster(){
    //Write FileNames to File ID file
    ngram_t_indexfcount num_files = (ngram_t_indexfcount) fileNameList.size();

    if (!num_files){
        return;
    }

    for(unsigned long i = 0; i < num_files; i++){
        masterFile->addFileId(fileNameList[i].c_str());
    }

    //Clear the vector
    fileNameList.clear();

    //Update filid with new value
    masterFile->updateIndexFileCount(masterFile->getNumIndexFiles() + 1);
}

// Flush the ngrams to the index files
void nGramIndex::flushIndex(ngram_t_indexfcount num_files){
    //Create the files
    indexSet* tIndex = new indexSet(baseFileName.c_str(), masterFile->getNumIndexFiles(), ngramLength);
    tIndex->create(num_files);

    uint64_t bytes_flushed = 0;

    for(uint32_t i = 0; i < maxNgram; i++){ //iterate through every ngram
        bytes_flushed += output_buffer[i].elements_size * sizeof(ngram_t_fidtype);
        tIndex->addNGrams(i, this->output_buffer[i].elements);
        output_buffer[i].elements_size = 0;

        if (output_buffer[i].elements->size() != 0)
            cout << "Not Zero" << endl;
    }

    buffer_memory_usage = 0;
    //cout << "Wrote" << bytes_flushed << " Bytes " << endl;

    tIndex->close();
    delete tIndex;
}


void nGramIndex::getStats(uint64_t& totalFiles, uint64_t& sessionFiles, uint64_t& indexFiles, bool& flushing){
    totalFiles = masterFile->getNumFiles();
    sessionFiles = numSessionFilesProcessed;
    indexFiles = masterFile->getNumIndexFiles();
    flushing = this->flushing;
}
