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


#ifndef FILEINDEXER_H
#define FILEINDEXER_H

#include <stdint.h>
#include <vector>


namespace snugglefish {
    //Class to keep track of nGram Index files
    class fileIndexer
    {
        public:
            
            //Constructor -- only takes ngramLength
            fileIndexer( uint32_t ngramLength );

            //Processes the nGrams from a file -- returns an allocated array of bools
            //Calling function must cleanup
            bool* processFile(const char* fileName, int flag); //flag is only there so we can overload 'processFile'
            std::vector<uint32_t>* processFile(const char* fileName);

            //Sets verbosity on process
            void setVerbose(bool verbose);

        protected:

        private:
            std::vector<uint32_t>* processNgrams(unsigned char *buf, uint64_t fileSize);
            bool* processNgrams(unsigned char *buf, uint64_t fileSize, bool ngramList[]);

            uint64_t filesProcessed;
            bool verbose;
            uint32_t ngramLength;
            uint64_t maxNgram;
            uint32_t pagesize;


            
    };
}
#endif
