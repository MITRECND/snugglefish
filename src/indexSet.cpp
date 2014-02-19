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


#include "indexSet.h"
#include <string>
#include <list>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <stdexcept>

using namespace snugglefish;
using namespace std;


indexSet::indexSet(const char* fileBase, uint32_t count, uint8_t nGramSize)
:indexFile(0), nGramFile(0), fileBase(fileBase), ngramLength(nGramSize),
writable(false), indexMap(0), nGramMap(0), count(count)
{
}

indexSet::~indexSet(){
    if (this->indexFile){
        this->indexFile->close();
        delete this->indexFile;
    }

    if (this->nGramFile){
        this->nGramFile->close();
        delete this->nGramFile;
    }
}

bool indexSet::close(){
    if (this->indexFile){
        this->indexFile->close();
        delete this->indexFile;
        this->indexFile = NULL;
    }
    if (this->nGramFile){
        this->nGramFile->close();
        delete this->nGramFile;
        this->nGramFile = NULL;
    }

}

bool indexSet::create(ngram_t_numfiles nFiles){
    if (indexFile || nGramFile){//Already opened?
        throw runtime_error("Error opening index or ngram file");
    }

    //Create the Files
    //First create the filenames
    string indexFileName, nGramFileName;
    indexFileName = nGramFileName = this->fileBase;
    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, this->count);

    indexFileName.append(INDEX_FILE_EXTENSION).append(number_string);
    nGramFileName.append(NGRAM_FILE_EXTENSION).append(number_string);

    this->indexFile = new file(indexFileName.c_str());
    this->nGramFile = new file(nGramFileName.c_str());

    if(indexFile->exists() || nGramFile->exists()){ //Already exist?
       throw runtime_error("index file or ngram file already exists"); 
    }

    indexFile->create();
    nGramFile->create();

    
    this->endian_check = ENDIAN_CHECK;
    this->version = VERSION;

    //Write standard header
    indexFile->write((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);
    indexFile->write((uint8_t*) (&(this->version)), VERSION_FIELD);
    indexFile->write((uint8_t*) (&(this->ngramLength)), NGRAM_SIZE_FIELD);
    indexFile->write((uint8_t*) (&nFiles), INDEX_HEADER_NUM_FILES_FIELD);

    offset = 0; //Offset in the ngramfile

    writable = true;

}

bool indexSet::open(){
    string indexFileName, nGramFileName;
    indexFileName = nGramFileName = this->fileBase;
    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, this->count);

    indexFileName.append(INDEX_FILE_EXTENSION).append(number_string);
    nGramFileName.append(NGRAM_FILE_EXTENSION).append(number_string);

    indexFile = new file(indexFileName.c_str());

    if(!indexFile->exists()){
        throw runtime_error("index file does not exist");
    }

    nGramFile = new file(nGramFileName.c_str());

    if (!nGramFile->exists()){
        throw runtime_error("index file does not exist");
    }

    indexFile->open('r');
    nGramFile->open('r');

    indexMap = indexFile->mmap();
    nGramMap = nGramFile->mmap();

    indexEntries = indexMap + INDEX_HEADER_SIZE;
}

bool indexSet::addIndexData(uint64_t offset, uint32_t nFiles){
    indexFile->write((uint8_t*) &offset, sizeof(uint64_t));
    indexFile->write((uint8_t*) &nFiles, sizeof(uint32_t));
}

bool indexSet::addNGrams(uint32_t ngram, list<ngram_t_fidtype>* files){
    if (!writable || !nGramFile){

    }

    uint32_t size = files->size();
    uint64_t off =  offset;
    uint32_t difference = sizeof(ngram_t_fidtype) * size;

    if (!size){
        off = 0;
    }

    //Pre-emptively add the index information, i.e, offset, numFiles
    addIndexData(off, size);

    while(size > 0){
        nGramFile->write((uint8_t*) (&(files->front())), sizeof(ngram_t_fidtype));
        files->pop_front();
        size--;
    } 

    offset =  offset + difference;

}

bool indexSet::updateNumFiles(ngram_t_numfiles count){
    if (!writable || !indexFile){
        //TODO
    }
    //TODO cleanup offset
    indexFile->write_at(INDEX_HEADER_SIZE - INDEX_HEADER_NUM_FILES_FIELD, (uint8_t*) (&count), INDEX_HEADER_NUM_FILES_FIELD);
}

size_t indexSet::getNGramCount(uint64_t ngram){
    index_entry* index_table = (index_entry*) (indexEntries + (ngram * (INDEX_ENTRY_SIZE)));
    return index_table->num_files;
}

ngram_t_fidtype* indexSet::getNGrams(uint64_t ngram, size_t* count){
    index_entry* index_table = (index_entry*) (indexEntries + (ngram * (INDEX_ENTRY_SIZE)));
    *count = index_table->num_files;
    ngram_t_fidtype* ptr = (ngram_t_fidtype*) (nGramMap + index_table->offset);

    return ptr;
}
