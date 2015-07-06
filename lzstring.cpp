//
// Created by alex on 05/07/2015.
//

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <map>
#include "utf8proc.h"

using namespace std;

// definitions
string decompress(string data);

typedef std::map<int, std::string> DICT;

string key_str = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void utf8char(utf8proc_uint8_t *buff, char utf8encoded[], int size) {
    memcpy(utf8encoded, (char *) buff, (size_t) size);
    utf8encoded[size] = '\0';
}

string decompress_from_base64(string data) {
    cout << data << endl;
    int str_len = data.size();
    if (str_len == 0)
        return NULL;

    string output = "";
    int _output = 0;
    int ol = 0;

    int enc1, enc2, enc3, enc4;
    int chr1, chr2, chr3, codepoint_len;
    int index = 0;

    utf8proc_uint8_t *codepoint_buff = new utf8proc_uint8_t[4];
    char utf8encoded[5];

    while (index < str_len) {
        enc1 = key_str.find(data[index]);
        index++;

        enc2 = key_str.find(data[index]);
        index++;

        enc3 = key_str.find(data[index]);
        index++;

        enc4 = key_str.find(data[index]);
        index++;

        chr1 = ((enc1 << 2) | (enc2 >> 4));
        chr2 = (((enc2 & 15) << 4) | (enc3 >> 2));
        chr3 = (((enc3 & 3) << 6) | enc4);

        if (ol % 2 == 0) {
            _output = chr1 << 8;

            if (enc3 != 64) {
                codepoint_len = utf8proc_encode_char(_output | chr2, codepoint_buff);
                utf8char(codepoint_buff, utf8encoded, codepoint_len);
                output += utf8encoded;
            }

            if (enc4 != 64) {
                _output = chr3 << 8;
            }
        } else {
            codepoint_len = utf8proc_encode_char(_output | chr1, codepoint_buff);
            utf8char(codepoint_buff, utf8encoded, codepoint_len);
            output += utf8encoded;

            if (enc3 != 64) {
                _output = chr2 << 8;
            }

            if (enc4 != 64) {
                codepoint_len = utf8proc_encode_char(_output | chr3, codepoint_buff);
                utf8char(codepoint_buff, utf8encoded, codepoint_len);
                output += utf8encoded;
            }
        }
        ol += 3;
    }
    return decompress(output);
}

int new_bits(int *maxpower, const char *data_string, int *data_index,
             utf8proc_int32_t *data_val, int *data_position) {
    int bits = 0;
    int power = 1;
    int resb = 0;
    while (power != *maxpower) {
        resb = *data_val & *data_position;
        *data_position >>= 1;
        if (*data_position == 0) {
            *data_position = 32768;
            *data_index += utf8proc_iterate((utf8proc_uint8_t *) data_string + *data_index, -1,
                                           data_val);
        }
        bits |= (resb > 0 ? 1 : 0) * power;
        power <<= 1;
    }
    return bits;
}

string decompress(string compressed) {
    int data_size = compressed.length();
    if (data_size == 0)
        return "";

    DICT dictionary;

    int enlargeIn = 4;
    int dictSize = 4;
    int numBits = 3;
    int index_c = 0;

    int codepoint_len = 0;
    utf8proc_uint8_t *codepoint_buff = new utf8proc_uint8_t[4];
    char c[5];

    string entry, result, w;
    const char *data_string = compressed.data();
    utf8proc_ssize_t data_index = 0;
    utf8proc_int32_t data_val;
    int data_position = 32768;

    data_index += utf8proc_iterate((utf8proc_uint8_t *) data_string, -1, &data_val);

    int maxpower = (int) pow(2, 2);
    int bits = new_bits(&maxpower, data_string, &data_index, &data_val, &data_position);
    int nnext = bits;

    switch (nnext) {
        case 0:
            maxpower = (int) pow(2, 8);
            bits = new_bits(&maxpower, data_string, &data_index, &data_val, &data_position);
            codepoint_len = utf8proc_encode_char(bits, codepoint_buff);
            utf8char(codepoint_buff, c, codepoint_len);
            break;
        case 1:
            maxpower = (int) pow(2, 16);
            bits = new_bits(&maxpower, data_string, &data_index, &data_val, &data_position);
            codepoint_len = utf8proc_encode_char(bits, codepoint_buff);
            utf8char(codepoint_buff, c, codepoint_len);
            break;
        case 2:  // error ?
            return "";
        default:break;
    }
    dictionary[3] = c;
    result = c;
    w = result;

    while (true) {
        if (data_index > data_size)
            return "";

        maxpower = (int) pow(2, numBits);
        bits = new_bits(&maxpower, data_string, &data_index, &data_val, &data_position);
        index_c = bits;

        switch (index_c) {
            case 0:
                maxpower = (int) pow(2, 8);
                bits = new_bits(&maxpower, data_string, &data_index, &data_val, &data_position);
                codepoint_len = utf8proc_encode_char(bits, codepoint_buff);
                utf8char(codepoint_buff, c, codepoint_len);
                dictionary[dictSize] = c;

                dictSize += 1;
                index_c = dictSize - 1;
                enlargeIn -= 1;
                break;
            case 1:
                maxpower = (int) pow(2, 16);

                bits = new_bits(&maxpower, data_string, &data_index, &data_val, &data_position);
                codepoint_len = utf8proc_encode_char(bits, codepoint_buff);
                utf8char(codepoint_buff, c, codepoint_len);
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
            entry = w + w[0];
        } else {
            return NULL;  // error
        }

        result += entry;
        dictionary[dictSize] = w + entry[0];
        dictSize += 1;
        enlargeIn -= 1;

        w = entry;

        if (enlargeIn == 0) {
            enlargeIn = (int) pow(2, numBits);
            numBits += 1;
        }
    }
}
