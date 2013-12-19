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

#ifndef SNGFILE_H
#define SNGFILE_H

#include <stdint.h>
#include <cstddef>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

namespace snugglefish {
   
    //Base class that keeps track of files
    //Abstraction from pure C method of writing files
    //Probably lighter-weight than C++ stream classes
    //It's re-inventing the wheel a bit, but I wanted to keep it
    // as light weight as possible without using pure C
    class file
    {

        public:
            //Constructor
            file(const char* fileName, size_t buffersize = (1024 * 1024 * 8 * 32));

            //Destructor
            //Calls close, then frees up malloc'd elements
            ~file();

            //Opens a file with flags O_RDWR | O_CREAT
            //Allocates a buffer for writing
            bool create(mode_t filemode = (mode_t)0755);

            //Opens a file either O_RDONLY or O_RDWR based on readwrite value
            //Allocates a buffer if in write mode
            bool open(char readwrite);


            //Mmaps the file read-only
            uint8_t* mmap();

            //Closes a file, flushes buffer first
            //frees buffer if allocated in open or create
            //Also closes mmap if opened
            bool close();

            
            //Read, just front-ends the read syscall
            bool read(uint8_t* destination, size_t length);

            //Read at specific locations (offset from SEEK_SET)
            //Using read_at and read together should be done carefully 
            bool read_at(int32_t location, uint8_t* destination, size_t length);

            //Buffered writer
            bool write(uint8_t* data, size_t length);

            //Non-Buffered write to specific locations (offset from SEEK_SET)
            bool write_at(int32_t location, uint8_t* data, size_t length);

            //Flush anything buffered
            bool flush();

            //Returns the size of the file using stat
            const size_t get_size();

            //Does file exist
            const bool exists();
            

        private:
            char* filename;
            bool atend;  //Is the write pointer at the end of the file (SEEK_END)?
            int32_t fd;  //File Descriptor
            uint8_t* mmapFile;
            size_t  size; //Size of File -- used for mmap purposes
            char* buffer;
            size_t buffersize;
            size_t bufferused;
            size_t bufferparam;

    };
    
}

#endif
