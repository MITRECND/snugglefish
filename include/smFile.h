#ifndef SNGMASTERFILE_H
#define SNGMASTERFILE_H


#include "file.h"
#include "common.h"

namespace snugglefish {

    class smFile: public file
    {

        public:
            smFile(const char* fileName, uint8_t nGramSize);

            bool create(ngram_t_fnlength maxfnLength);        
            bool open(char readwrite);

            bool addFileId(const char* fileName);
            bool updateIndexFileCount(ngram_t_indexcount count);
            bool updateFileCount(ngram_t_fidtype count);

        private:
            //Index File header elements
            ngram_t_endian      endian_check;
            ngram_t_version     version;
            ngram_t_size        ngramLength;
            ngram_t_fnlength    maxFileNameLength;
            ngram_t_indexcount  numIndexFiles;
            ngram_t_fidtype     numFiles;

    };

}
#endif
