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

//
//  WordInk.m
//  json2tap
//
//  Created by Stanislav Miasnikov on 2/28/16.
//  Copyright © 2016 PhatWare Corp. All rights reserved.
//

#import "WordInk.h"
#import "RecognizerManager.h"
#import "LanguageManager.h"

@implementation WordInk


- (BOOL) testWordRecogntion:(NSString *)dictPath verbose:(BOOL)verbose singleWord:(BOOL)singleWord
{
    INK_DATA_PTR inkData = INK_InitData();
    void *	pInk = (void *)self.ink.bytes;
    long	cbData = self.ink.length;
    INK_EnableUndo( inkData, false );
    if ( INK_Serialize(inkData, FALSE, NULL, &pInk, &cbData, TRUE, FALSE) )
    {
        // recognize this ink object
        LanguageManager * langManager = [LanguageManager sharedManager];

        NSString * mainDictPath = nil;
        if ( dictPath != nil )
        {
            mainDictPath = [dictPath stringByAppendingPathComponent:[langManager mainDictionaryPath]];
        }
        RECOGNIZER_PTR recognizer = HWR_InitRecognizer( (mainDictPath == nil) ? NULL : [mainDictPath UTF8String],
                                                       NULL, NULL, NULL,
                                                       [langManager getLanguageID], NULL );
        if ( recognizer != NULL )
        {
            // set recognizer options
            unsigned int	flags = HWR_GetRecognitionFlags( recognizer );
            flags &= FLAG_MAINDICT;
            flags |= (FLAG_USERDICT | FLAG_NOSPACE | FLAG_SMOOTHSTROKES);
            if ( singleWord )
                flags |= FLAG_SINGLEWORDONLY;
            HWR_SetRecognitionFlags( recognizer, flags );
            HWR_SetDefaultShapes( recognizer );
            
            switch ( self.type )
            {
                case 1 :
                    HWR_SetRecognitionMode( recognizer, RECMODE_GENERAL );
                    break;
                    
                case 4 :
                    HWR_SetRecognitionMode( recognizer, RECMODE_CAPS );
                    break;
                    
                case 3 : 
                    HWR_SetRecognitionMode( recognizer, RECMODE_NUMBERSPURE );
                    break;
                    
                case 2 :
                {
                    NSString * punct = @"~`!@#$%^&*()_+:\'\"[]{}|\\<>,.?/;-=";
                    unichar buffP[200] = {0};
                    [punct getCharacters:buffP];
                    unichar buffN[1] = {0};
                    HWR_SetRecognitionMode( recognizer, RECMODE_NUM );
                    HWR_SetCustomCharset( recognizer, buffN, buffP);
                    break;
                }
            }

            HWR_NewUserDict( recognizer );
            
            NSString * testWord = @"";
            
            NSArray * subWords = [self.word componentsSeparatedByString:@" "];
            if ( [subWords count] > 1 )
            {
                for ( NSString * wrd in subWords )
                {
                    const UCHR * pWord = [RecognizerManager uchrFromString:wrd];
                    if ( NULL != pWord )
                    {
                        HWR_AddUserWordToDict( recognizer, pWord, NO );
                    }
                    testWord = [testWord stringByAppendingString:wrd];
                }
                const UCHR * pWord = [RecognizerManager uchrFromString:testWord];
                if ( NULL != pWord )
                {
                    HWR_AddUserWordToDict( recognizer, pWord, NO );
                }
            }
            else
            {
                testWord = self.word;
                const UCHR * pWord = [RecognizerManager uchrFromString:self.word];
                if ( NULL != pWord )
                {
                    HWR_AddUserWordToDict( recognizer, pWord, NO );
                }
            }

            self.alternative = -1;
            self.probability = 0;
            self.words = 0;
            NSString * result = @"";
            const UCHR * pText = HWR_RecognizeInkData( recognizer, inkData, 0, -1, FALSE, FALSE, FALSE, FALSE );
            if ( NULL != pText )
            {
                self.words = HWR_GetResultWordCount( recognizer );
                for ( int word = 0; word < self.words; word++ )
                {
                    int altCnt = MIN( 4, HWR_GetResultAlternativeCount( recognizer, word ) );
                    for ( int alt = 0; alt < altCnt; alt++ )
                    {
                        const UCHR * pWord = HWR_GetResultWord( recognizer, word, alt );
                        if ( pWord != NULL )
                        {
                            NSString *	theWord = [RecognizerManager stringFromUchr:pWord];
                            if ( [theWord isEqualToString:testWord] && self.alternative < 0 )
                            {
                                self.alternative = alt;
                                self.probability = HWR_GetResultWeight(recognizer, word, alt);
                            }
                            result = [result stringByAppendingFormat:@"%@ [%d], ", theWord, HWR_GetResultWeight(recognizer, word, alt)];
                        }
                    }
                }
            }
            
            if ( self.words < 1 )
            {
                result = [NSString stringWithFormat:@"WORD '%@' : Nothing was recognized.\n", self.word];
            }
            else
            {
                result = [NSString stringWithFormat:@"WORD '%@' [%@] [index:%d] [prob:%d]: %@\n", self.word, testWord, self.alternative, self.probability, result];
            }
            if ( verbose )
                printf( "%s", result.UTF8String );
            HWR_FreeRecognizer(recognizer, NULL, NULL, NULL);
        }
    }
    else
    {
        if ( verbose )
            printf( "WORD '%s' : Unable to deserialize ink!\n", self.word.UTF8String );
    }
    INK_FreeData(inkData);
    return true;
}


@end
