#include "indexSet.h"
#include <string>

using namespace snugglefish;
using namespace std;


indexSet::indexSet(const char* fileBase, uint8_t nGramSize)
:indexFile(0), nGramFile(0), fileBase(fileBase), ngramLength(nGramSize),
writable(false), indexMap(0), nGramMap(0)
{
}

indexSet::~indexSet(){
    if (this->indexFile){
        delete this->indexFile;
    }

    if (this->nGramFile){
        delete this->nGramFile;
    }
}

bool indexSet::create(ngram_t_numfiles nFiles){
    if (indexFile || nGramFile){//Already opened?
        //some error
    }

    //Create the Files
    //First create the filenames
    string indexFileName, nGramFileName;
    indexFileName = nGramFileName = this->fileBase;
    indexFileName.append(INDEX_FILE_EXTENSION);
    nGramFileName.append(NGRAM_FILE_EXTENSION);

    this->indexFile = new file(indexFileName.c_str());
    this->nGramFile = new file(nGramFileName.c_str());

    if(indexFile->exists() || nGramFile->exists()){ //Already exist?
        //Some Error
    }

    indexFile->create();
    nGramFile->create();

    
    this->endian_check = ENDIAN_CHECK;
    this->version = VERSION;

    //Write standard header
    indexFile->write((uint8_t*) (&(this->endian_check)), ENDIAN_CHECK_FIELD);
    indexFile->write((uint8_t*) (&(this->version)), VERSION_FIELD);
    indexFile->write((uint8_t*) (&(this->ngramLength)), NGRAM_SIZE_FIELD);
    indexFile->write((uint8_t*) (&nFiles), INDEX_HEADER_NUM_FILES_FIELD);

    writable = true;

}

bool indexSet::addNGrams(uint8_t* data, size_t length){
    if (!writable || !nGramFile){
        //TODO
    }
    nGramFile->write(data, length);
}


bool indexSet::updateNumFiles(ngram_t_numfiles count){
    if (!writable || !indexFile){
        //TODO
    }
    //TODO cleanup offset
    indexFile->write_at(INDEX_HEADER_SIZE - INDEX_HEADER_NUM_FILES_FIELD, (uint8_t*) (&count), INDEX_HEADER_NUM_FILES_FIELD);
}

bool indexSet::addIndexData(uint8_t* data, size_t length){
    if (!writable || !indexFile){
        //TODO
    }
    indexFile->write(data, length);
}


bool open(){

}

uint8_t* getNGrams(uint64_t ngram, size_t* count){

}

vector<uint64_t> getOrderedNGrams(vector<uint64_t> nGrams){

}
