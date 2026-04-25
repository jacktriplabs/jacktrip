//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2026 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file Http3Protocol.cpp
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#include "Http3Protocol.h"

#include <iostream>

using std::cerr;
using std::endl;

namespace Http3
{

//*******************************************************************************
// QPACK static table (RFC 9204 Appendix A)
// Each entry is (name, value); access with name = TABLE[index*2], value =
// TABLE[index*2+1]
const char* QPACK_STATIC_TABLE[] = {
    ":authority",
    "",  // 0
    ":path",
    "/",  // 1
    "age",
    "0",  // 2
    "content-disposition",
    "",  // 3
    "content-length",
    "0",  // 4
    "cookie",
    "",  // 5
    "date",
    "",  // 6
    "etag",
    "",  // 7
    "if-modified-since",
    "",  // 8
    "if-none-match",
    "",  // 9
    "last-modified",
    "",  // 10
    "link",
    "",  // 11
    "location",
    "",  // 12
    "referer",
    "",  // 13
    "set-cookie",
    "",  // 14
    ":method",
    "CONNECT",  // 15
    ":method",
    "DELETE",  // 16
    ":method",
    "GET",  // 17
    ":method",
    "HEAD",  // 18
    ":method",
    "OPTIONS",  // 19
    ":method",
    "POST",  // 20
    ":method",
    "PUT",  // 21
    ":scheme",
    "http",  // 22
    ":scheme",
    "https",  // 23
    ":status",
    "103",  // 24
    ":status",
    "200",  // 25
    ":status",
    "304",  // 26
    ":status",
    "404",  // 27
    ":status",
    "503",  // 28
    "accept",
    "*/*",  // 29
    "accept",
    "application/dns-message",  // 30
    "accept-encoding",
    "gzip, deflate, br",  // 31
    "accept-ranges",
    "bytes",  // 32
    "access-control-allow-headers",
    "cache-control",  // 33
    "access-control-allow-headers",
    "content-type",  // 34
    "access-control-allow-origin",
    "*",  // 35
    "cache-control",
    "max-age=0",  // 36
    "cache-control",
    "max-age=2592000",  // 37
    "cache-control",
    "max-age=604800",  // 38
    "cache-control",
    "no-cache",  // 39
    "cache-control",
    "no-store",  // 40
    "cache-control",
    "public, max-age=31536000",  // 41
    "content-encoding",
    "br",  // 42
    "content-encoding",
    "gzip",  // 43
    "content-type",
    "application/dns-message",  // 44
    "content-type",
    "application/javascript",  // 45
    "content-type",
    "application/json",  // 46
    "content-type",
    "application/x-www-form-urlencoded",  // 47
    "content-type",
    "image/gif",  // 48
    "content-type",
    "image/jpeg",  // 49
    "content-type",
    "image/png",  // 50
    "content-type",
    "text/css",  // 51
    "content-type",
    "text/html; charset=utf-8",  // 52
    "content-type",
    "text/plain",  // 53
    "content-type",
    "text/plain;charset=utf-8",  // 54
    "range",
    "bytes=0-",  // 55
    "strict-transport-security",
    "max-age=31536000",  // 56
    "strict-transport-security",
    "max-age=31536000; includesubdomains",  // 57
    "strict-transport-security",
    "max-age=31536000; includesubdomains; preload",  // 58
    "vary",
    "accept-encoding",  // 59
    "vary",
    "origin",  // 60
    "x-content-type-options",
    "nosniff",  // 61
    "x-xss-protection",
    "1; mode=block",  // 62
    ":status",
    "100",  // 63
    ":status",
    "204",  // 64
    ":status",
    "206",  // 65
    ":status",
    "302",  // 66
    ":status",
    "400",  // 67
    ":status",
    "403",  // 68
    ":status",
    "421",  // 69
    ":status",
    "425",  // 70
    ":status",
    "500",  // 71
    "accept-language",
    "",  // 72
    "access-control-allow-credentials",
    "FALSE",  // 73
    "access-control-allow-credentials",
    "TRUE",  // 74
    "access-control-allow-headers",
    "*",  // 75
    "access-control-allow-methods",
    "get",  // 76
    "access-control-allow-methods",
    "get, post, options",  // 77
    "access-control-allow-methods",
    "options",  // 78
    "access-control-expose-headers",
    "content-length",  // 79
    "access-control-request-headers",
    "content-type",  // 80
    "access-control-request-method",
    "get",  // 81
    "access-control-request-method",
    "post",  // 82
    "alt-svc",
    "clear",  // 83
    "authorization",
    "",  // 84
    "content-security-policy",
    "script-src 'none'; object-src 'none'; base-uri 'none'",  // 85
    "early-data",
    "1",  // 86
    "expect-ct",
    "",  // 87
    "forwarded",
    "",  // 88
    "if-range",
    "",  // 89
    "origin",
    "",  // 90
    "purpose",
    "prefetch",  // 91
    "server",
    "",  // 92
    "timing-allow-origin",
    "*",  // 93
    "upgrade-insecure-requests",
    "1",  // 94
    "user-agent",
    "",  // 95
    "x-forwarded-for",
    "",  // 96
    "x-frame-options",
    "deny",  // 97
    "x-frame-options",
    "sameorigin",  // 98
};

//*******************************************************************************
// Complete HPACK Huffman table (RFC 7541 Appendix B), sorted by code
const HuffmanCode HUFFMAN_CODES[] = {
    // 5-bit codes (0x00-0x09)
    {0x00, 5, '0'},
    {0x01, 5, '1'},
    {0x02, 5, '2'},
    {0x03, 5, 'a'},
    {0x04, 5, 'c'},
    {0x05, 5, 'e'},
    {0x06, 5, 'i'},
    {0x07, 5, 'o'},
    {0x08, 5, 's'},
    {0x09, 5, 't'},
    // 6-bit codes (0x14-0x2d)
    {0x14, 6, ' '},
    {0x15, 6, '%'},
    {0x16, 6, '-'},
    {0x17, 6, '.'},
    {0x18, 6, '/'},
    {0x19, 6, '3'},
    {0x1a, 6, '4'},
    {0x1b, 6, '5'},
    {0x1c, 6, '6'},
    {0x1d, 6, '7'},
    {0x1e, 6, '8'},
    {0x1f, 6, '9'},
    {0x20, 6, '='},
    {0x21, 6, 'A'},
    {0x22, 6, '_'},
    {0x23, 6, 'b'},
    {0x24, 6, 'd'},
    {0x25, 6, 'f'},
    {0x26, 6, 'g'},
    {0x27, 6, 'h'},
    {0x28, 6, 'l'},
    {0x29, 6, 'm'},
    {0x2a, 6, 'n'},
    {0x2b, 6, 'p'},
    {0x2c, 6, 'r'},
    {0x2d, 6, 'u'},
    // 7-bit codes (0x5c-0x7b)
    {0x5c, 7, ':'},
    {0x5d, 7, 'B'},
    {0x5e, 7, 'C'},
    {0x5f, 7, 'D'},
    {0x60, 7, 'E'},
    {0x61, 7, 'F'},
    {0x62, 7, 'G'},
    {0x63, 7, 'H'},
    {0x64, 7, 'I'},
    {0x65, 7, 'J'},
    {0x66, 7, 'K'},
    {0x67, 7, 'L'},
    {0x68, 7, 'M'},
    {0x69, 7, 'N'},
    {0x6a, 7, 'O'},
    {0x6b, 7, 'P'},
    {0x6c, 7, 'Q'},
    {0x6d, 7, 'R'},
    {0x6e, 7, 'S'},
    {0x6f, 7, 'T'},
    {0x70, 7, 'U'},
    {0x71, 7, 'V'},
    {0x72, 7, 'W'},
    {0x73, 7, 'Y'},
    {0x74, 7, 'j'},
    {0x75, 7, 'k'},
    {0x76, 7, 'q'},
    {0x77, 7, 'v'},
    {0x78, 7, 'w'},
    {0x79, 7, 'x'},
    {0x7a, 7, 'y'},
    {0x7b, 7, 'z'},
    // 8-bit codes (0xf8-0xff)
    {0xf8, 8, '&'},
    {0xf9, 8, '*'},
    {0xfa, 8, ','},
    {0xfb, 8, ';'},
    {0xfc, 8, 'X'},
    {0xfd, 8, 'Z'},
    // 10-bit codes
    {0x3f8, 10, '!'},
    {0x3f9, 10, '"'},
    {0x3fa, 10, '('},
    {0x3fb, 10, ')'},
    {0x3fc, 10, '?'},
    // 11-bit codes
    {0x7fa, 11, '\''},
    {0x7fb, 11, '+'},
    {0x7fc, 11, '|'},
    // 12-bit codes
    {0xffa, 12, '#'},
    {0xffb, 12, '>'},
    // 13-bit codes
    {0x1ff8, 13, '\x00'},
    {0x1ff9, 13, '$'},
    {0x1ffa, 13, '@'},
    {0x1ffb, 13, '['},
    {0x1ffc, 13, ']'},
    {0x1ffd, 13, '~'},
    // 14-bit codes
    {0x3ffc, 14, '^'},
    {0x3ffd, 14, '}'},
    // 15-bit codes
    {0x7ffc, 15, '<'},
    {0x7ffd, 15, '`'},
    {0x7ffe, 15, '{'},
    // 19-bit codes
    {0x7fff0, 19, '\\'},
    // 20-bit codes
    {0xfffe6, 20, 0xc3},
    {0xfffe7, 20, 0xd0},
    {0xfffe8, 20, 0x80},
    {0xfffe9, 20, 0x82},
    {0xfffea, 20, 0x83},
    {0xfffeb, 20, 0xa2},
    {0xfffec, 20, 0xb8},
    {0xfffed, 20, 0xc2},
    {0xfffee, 20, 0xe0},
    {0xfffef, 20, 0xe2},
    // 21-bit codes
    {0x1fffdc, 21, 0x99},
    {0x1fffdd, 21, 0xa1},
    {0x1fffde, 21, 0xa7},
    {0x1fffdf, 21, 0xac},
    {0x1fffe0, 21, 0xb0},
    {0x1fffe1, 21, 0xb1},
    {0x1fffe2, 21, 0xb3},
    {0x1fffe3, 21, 0xd1},
    {0x1fffe4, 21, 0xd8},
    {0x1fffe5, 21, 0xd9},
    {0x1fffe6, 21, 0xe3},
    {0x1fffe7, 21, 0xe5},
    {0x1fffe8, 21, 0xe6},
    // 22-bit codes
    {0x3fffd2, 22, 0x81},
    {0x3fffd3, 22, 0x84},
    {0x3fffd4, 22, 0x85},
    {0x3fffd5, 22, 0x86},
    {0x3fffd6, 22, 0x88},
    {0x3fffd7, 22, 0x92},
    {0x3fffd8, 22, 0x9a},
    {0x3fffd9, 22, 0x9c},
    {0x3fffda, 22, 0xa0},
    {0x3fffdb, 22, 0xa3},
    {0x3fffdc, 22, 0xa4},
    {0x3fffdd, 22, 0xa9},
    {0x3fffde, 22, 0xaa},
    {0x3fffdf, 22, 0xad},
    {0x3fffe0, 22, 0xb2},
    {0x3fffe1, 22, 0xb5},
    {0x3fffe2, 22, 0xb9},
    {0x3fffe3, 22, 0xba},
    {0x3fffe4, 22, 0xbb},
    {0x3fffe5, 22, 0xbd},
    {0x3fffe6, 22, 0xbe},
    {0x3fffe7, 22, 0xc4},
    {0x3fffe8, 22, 0xc6},
    {0x3fffe9, 22, 0xe4},
    {0x3fffea, 22, 0xe8},
    {0x3fffeb, 22, 0xe9},
    // 23-bit codes
    {0x7fffd8, 23, 0x01},
    {0x7fffd9, 23, 0x87},
    {0x7fffda, 23, 0x89},
    {0x7fffdb, 23, 0x8a},
    {0x7fffdc, 23, 0x8b},
    {0x7fffdd, 23, 0x8c},
    {0x7fffde, 23, 0x8d},
    {0x7fffdf, 23, 0x8f},
    {0x7fffe0, 23, 0x93},
    {0x7fffe1, 23, 0x95},
    {0x7fffe2, 23, 0x96},
    {0x7fffe3, 23, 0x97},
    {0x7fffe4, 23, 0x98},
    {0x7fffe5, 23, 0x9b},
    {0x7fffe6, 23, 0x9d},
    {0x7fffe7, 23, 0x9e},
    {0x7fffe8, 23, 0xa5},
    {0x7fffe9, 23, 0xa6},
    {0x7fffea, 23, 0xa8},
    {0x7fffeb, 23, 0xae},
    {0x7fffec, 23, 0xaf},
    {0x7fffed, 23, 0xb4},
    {0x7fffee, 23, 0xb6},
    {0x7fffef, 23, 0xb7},
    {0x7ffff0, 23, 0xbc},
    {0x7ffff1, 23, 0xbf},
    {0x7ffff2, 23, 0xc5},
    {0x7ffff3, 23, 0xe7},
    {0x7ffff4, 23, 0xef},
    // 24-bit codes
    {0xffffea, 24, 0x09},
    {0xffffeb, 24, 0x8e},
    {0xffffec, 24, 0x90},
    {0xffffed, 24, 0x91},
    {0xffffee, 24, 0x94},
    {0xffffef, 24, 0x9f},
    {0xfffff0, 24, 0xab},
    {0xfffff1, 24, 0xce},
    {0xfffff2, 24, 0xd7},
    {0xfffff3, 24, 0xe1},
    {0xfffff4, 24, 0xec},
    {0xfffff5, 24, 0xed},
    // 25-bit codes
    {0x1ffffec, 25, 0xc7},
    {0x1ffffed, 25, 0xcf},
    {0x1ffffee, 25, 0xea},
    {0x1ffffef, 25, 0xeb},
    // 26-bit codes
    {0x3ffffe0, 26, 0xc0},
    {0x3ffffe1, 26, 0xc1},
    {0x3ffffe2, 26, 0xc8},
    {0x3ffffe3, 26, 0xc9},
    {0x3ffffe4, 26, 0xca},
    {0x3ffffe5, 26, 0xcd},
    {0x3ffffe6, 26, 0xd2},
    {0x3ffffe7, 26, 0xd5},
    {0x3ffffe8, 26, 0xda},
    {0x3ffffe9, 26, 0xdb},
    {0x3ffffea, 26, 0xee},
    {0x3ffffeb, 26, 0xf0},
    {0x3ffffec, 26, 0xf2},
    {0x3ffffed, 26, 0xf3},
    {0x3ffffee, 26, 0xff},
    // 27-bit codes
    {0x7ffffde, 27, 0xcb},
    {0x7ffffdf, 27, 0xcc},
    {0x7ffffe0, 27, 0xd3},
    {0x7ffffe1, 27, 0xd4},
    {0x7ffffe2, 27, 0xd6},
    {0x7ffffe3, 27, 0xdd},
    {0x7ffffe4, 27, 0xde},
    {0x7ffffe5, 27, 0xdf},
    {0x7ffffe6, 27, 0xf1},
    {0x7ffffe7, 27, 0xf4},
    {0x7ffffe8, 27, 0xf5},
    {0x7ffffe9, 27, 0xf6},
    {0x7ffffea, 27, 0xf7},
    {0x7ffffeb, 27, 0xf8},
    {0x7ffffec, 27, 0xfa},
    {0x7ffffed, 27, 0xfb},
    {0x7ffffee, 27, 0xfc},
    {0x7ffffef, 27, 0xfd},
    {0x7fffff0, 27, 0xfe},
    // 28-bit codes
    {0xfffffe2, 28, 0x02},
    {0xfffffe3, 28, 0x03},
    {0xfffffe4, 28, 0x04},
    {0xfffffe5, 28, 0x05},
    {0xfffffe6, 28, 0x06},
    {0xfffffe7, 28, 0x07},
    {0xfffffe8, 28, 0x08},
    {0xfffffe9, 28, 0x0b},
    {0xfffffea, 28, 0x0c},
    {0xfffffeb, 28, 0x0e},
    {0xfffffec, 28, 0x0f},
    {0xfffffed, 28, 0x10},
    {0xfffffee, 28, 0x11},
    {0xfffffef, 28, 0x12},
    {0xffffff0, 28, 0x13},
    {0xffffff1, 28, 0x14},
    {0xffffff2, 28, 0x15},
    {0xffffff3, 28, 0x17},
    {0xffffff4, 28, 0x18},
    {0xffffff5, 28, 0x19},
    {0xffffff6, 28, 0x1a},
    {0xffffff7, 28, 0x1b},
    {0xffffff8, 28, 0x1c},
    {0xffffff9, 28, 0x1d},
    {0xffffffa, 28, 0x1e},
    {0xffffffb, 28, 0x1f},
    {0xffffffc, 28, 0x7f},
    {0xffffffd, 28, 0xdc},
    {0xffffffe, 28, 0xf9},
    // Note: 30-bit EOS marker (0x3fffffff) is handled via padding check
};

const size_t HUFFMAN_CODES_SIZE = sizeof(HUFFMAN_CODES) / sizeof(HUFFMAN_CODES[0]);

//*******************************************************************************
int64_t readVarint(const uint8_t* data, size_t len, size_t& pos)
{
    if (pos >= len) {
        return -1;
    }

    uint8_t firstByte = data[pos];
    uint8_t prefix    = firstByte >> 6;    // Top 2 bits indicate length
    int64_t value     = firstByte & 0x3F;  // Bottom 6 bits are part of value

    size_t numBytes = 1 << prefix;  // 1, 2, 4, or 8 bytes

    if (pos + numBytes > len) {
        return -1;
    }

    pos++;  // Move past first byte

    for (size_t i = 1; i < numBytes; i++) {
        value = (value << 8) | data[pos++];
    }

    return value;
}

//*******************************************************************************
size_t encodeQuicVarint(uint64_t value, uint8_t* output)
{
    if (value <= 63) {
        output[0] = static_cast<uint8_t>(value);
        return 1;
    } else if (value <= 16383) {
        output[0] = static_cast<uint8_t>(0x40 | (value >> 8));
        output[1] = static_cast<uint8_t>(value & 0xFF);
        return 2;
    } else if (value <= 1073741823) {
        output[0] = static_cast<uint8_t>(0x80 | (value >> 24));
        output[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        output[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        output[3] = static_cast<uint8_t>(value & 0xFF);
        return 4;
    } else {
        output[0] = static_cast<uint8_t>(0xC0 | (value >> 56));
        output[1] = static_cast<uint8_t>((value >> 48) & 0xFF);
        output[2] = static_cast<uint8_t>((value >> 40) & 0xFF);
        output[3] = static_cast<uint8_t>((value >> 32) & 0xFF);
        output[4] = static_cast<uint8_t>((value >> 24) & 0xFF);
        output[5] = static_cast<uint8_t>((value >> 16) & 0xFF);
        output[6] = static_cast<uint8_t>((value >> 8) & 0xFF);
        output[7] = static_cast<uint8_t>(value & 0xFF);
        return 8;
    }
}

//*******************************************************************************
int64_t readQpackInt(const uint8_t* data, size_t len, size_t& pos, int prefixBits)
{
    if (pos >= len || prefixBits < 1 || prefixBits > 8) {
        return -1;
    }

    uint8_t mask  = (1 << prefixBits) - 1;
    int64_t value = data[pos++] & mask;

    if (value == mask) {
        int m = 0;
        uint8_t b;
        do {
            if (pos >= len) {
                return -1;
            }
            b = data[pos++];
            value += (b & 0x7F) * (1 << m);
            m += 7;
        } while ((b & 0x80) != 0);
    }

    return value;
}

//*******************************************************************************
bool parseHttp3Frame(const uint8_t* data, size_t len, const uint8_t*& payloadData,
                     size_t& payloadLen)
{
    size_t pos = 0;

    while (pos < len) {
        int64_t frameType = readVarint(data, len, pos);
        if (frameType < 0) {
            cerr << "Http3Protocol: Failed to read frame type at pos " << pos << endl;
            return false;
        }

        int64_t frameLength = readVarint(data, len, pos);
        if (frameLength < 0) {
            cerr << "Http3Protocol: Failed to read frame length at pos " << pos << endl;
            return false;
        }

        if (pos + frameLength > len) {
            cerr << "Http3Protocol: Frame length exceeds buffer" << endl;
            return false;
        }

        if (frameType == FRAME_HEADERS) {
            payloadData = &data[pos];
            payloadLen  = static_cast<size_t>(frameLength);
            return true;
        }

        pos += static_cast<size_t>(frameLength);
    }

    cerr << "Http3Protocol: No HEADERS frame found in stream data" << endl;
    return false;
}

//*******************************************************************************
QString decodeHuffman(const uint8_t* data, size_t len)
{
    QString result;
    uint64_t accum = 0;
    int bits       = 0;

    size_t bytePos = 0;
    while (bytePos < len || bits >= 5) {
        while (bits < 32 && bytePos < len) {
            accum = (accum << 8) | data[bytePos++];
            bits += 8;
        }

        if (bits < 5)
            break;

        bool found = false;
        for (size_t i = 0; i < HUFFMAN_CODES_SIZE && !found; i++) {
            int codeBits = HUFFMAN_CODES[i].bits;
            if (codeBits > bits)
                continue;

            uint32_t code = static_cast<uint32_t>(accum >> (bits - codeBits));
            uint32_t mask = (1u << codeBits) - 1;
            code &= mask;

            if (code == HUFFMAN_CODES[i].code) {
                result += QChar(HUFFMAN_CODES[i].sym);
                bits -= codeBits;
                accum &= (1ULL << bits) - 1;
                found = true;
            }
        }

        if (!found) {
            if (bits <= 7) {
                uint32_t remaining  = static_cast<uint32_t>(accum & ((1ULL << bits) - 1));
                uint32_t eosPadding = (1u << bits) - 1;
                if (remaining == eosPadding) {
                    break;
                }
            }
            bits--;
            if (bits > 0) {
                accum &= (1ULL << bits) - 1;
            }
        }
    }

    return result;
}

//*******************************************************************************
bool decodeQPackHeaders(const uint8_t* data, size_t len, QMap<QString, QString>& headers)
{
    if (len < 2) {
        return false;
    }

    size_t pos = 0;

    int64_t requiredInsertCount = readQpackInt(data, len, pos, 8);
    if (requiredInsertCount < 0) {
        cerr << "Http3Protocol: Failed to read Required Insert Count" << endl;
        return false;
    }

    if (pos >= len) {
        cerr << "Http3Protocol: Buffer too short for Delta Base" << endl;
        return false;
    }

    int64_t deltaBase = readQpackInt(data, len, pos, 7);
    if (deltaBase < 0) {
        cerr << "Http3Protocol: Failed to read Delta Base" << endl;
        return false;
    }

    while (pos < len) {
        uint8_t byte = data[pos];

        // Indexed Header Field (pattern: 1T)
        if (byte & 0x80) {
            bool isStatic = (byte & 0x40) != 0;
            int64_t index = readQpackInt(data, len, pos, 6);

            if (index < 0) {
                cerr << "    Failed to read index" << endl;
                return false;
            }

            if (isStatic
                && static_cast<size_t>(index) * 2 + 1
                       < sizeof(QPACK_STATIC_TABLE) / sizeof(char*)) {
                const char* name  = QPACK_STATIC_TABLE[index * 2];
                const char* value = QPACK_STATIC_TABLE[index * 2 + 1];
                if (name && value) {
                    headers[QString::fromUtf8(name)] = QString::fromUtf8(value);
                }
            }
        }
        // Literal Header Field with Name Reference (pattern: 01NT)
        else if (byte & 0x40) {
            bool isStatic     = (byte & 0x10) != 0;
            int64_t nameIndex = readQpackInt(data, len, pos, 4);

            if (nameIndex < 0) {
                cerr << "    Failed to read name index" << endl;
                return false;
            }

            QString name;
            if (isStatic
                && static_cast<size_t>(nameIndex) * 2
                       < sizeof(QPACK_STATIC_TABLE) / sizeof(char*)) {
                const char* nameStr = QPACK_STATIC_TABLE[nameIndex * 2];
                if (nameStr) {
                    name = QString::fromUtf8(nameStr);
                }
            }

            if (pos >= len) {
                cerr << "    Buffer too short for value" << endl;
                return false;
            }

            bool huffman     = (data[pos] & 0x80) != 0;
            int64_t valueLen = readQpackInt(data, len, pos, 7);

            if (valueLen < 0) {
                cerr << "    Failed to read value length" << endl;
                return false;
            }

            QString value;
            if (huffman) {
                if (pos + valueLen > len) {
                    cerr << "    Huffman value length exceeds buffer" << endl;
                    return false;
                }
                value = decodeHuffman(&data[pos], static_cast<size_t>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else if (pos + valueLen <= len) {
                value = QString::fromUtf8(reinterpret_cast<const char*>(&data[pos]),
                                          static_cast<int>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else {
                cerr << "    Value length exceeds buffer" << endl;
                return false;
            }

            if (!name.isEmpty() && !value.isEmpty()) {
                headers[name] = value;
            }
        }
        // Literal Header Field with Literal Name (pattern: 001N)
        else if (byte & 0x20) {
            bool huffmanName = (data[pos] & 0x08) != 0;
            int64_t nameLen  = readQpackInt(data, len, pos, 3);

            if (nameLen < 0) {
                cerr << "    Failed to read name length" << endl;
                return false;
            }

            QString name;
            if (huffmanName) {
                if (pos + nameLen > len) {
                    cerr << "    Huffman name length exceeds buffer" << endl;
                    return false;
                }
                name = decodeHuffman(&data[pos], static_cast<size_t>(nameLen));
                pos += static_cast<size_t>(nameLen);
            } else if (pos + nameLen <= len) {
                name = QString::fromUtf8(reinterpret_cast<const char*>(&data[pos]),
                                         static_cast<int>(nameLen));
                pos += static_cast<size_t>(nameLen);
            } else {
                cerr << "    Name length exceeds buffer" << endl;
                return false;
            }

            if (pos >= len) {
                cerr << "    Buffer too short for value" << endl;
                return false;
            }

            bool huffmanValue = (data[pos] & 0x80) != 0;
            int64_t valueLen  = readQpackInt(data, len, pos, 7);

            if (valueLen < 0) {
                cerr << "    Failed to read value length" << endl;
                return false;
            }

            QString value;
            if (huffmanValue) {
                if (pos + valueLen > len) {
                    cerr << "    Huffman value length exceeds buffer" << endl;
                    return false;
                }
                value = decodeHuffman(&data[pos], static_cast<size_t>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else if (pos + valueLen <= len) {
                value = QString::fromUtf8(reinterpret_cast<const char*>(&data[pos]),
                                          static_cast<int>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else {
                cerr << "    Value length exceeds buffer" << endl;
                return false;
            }

            if (!name.isEmpty()) {
                headers[name] = value;
            }
        } else {
            pos++;
        }
    }

    return !headers.isEmpty();
}

}  // namespace Http3
