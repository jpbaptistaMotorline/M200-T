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

#include <stddef.h>

#include "utf8.h"
#include "logger.h"


Utf8Type utf8type(const utf8chr_t* hex_str, int32_t* cdp)
{
    int32_t codepoint = 0;
    short shift = 0;

    for (const utf8chr_t* s = hex_str; *s != END; ++ s)
    {
        codepoint = ((codepoint << shift) | hexchr_to_hex(*s));
        shift = 4;
    }

    if (cdp != NULL)
        *cdp = codepoint;

    if (codepoint >= 0x0000 && codepoint <= 0x007f)
        return US_ASCII;
    else if (codepoint > 0x007f && codepoint <= 0x07ff)
        return LatinExtra;
    else if (codepoint > 0x07ff && codepoint <= 0xffff)
        return BasicMultiLingual;
    else if (codepoint > 0xffff && codepoint <= 0x10ffff)
        return OthersPlanesUnicode;

    return OutRange;
}

int to_utf8(unsigned short code_point, unsigned char utf8_bytes[]) {
    int num_bytes = 1;
    
    //One byte needed
    if (code_point < 0x0080){
        num_bytes = 1;
        unsigned char fill = code_point & ((1<<7)-1); //Accessing the last 7 bits of the code point
       
        utf8_bytes[0] = fill;
        
    }
    //Two bytes needed
    else if (code_point < 0x07ff){
        num_bytes = 2;
        unsigned short unit = 1;
        
       
        unsigned char fill_1 = code_point >> 6;
        
        unsigned char mask = 6 << 5;
        fill_1 = mask | fill_1;
        utf8_bytes[0] = fill_1;
        
        unsigned char fill_2 = (code_point & ((unit<<6)-1));
        unsigned char mask2 = 2 << 6;
        fill_2 = mask2 | fill_2;
        utf8_bytes[1] = fill_2;

        
    }
    //Three bytes needed
    else {
        num_bytes = 3;
        unsigned short unit = 1;
        
        unsigned char mask = 14 << 4;
        
        unsigned char fill_1 = (code_point >> 12);
        utf8_bytes[0] = mask | fill_1;

        unsigned char mask2 = 2 << 6;

        //Shifting the code_point to the right by six will give us the lasst 8 bits but we want to keep only the last 6 the same and the leading two bits as 0. This can be done by using the & operator with 0b00111111 = 63.
        unsigned char fill_2 = (code_point >> 6) & 63; 
        utf8_bytes[1] = mask2 | fill_2;

        unsigned char mask3 = 2 << 6;
        
        unsigned char fill_3 = (code_point & ((unit<<6)-1));
        utf8_bytes[2] = mask3 | fill_3;
        
        
    }
    

    return num_bytes;
}

size_t utf8_encode(char* buffer, rune_t rune)
{
	// 1 byte ASCII.
	if (rune <= 0x7F)
	{	
		if (buffer != NULL)
			*buffer = (char)rune;

		return 1;
	}

	// 2 byte.
	if (rune >= 0x80 && rune <= 0x7FF)
	{
		if (buffer != NULL)
		{
			buffer[0] = (char)(0xC0 | ((rune >> 6) & 0x1F));
			buffer[1] = (char)(0x80 | (rune & 0x3F));
		}

		return 2;
	}

	// 3 byte.
	if (rune >= 0x800 && rune <= 0xFFFF)
	{
		if (buffer != NULL)
		{
			buffer[0] = (char)(0xE0 | ((rune >> 12) & 0x0F));
			buffer[1] = (char)(0x80 | ((rune >> 6) & 0x3F));
			buffer[2] = (char)(0x80 | (rune & 0x3F));
		}

		return 3;
	}

	// 4 byte.
	if (rune >= 0x10000 && rune <= 0x10FFFF)
	{
		if (buffer != NULL)
		{
			buffer[0] = (char)(0xF0 | ((rune >> 18) & 0x07));
			buffer[1] = (char)(0x80 | ((rune >> 12) & 0x3F));
			buffer[2] = (char)(0x80 | ((rune >> 6) & 0x3F));
			buffer[3] = (char)(0x80 | (rune & 0x3F));
		}

		return 4;
	}

	return 0;
}

int utf8_is_valid(const char* data, size_t size)
{
	for (size_t i = 0; i < size;) 
	{
		ssize_t s=0;// = utf8_decode(&data[i], size - i, NULL);
		if (s == 0)
			return 0;

		i += s;
	}

	return 1;
}

