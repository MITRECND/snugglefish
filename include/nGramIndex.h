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

#ifndef NGRAMINDEX_H
#define NGRAMINDEX_H

#include "nGramBase.h"
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>
#include "smFile.h"
#include "indexSet.h"

#define BUFFER_NODE_SIZE        sizeof(ngram_t_fidtype) + 24 //24 is upper bound per node

//Index File Constants
#define DEFAULT_MAX_FILENAME_SIZE                   65 //64 characters + null terminator
#define TWO_GB                                      2147483648 //1024 * 1024 * 1204 * 2 
#define FOUR_GB                                     TWO_GB * 2
#define MAX_BUFFER_SIZE                             FOUR_GB 

    //4MB = 4194304 
    //8MB = 8388608 
#define WRITE_BUFFER_SIZE                           1024 * 1024 * 8 * 32 //256MB

namespace snugglefish {

    //Storage container for output buffer
    struct buffer_element{
        uint64_t    elements_size; //how many elements in list
                                   //stl list size() will iterate everytime, easier to keep
                                   //static counter
        std::list<ngram_t_fidtype>* elements;
    };


    //Class to keep track of nGram Index files
    class nGramIndex: public nGramBase
    {
        public:
            
            nGramIndex( uint32_t ngramLength, std::string indexFileName);
            ~nGramIndex();

            //Accessors
            const uint32_t getmaxFileNameLength(){return this->maxFileNameLength;}   
            const uint64_t getmaxBufferSize(){return this->bufferMax;}

            //Setters
            void setmaxFileNameLength(uint32_t length){ this->maxFileNameLength = length; }
            void setmaxBufferSize(uint64_t size){this->bufferMax = size;}
            
            //Write Mode
            void addNGrams(std::vector<uint32_t>* nGramList, std::string filename);
            //void addNGrams(bool nGramList[], std::string filename, int flag);

            void getStats(uint64_t & totalFiles, uint64_t& sessionFiles, uint64_t& indexFiles, bool& flushing);
    
        private:
            //Write Mode Functions
            void flushAll();
            void flushMaster();
            void flushIndex(ngram_t_indexfcount num_files );

           
            //Write Mode Variables
            uint64_t bufferMax;
            buffer_element* output_buffer;
            uint64_t buffer_memory_usage; //how many bytes is the buffer storing (only file ids)
            std::vector< std::string > fileNameList;
            bool flush;
            bool flushing;


            smFile* masterFile;

            uint64_t numFilesProcessed;
            uint64_t numSessionFilesProcessed; //How many files have been processed this session
 

    };

}
#endif
