/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2016 PhatWare (r) Corp. All rights reserved.
 *
 *  Licensing and other inquires: <developer@phatware.com>
 *  Developer: Stan Miasnikov, et al. (c) PhatWare Corp. <http://www.phatware.com>
 *
 *  WRITEPAD HWRE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 *  AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 *  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 *  FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL PHATWARE CORP.
 *  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
 *  INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS
 *  OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT PHATWARE CORP.
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 *  POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WritePad.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************************/

#import "GTLBase64.h"

// Based on Cyrus Najmabadi's elegent little encoder and decoder from
// http://www.cocoadev.com/index.pl?BaseSixtyFour

static char gStandardEncodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char gWebSafeEncodingTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

#pragma mark Encode

static NSString *EncodeBase64StringCommon(NSData *data, const char *table)
{
    if (data == nil) return nil;

    const uint8_t* input = [data bytes];
    NSUInteger length = [data length];

    NSUInteger bufferSize = ((length + 2) / 3) * 4;
    NSMutableData* buffer = [NSMutableData dataWithLength:bufferSize];

    int8_t *output = [buffer mutableBytes];

    for (NSUInteger i = 0; i < length; i += 3) {
        NSUInteger value = 0;
        for (NSUInteger j = i; j < (i + 3); j++) {
            value <<= 8;

            if (j < length) {
                value |= (0xFF & input[j]);
            }
        }

        NSInteger idx = (i / 3) * 4;
        output[idx + 0] =                    table[(value >> 18) & 0x3F];
        output[idx + 1] =                    table[(value >> 12) & 0x3F];
        output[idx + 2] = (i + 1) < length ? table[(value >> 6)  & 0x3F] : '=';
        output[idx + 3] = (i + 2) < length ? table[(value >> 0)  & 0x3F] : '=';
    }

    NSString *result = [[NSString alloc] initWithData:buffer
                                              encoding:NSASCIIStringEncoding];
    return result;
}

NSString *GTLEncodeBase64(NSData *data)
{
    return EncodeBase64StringCommon(data, gStandardEncodingTable);
}

NSString *GTLEncodeWebSafeBase64(NSData *data)
{
    return EncodeBase64StringCommon(data, gWebSafeEncodingTable);
}

#pragma mark Decode

static void CreateDecodingTable(const char *encodingTable,
                                size_t encodingTableSize, char *decodingTable) {
    memset(decodingTable, 0, 128);
    for (unsigned int i = 0; i < encodingTableSize; i++) {
        decodingTable[(unsigned int) encodingTable[i]] = (char)i;
    }
}

static NSData *DecodeBase64StringCommon(NSString *base64Str,
                                        char *decodingTable) {
    // The input string should be plain ASCII
    const char *cString = [base64Str cStringUsingEncoding:NSASCIIStringEncoding];
    if (cString == nil) return nil;

    NSInteger inputLength = (NSInteger)strlen(cString);
    if (inputLength % 4 != 0) return nil;
    if (inputLength == 0) return [NSData data];

    while (inputLength > 0 && cString[inputLength - 1] == '=') {
        inputLength--;
    }

    NSInteger outputLength = inputLength * 3 / 4;
    NSMutableData* data = [NSMutableData dataWithLength:(NSUInteger)outputLength];
    uint8_t *output = [data mutableBytes];

    NSInteger inputPoint = 0;
    NSInteger outputPoint = 0;
    char *table = decodingTable;

    while (inputPoint < inputLength) {
        int i0 = cString[inputPoint++];
        int i1 = cString[inputPoint++];
        int i2 = inputPoint < inputLength ? cString[inputPoint++] : 'A'; // 'A' will decode to \0
        int i3 = inputPoint < inputLength ? cString[inputPoint++] : 'A';

        output[outputPoint++] = (uint8_t)((table[i0] << 2) | (table[i1] >> 4));
        if (outputPoint < outputLength) {
            output[outputPoint++] = (uint8_t)(((table[i1] & 0xF) << 4) | (table[i2] >> 2));
        }
        if (outputPoint < outputLength) {
            output[outputPoint++] = (uint8_t)(((table[i2] & 0x3) << 6) | table[i3]);
        }
    }

    return data;
}

NSData *GTLDecodeBase64(NSString *base64Str) {
    static char decodingTable[128];
    static BOOL hasInited = NO;

    if (!hasInited) {
        CreateDecodingTable(gStandardEncodingTable, sizeof(gStandardEncodingTable),
                            decodingTable);
        hasInited = YES;
    }
    return DecodeBase64StringCommon(base64Str, decodingTable);
}

NSData *GTLDecodeWebSafeBase64(NSString *base64Str) {
    static char decodingTable[128];
    static BOOL hasInited = NO;
    
    if (!hasInited) {
        CreateDecodingTable(gWebSafeEncodingTable, sizeof(gWebSafeEncodingTable),
                            decodingTable);
        hasInited = YES;
    }
    return DecodeBase64StringCommon(base64Str, decodingTable);
}
