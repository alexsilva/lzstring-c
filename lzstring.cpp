//
// Created by alex on 05/07/2015.
//

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <map>
#include <vector>
#include "utf8proc.h"
#include "string.hxx"

using namespace std;

// definitions
utf8::string decompress(utf8::string data);
typedef std::map<int, utf8::string> DICT;

const string BASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

utf8::string utf8encode(int data) {
    char dst[5];

    utf8proc_uint8_t *buff = new utf8proc_uint8_t[4];
    utf8proc_ssize_t size = utf8proc_encode_char(data, buff);

    memcpy(dst, (char *) buff, (size_t) size);
    dst[size] = '\0';

    return utf8::string(dst);
}

utf8::string decompress_from_base64(string data) {
    int str_len = data.size();
    if (str_len == 0)
        return NULL;

    utf8::string output("");
    int _output = 0;
    int ol = 0;

    int enc1, enc2, enc3, enc4;
    int chr1, chr2, chr3;
    int index = 0;

    while (index < str_len) {
        enc1 = BASE64.find(data[index]);
        index++;

        enc2 = BASE64.find(data[index]);
        index++;

        enc3 = BASE64.find(data[index]);
        index++;

        enc4 = BASE64.find(data[index]);
        index++;

        chr1 = ((enc1 << 2) | (enc2 >> 4));
        chr2 = (((enc2 & 15) << 4) | (enc3 >> 2));
        chr3 = (((enc3 & 3) << 6) | enc4);

        if (ol % 2 == 0) {
            _output = chr1 << 8;

            if (enc3 != 64) {
                output += utf8encode(_output | chr2);
            }

            if (enc4 != 64) {
                _output = chr3 << 8;
            }
        } else {
            output += utf8encode(_output | chr1);

            if (enc3 != 64) {
                _output = chr2 << 8;
            }

            if (enc4 != 64) {
                output += utf8encode(_output | chr3);
            }
        }
        ol += 3;
    }
    return decompress(output);
}

int new_bits(int &maxpower, utf8::string &data_string,
             int &data_index, int &data_val, int &data_position) {
    int bits = 0;
    int power = 1;
    int resb = 0;
    while (power != maxpower) {
        resb = data_val & data_position;
        data_position >>= 1;
        if (data_position == 0) {
            data_position = 32768;
            data_val = data_string[data_index];
            data_index++;
        }
        bits |= (resb > 0 ? 1 : 0) * power;
        power <<= 1;
    }
    return bits;
}

utf8::string decompress(utf8::string data_string) {
    int data_size = data_string.length();
    if (data_size == 0)
        return "";

    DICT dictionary;

    int enlargeIn = 4;
    int dictSize = 4;
    int numBits = 3;
    int index_c = 0;

    utf8::string entry, result, w, c;
    int data_index = 1;
    int data_val;
    int data_position = 32768;
    data_val = data_string[0];

    int maxpower = (int) pow(2, 2);
    int bits = new_bits(maxpower, data_string, data_index, data_val, data_position);
    int nnext = bits;

    switch (nnext) {
        case 0:
            maxpower = (int) pow(2, 8);
            bits = new_bits(maxpower, data_string, data_index, data_val, data_position);
            c = utf8encode(bits);
            break;
        case 1:
            maxpower = (int) pow(2, 16);
            bits = new_bits(maxpower, data_string, data_index, data_val, data_position);
            c = utf8encode(bits);
            break;
        case 2:  // error ?
            return "error: bad data";
        default:break;
    }
    dictionary[3] = c;
    result = c;
    w = result;

    while (true) {
        if (data_index > data_size)
            return "";

        maxpower = (int) pow(2, numBits);
        bits = new_bits(maxpower, data_string, data_index, data_val, data_position);
        index_c = bits;

        switch (index_c) {
            case 0:
                maxpower = (int) pow(2, 8);
                bits = new_bits(maxpower, data_string, data_index, data_val, data_position);
                c = utf8encode(bits);
                dictionary[dictSize] = c;

                dictSize += 1;
                index_c = dictSize - 1;
                enlargeIn -= 1;
                break;
            case 1:
                maxpower = (int) pow(2, 16);
                bits = new_bits(maxpower, data_string, data_index, data_val, data_position);
                c = utf8encode(bits);
                dictionary[dictSize] = c;

                dictSize += 1;
                index_c = dictSize - 1;
                enlargeIn -= 1;
                break;
            case 2:
                return result;
            default:break;
        }

        if (enlargeIn == 0) {
            enlargeIn = (int) pow(2, numBits);
            numBits += 1;
        }

        if (dictionary.count(index_c) > 0) {
            entry = dictionary.find(index_c)->second;
        } else if (index_c == dictSize) {
            w.push_back(w[0]);
            entry = w;
        } else {
            return result;  // invalid result
        }

        result += entry;

        w.push_back(entry[0]);
        dictionary[dictSize] = w;
        dictSize += 1;
        enlargeIn -= 1;

        w = entry;

        if (enlargeIn == 0) {
            enlargeIn = (int) pow(2, numBits);
            numBits += 1;
        }
    }
}
