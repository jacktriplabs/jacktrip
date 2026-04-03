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
 * \file Http3Protocol.h
 * \author Mike Dickey + Claude AI
 * \date 2026
 *
 * Reusable HTTP/3 protocol utilities: QUIC varints, QPACK decoding,
 * Huffman decoding, and HTTP/3 frame parsing.
 */

#ifndef __HTTP3PROTOCOL_H__
#define __HTTP3PROTOCOL_H__

#include <QMap>
#include <QString>
#include <cstddef>
#include <cstdint>

namespace Http3
{

// HTTP/3 frame types (RFC 9114)
enum FrameType {
    FRAME_DATA         = 0x00,
    FRAME_HEADERS      = 0x01,
    FRAME_CANCEL_PUSH  = 0x03,
    FRAME_SETTINGS     = 0x04,
    FRAME_PUSH_PROMISE = 0x05,
    FRAME_GOAWAY       = 0x07,
    FRAME_MAX_PUSH_ID  = 0x0D,
};

/** \brief HPACK/QPACK Huffman code entry (RFC 7541 Appendix B) */
struct HuffmanCode {
    uint32_t code;
    uint8_t bits;
    uint8_t sym;
};

/** \brief Read a QUIC variable-length integer (RFC 9000 §16)
 *
 * Returns the value and advances pos past the integer.
 * Returns -1 on error (insufficient data).
 */
int64_t readVarint(const uint8_t* data, size_t len, size_t& pos);

/** \brief Encode a QUIC variable-length integer
 *
 * Writes the encoded value into output (must have room for up to 8 bytes).
 * Returns the number of bytes written.
 */
size_t encodeQuicVarint(uint64_t value, uint8_t* output);

/** \brief Read a QPACK integer with specified prefix bit count (RFC 9204)
 *
 * prefixBits: number of bits in the first byte used for the value (1–8).
 * Returns the value and advances pos. Returns -1 on error.
 */
int64_t readQpackInt(const uint8_t* data, size_t len, size_t& pos, int prefixBits);

/** \brief Parse an HTTP/3 frame and extract the first HEADERS frame payload
 *
 * Returns true if a HEADERS frame was found.
 * Sets payloadData/payloadLen to point into data (no copy).
 */
bool parseHttp3Frame(const uint8_t* data, size_t len, const uint8_t*& payloadData,
                     size_t& payloadLen);

/** \brief Decode a Huffman-encoded string (RFC 7541 Appendix B)
 *
 * Returns the decoded QString, or an empty string on error.
 */
QString decodeHuffman(const uint8_t* data, size_t len);

/** \brief Decode QPACK-encoded HTTP/3 headers (RFC 9204)
 *
 * Minimal decoder sufficient for WebTransport CONNECT requests.
 * Handles static-table indexed fields and literal fields (with/without Huffman).
 * Returns true if at least one header was decoded.
 */
bool decodeQPackHeaders(const uint8_t* data, size_t len, QMap<QString, QString>& headers);

/** \brief QPACK static table (RFC 9204 Appendix A)
 *
 * Pairs of (name, value) strings; access as QPACK_STATIC_TABLE[index*2] (name)
 * and QPACK_STATIC_TABLE[index*2+1] (value).
 */
extern const char* QPACK_STATIC_TABLE[];

/** \brief Complete HPACK Huffman table (RFC 7541 Appendix B) */
extern const HuffmanCode HUFFMAN_CODES[];
extern const size_t HUFFMAN_CODES_SIZE;

}  // namespace Http3

#endif  // __HTTP3PROTOCOL_H__
