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
    {

    if(ngramLength == 3 || ngramLength == 4){
        this->ngramLength = ngramLength;
        this->maxNgram = (uint64_t) (1) << (8*ngramLength);
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
}

