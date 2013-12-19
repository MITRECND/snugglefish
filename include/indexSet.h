#ifndef SNGINDEXSET_H
#define SNGINDEXSET_H


#include "file.h"
#include "common.h"
#include <string>
#include <vector>

namespace snugglefish {

    class indexSet 
    {

        public:
            indexSet(const char* fileBase, uint8_t nGramSize);
            ~indexSet();

            bool create(ngram_t_numfiles nFiles = 0);
            bool addNGrams(uint8_t* data, size_t length);
            bool updateNumFiles(ngram_t_numfiles count);
            bool addIndexData(uint8_t* data, size_t length);

            //Opens and mmaps both the Index and NGram File
            bool open();

            //Returns mmap'd loation of given ngram
            uint8_t* getNGrams(uint64_t ngram, size_t* count);

            //Takes a list of NGrams and returns a list in ascending order
            //nGrams with zero entries will be removed
            std::vector<uint64_t> getOrderedNGrams(std::vector<uint64_t> nGrams);

        private:
            file* indexFile;
            file* nGramFile;

            uint8_t* indexMap;
            uint8_t* nGramMap;

            std::string fileBase;

            bool writable;

            //Index File header elements
            ngram_t_endian      endian_check;
            ngram_t_version     version;
            ngram_t_size        ngramLength;
            ngram_t_numfiles    numFiles;

    };

}
#endif