// void assert_encode(const uint32_t *chars, int charcnt, const char *expected)
// {
//     // conservatively taking 4 bytes per char + null byte
//     char *buffer = malloc(charcnt * 4 + 1);
//     int valid = 0;

//     utf8_encode(chars, charcnt, buffer);


    
//     valid = utf8_verify(buffer);

//     /* if (strcmp(buffer, expected) != 0) {
//         //////printf("FAILED: %s != %s\n", expected, buffer);
//     } else {
//         //////printf("PASSED: %s == %s\n", expected, buffer);
//     } */

//     if (valid != 0) {
//         //////printf("Buffer contains invalid bytes: %s\n", buffer);
//     }

//     free(buffer);
// }

// int utf8_verify(const char *buffer)
// {
//     unsigned char prev, cur;

//     prev = *buffer;
//     buffer++;
//     cur = *buffer;

//     while (cur != '\0') {
//         // 0xd800 <= byte <= 0xdfff
//         if ((0xd8 <= prev) && (prev <= 0xdf)) {
//             return -1;
//         }

//         // 0xfffe <= byte <= 0xffff
//         if ((prev == 0xff) && ((0xfe <= cur) && (cur <= 0xff))) {
//             return -1;
//         }

//         prev = cur;
//         buffer++;
//         cur = *buffer;
//     }

//     return 0;
// }

// int utf8_encode(const uint32_t *chars, int charcnt, char *buffer)
// {
//     int i;
//     int cnt = 0;
//     uint32_t ch;

//     for (i = 0; i < charcnt; i++) {
//         ch = chars[i];

//         if (ch <= 0x7f) {
//             buffer[cnt++] = ch;

//         } else if (ch <= 0x7ff) {
//             buffer[cnt++] = 0xc0 | (ch >> 6);
//             buffer[cnt++] = 0x80 | (ch & 0x3f);

//         } else if (ch <= 0xffff) {
//             buffer[cnt++] = 0xe0 | (ch >> 12);
//             buffer[cnt++] = 0x80 | ((ch >> 6) & 0x3f);
//             buffer[cnt++] = 0x80 | (ch & 0x3f);

//         } else if (ch <= 0x1fffff) {
//             buffer[cnt++] = 0xf0 | (ch >> 18);
//             buffer[cnt++] = 0x80 | ((ch >> 12) & 0x3f);
//             buffer[cnt++] = 0x80 | ((ch >>  6) & 0x3f);
//             buffer[cnt++] = 0x80 | (ch & 0x3f);

//         }
//     }

//      //////printf("\n\nBuffer encode %s\n\n", buffer);
//     buffer[cnt++] = '\0';
 
//     return cnt;
// }


utf8chr_t hexchr_to_hex(const utf8chr_t hex_chr)
{
    if ('0' <= hex_chr && hex_chr <= '9')
        return hex_chr - 48;
    if ('A' <= hex_chr && hex_chr <= 'F')
        return hex_chr - 55;
    if ('a' <= hex_chr && hex_chr <= 'f')
        return hex_chr - 87;
    return -1;
}

