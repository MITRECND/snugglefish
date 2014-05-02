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


#ifndef SNGINDEXSET_H
#define SNGINDEXSET_H


#include "file.h"
#include "common.h"
#include <string>
#include <vector>
#include <list>

namespace snugglefish {

    //Storage container for index entries
    //Due to padding we should typecast on the fly
    //when we want to use this
    struct index_entry{
        uint64_t offset;
        uint32_t num_files;
    };

    class indexSet 
    {

        public:
            indexSet(const char* fileBase, uint32_t count, uint8_t nGramSize);
            ~indexSet();

            void create(ngram_t_numfiles nFiles = 0);
            void addNGrams(uint32_t ngram, std::list<ngram_t_fidtype> *files);
            void updateNumFiles(ngram_t_numfiles count);

            //Opens and mmaps both the Index and NGram File
            void open();

            void close();

            //Get number of files with given ngram
            size_t getNGramCount(uint64_t ngram);

            //Returns mmap'd loation of given ngram
            ngram_t_fidtype* getNGrams(uint64_t ngram, size_t* count);

        private:
            void addIndexData(uint64_t offset, uint32_t nFiles);


            file* indexFile;
            file* nGramFile;

            uint8_t* indexMap;
            uint8_t* nGramMap;
            uint8_t* indexEntries;

            std::string fileBase;

            bool writable;

            //Index File header elements
            ngram_t_endian      endian_check;
            ngram_t_version     version;
            ngram_t_size        ngramLength;
            ngram_t_numfiles    numFiles;

            uint32_t count;
            uint64_t offset;

    };

}
#endif
