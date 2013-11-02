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


#include "nGramBase.h"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include <libgen.h> //for dirname and basename()

using namespace snugglefish;
using namespace std;


nGramBase::nGramBase( uint32_t ngramLength, string indexFileName)
    :endian_check(ENDIAN_CHECK), version(VERSION) {

    if(ngramLength == 3 || ngramLength == 4){
        this->ngramLength = ngramLength;
        this->maxNgram = 1 << (8*ngramLength);// ~=pow(256, ngramLength);


        this->numFilesProcessed = 0;
        this->numIndexFiles = 0;
    }
    else {
        throw std::runtime_error("Ngram len must be 3 or 4");
    }



    size_t pos;
    string baseFileName = indexFileName;
    // Check to see if the index file ends with any of the extentions we use
    // And if so, remove them to get the base filename
    pos = baseFileName.rfind( NGRAM_FILE_EXTENSION );
    // Make sure that it is at the end of the string
    if(pos == (baseFileName.size() - 6)){
        baseFileName = baseFileName.substr(0, pos);
    } else{
        pos = baseFileName.rfind( INDEX_FILE_EXTENSION );
        if(pos == (baseFileName.size() - 6)){
            baseFileName = baseFileName.substr(0, pos);
        } else{
            pos = baseFileName.rfind( FILEID_FILE_EXTENSION );
            if(pos == (baseFileName.size() - 6)){
            baseFileName = baseFileName.substr(0, pos);
            }
        }
    }


    this->baseFileName = baseFileName;

    this->nGramFileName = baseFileName;
    this->nGramFileName.append(NGRAM_FILE_EXTENSION);

    this->indexFileName = baseFileName;
    this->indexFileName.append(INDEX_FILE_EXTENSION);

    this->fileIdFileName = baseFileName;
    this->fileIdFileName.append(FILEID_FILE_EXTENSION);

    


}

nGramBase::~nGramBase(){
    

}


/* File Status Related Functions */
bool nGramBase::notExists(){
    struct stat st;
    if(stat(this->fileIdFileName.c_str(), &st) == 0){
        //file exists 
        return false;
    }else{
        return true;
    }

}



bool nGramBase::loadFileIdFile(uint32_t filid_fd){
   
    
    ngram_t_endian endian_check = 0;
    ngram_t_version version = 0;
    ngram_t_size ngram_size = 0;

    read(filid_fd, &endian_check, ENDIAN_CHECK_FIELD);

    if (endian_check != this->endian_check){
        //endian matches
        cout << "Endian MisMatch" <<endl;
        return false;
    }

    read(filid_fd, &version, VERSION_FIELD);
    read(filid_fd, &ngram_size, NGRAM_SIZE_FIELD);
    if(ngram_size != ngramLength){
        cout << "nGram Size MisMatch"<< endl;
        return false;
    }

    read(filid_fd, &maxFileNameLength, MAX_FILENAME_LENGTH_FIELD); 
    read(filid_fd, &numIndexFiles, NUM_INDEX_FILES_FIELD);
    read(filid_fd, &numFilesProcessed, NUM_FILES_FIELD);

    this->fileIdFile = filid_fd;

    return true;
}