void utf8decode(const utf8chr_t* hex_str, utf8chr_t* dest)
{
    int32_t cdp = 0;
    Utf8Type type = utf8type(hex_str, &cdp);
    // those are common to all the utf8 types
    utf8chr_t c1 = hexchr_to_hex(hex_str[1]);
    utf8chr_t c2 = hexchr_to_hex(hex_str[2]);
    utf8chr_t c3 = hexchr_to_hex(hex_str[3]);

    switch (type)
    {
        case US_ASCII:
        {
            int32_t c0 = 0;
            utf8type(hex_str, &c0);
            dest[0] = c0;
            dest[1] = END;

#if defined (UTF8_DECODER_LOG)
            Log(INFO, "%X", dest[0]);
#endif
            break;
        }

        case LatinExtra:
        {
            // first char
            dest[0] = LATIN_EXTRA_BEGIN;
            dest[0] |= ((c1 & 0x7) << 2);
            dest[0] |= ((c2 & 0xc) >> 2);

            // second char
            dest[1] = SECONDARY_CHAR_BEGIN;
            dest[1] |= ((c2 & 0x3) << 4);
            dest[1] |= c3;

            // end char
            dest[2] = END;

#if defined (UTF8_DECODER_LOG)
            Log(INFO, "%X %X", dest[0], dest[1]);
#endif
            break;
        }

        case BasicMultiLingual:
        {
            utf8chr_t c0 = hexchr_to_hex(hex_str[0]);

            // first char
            dest[0] = BASIC_MULTILINGUAL_BEGIN;
            dest[0] |= c0;

            // second char
            dest[1] = SECONDARY_CHAR_BEGIN;
            dest[1] |= (c1 << 2);
            dest[1] |= ((c2 & 0xc) >> 2);

            // third char
            dest[2] = SECONDARY_CHAR_BEGIN;
            dest[2] |= ((c2 & 0x3) << 4);
            dest[2] |= c3;

            // end char
            dest[3] = END;

#if defined (UTF8_DECODER_LOG)
            Log(INFO, "%X %X %X", dest[0], dest[1], dest[2]);
#endif
            break;
        }

        case OthersPlanesUnicode:
        {
            utf8chr_t c0 = hexchr_to_hex(hex_str[0]);
            utf8chr_t c4 = hexchr_to_hex(hex_str[4]);

            if (cdp <= 0xfffff)
            {
                // first char
                dest[0] = OTHERS_PLANES_UNICODE_BEGIN;
                dest[0] |= ((c0 & 0xc) >> 2);

                // second char
                dest[1] = SECONDARY_CHAR_BEGIN;
                dest[1] |= ((c0 & 0x3) << 4);
                dest[1] |= c1;

                // third char
                dest[2] = SECONDARY_CHAR_BEGIN;
                dest[2] |= (c2 << 2);
                dest[2] |= ((c3 & 0xc) >> 2);

                // fourth char
                dest[3] = SECONDARY_CHAR_BEGIN;
                dest[3] |= ((c3 & 0x3) << 4);
                dest[3] |= c4;

                // end char
                dest[4] = END;

//#if defined (UTF8_DECODER_LOG)
                //Log(INFO, "%X %X %X %X", dest[0], dest[1], dest[2],  dest[3]);
//#endif
            }
            else
            {
                utf8chr_t c5 = hexchr_to_hex(hex_str[5]);

                // first char
                dest[0] = OTHERS_PLANES_UNICODE_BEGIN;
                dest[0] |= ((c0 & 0x1) << 2);
                dest[0] |= ((c1 & 0xc) >> 2);

                // second char
                dest[1] = SECONDARY_CHAR_BEGIN;
                dest[1] |= ((c1 & 0x3) << 4);
                dest[1] |= ((c1 & 0xc) >> 2);
                dest[1] |= c2;

                // third char
                dest[2] = SECONDARY_CHAR_BEGIN;
                dest[2] |= (c3 << 2);
                dest[2] |= ((c4 & 0xc) >> 2);

                // fourth char
                dest[3] = SECONDARY_CHAR_BEGIN;
                dest[3] |= ((c4 & 0x3) << 4);
                dest[3] |= c5;

                // end char
                dest[4] = END;

#if defined (UTF8_DECODER_LOG)
                Log(INFO, "%X %X %X %X", dest[0], dest[1], dest[2],  dest[3]);
#endif
            }

            break;
        }

        case OutRange:
            *dest = END;
#if defined (UTF8_DECODER_LOG)
            Log(WARNING, "String is empty, we are out of utf8 range !");
#endif
            break;
    }


}

