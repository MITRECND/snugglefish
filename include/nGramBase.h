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


#ifndef NGRAMBASE_H
#define NGRAMBASE_H

#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>
#include "common.h"


namespace snugglefish {

    //Storage container for index entries
    //Due to padding we should typecast on the fly
    //when we want to use this
    struct index_entry{
        uint64_t offset;
        uint32_t num_files;
    };

    //Class to keep track of nGram Index files
    class nGramBase
    {
        public:
            
            nGramBase( uint32_t ngramLength, std::string indexFileName);
            ~nGramBase();

            
        protected:
			
            //FUNCTIONS
            //General Functions
            bool notExists(); //Checks to see if files exist
            bool loadFileIdFile(uint32_t filid_fd);
            uint16_t maxFileNameLength;

			//VARIABLES
            //General Variables
            std::string baseFileName;
            std::string nGramFileName;
            std::string indexFileName;
            std::string fileIdFileName;
            const uint32_t endian_check;
            const uint8_t  version;
            uint8_t ngramLength; // the ngram size we're using
            uint64_t maxNgram;
            uint32_t numIndexFiles; //number of index files
            ngram_t_fidtype numFilesProcessed;
            uint32_t fileIdFile;
			
			
		

        private:
            

    };

}
#endif
