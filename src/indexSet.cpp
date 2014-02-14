#include "indexSet.h"
#include <string>
#include <list>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <stdexcept>

using namespace snugglefish;
using namespace std;


indexSet::indexSet(const char* fileBase, uint32_t count, uint8_t nGramSize)
:indexFile(0), nGramFile(0), fileBase(fileBase), ngramLength(nGramSize),
writable(false), indexMap(0), nGramMap(0), count(count)
{
}

indexSet::~indexSet(){
    if (this->indexFile){
        this->indexFile->close();
        delete this->indexFile;
    }

    if (this->nGramFile){
        this->nGramFile->close();
        delete this->nGramFile;
    }
}

bool indexSet::close(){
    if (this->indexFile){
        this->indexFile->close();
        delete this->indexFile;
        this->indexFile = NULL;
    }
    if (this->nGramFile){
        this->nGramFile->close();
        delete this->nGramFile;
        this->nGramFile = NULL;
    }

}

bool indexSet::create(ngram_t_numfiles nFiles){
    if (indexFile || nGramFile){//Already opened?
        throw runtime_error("Error opening index or ngram file");
    }

    //Create the Files
    //First create the filenames
    string indexFileName, nGramFileName;
    indexFileName = nGramFileName = this->fileBase;
    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, this->count);

    indexFileName.append(INDEX_FILE_EXTENSION).append(number_string);
    nGramFileName.append(NGRAM_FILE_EXTENSION).append(number_string);

    this->indexFile = new file(indexFileName.c_str());
    this->nGramFile = new file(nGramFileName.c_str());

    if(indexFile->exists() || nGramFile->exists()){ //Already exist?
       throw runtime_error("index file or ngram file already exists"); 
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

    offset = 0; //Offset in the ngramfile

    writable = true;

}

bool indexSet::open(){
    string indexFileName, nGramFileName;
    indexFileName = nGramFileName = this->fileBase;
    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, this->count);

    indexFileName.append(INDEX_FILE_EXTENSION).append(number_string);
    nGramFileName.append(NGRAM_FILE_EXTENSION).append(number_string);

    indexFile = new file(indexFileName.c_str());

    if(!indexFile->exists()){
        throw runtime_error("index file does not exist");
    }

    nGramFile = new file(nGramFileName.c_str());

    if (!nGramFile->exists()){
        throw runtime_error("index file does not exist");
    }

    indexFile->open('r');
    nGramFile->open('r');

    indexMap = indexFile->mmap();
    nGramMap = nGramFile->mmap();

    indexEntries = indexMap + INDEX_HEADER_SIZE;
}

bool indexSet::addIndexData(uint64_t offset, uint32_t nFiles){
    indexFile->write((uint8_t*) &offset, sizeof(uint64_t));
    indexFile->write((uint8_t*) &nFiles, sizeof(uint32_t));
}

bool indexSet::addNGrams(uint32_t ngram, list<ngram_t_fidtype>* files){
    if (!writable || !nGramFile){

    }

    uint32_t size = files->size();
    uint64_t off =  offset;
    uint32_t difference = sizeof(ngram_t_fidtype) * size;

    if (!size){
        off = 0;
    }

    //Pre-emptively add the index information, i.e, offset, numFiles
    addIndexData(off, size);

    while(size > 0){
        nGramFile->write((uint8_t*) (&(files->front())), sizeof(ngram_t_fidtype));
        files->pop_front();
        size--;
    } 

    offset =  offset + difference;

}

bool indexSet::updateNumFiles(ngram_t_numfiles count){
    if (!writable || !indexFile){
        //TODO
    }
    //TODO cleanup offset
    indexFile->write_at(INDEX_HEADER_SIZE - INDEX_HEADER_NUM_FILES_FIELD, (uint8_t*) (&count), INDEX_HEADER_NUM_FILES_FIELD);
}

size_t indexSet::getNGramCount(uint64_t ngram){
    index_entry* index_table = (index_entry*) (indexEntries + (ngram * (INDEX_ENTRY_SIZE)));
    return index_table->num_files;
}

ngram_t_fidtype* indexSet::getNGrams(uint64_t ngram, size_t* count){
    index_entry* index_table = (index_entry*) (indexEntries + (ngram * (INDEX_ENTRY_SIZE)));
    *count = index_table->num_files;
    ngram_t_fidtype* ptr = (ngram_t_fidtype*) (nGramMap + index_table->offset);

    return ptr;
}