bool utf8valid(const utf8chr_t *str)
{
    const utf8chr_t* s = str;

    if (str == NULL)
    {
#if defined (UTF8_DECODER_LOG)
        Log(WARNING, "Null string");
#endif
        return UTF8_BAD_CHAR;
    }

    while (*s != END)
    {
        if (0xf0 == (0xf8 & *s))
        {
            // ensure each of the 3 following bytes in this 4 byte
            // utf8 codepoint began with 0b10xxxxxx
            if ((0x80 != (0xc0 & s[1])) || (0x80 != (0xc0 & s[2])) || (0x80 != (0xc0 & s[3])))
                return UTF8_BAD_CHAR;

            // ensure that our utf8 codepoint ended after 4 byte
            if (0x80 == (0xc0 & s[4]))
                return UTF8_BAD_CHAR;

            // ensure that the top 5 bits of this 4 byte utf8
            // codepoint were not 0, as then we could have used
            // one of the smaller encodings
            if ((0 == (0x07 & s[0])) && (0 == (0x30 & s[1])))
                return UTF8_BAD_CHAR;

            // 4-byte utf8 code point (began with 0b11110xxx)
            s += 4;
        }
        else if (0xe0 == (0xf0 & *s))
        {
            // ensure each of the 2 following bytes in this 3 byte
            // utf8 codepoint began with 0b10xxxxxx
            if ((0x80 != (0xc0 & s[1])) || (0x80 != (0xc0 & s[2])))
                return UTF8_BAD_CHAR;

            // ensure that our utf8 codepoint ended after 3 bytes
            if (0x80 == (0xc0 & s[3]))
                return UTF8_BAD_CHAR;

            // ensure that the top 5 bits of this 3 byte utf8
            // codepoint were not 0, as then we could have used
            // one of the smaller encodings
            if((0 == (0x0f & s[0])) && (0 == (0x20 & s[1])))
                return UTF8_BAD_CHAR;

            // 3-byte utf8 code point (began with 0b1110xxxx)
            s += 3;
        }
        else if (0xc0 == (0xe0 & *s))
        {
            // ensure the 1 following byte in this 2 byte
            // utf8 codepoint began with 0b10xxxxxx
            if (0x80 != (0xc0 & s[1]))
                return UTF8_BAD_CHAR;

            // ensure that our utf8 codepoint ended after 2 bytes
            if (0x80 == (0xc0 & s[2]))
                return UTF8_BAD_CHAR;

            // ensure that the top 4 bits of this 2 byte utf8
            // codepoint were not 0, as then we could have used
            // one of the smaller encodings
            if (0 == (0x1e & s[0]))
                return UTF8_BAD_CHAR;

            // 2-byte utf8 code point (began with 0b110xxxxx)
            s += 2;
        }
        else if (0x00 == (0x80 & *s))
        {
            // 1-byte ascii (began with 0b0xxxxxxx)
            s += 1;
        }
        else
        {
            // we have an invalid 0b1xxxxxxx utf8 code point entry
            return UTF8_BAD_CHAR;
        }
    }

    return UTF8_GOOD_CHAR;
}



int32_t utf8codepoint(const char* str)
{
    int32_t codepoint = 0;
    const utf8chr_t* s = &str;

    if (utf8valid(&str))
    {
        if (str == NULL)
        {
#if defined (UTF8_DECODER_LOG)
            Log(WARNING, "Null string");
#endif
            return -1;
        }

        while (*s != END)
        {
            if (0xf0 == (0xf8 & *s))
            {
                // four byte
                codepoint = ((0x07 & s[0]) << 18) | ((0x3f & s[1]) << 12) | ((0x3f & s[2]) << 6) | (0x3f & s[3]);
                s += 4;
            }
            else if (0xe0 == (0xf0 & *s))
            {
                // three byte
                codepoint = ((0x0f & s[0]) << 12) | ((0x3f & s[1]) << 6) | (0x3f & s[2]);
                s += 3;
            }
            else if (0xc0 == (0xe0 & *s))
            {
                // two byte
                codepoint = ((0x1f & s[0]) << 6) | (0x3f & s[1]);
                s += 2;
            }
            else if (0x00 == (0x80 & *s))
            {
                // one byte
                codepoint = s[0];
                ++ s;
            }
            else
            {
#if defined (UTF8_DECODER_LOG)
                Log(WARNING, "Invalid codepoint");
#endif
                return -1;
            }
        }
    }

    return codepoint;
}

