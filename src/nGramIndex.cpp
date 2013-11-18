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


#include "nGramIndex.h"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include <libgen.h> //for dirname and basename()

using namespace snugglefish;
using namespace std;


nGramIndex::nGramIndex( uint32_t ngramLength, string indexFileName)
    :nGramBase(ngramLength, indexFileName), bufferMax(MAX_BUFFER_SIZE),
    buffer_memory_usage(0), flush(false), maxFiles(MAX_FILES) {


    this->maxFileNameLength = DEFAULT_MAX_FILENAME_SIZE;
    
	//allocate output buffer
	this->output_buffer = new buffer_element[maxNgram];
	for(uint64_t i = 0; i < maxNgram; i++){
		this->output_buffer[i].elements_size = 0;
		this->output_buffer[i].elements = new list<ngram_t_fidtype>;
	}

	if(notExists()){ //FileIdFile
		createFiles();
	}
    

    
    if(!loadFileIdFile()){
        throw runtime_error("Error opening File Id File");
    }

   
}

nGramIndex::~nGramIndex(){
    
	this->flushAll();
	//should we bother properly cleaning up?
	for(uint64_t i = 0; i < maxNgram; i++){
		delete this->output_buffer[i].elements;
	}
	delete[] output_buffer;    

}

bool nGramIndex::loadFileIdFile(){
    uint32_t filid_fd;
    
   
	filid_fd = open(this->fileIdFileName.c_str(), O_RDWR);

    if(filid_fd == -1){
        return false;
    }

    return nGramBase::loadFileIdFile(filid_fd);
}

//Creates and opens an Ngram file
//which is effectively an empty file with the right name
bool nGramIndex::createNGramFile(){
    uint32_t ngram_fd;
    string filename = nGramFileName;

    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, numIndexFiles);
    filename = filename.append(number_string);

    ngram_fd = open(filename.c_str(), O_RDWR | O_CREAT, FILE_MODE);

    if(ngram_fd == -1){
        cout << "Error Number: " << errno << endl;
        return false;
    }

    nGramFile = ngram_fd;
    return true; 
}


//Creates and opens an index file
//Which only has the standard header written
bool nGramIndex::createIndexFile(ngram_t_indexfcount num_files){
    uint32_t index_fd;
    
    string filename = indexFileName;

    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, numIndexFiles);
    filename = filename.append(number_string);

    index_fd = open(filename.c_str(), O_RDWR|O_CREAT, FILE_MODE);
    if(index_fd == -1){
        cout << "Error Number: " << errno << endl;
        return false;
    }

    //Write out initial Header for Index File
    write(index_fd, &endian_check, ENDIAN_CHECK_FIELD);
    write(index_fd, &version, VERSION_FIELD);
    write(index_fd, &ngramLength, NGRAM_SIZE_FIELD);
    write(index_fd, &num_files, INDEX_HEADER_NUM_FILES_FIELD);
    //Num files field written when flushing

    indexFile = index_fd;
    return true;
    
}


//Creates then closes a new file id file
//Should be loaded later
bool nGramIndex::createFileIdFile(){
    uint32_t filid_fd;
    uint32_t four_byte_zero = 0;
    uint64_t eight_byte_zero = 0;
    filid_fd = open(fileIdFileName.c_str(), O_RDWR | O_CREAT , FILE_MODE);
    if(filid_fd == -1){
        cout << "Error Number: " << errno << endl;
        return false;
    }

    write(filid_fd, &endian_check, ENDIAN_CHECK_FIELD);
    write(filid_fd, &version, VERSION_FIELD);
    write(filid_fd, &ngramLength, NGRAM_SIZE_FIELD);
    write(filid_fd, &this->maxFileNameLength, MAX_FILENAME_LENGTH_FIELD);
    write(filid_fd, &four_byte_zero, NUM_INDEX_FILES_FIELD); 
    write(filid_fd, &eight_byte_zero, NUM_FILES_FIELD); 

    close(filid_fd);

    return true;
}

void nGramIndex::createFiles(){
    cout << "Creating File ... " << endl;
    createFileIdFile();
}



/*  NGram Related Functions */
void nGramIndex::addNGrams(vector<uint32_t>* nGramList, string filename){
    //POSIX basename may modify argument so create a copy
    char* temp_filename = new char[filename.length() + 1];
    strncpy(temp_filename, filename.c_str(), filename.length() + 1);
    filename = basename(temp_filename);
    delete[] temp_filename;


    ngram_t_fidtype file_id = numFilesProcessed++;
    fileNameList.push_back(filename);

    for(uint32_t i = 0; i < nGramList->size(); i++){
        addNGram((*nGramList)[i], file_id);
    }

    //We cleanup the memory
    delete nGramList;


    //Maximum number of files per index
    if(fileNameList.size() > maxFiles){
        flush = true;
    }

    if (flush){
        flushAll();
        flush = false;
    }


}

//Takes an array of bools that is of size 2^(ngram bits)
void nGramIndex::addNGrams(bool nGramList[], string filename, int flag){
    //POSIX basename may modify argument so create a copy
    char* temp_filename = new char[filename.length() + 1];
    strncpy(temp_filename, filename.c_str(), filename.length() + 1);
    filename = basename(temp_filename);
    delete[] temp_filename;


    ngram_t_fidtype file_id = numFilesProcessed++;
    fileNameList.push_back(filename);

    for(uint64_t i = 0; i < maxNgram; i++){
        if (nGramList[i]){
            addNGram(i, file_id);
        }
    }

    //We cleanup the memory
    delete[] nGramList;


    //Maximum number of files per index
    if(fileNameList.size() > maxFiles){
        flush = true;
    }

    if (flush){
        flushAll();
        flush = false;
    }


}


