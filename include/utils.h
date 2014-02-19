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


#ifndef SNUGGLE_UTILS_H
#define SNUGGLE_UTILS_H


#include <stdint.h>
#include <list>
#include <vector>
#include <algorithm> // std::sort

//TODO REMOVE
#include <iostream>
#include <bitset>


namespace snugglefish{

   
    std::list<uint8_t> vbencode_number(uint32_t number){
        std::list<uint8_t> bytes; 
        while (true){
            bytes.push_front(number % 128);
            if (number < 128){
                break;
            }
            number /= 128;
        }
        bytes.back() += 128;
        return bytes;
    } 


    std::list<uint8_t> vbencode_numbers(const std::vector<uint32_t>& numbers){
        std::list<uint8_t> output;
        for(int32_t i = 0; i < numbers.size(); i++){
            std::list<uint8_t> out = vbencode_number(numbers[i]);
            if (i == 0){
                output = out;
            }else{
                output.splice(output.end(), out);
            }   
        }

        return output;
    }

    std::vector<uint32_t> vbdecode_numbers(std::list<uint8_t> numbers){
        std::vector<uint32_t> output;
        uint32_t n = 0;

        for (std::list<uint8_t>::const_iterator iterator = numbers.begin(); iterator != numbers.end(); iterator++){
            if (*iterator < 128){
                n = 128 * n + *iterator;
            }else{
                n = 128 * n + (*iterator - 128);
                output.push_back(n);
                n = 0;
            }
        } 

        return output;
    }


    uint32_t pfor_analyze_bits(std::vector<uint32_t> v, int b, uint32_t& length, uint32_t& exceptions){
        uint32_t len = 0, min = 0, range = 1 << b;

        for(uint32_t lo = 0, hi = 0; hi < v.size(); hi++){
            if(v[hi] - v[lo] >= range){
                if(hi - lo > len){
                    min = lo; 
                    len = hi - lo;
                }     
                while(v[hi] - v[lo] >= range) lo++;
            }
        }
        exceptions = v.size() - len;
        length = len + 1;
        return min;
    }


    uint32_t bit_encode(uint32_t number, uint32_t bits, uint32_t& overflow){
        overflow = number >> bits;
        return number & (( 1 << bits) - 1);
    }


    uint32_t pfordelta_bitsize(const std::vector<uint32_t>& numbers, uint32_t& exceptions, uint32_t max_exceptions = 16){
        //Sort the array
        std::vector<uint32_t> sorted_deltas(numbers);
        std::sort(sorted_deltas.begin(), sorted_deltas.end());

        uint32_t low_ratio = 0xFFFFFFFF;
        uint8_t bits = 0;
        for(uint32_t i = 3; i < 16; i++){//Assume 1 or 2 are too small TODO figure out best lower bound
            uint32_t length;
            uint32_t excep;
            pfor_analyze_bits(sorted_deltas, i, length, excep);
            uint32_t compression_ratio = i + (excep / (bool) sorted_deltas.size()) * (8 * sorted_deltas.size());
            //std::cout << i << " = " << exceptions << "  " << compression_ratio<< std::endl;           
            if (compression_ratio < low_ratio){
                exceptions = excep;
                low_ratio = compression_ratio;
                bits = i;
            }else{ // >=
                //break;
            }
        }

        if(exceptions > max_exceptions){
            bits = 0;
        }

        return bits;
    } 

    uint8_t* packer(const std::vector<uint32_t>& numbers, uint32_t packsize){
        uint32_t total_size = (numbers.size() * packsize) / 8; //TODO account for float
        uint8_t* output = new uint8_t[total_size](); //the () initializes the values to 0


        if (packsize <= 3){ //Less than half a byte
            int left = 0;//how many bits are left in the output byte
            int shift = 0;//how many bits have we used in the input byte

            for(int i = 0, j =0; i < numbers.size(); i++){
                if(left){
                    if (left >= packsize){
                        output[j] |= numbers[i] << (left - packsize);
                        left -= packsize;
                        if(left){
                            continue;
                        }
                        else{
                            j++;
                            continue;
                        }
                    }else{
                        output[j] |= numbers[i] >> (packsize - left);
                        shift = left;
                        left = 0;
                        j++;
                    }
    
                }

                output[j] |= (numbers[i] << (8 - packsize)) << shift;
                left = 8 - packsize + shift;
                shift = 0;
            }
         
        }else if (packsize <= 8){ //1 byte or less
            //Iterate through every number
            int left = 0;
            int shift = 0;
            for(int i = 0, j = 0; i < numbers.size(); i++){
                if (left){
                    output[j++] |= numbers[i] >> (packsize - left);
                    if (left == packsize){
                        left = 0;
                        continue;
                    }
                    shift = left;
                    left = 0;
                }
    
                output[j] |= (numbers[i] & (((1 << packsize) - 1) >> shift))  << (8 - (packsize - shift));
                left = 8 - (packsize - shift); // how many bits are left in this byte
                shift = 0;
            }

            return output;
        

        }else if (packsize <= 16) { //2 bytes or less
        
        }else {
            //error

        }
    }

    /*
        This code written based on information gleaned from:

        Hao Yan, Shuai Ding, and Torsten Suel. “Inverted index compression and
        query processing with optimized document ordering.”
        In Proceedings of the 18th international conference on World wide web (WWW ’09).
        ACM, New York, NY, USA, 401-410
    */    

    void pfordelta_encode(const std::vector<uint32_t>& numbers, uint32_t max_exceptions = 16){

        std::vector<uint32_t> deltas;

        //Create Delta Array
        for(uint32_t i = 0; i < numbers.size(); i++){
            if (i == 0){
                deltas.push_back(numbers[i]);
            }else{
                deltas.push_back(numbers[i] - numbers[i - 1]);
            }
        }
        
        uint32_t exceptions = 0;
        uint32_t bits = pfordelta_bitsize(deltas, exceptions);


        if(!bits){//wasn't able to find a number that matches constraints
            //TODO
        }

        //At this point we should have the bitsize and the number of exceptions

        std::vector<uint32_t> compressed;
        std::vector<uint32_t> exception_index;
        std::vector<uint32_t> exception_values;
        for (uint32_t i = 0; i < deltas.size(); i++){
            uint32_t overflow;
            compressed.push_back(bit_encode(deltas[i], bits, overflow));
            if(overflow){
                exception_index.push_back(i);
                exception_values.push_back(overflow); 
            }
            std::bitset<6> foo;
            foo = compressed[i];
            std::cout << foo << std::endl;
        }

        std::bitset<8> bit_val;
        //Bitpack
        uint8_t leader = bits << 4 | exceptions;
        uint8_t* packed = packer(compressed, bits);
        std::list<uint8_t> e_index = vbencode_numbers(exception_index);
        std::list<uint8_t> e_vals = vbencode_numbers(exception_values);



        bit_val = leader;
        std::cout << bit_val << std::endl;
        for(int i = 0; i < (compressed.size() * bits) / 8 ; i++){
            bit_val = packed[i];
            std::cout << bit_val << std::endl;
        }

        std::cout<< "Exception Indexes: " << std::endl;

        for(std::list<uint8_t>::iterator iter = e_index.begin(); iter != e_index.end(); iter++){
            bit_val = *iter;
            std::cout << bit_val << std::endl;
        }


        std::cout<< "Exception Values: " << std::endl;
        for(std::list<uint8_t>::iterator iter = e_vals.begin(); iter != e_vals.end(); iter++){
            bit_val = *iter;
            std::cout << bit_val << std::endl;
        }

    }
    

}

#endif
