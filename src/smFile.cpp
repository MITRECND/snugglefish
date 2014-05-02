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


#include "smFile.h"
#include <cstring>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>
#include <iostream>
#include <errno.h>


using namespace snugglefish;
using namespace std;


smFile::smFile(string fileBase, uint8_t nGramSize)
:file(fileBase.append(FILEID_FILE_EXTENSION).c_str()), ngramLength((ngram_t_size) nGramSize)
{

}

smFile::~smFile(){
    flush();
}

bool smFile::flush(){
    if (this->readonly)
        return true;

    file::flush();
    write_at((int32_t) FILID_NUM_FILES_OFFSET, (uint8_t*) &numFiles, (size_t) NUM_FILES_FIELD);

    return true;

}

void smFile::create(ngram_t_fnlength maxfnLength){
    if (this->exists()){
        cerr << "Unable to create file: " << this->filename << " -- Already Exists" << endl;
        throw runtime_error("Creating File");
    }

    //Create the File
    file::create();
    
    this->endian_check = ENDIAN_CHECK;
    this->version = VERSION;
    this->maxFileNameLength = maxfnLength;
    
    ngram_t_indexcount four_byte_zero = this->numIndexFiles = 0;
    ngram_t_fidtype eight_byte_zero = this->numFiles = 0;

    //Write standard header
    write((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);
    write((uint8_t*) (&(this->version)), VERSION_FIELD);
    write((uint8_t*) (&(this->ngramLength)), NGRAM_SIZE_FIELD);
    write((uint8_t*) (&(this->maxFileNameLength)), MAX_FILENAME_LENGTH_FIELD);
    write((uint8_t*) &four_byte_zero, NUM_INDEX_FILES_FIELD);
    write((uint8_t*) &eight_byte_zero, NUM_FILES_FIELD);

    fileBuffer = (char*) malloc((maxFileNameLength + 1)* sizeof(char));

}

void smFile::open(char readwrite){
    file::open(readwrite);

    ngram_t_size ngramL;

    read((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);

    if (this->endian_check != ENDIAN_CHECK){
       throw runtime_error("Endian Mismatch"); 
    }

    read((uint8_t*) (&(this->version)), VERSION_FIELD);
    read((uint8_t*) (&ngramL), NGRAM_SIZE_FIELD);

    if (this->ngramLength != ngramL){
       throw runtime_error("N Gram Length Mismatch"); 
    }  

    read((uint8_t*) (&(this->maxFileNameLength)), MAX_FILENAME_LENGTH_FIELD);
    read((uint8_t*) (&(this->numIndexFiles)), NUM_INDEX_FILES_FIELD);
    read((uint8_t*) (&(this->numFiles)), NUM_FILES_FIELD);

    fileBuffer = (char*) malloc((maxFileNameLength + 1)* sizeof(char));

}


void smFile::addFileId(const char * fileName){
    strncpy(fileBuffer, fileName, maxFileNameLength);
    write((uint8_t*) fileBuffer, maxFileNameLength * sizeof(char));

    numFiles++;
    //Writing of the numFiles to the file is delayed until a flush
}

const char* smFile::getFilebyId(uint64_t id){
    read_at(FILID_HEADER_SIZE + (id * maxFileNameLength * sizeof(char)), (uint8_t*) fileBuffer, maxFileNameLength * sizeof(char)); 
    return fileBuffer;
}

void smFile::updateIndexFileCount(ngram_t_indexcount count){
    write_at((int32_t) FILID_NUM_INDEX_OFFSET, (uint8_t*) &count, (size_t) NUM_INDEX_FILES_FIELD); 
    numIndexFiles = count; 
}