// Insert ngram into the list, add the new node to the memory usage variable,
// and check if the maximum memory has been used, and if so, indicate that the
// nGrams should be flushed to disk
void nGramIndex::addNGram(uint64_t nGram, ngram_t_fidtype file_id){
    output_buffer[nGram].elements_size++;
    output_buffer[nGram].elements->push_back(file_id);

    buffer_memory_usage += BUFFER_NODE_SIZE; //Add size of node

    if(buffer_memory_usage >= bufferMax){ 
        flush = true;
    }
}

void nGramIndex::flushAll(){
    cout<<"Flushing ... " << endl;
    cout<<"\tCurrent Buffer Usage: " << buffer_memory_usage<< endl;
    ngram_t_indexfcount num_files = flushFiles();
    flushIndex(num_files);
    cout<<"\tAfter Flush: " << buffer_memory_usage << endl;
}

// Flush the file names to the file id file
ngram_t_indexfcount nGramIndex::flushFiles(){
    //Write FileNames to File ID file
    ngram_t_indexfcount num_files = (ngram_t_indexfcount) fileNameList.size();
    char* fileNameBuffer = new char[maxFileNameLength * fileNameList.size()];

    for(unsigned long i = 0; i < fileNameList.size(); i++){
        strncpy(fileNameBuffer + (i * maxFileNameLength), fileNameList[i].c_str(), maxFileNameLength);
    }

    lseek(fileIdFile, 0, SEEK_END);
    write(fileIdFile, fileNameBuffer, sizeof(char) * (maxFileNameLength * fileNameList.size()));

    delete[] fileNameBuffer;

    //Upate number of files
    lseek(fileIdFile, FILID_NUM_FILES_OFFSET , SEEK_SET); 
    write(fileIdFile, &numFilesProcessed, NUM_FILES_FIELD); 

    //Clear the vector
    fileNameList.clear();


    return num_files;
}

// Flush the ngrams to the index files
void nGramIndex::flushIndex(ngram_t_indexfcount num_files){
    cout << "Flushing Index..." << endl;

    //Create the files
    createIndexFile(num_files);   
    createNGramFile();


    uint64_t bytes_flushed = 0;
    uint64_t offset = 0;

    //Write Buffer -- write out entires in chunks so writes aren't too small
    uint64_t max_entries = WRITE_BUFFER_SIZE  / NUM_FILES_FIELD;
    ngram_t_fidtype* write_buffer = new ngram_t_fidtype[max_entries];
    size_t current_entries = 0;

    //Index Buffer -- write it out in one big chunk
    uint8_t* index_buffer = new uint8_t[(INDEX_ENTRY_SIZE) * maxNgram];  

    for(uint64_t i = 0; i < maxNgram; i++){ //iterate through every ngram

        if(this->output_buffer[i].elements_size > 0){ //if it has any entries

            //Is the buffer full enough?
            if(current_entries + output_buffer[i].elements_size > max_entries){
                //Write all that we buffered up
                ssize_t written = write(nGramFile, write_buffer, current_entries * NUM_FILES_FIELD);
                if(written == -1){
                    cerr << "Error: " <<strerror(errno) << endl;
                    throw runtime_error("Write buffer not written");
                }
                bytes_flushed += written;
                current_entries = 0;
            }

            uint32_t counter = 0;
            while(output_buffer[i].elements_size > 0){
                counter++;
                write_buffer[current_entries++] = output_buffer[i].elements->front();
                output_buffer[i].elements->pop_front();
                buffer_memory_usage -= BUFFER_NODE_SIZE;
                output_buffer[i].elements_size--;
            }

            //Update Index Buffer
            ngram_t_offset* offset_location = (ngram_t_offset*) (index_buffer + (i * (INDEX_ENTRY_SIZE)));
            ngram_t_indexfcount* file_count_location = (ngram_t_indexfcount*) (index_buffer + (i * (INDEX_ENTRY_SIZE) + OFFSET_FIELD));
            *offset_location = offset;
            *file_count_location = counter;
            

            //Update the offset with the size of this ngram block
            offset += (counter * NUM_FILES_FIELD);
        }

    }

    //Flush anything left
    if (current_entries > 0){
        //Write all that we buffered up
        ssize_t written = write(nGramFile, write_buffer, (size_t) (current_entries * NUM_FILES_FIELD));
        if(written == -1){
            cerr << "Error: " <<strerror(errno) << endl;
            throw runtime_error("Write buffer not written");
        }
        bytes_flushed += written;
    }
    delete[] write_buffer;

    //Flush the index file
    write(indexFile, index_buffer, (INDEX_ENTRY_SIZE) * maxNgram);
    delete[] index_buffer;

    cout << "Flushed " << bytes_flushed << " Bytes " << endl;
    close(nGramFile);
    close(indexFile);

    //Update filid with new value
    numIndexFiles++;
    lseek(fileIdFile, FILID_NUM_INDEX_OFFSET , SEEK_SET);
    write(fileIdFile, &numIndexFiles, NUM_INDEX_FILES_FIELD);
}




