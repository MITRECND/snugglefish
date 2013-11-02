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


#include "nGramSearch.h"
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <libgen.h> //for dirname and basename()

using namespace snugglefish;
using namespace std;


nGramSearch::nGramSearch( uint32_t ngramLength, string indexFileName)
    :nGramBase(ngramLength, indexFileName) {
    
    if(!loadFileIdFile()){
        throw runtime_error("Error opening File Id File");
    }
        


}

nGramSearch::~nGramSearch(){
    
}



bool nGramSearch::loadFileIdFile(){
    uint32_t filid_fd;
    
    filid_fd = open(this->fileIdFileName.c_str(), O_RDONLY);
    
    if(filid_fd == -1){
        return false;
    }

    return nGramBase::loadFileIdFile(filid_fd);
}

bool nGramSearch::loadIndexFile(uint32_t id){
    uint32_t index_fd; 
    string filename = indexFileName;

    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, id);
    filename = filename.append(number_string);

    index_fd = open(filename.c_str(), O_RDONLY);

    if(index_fd == -1){
        return false;
    }

    ngram_t_endian endian_check = 0;
    ngram_t_version version = 0;
    ngram_t_size ngram_size = 0;
    ngram_t_indexfcount ngram_files = 0;

    read(index_fd, &endian_check, ENDIAN_CHECK_FIELD);
    read(index_fd, &version, VERSION_FIELD);
    read(index_fd, &ngram_size, NGRAM_SIZE_FIELD);
    read(index_fd, &ngram_files, INDEX_HEADER_NUM_FILES_FIELD);


    struct stat st;
    if(stat(filename.c_str(), &st) == 0){
       this->indexMmapSize = st.st_size;
    }else{
        throw runtime_error("Stat Index File");
    }

    indexMmap = (uint8_t*) mmap(NULL, (size_t) indexMmapSize, PROT_READ, MAP_SHARED, index_fd, 0);

    if(indexMmap == MAP_FAILED){
        cerr << "Error: " << strerror(errno) << endl;
        throw runtime_error("Load Index Map");
    }   

    //Pointer Math
    indexEntries = indexMmap + INDEX_HEADER_SIZE;
    
    close(index_fd);

    return true; 
}

bool nGramSearch::loadNGramFile(uint32_t id){
    uint32_t ngram_fd;
    string filename = nGramFileName;

    char number_string[FILE_NUM_BUFFER_SIZE];
    snprintf(number_string, FILE_NUM_BUFFER_SIZE, FILE_NUM_SPRINTF_STRING, id);
    filename = filename.append(number_string);

    ngram_fd = open(filename.c_str(), O_RDONLY);

    if(ngram_fd == -1){
        return false;
    }

    struct stat st;
    if(stat(filename.c_str(), &st) == 0){
        this->nGramMmapSize = st.st_size; 
    }else{
        throw runtime_error("Stat NGram File");
    }

    nGramMmap = (uint8_t*) mmap(NULL, (size_t) nGramMmapSize, PROT_READ, MAP_SHARED, ngram_fd, 0);

    if (nGramMmap == MAP_FAILED){
        cerr << "Error: " << strerror(errno) << endl;
        throw runtime_error("Map NGram File");
    }
    
    close(ngram_fd);

    return true;
}


bool nGramSearch::unloadNGramFile(){ 
    munmap(nGramMmap, (size_t) nGramMmapSize);
    return true;
}

bool nGramSearch::unloadIndexFile(){
    munmap(indexMmap, (size_t) indexMmapSize);
    return true;
}




vector<uint64_t> nGramSearch::stringToNGrams(string searchString){
    uint64_t nGram;
    vector <uint64_t> ngrams;

    for(size_t i = 0; i + ngramLength - 1 < searchString.length(); i++){
        nGram = 0;
        for(size_t j = 0; j < ngramLength; j++){
	    // (1 << (8*j)) is equivalent to pow(256,j)
            nGram += (unsigned char)searchString[i+j] * (1 << (8*j));
        }
        ngrams.push_back(nGram);
    }

    return ngrams;
}


