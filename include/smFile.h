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

#ifndef SNGMASTERFILE_H
#define SNGMASTERFILE_H


#include "file.h"
#include "common.h"
#include <string>

namespace snugglefish {

    class smFile: public file
    {

        public:
            smFile(std::string fileBase, uint8_t nGramSize);
            ~smFile();

            void create(ngram_t_fnlength maxfnLength);
            void open(char readwrite);

            bool flush();

            void addFileId(const char* fileName);
            void updateIndexFileCount(ngram_t_indexcount count);

            const ngram_t_indexcount getNumIndexFiles() { return numIndexFiles; }
            const ngram_t_fidtype getNumFiles() { return numFiles; }

            const char* getFilebyId(uint64_t id);

        private:
            //Index File header elements
            ngram_t_endian      endian_check;
            ngram_t_version     version;
            ngram_t_size        ngramLength;
            ngram_t_fnlength    maxFileNameLength;
            ngram_t_indexcount  numIndexFiles;
            ngram_t_fidtype     numFiles;


            char*   fileBuffer;

    };

}
#endif
