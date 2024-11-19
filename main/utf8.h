// The MIT License (MIT)

// Copyright (c) 2021 Pierre Pharel Angoro Abiaga

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef UTF8_DECODER_H
#define UTF8_DECODER_H

#include <stdbool.h>

#if defined(_MSC_VER)
typedef __int32 utf8_int32_t;
#pragma warning(disable : 4309)
#else
#include <stdint.h>
typedef int32_t utf8_int32_t;
#endif

#if defined(UTF8_DECODER_LOG)
#include "logger.h"
#endif

typedef uint8_t utf8chr_t;

#define LATIN_EXTRA_BEGIN 0xc0           // 0b110xxxxx
#define BASIC_MULTILINGUAL_BEGIN 0xe0    // 0b1110xxxx
#define OTHERS_PLANES_UNICODE_BEGIN 0xf0 // 0b11110xxxx
#define SECONDARY_CHAR_BEGIN 0x80        // 0b10xxxxxx

#define END ('\0')
#define UTF8_BAD_CHAR false
#define UTF8_GOOD_CHAR true

typedef uint32_t rune_t;

typedef enum
{
    US_ASCII,
    LatinExtra,
    BasicMultiLingual,
    OthersPlanesUnicode,
    OutRange
} Utf8Type;

// Return a char decimal value greater than or equal to zero and less than zero if char is out of range
// Convert hexadecimal char beetwen '0'-'F'('a'-'z' & 'A'-'Z') in decimal value
extern utf8chr_t hexchr_to_hex(const utf8chr_t hex_chr);

// Return a the Utf8Type defined above
// But you can get codepoint (decimal value) of hexadecimal string if you pass an int32_t pointer instead of NULL
// Convert hexadecimal string in a decimal value, and determinate what is the range of utf8 string
extern Utf8Type utf8type(const utf8chr_t *hex_str, int32_t *cdp);

// Convert hexadecimal utf8 string into usable utf8 string and store it in dest
// dest will be empty if hexadecimal string is invalid or out of range
// NB: min size of dest must be 2 and max 5 with null char
extern void utf8decode(const utf8chr_t *hex_str, utf8chr_t *dest);

// Return a bool (true, if valid and false otherwise)
// Check if str is a valid utf8 string
extern bool utf8valid(const utf8chr_t *str);

// Return codepoint greater than or equal to zero and less than zero if char is out of range or if string is NULL
// Convert utf8 string in codepoint (decimal value)
extern int32_t utf8codepoint(const char *str);

// Convert codepoint into usable utf8 string and store it in dest
// dest will be empty if hexadecimal string is invalid or out of range
// NB: min size of dest must be 2 and max 5 with null char
extern void utf8chr(const int32_t codepoint, utf8chr_t *dest);

/* void assert_encode(const uint32_t *chars, int charcnt, const char *expected);

int utf8_encode(const uint32_t *chars, int charcnt, char *buffer);

int utf8_verify(const char *buffer); */

size_t utf8_encode(char *buffer, rune_t rune);
int utf8_is_valid(const char *data, size_t size);

int to_utf8(unsigned short code_point, unsigned char utf8_bytes[]);

#include <stdint.h>
#include <stddef.h>
#include <wchar.h>

typedef uint8_t utf8_t;   // The type of a single UTF-8 character
typedef uint16_t utf16_t; // The type of a single UTF-16 character

/*
 * Converts a UTF-16 string to a UTF-8 string.
 *
 * utf16:
 * The UTF-16 string, not null-terminated.
 *
 * utf16_len:
 * The length of the UTF-16 string, in 16-bit characters.
 *
 * utf8:
 * The buffer where the resulting UTF-8 string will be stored.
 * If set to NULL, indicates that the function should just calculate
 * the required buffer size and not actually perform any conversions.
 *
 * utf8_len:
 * The length of the UTF-8 buffer, in 8-bit characters.
 * Ignored if utf8 is NULL.
 *
 * return:
 * If utf8 is NULL, the size of the required UTF-8 buffer.
 * Otherwise, the number of characters written to the utf8 buffer.
 *
 */

typedef uint8_t utf8_char;
//2 Byte size variable (UTF-16 Chars)
typedef uint16_t utf16_char;
//4 Byte size variable (TEMP Conversion)
typedef uint32_t utf32_char;

//Pointers of UTF-8 chars (UTF8-String)
typedef utf8_char *utf8_string;
//Pointers of UTF-16 chars (UTF16-String)
typedef utf16_char *utf16_string;

size_t utf16_to_utf8(
    utf16_t const *utf16, size_t utf16_len,
    utf8_t *utf8, size_t utf8_len);

unsigned char *utf8_encode_string(wchar_t *string, int *length);
static unsigned int utf8_wstrlen(wchar_t *s);
utf16_string utf8_to_utf16(utf8_string chars);
size_t utf8_string_length(utf8_string s);
/*
 * Converts a UTF-8 string to a UTF-16 string.
 *
 * utf8:
 * The UTF-8 string, not null-terminated.
 *
 * utf8_len:
 * The length of the UTF-8 string, in 8-bit characters.
 *
 * utf16:
 * The buffer where the resulting UTF-16 string will be stored.
 * If set to NULL, indicates that the function should just calculate
 * the required buffer size and not actually perform any conversions.
 *
 * utf16_len:
 * The length of the UTF-16 buffer, in 16-bit characters.
 * Ignored if utf16 is NULL.
 *
 * return:
 * If utf16 is NULL, the size of the required UTF-16 buffer,
 * in 16-bit characters.
 * Otherwise, the number of characters written to the utf8 buffer, in
 * 16-bit characters.
 *
 */


#endif