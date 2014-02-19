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

#ifndef NGRAMSEARCH_H
#define NGRAMSEARCH_H

#include "nGramBase.h"
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <stdint.h>
#include <utility>
#include <pthread.h>
#include <queue>
#include "smFile.h"
#include "indexSet.h"

namespace snugglefish {

    typedef struct _thread_data{
        uint32_t queue;
        uint32_t maximumIndex;
        uint32_t ngramLength;

        smFile* masterFile;
        std::vector<std::string>* matchedFiles;
        std::string* baseFileName;
        std::vector<uint64_t>* nGramQuery;

        pthread_mutex_t queueMutex;
        pthread_mutex_t smFileMutex; 
        pthread_mutex_t mfMutex;
    } thread_data;


    //Class to keep track of nGram Index files
    class nGramSearch: public nGramBase
    {
        public:
            
            nGramSearch( uint32_t ngramLength, std::string indexFileName);
            nGramSearch( uint32_t ngramLength, std::string indexFileName, uint32_t threads);
            ~nGramSearch();


            //Read Mode
            std::vector<std::string>* searchNGrams(std::vector<uint64_t> nGramQuery);
            std::vector<uint64_t>* stringToNGrams(std::string searchString);

        protected:

        private:
            //FUNCTIONS
            static std::list< std::pair<uint64_t, size_t> > orderNGrams(indexSet* index, const std::vector<uint64_t>& nGramQuery);
            //Alpha is just a placeholder name for this search type
            //I envision there will be multiple search types
            static void searchAlpha(indexSet* index, std::list< std::pair<uint64_t,size_t> > & queryList, std::list<ngram_t_fidtype>& matchedIds);
            static void* searchNGramThread(void* input);


            uint32_t numThreads;


            //Read Mode Variables
            smFile* masterFile;
            uint32_t numIndexFiles;
            uint32_t numFiles;

            
 

    };

}
#endif
