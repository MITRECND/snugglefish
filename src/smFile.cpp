#include "smFile.h"
#include <cstring>


using namespace snugglefish;
using namespace std;


smFile::smFile(const char* fileName, uint8_t nGramSize)
:file(fileName), ngramLength((ngram_t_size) nGramSize)
{

}

bool smFile::create(ngram_t_fnlength maxfnLength){
    if (this->exists()){
        //some error
    }

    //Create the File
    file::create();
    
    this->endian_check = ENDIAN_CHECK;
    this->version = VERSION;
    this->maxFileNameLength = maxfnLength;
    uint32_t four_byte_zero = 0;
    uint64_t eight_byte_zero = 0;

    //Write standard header
    write((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);
    write((uint8_t*) (&(this->version)), VERSION_FIELD);
    write((uint8_t*) (&(this->ngramLength)), NGRAM_SIZE_FIELD);
    write((uint8_t*) (&(this->maxFileNameLength)), MAX_FILENAME_LENGTH_FIELD);
    write((uint8_t*) &four_byte_zero, NUM_INDEX_FILES_FIELD);
    write((uint8_t*) &eight_byte_zero, NUM_FILES_FIELD);

}

bool smFile::open(char readwrite){
    file::open(readwrite);

    ngram_t_size ngramL;

    read((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);

    if (this->endian_check != ENDIAN_CHECK){
        //TODO
    }

    read((uint8_t*) (&(this->version)), VERSION_FIELD);
    read((uint8_t*) (&ngramL), NGRAM_SIZE_FIELD);

    if (this->ngramLength != ngramL){
        //TODO
    }  

    read((uint8_t*) (&(this->maxFileNameLength)), MAX_FILENAME_LENGTH_FIELD);
    read((uint8_t*) (&(this->numIndexFiles)), NUM_INDEX_FILES_FIELD);
    read((uint8_t*) (&(this->numFiles)), NUM_FILES_FIELD);


}


bool smFile::addFileId(const char * fileName){
    write((uint8_t*) fileName, strlen(fileName) * sizeof(char));
}

//These two functions are inefficient -- any better way?
bool smFile::updateIndexFileCount(ngram_t_indexcount count){
   write_at((int32_t) FILID_NUM_INDEX_OFFSET, (uint8_t*) &count, (size_t) NUM_INDEX_FILES_FIELD); 
}

bool smFile::updateFileCount(ngram_t_fidtype count){
    write_at((int32_t) FILID_NUM_FILES_OFFSET, (uint8_t*) &count, (size_t) NUM_FILES_FIELD);
}

