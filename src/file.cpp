#include "file.h"
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

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
    this->close();
    
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
    //if fd != 0 some error

    if (this->bufferparam > 0){
        this->buffersize = this->bufferparam;
        this->buffer = (char*) malloc(buffersize);
    }
    this->fd = ::open(this->filename, O_RDWR | O_CREAT, filemode);
    this->atend = true;

    if (this->fd <= 0){
        //Throw some error
    }

    return true;
}

bool file::open(char readwrite){
    //if fd != 0 some error

    switch (readwrite){
        case 'r':
        {    
            this->fd = ::open(this->filename, O_RDONLY);
            this->size = this->get_size();
            break;
        }
        case 'w':
        {
            if (this->bufferparam > 0){
                this->buffersize = this->bufferparam;
                this->buffer = (char*) malloc(buffersize);
            }
            this->fd = ::open(this->filename, O_RDWR);
            this->atend = true;
            break;
        }
        default:
            //Throw some error
            break;
    }

    if (this->fd <= 0){
        //Throw some error
    }

    return true;
}

uint8_t* file::mmap(){

    //Open the file first
    if (this->fd == 0)
        this->open('r');

    this->mmapFile = (uint8_t*) ::mmap(NULL, this->size, PROT_READ, MAP_SHARED, this->fd, 0);

    if(this->mmapFile == MAP_FAILED){
        throw runtime_error("Loading Map");
    }

    return this->mmapFile;

}

bool file::close(){
    //Flush any data that's been buffered
    this->flush(); 

    //Close the mmap if opened
    if (this->mmapFile){
        munmap(this->mmapFile, this->size); 
        this->mmapFile = 0;
        this->size = 0;
    }


    if (this->buffer){
        free(this->buffer);
        this->buffer = NULL;
        this->buffersize = this->bufferused = 0;
    }

    int32_t retval = ::close(this->fd);
    if(retval){//non zero
        //Throw some error
    }

    this->fd = 0;

    return true;

}

bool file::flush(){
    if (this->bufferused){
        //Seek to the end of the file first
        if (!this->atend){
            lseek(this->fd, 0, SEEK_END);
            this->atend = true;
        }

        ssize_t written = ::write(this->fd, this->buffer, this->bufferused); 
        if (written == -1){
            //Some Error
        }
        this->bufferused = 0;
    }

    return true;
}


bool file::write(uint8_t * data, size_t length){
    //Technically, I don't need to do this * sizeof(uint8_t) business, but whatever
    if((this->bufferused + (length * sizeof(uint8_t))) > this->buffersize) {
        this->flush();

        if (length < this->buffersize){
            memcpy(this->buffer, data, length * sizeof(uint8_t));
            this->bufferused = length * sizeof(uint8_t);
        }else{ //Data being passed in is larger than buffer, write out directly
            //Seek to the end of the file first
            if (!this->atend){
                lseek(this->fd, 0, SEEK_END);
                this->atend = true;
            }

            ssize_t written = ::write(this->fd, data, length * sizeof(uint8_t));
            if (written == -1){
                //Some Error
            }
        }
    }else{ // just buffer it up
        memcpy(this->buffer + this->bufferused, data, length * sizeof(uint8_t));
        this->bufferused += length * sizeof(uint8_t);
    }

    return true;
}


bool file::write_at(int32_t location, uint8_t * data, size_t length){
    this->atend = false;

    lseek(this->fd, location, SEEK_SET);

    ssize_t written = ::write(this->fd, data, length * sizeof(uint8_t));
    if (written == -1){
        //Some Error
    }

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
