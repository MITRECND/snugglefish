#include "file.h"
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <errno.h>

using namespace snugglefish;
using namespace std;

file::file(const char* fileName, size_t buffersize){
    this->bufferparam = buffersize;

    this->buffer = NULL;
    this->buffersize = 0;
    this->bufferused = 0;

    this->fd = 0;
    this->mmapFile = 0;
    this->size = 0;

    this->filename = (char*) malloc(sizeof(char) * (strlen(fileName) + 1));
    strncpy(this->filename, fileName, strlen(fileName) + 1);
}


file::~file(){
    //File Descriptor is not zero
    if(this->fd){
        this->close();
    }
    
    if (this->filename != NULL){
        free(this->filename);
    }

    if (this->buffer){
        free(this->buffer);
        this->buffer = NULL;
        this->buffersize = this->bufferused = 0;
    }
}

bool file::create(mode_t filemode){

    if (this->bufferparam > 0){
        this->buffersize = this->bufferparam;
        this->buffer = (char*) malloc(buffersize);
    }
    this->fd = ::open(this->filename, O_RDWR | O_CREAT, filemode);
    this->readonly = false;

    if (this->fd <= 0){
        cerr << "Unable to Create File: " << filename << " -- Error: " << strerror(errno) << endl;
        throw runtime_error("Creating File");
    }

    return true;
}

bool file::open(char readwrite){
    switch (readwrite){
        case 'r':
        {    
            this->fd = ::open(this->filename, O_RDONLY);
            this->size = this->get_size();
            this->readonly = true;
            break;
        }
        case 'w':
        {
            if (this->bufferparam > 0){
                this->buffersize = this->bufferparam;
                this->buffer = (char*) malloc(buffersize);
            }
            this->fd = ::open(this->filename, O_RDWR);
            this->readonly = false;
            break;
        }
        default:
            throw runtime_error("Unrecognized read/write mode");
            break;
    }

    if (this->fd <= 0){
        cerr << "Error Opening File: " << this->filename << " -- Error: " << strerror(errno) << endl;
        throw runtime_error("Opening File");
    }

    return true;
}

uint8_t* file::mmap(){
    //Open the file first 
    if (this->fd == 0)
        this->open('r');

    if (!this->size)
        return NULL;

    this->mmapFile = (uint8_t*) ::mmap(NULL, this->size, PROT_READ, MAP_SHARED, this->fd, 0);

    if(this->mmapFile == MAP_FAILED){
        cerr << "Error Loading Map for File : " << this->filename<< " -- Error: " << strerror(errno) << endl;
        throw runtime_error("Loading Map");
    }

    return this->mmapFile;

}

bool file::close(){
    if (!this->fd){ // already closed?
        return true;
    }

    this->flush(); 

    //Close the mmap if opened
    if (this->mmapFile){
        munmap(this->mmapFile, this->size); 
        this->mmapFile = 0;
        this->size = 0;
    }

    //Free the write buffer if allocated
    if (this->buffer){
        free(this->buffer);
        this->buffer = NULL;
        this->buffersize = this->bufferused = 0;
    }

    int32_t retval = ::close(this->fd);
    if(retval){//non zero
        cerr << "Error Closing File: " << this->filename << " -- Error: " << strerror(errno) << endl;
        throw runtime_error("Closing File");
    }

    this->fd = 0;

    return true;

}

bool file::real_write(int fd, uint8_t* data, size_t length){
        ssize_t written = ::write(fd, data, length); 
        if (written == -1){//TODO what if partial write?
            cerr << "Unable to write to file: " << this->filename << " -- Error: " << strerror(errno) << endl;
            throw runtime_error("Write Error");
        }

    return true;
}

bool file::flush(){
    if(this->readonly){
        return true;
    }

    if (this->bufferused){
        lseek(this->fd, 0, SEEK_END);
        this->real_write(this->fd, (uint8_t*) this->buffer, this->bufferused * sizeof(char));
        this->bufferused = 0;
    }

    return true;
}


bool file::write(uint8_t * data, size_t length){
    if (this->readonly){
        throw runtime_error("Write command on read-only file");
    }

    if((this->bufferused + (length)) > this->buffersize) {
        this->flush();

        if (length < this->buffersize){
            memcpy(this->buffer, data, length);
            this->bufferused = length;
        }else{  
            //Data being passed in is larger than buffer, write out directly
            //Seek to the end of the file first
            lseek(this->fd, 0, SEEK_END);
            this->real_write(this->fd, data, length);
        }
    }else{ // just buffer it up
        memcpy(this->buffer + this->bufferused, data, length);
        this->bufferused += length;
    }

    return true;
}


bool file::write_at(int32_t location, uint8_t * data, size_t length){
    if (this->readonly){
        throw runtime_error("Write command on reaodnly file");
    }

    //Flush before writing at locations
    flush();
    lseek(this->fd, location, SEEK_SET);
    this->real_write(this->fd, data, length * sizeof(uint8_t));

    return true;

}

bool file::read(uint8_t* dest, size_t length){
   ::read(this->fd, dest, length); 
}

bool file::read_at(int32_t location, uint8_t* dest, size_t length){
    lseek(this->fd, location, SEEK_SET);
    ::read (this->fd, dest, length);
}

const size_t file::get_size(){
    struct stat st;
    size_t filesize = 0;
    if (stat(this->filename, &st) == 0){
       filesize = st.st_size;
    }else{
        throw runtime_error("Statting File");
    } 

    return filesize;
}

const bool file::exists(){
    struct stat st;
    if(stat(this->filename, &st) == 0){
        return true;
    }else{
        return false;
    }
}
