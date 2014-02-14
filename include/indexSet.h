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

            bool create(ngram_t_numfiles nFiles = 0);
            bool addNGrams(uint32_t ngram, std::list<ngram_t_fidtype> *files);
            bool updateNumFiles(ngram_t_numfiles count);

            //Opens and mmaps both the Index and NGram File
            bool open();

            bool close();

            //Get number of files with given ngram
            size_t getNGramCount(uint64_t ngram);

            //Returns mmap'd loation of given ngram
            ngram_t_fidtype* getNGrams(uint64_t ngram, size_t* count);

        private:
            bool addIndexData(uint64_t offset, uint32_t nFiles);


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