void utf8chr(const int32_t codepoint, utf8chr_t* dest)
{
    if (codepoint >= 0x0000 && codepoint <= 0x007f)
    {
        dest[0] = codepoint;
        dest[1] = END;
#if defined (UTF8_DECODER_LOG)
        Log(INFO, "%X", dest[0]);
#endif
    }
    else if (codepoint > 0x007f && codepoint <= 0x07ff)
    {
        dest[0] = LATIN_EXTRA_BEGIN;
        dest[1] = SECONDARY_CHAR_BEGIN;

        if (codepoint > 0xff)
            dest[0] |= (codepoint >> 6);
        dest[0] |= ((codepoint & 0xc0) >> 6);
        dest[1] |= (codepoint & 0x3f);

        dest[2] = END;
#if defined (UTF8_DECODER_LOG)
        Log(INFO, "%X %X", dest[0], dest[1]);
#endif
    }
    else if (codepoint > 0x07ff && codepoint <= 0xffff)
    {
        dest[0] = BASIC_MULTILINGUAL_BEGIN;
        dest[1] = SECONDARY_CHAR_BEGIN;
        dest[2] = SECONDARY_CHAR_BEGIN;

        if (codepoint > 0xfff)
            dest[0] |= ((codepoint & 0xf000) >> 12);
        dest[1] |= ((codepoint & 0xf00) >> 6);
        dest[1] |= ((codepoint & 0xf0) >> 6);
        dest[2] |= (codepoint & 0x30);
        dest[2] |= (codepoint & 0xf);

        dest[3] = END;
#if defined (UTF8_DECODER_LOG)
        Log(INFO, "%X %X %X", dest[0], dest[1], dest[2]);
#endif
    }
    else if (codepoint > 0xffff && codepoint <= 0x10ffff)
    {
        dest[0] = OTHERS_PLANES_UNICODE_BEGIN;
        dest[1] = SECONDARY_CHAR_BEGIN;
        dest[2] = SECONDARY_CHAR_BEGIN;
        dest[3] = SECONDARY_CHAR_BEGIN;

        if (codepoint > 0xfffff)
            dest[0] |= ((codepoint & 0x100000) >> 18);
        dest[0] |= ((codepoint & 0xc0000) >> 18);
        dest[1] |= ((codepoint & 0x30000) >> 12);
        dest[1] |= ((codepoint & 0xf000) >> 12);
        dest[2] |= ((codepoint & 0xf00) >> 6);
        dest[2] |= ((codepoint & 0xc0) >> 6);
        dest[3] |= (codepoint & 0x30);
        dest[3] |= (codepoint & 0xf);

        dest[4] = END;
#if defined (UTF8_DECODER_LOG)
        Log(INFO, "%X %X %X %X", dest[0], dest[1], dest[2], dest[3]);
#endif
    }
    else
    {
        *dest = END;
#if defined (UTF8_DECODER_LOG)
        Log(WARNING, "String is empty, we are out of utf8 range !");
#endif
    }
}

static void utf8_encode_code(unsigned char **buffer, unsigned int code)
{
    if( code < 0x80 ){
            // 0xxxxxxx
            // 8 bits
        *(*buffer)++ = code;
    } else if( code < 0x800 ){
            // 110xxxxx 10xxxxxx
            // 2 bits
        *(*buffer)++ = ((code>>6) & 0x1f) | 0xc0;
        *(*buffer)++ = ( code     & 0x3f) | 0x80;
    } else if( code < 0x10000 ){
            // 1110xxxx 10xxxxxx 10xxxxxx
            // 3 bits
        *(*buffer)++ = ((code>>12) & 0x0f) | 0xe0;
        *(*buffer)++ = ((code>>6 ) & 0x3f) | 0x80;
        *(*buffer)++ = ( code      & 0x3f) | 0x80;
    } else if( code < 0x200000 ){
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            // 4 bits
        *(*buffer)++ = ((code>>18) & 0x07) | 0xf0;
        *(*buffer)++ = ((code>>12) & 0x3f) | 0x80;
        *(*buffer)++ = ((code>>6 ) & 0x3f) | 0x80;
        *(*buffer)++ = ( code      & 0x3f) | 0x80;
    } else if( code < 0x4000000 ){
            // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            // 5 bits
        *(*buffer)++ = ((code>>24) & 0x03) | 0xf8;
        *(*buffer)++ = ((code>>18) & 0x3f) | 0x80;
        *(*buffer)++ = ((code>>12) & 0x3f) | 0x80;
        *(*buffer)++ = ((code>>6 ) & 0x3f) | 0x80;
        *(*buffer)++ = ( code      & 0x3f) | 0x80;
    } else if( code < 0x80000000 ){
            // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            // 6 bits
        *(*buffer)++ = ((code>>30) & 0x01) | 0xfc;
        *(*buffer)++ = ((code>>24) & 0x3f) | 0x80;
        *(*buffer)++ = ((code>>18) & 0x3f) | 0x80;
        *(*buffer)++ = ((code>>12) & 0x3f) | 0x80;
        *(*buffer)++ = ((code>>6 ) & 0x3f) | 0x80;
        *(*buffer)++ = ( code      & 0x3f) | 0x80;
    }
}

size_t utf8_string_length(utf8_string s)
{
    size_t length = 0;
    char c;
    while ((c = *s++) != '\0')
    {
        if (c < 0 || c > 127)
        {
            if ((c & 0xE0) == 0xC0)
                s += 1;
            else if ((c & 0xF0) == 0xE0)
                s += 2;
            else if ((c & 0xF8) == 0xF0)
                s += 3;
        }

        length++;
    }
    return length;
}

