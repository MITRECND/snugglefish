#include "smFile.h"
#include <cstring>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>
#include <iostream>
#include <errno.h>


using namespace snugglefish;
using namespace std;


smFile::smFile(string fileBase, uint8_t nGramSize)
:file(fileBase.append(FILEID_FILE_EXTENSION).c_str()), ngramLength((ngram_t_size) nGramSize)
{

}

smFile::~smFile(){
    flush();
}

bool smFile::flush(){
    if (this->readonly)
        return true;

    file::flush();
    write_at((int32_t) FILID_NUM_FILES_OFFSET, (uint8_t*) &numFiles, (size_t) NUM_FILES_FIELD);

    return true;

}

bool smFile::create(ngram_t_fnlength maxfnLength){
    if (this->exists()){
        cerr << "Unable to create file: " << this->filename << " -- Already Exists" << endl;
        throw runtime_error("Creating File");
    }

    //Create the File
    file::create();
    
    this->endian_check = ENDIAN_CHECK;
    this->version = VERSION;
    this->maxFileNameLength = maxfnLength;
    
    ngram_t_indexcount four_byte_zero = this->numIndexFiles = 0;
    ngram_t_fidtype eight_byte_zero = this->numFiles = 0;

    //Write standard header
    write((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);
    write((uint8_t*) (&(this->version)), VERSION_FIELD);
    write((uint8_t*) (&(this->ngramLength)), NGRAM_SIZE_FIELD);
    write((uint8_t*) (&(this->maxFileNameLength)), MAX_FILENAME_LENGTH_FIELD);
    write((uint8_t*) &four_byte_zero, NUM_INDEX_FILES_FIELD);
    write((uint8_t*) &eight_byte_zero, NUM_FILES_FIELD);

    fileBuffer = (char*) malloc((maxFileNameLength + 1)* sizeof(char));

}

bool smFile::open(char readwrite){
    file::open(readwrite);

    ngram_t_size ngramL;

    read((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);

    if (this->endian_check != ENDIAN_CHECK){
       throw runtime_error("Endian Mismatch"); 
    }

    read((uint8_t*) (&(this->version)), VERSION_FIELD);
    read((uint8_t*) (&ngramL), NGRAM_SIZE_FIELD);

    if (this->ngramLength != ngramL){
       throw runtime_error("N Gram Length Mismatch"); 
    }  

    read((uint8_t*) (&(this->maxFileNameLength)), MAX_FILENAME_LENGTH_FIELD);
    read((uint8_t*) (&(this->numIndexFiles)), NUM_INDEX_FILES_FIELD);
    read((uint8_t*) (&(this->numFiles)), NUM_FILES_FIELD);

    fileBuffer = (char*) malloc((maxFileNameLength + 1)* sizeof(char));

}


bool smFile::addFileId(const char * fileName){
    strncpy(fileBuffer, fileName, maxFileNameLength);
    write((uint8_t*) fileBuffer, maxFileNameLength * sizeof(char));

    numFiles++;
    //Writing of the numFiles to the file is delayed until a flush
}

const char* smFile::getFilebyId(uint64_t id){
    read_at(FILID_HEADER_SIZE + (id * maxFileNameLength * sizeof(char)), (uint8_t*) fileBuffer, maxFileNameLength * sizeof(char)); 
    return fileBuffer;
}

bool smFile::updateIndexFileCount(ngram_t_indexcount count){
    write_at((int32_t) FILID_NUM_INDEX_OFFSET, (uint8_t*) &count, (size_t) NUM_INDEX_FILES_FIELD); 
    numIndexFiles = count; 
}

bool smFile::updateFileCount(ngram_t_fidtype count){
    //This is delayed until flush() is called
    numFiles = count;
}

