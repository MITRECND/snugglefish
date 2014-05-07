/*
Approved for Public Release; Distribution Unlimited: 13-1937

Copyright (c) 2014 The MITRE Corporation. All rights reserved.

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

#ifndef SNGCOMMON_H
#define SNGCOMMON_H

#include <vector>
#include <string>

typedef struct _mi_data {
    std::vector <std::string>* fileList;
    uint32_t queue;
    void* ngramindex;

    uint32_t ngramSize;

    pthread_mutex_t filesMutex;
    pthread_mutex_t nGramIndexMutex;
} mi_data;

#define ngram_t_endian              uint32_t
#define ngram_t_version             uint8_t
#define ngram_t_size                uint8_t
#define ngram_t_fnlength            uint16_t
#define ngram_t_indexcount          uint32_t
#define ngram_t_fidtype             uint32_t

#define ngram_t_indexfcount         uint32_t
#define ngram_t_offset              uint64_t
#define ngram_t_numfiles            uint32_t


#define ENDIAN_CHECK                            0x01234567
#define VERSION                                 0x01

//File Header Fields in bytes
//Shared
#define ENDIAN_CHECK_FIELD                      sizeof(ngram_t_endian)
#define VERSION_FIELD                           sizeof(ngram_t_version) 
#define NGRAM_SIZE_FIELD                        sizeof(ngram_t_size) 
//filid file only
#define MAX_FILENAME_LENGTH_FIELD               sizeof(ngram_t_fnlength) 
//#define MAX_FILES_PER_NGRAM_FIELD               4 //TODO delete
#define NUM_INDEX_FILES_FIELD                   sizeof(ngram_t_indexcount) 
#define NUM_FILES_FIELD                         sizeof(ngram_t_fidtype) //Number of files in catalog

#define INDEX_HEADER_NUM_FILES_FIELD            sizeof(ngram_t_indexfcount)//number of files in an index

//index file only
#define OFFSET_FIELD                            sizeof(ngram_t_offset) //64-bit offset into ngram file
#define INDEX_NUM_FILES_FIELD                   sizeof(ngram_t_numfiles) //how many files in that ngram

#define FILID_HEADER_SIZE   ENDIAN_CHECK_FIELD + \
                            VERSION_FIELD+ \
                            NGRAM_SIZE_FIELD+ \
                            MAX_FILENAME_LENGTH_FIELD+ \
                            NUM_INDEX_FILES_FIELD + \
                            NUM_FILES_FIELD
                    
#define FILID_NUM_INDEX_OFFSET  ENDIAN_CHECK_FIELD + \
                                VERSION_FIELD +  \
                                NGRAM_SIZE_FIELD +  \
                                MAX_FILENAME_LENGTH_FIELD

#define FILID_NUM_FILES_OFFSET  ENDIAN_CHECK_FIELD + \
                                VERSION_FIELD + \
                                NGRAM_SIZE_FIELD + \
                                MAX_FILENAME_LENGTH_FIELD + \
                                NUM_INDEX_FILES_FIELD

#define INDEX_HEADER_SIZE   ENDIAN_CHECK_FIELD + \
                            VERSION_FIELD + \
                            NGRAM_SIZE_FIELD + \
                            INDEX_HEADER_NUM_FILES_FIELD

#define INDEX_ENTRY_SIZE    OFFSET_FIELD + \
                            INDEX_NUM_FILES_FIELD
//Ngram File Constants
#define NGRAM_FILE_EXTENSION    ".ngram"
#define INDEX_FILE_EXTENSION    ".index"
#define FILEID_FILE_EXTENSION   ".sngfs"

// These defines govern the file number at the end of the file
// The string governs the amount of 0's that are used
// The buffer size, indicates the size of the string buffer
// the string number (08 by default) should be less than 
// the buffer size + 1
#define FILE_NUM_SPRINTF_STRING "%08u"
#define FILE_NUM_BUFFER_SIZE 30
#define FILE_MODE (mode_t)0775

#endif