utf16_string utf8_to_utf16(utf8_string chars)
{
    size_t length = utf8_string_length(chars);
    utf16_char *str = (utf16_char *)malloc(2 * (length + 1));
    str[length] = 0;

    size_t n = 0;
    size_t i = 0;
    
    while (true)
    {
        utf8_char ch = chars[i++];

        if (ch == '\0')
            break;

        if (ch >> 7 == 0)
        {
            str[n] = ch;
            n++;
            continue;
        }

        //ERROR CHECK:
        /*
        if (c >> 6 == 0x02)
        {
            continue;
        }
        */

        // Get byte length
        char extraChars = 0;
        if (ch >> 5 == 0x06)
        {
            extraChars = 1;
        }
        else if (ch >> 4 == 0x0e)
        {
            extraChars = 2;
        }
        else if (ch >> 3 == 0x1e)
        {
            extraChars = 3;
        }
        else if (ch >> 2 == 0x3e)
        {
            extraChars = 4;
        }
        else if (ch >> 1 == 0x7e)
        {
            extraChars = 5;
        }
        else
        {
            continue;
        }

        utf32_char mask = (1 << (8 - extraChars - 1)) - 1;
        utf32_char res = ch & mask;
        utf8_char nextChar;
        size_t count;

        for (count = 0; count < extraChars; count++)
        {
            nextChar = chars[i++];

            if (nextChar >> 6 != 0x02)
                break;

            res = (res << 6) | (nextChar & 0x3f);
        }

        if (count != extraChars)
        {
            i--;
            continue;
        }

        if (res <= 0xffff)
        {
            str[n] = res;
            n++;
            continue;
        }

        res -= 0x10000;

        utf16_char high = ((res >> 10) & 0x3ff) + 0xd800;
        utf16_char low = (res & 0x3ff) + 0xdc00;

        str[n] = high;
        str[n + 1] = low;
        n += 2;
    }

    str[n] = 0;

    return str;
}

static unsigned int utf8_encode_code_lenght(unsigned int code)
{
    if( code < 0x80 ){
            // 0xxxxxxx
            // 8 bits
        return 1;
    } else if( code < 0x800 ){
            // 110xxxxx 10xxxxxx
            // 2 bits
        return 2;
    } else if( code < 0x10000 ){
            // 1110xxxx 10xxxxxx 10xxxxxx
            // 3 bits
        return 3;
    } else if( code < 0x200000 ){
            // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            // 4 bits
        return 4;
    } else if( code < 0x4000000 ){
            // 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            // 5 bits
        return 5;
    } else if( code < 0x80000000 ){
            // 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
            // 6 bits
        return 6;
    } else {
            // don't handle this code
        return 0;
    }
}

static unsigned int utf8_wstrlen(wchar_t *s){
    unsigned int count = 0;
    if( s ){
        while(*s++) {
            ++count;
        }
    }
    return count;
}

unsigned char* utf8_encode_string(wchar_t *string, int *length)
{
    if( string ){
        int len = 0;
        if(length){
            if(*length < 0){
                len = utf8_wstrlen(string);
            } else {
                len = *length;
            }
        } else {
            len = utf8_wstrlen(string);
        }
        wchar_t *end = string + len;
        wchar_t *s = string;
        unsigned int buffer_length = 0;
        while( s<end ){
            buffer_length += utf8_encode_code_lenght(*s);
            ++s;
        }
        unsigned char *buffer = NULL;
        if( buffer_length ){
            buffer = malloc(sizeof(unsigned char)*buffer_length + 1);
        }
        if( buffer ){
            unsigned char *tmp_buffer = buffer;
            s = string;
            while( *s ){
                utf8_encode_code(&tmp_buffer, *s);
                ++s;
            }
            *tmp_buffer = 0;
            if( length ){
                *length = buffer_length;
            }
            return buffer;
        }
    }
    if( length ){
        *length = 0;
    }
    return NULL;
}
#undef LATIN_EXTRA_BEGIN
#undef BASIC_MULTILINGUAL_BEGIN
#undef OTHERS_PLANES_UNICODE_BEGIN
#undef SECONDARY_CHAR_BEGIN
#undef END
#undef UTF8_BAD_CHAR
#undef UTF8_GOOD_CHAR