vector<string> nGramSearch::searchNGrams(vector<uint64_t> nGramQuery){
    //File IDs file should be loaded already
    //All others need to be loaded on the fly    

    vector<string> matchedFiles;

    //Go through each index fileset
    for(uint32_t i = 0; i < numIndexFiles; i++){
        loadIndexFile(i);
        loadNGramFile(i);

        //Get ordered list of NGrams
	// In assending order by number of files that contain that ngram
        list<index_entry> queryList = orderNGrams(nGramQuery);

        list<ngram_t_fidtype> matchedIds;
        //Get list of File Ids that match NGrams
        searchAlpha(queryList, matchedIds);

        //Convert File IDs to filenames
        for(list<ngram_t_fidtype>::iterator ft = matchedIds.begin();
                ft != matchedIds.end(); ft++){
            
            ngram_t_offset file_id_offset = FILID_HEADER_SIZE + ((*ft) * maxFileNameLength);
            char * buffer = new char[maxFileNameLength + 1];
            lseek(fileIdFile, file_id_offset  ,SEEK_SET);
            read(fileIdFile, buffer, maxFileNameLength);
            string matched_filename(buffer);
    
            matchedFiles.push_back(matched_filename);
            delete[] buffer;
        }



        unloadIndexFile();
        unloadNGramFile();
    }

    close(fileIdFile);
    return matchedFiles;

}

list<index_entry> nGramSearch::orderNGrams(const vector<uint64_t> & nGramQuery){
    bool nomatch = false;
    list<index_entry> queryList;
    for(uint32_t j = 0; j < nGramQuery.size(); j++){
        index_entry* index_table = (index_entry*) (indexEntries + (nGramQuery[j] * (INDEX_ENTRY_SIZE)));

        if(index_table->num_files  == 0){
            nomatch = true;
            break;
            //No Files Match
        }

        if( queryList.empty()){
            queryList.push_back(*index_table);
        }else{
            bool placed = false;
            for (list<index_entry>::iterator it = queryList.begin(); 
              it != queryList.end(); it++ ){
                if((*it).num_files > index_table->num_files){
                    queryList.insert(it, *index_table);
                    placed = true;
                    break;
                }
            }
            if (!placed){
                queryList.push_back(*index_table);
            }
        }
    }

    if (nomatch){
        queryList.clear();
    }

    return queryList;

}

// Takes the list of sorted fids, and puts the common fids into matchedIds
void nGramSearch::searchAlpha(list<index_entry> &queryList, list<ngram_t_fidtype> &matchedIds){

        if(queryList.size() == 0){
            return;
        }
	// this gets te ngram list for the first file, and places them 
	// into the matchedIds list
        ngram_t_fidtype* ngram_location = (ngram_t_fidtype*) (nGramMmap + queryList.front().offset);
        for(uint32_t j = 0; j < queryList.front().num_files; j++){
            matchedIds.push_back(ngram_location[j]);
        }


        //Dont need the front element anymore so get rid of it
        queryList.pop_front();

        //For every id see if it's in the remaining Ngrams

        //Now for every subsequent NGram whittle down the list
        for(list<index_entry>::iterator it = queryList.begin();
                it != queryList.end(); it++){


                ngram_t_numfiles ngram_elements = (*it).num_files;
                ngram_location = (ngram_t_fidtype*) (nGramMmap + (*it).offset);
		uint32_t ngram_index = 0;
                list<ngram_t_fidtype>::iterator ft = matchedIds.begin();
                    while(ft != matchedIds.end()){
                    bool found = false;
		    // Checks each element of the next fid array for
		    // the current fid in matchedIds
		    while(ngram_index < ngram_elements){
                        if (ngram_location[ngram_index] == (*ft)){
                            found = true;
                            break;
                        }else if(ngram_location[ngram_index] > (*ft)){
                            //update ngram location
			    // If the fid is too large, it is
			    // necessary to recheck the same index on the
			    // next pass, as to make sure it isn't missed
                            break;
                        }
			ngram_index++;
                    }

                    if(!found){
                        ft = matchedIds.erase(ft);
                    }else{
			// ft++ is only needed if erase isn't called as 
			// erase moves the iterator ahead
		        ft++;
		    }
                }

        }
}
