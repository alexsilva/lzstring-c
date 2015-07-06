//
// Created by alex on 05/07/2015.
//

#include <iostream>
#include <string.h>
#include <stdio.h>
#include "utf8proc.h"

using namespace std;

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
    char utf8encoded[4];

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
    return output;
}


