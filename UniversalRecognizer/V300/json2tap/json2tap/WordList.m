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
//  WordList.m
//  json2tap
//
//  Created by Stanislav Miasnikov on 2/28/16.
//  Copyright © 2016 PhatWare Corp. All rights reserved.
//

#import "WordList.h"
#import "WordInk.h"
#import "GTLBase64.h"
#import "RecognizerWrapper.h"
#import "LanguageManager.h"

#import <math.h>

#import "TAP.H"

extern float overallCharsProb;
extern int   overallCharsCount;
extern float overallNumbersProb;
extern int   overallNumbersCount;
extern float overallPunktProb;
extern int   overallPunktCount;

#define VER_STRING      "WritePad Tap Converter 1.2"

static BOOL WriteTap( _HTAP hTap, INK_DATA_PTR ink, p_CHAR text, BOOL bTapFirstEntry );

@implementation WordList

- (id) initWithDictionary:(NSDictionary *)dict
{
    if ((self = [super init]) == nil )
        return self;

    self.language = dict[@"language"];
    self.sub_language = dict[@"sub_language"];
    self.words = [NSMutableArray array];
    
    NSArray * ws = dict[@"words"];

    for ( NSDictionary * w in ws )
    {
        NSString * ink = w[@"ink"];
        if ( nil != ink )
        {
            NSData * inkData = GTLDecodeBase64( ink );
            if ( nil != inkData )
            {
                WordInk * wi = [[WordInk alloc] init];
                wi.word = w[@"word"];
                wi.type = [w[@"type"] intValue];
                wi.ink = inkData;
                [self.words addObject:wi];
            }
        }
    }
    return self;
}

- (BOOL) testRecognitionEngine
{
    BOOL        result = false;
    
    // save gradient representation
    static const float epsilon = 0.0001;     // TODO: adjustable
    static const float gradient_delta = (4.0 * 3.14159265/100.0);   // TODO: adjustable

    @autoreleasepool
    {
        LanguageManager * langManager = [LanguageManager sharedManager];

        if ( [self.sub_language isEqualToString:@"en_US"] )
        {
            [langManager changeCurrentLanguage:WPLanguageEnglishUS];
        }
        else if ( [self.sub_language isEqualToString:@"en_UK"] )
        {
            [langManager changeCurrentLanguage:WPLanguageEnglishUK];
        }
        else if ( [self.language isEqualToString:@"es"] )
        {
            [langManager changeCurrentLanguage:WPLanguageSpanish];
        }
        else if ( [self.language isEqualToString:@"fr"] )
        {
            [langManager changeCurrentLanguage:WPLanguageFrench];
        }
        else if ( [self.language isEqualToString:@"de"] )
        {
            [langManager changeCurrentLanguage:WPLanguageGerman];
        }
        
        if ( self.verbose )
        {
            printf( "\nRecognizer test started for %s: [%s]\n", self.fileName.UTF8String, self.sub_language.UTF8String );
        }
        NSTimeInterval time = [NSDate timeIntervalSinceReferenceDate];
        for ( WordInk * wi in self.words )
        {
            [wi testWordRecogntion:self.dictPath verbose:self.verbose singleWord:self.singleWord];
        }
        
    
        time = [NSDate timeIntervalSinceReferenceDate] - time;
        if ( self.verbose )
        {
            printf( "Recognizer test finished in %.5f sec.\n", time );
        }

        // exporting to TAP

        NSInteger   alt = 0;
        NSInteger   count = [self.words count];
        NSInteger   aveProb = 0;
        NSInteger   words = 0;
        float       charsProb = 0.0, numbersProb = 0.0, punktProb = 0.0;
        int         charsCount = 0, numbersCount = 0, punktCount = 0;
        
        for ( WordInk * wi in self.words )
        {
            alt += (wi.alternative >= 0) ? 1 : 0;
            words += wi.words;
            aveProb += wi.probability;
            switch ( wi.type )
            {
                case 1 :
                case 4 :
                    if ( wi.alternative >= 0 )
                        charsProb += (1.0 - (0.01 * wi.alternative));
                    else
                        charsProb += 0.0;
                    charsCount ++;
                    break;
                    
                case 3 :
                    if ( wi.alternative >= 0 )
                        numbersProb += (1.0 - (0.01 * wi.alternative));
                    else
                        numbersProb += 0.0;
                    numbersCount ++;
                    break;
                    
                case 2 :
                    if ( wi.alternative >= 0 )
                        punktProb += (1.0 - (0.01 * wi.alternative));
                    else
                        punktProb += 0.0;
                    punktCount ++;
                    break;
            }
            
            
        }
        aveProb = aveProb / count;
        if ( alt >= count/3 && alt < count && words >= count )
        {
            overallCharsProb += charsProb;
            overallCharsCount += charsCount;
            overallNumbersProb += numbersProb;
            overallNumbersCount += numbersCount;
            overallPunktProb += punktProb;
            overallPunktCount += punktCount;
            if ( self.verbose )
            {
                printf( "Recognizer Test summary: words %ld, detected words %ld, recognized words %ld, average probability %ld\n", count, words, alt, aveProb );

                printf( "\nConvertion to TAP started for %s: [%s]\n", self.fileName.UTF8String, self.sub_language.UTF8String );
            }

            time = [NSDate timeIntervalSinceReferenceDate];

            NSError *   error = nil;
            NSString *  tapFileName = [self.filePath stringByDeletingPathExtension];
            NSString *  csvFile = [tapFileName stringByAppendingPathExtension:@"csv"];
            tapFileName = [tapFileName stringByAppendingPathExtension:@"tap"];
            
            if ( ![[NSFileManager defaultManager] removeItemAtPath:tapFileName error:&error] )
            {
                // NSLog( @"removeItemAtPath failed: %@", error.description );
            }
            
            if ( self.createCSVFile && ![[NSFileManager defaultManager] removeItemAtPath:csvFile error:&error] )
            {
                // NSLog( @"removeItemAtPath failed: %@", error.description );
            }
            
            _HTAP   hTap = TapOpenFile( (p_CHAR)tapFileName.UTF8String, TAP_RDWR );
            if ( hTap != nil )
            {
                BOOL         bTapFirstEntry = TRUE;
                INK_DATA_PTR inkData = INK_InitData();
                BOOL         bResult = FALSE;
                
                INK_EnableUndo( inkData, false );

                NSFileHandle * fileHandle = nil;
                if ( self.createCSVFile )
                {
                    bResult = [[NSFileManager defaultManager] createFileAtPath:csvFile contents:nil  attributes:nil];
                    fileHandle = [NSFileHandle fileHandleForWritingToURL:[NSURL fileURLWithPath:csvFile] error:&error];
                }
                
                for ( WordInk * wi in self.words )
                {
                    NSMutableArray *  components = [[csvFile pathComponents] mutableCopy];
                    [components removeLastObject];
                    NSString * name = [wi.word stringByReplacingOccurrencesOfString:@" " withString:@""];
                    name = [name stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet characterSetWithCharactersInString:@"asdfghjklzxcvbnmqwertyuiop1234567890QWERTYUIOPASDFGHJKLZXCVBNM-_+=(){}[]<>@#$%^"]];
                    [components addObject:[NSString stringWithFormat:@"%@.dat", name]];
                    
                    NSFileHandle * wordHandle = nil;
                    if ( self.createWordFile )
                    {
                        NSString * wordFile = [NSString pathWithComponents:components];
                        if ( ! [[NSFileManager defaultManager] fileExistsAtPath:wordFile] )
                        {
                            bResult = [[NSFileManager defaultManager] createFileAtPath:wordFile contents:nil  attributes:nil];
                            NSString * word = [NSString stringWithFormat:@"%@\n", wi.word];
                            word = [word stringByReplacingOccurrencesOfString:@" " withString:@""];
                            [word writeToFile:wordFile atomically:YES encoding:NSUTF8StringEncoding error:&error];
                        }
                        wordHandle = [NSFileHandle fileHandleForWritingToURL:[NSURL fileURLWithPath:wordFile] error:&error];
                        [wordHandle seekToEndOfFile];
                    }
                    
                    void *	pInk = (void *)wi.ink.bytes;
                    long	cbData = wi.ink.length;
                    if ( INK_Serialize( inkData, FALSE, NULL, &pInk, &cbData, TRUE, FALSE) )
                    {
                        bResult = WriteTap( hTap, inkData,
                                           (p_CHAR)[wi.word cStringUsingEncoding:NSWindowsCP1252StringEncoding],
                                           bTapFirstEntry);
                        if ( !bResult )
                            break;
                        bTapFirstEntry = FALSE;
                    }
                    
                    CGPoint * points = NULL;
                    int strokeCnt = INK_StrokeCount(inkData, FALSE);
                    NSMutableString * str = [NSMutableString stringWithFormat:@"%@", wi.word];
                    float grad, g, dx, dy;
                    int   last_index = 0;
                    for ( int stroke = 0; stroke < strokeCnt; stroke++ )
                    {
                        int len = INK_GetStroke(inkData, stroke, &points, NULL, NULL);
                        if ( len > 3 )
                        {
                            dx = points[1].x - points[0].x;
                            dy = points[1].y - points[0].y;
                            last_index = 1;
                            grad = atanf(dx/(dy+epsilon));
                            
                            [str appendFormat:@",%.3f", grad];
                            for ( int k = 2; k < len; k++ )
                            {
                                dx = points[k].x - points[last_index].x;
                                dy = points[k].y - points[last_index].y;
                                g = atanf(dx/(dy+epsilon));
                                if ( fabsf(grad-g) > gradient_delta )
                                {
                                    grad = g;
                                    last_index = k;
                                    [str appendFormat:@",%.3f", grad];
                                }
                            }
                        }
                        else
                        {
                            // single pixel
                            grad = -2;
                            [str appendFormat:@",%.3f", grad];
                        }
                        // stroke separator
                        // strokerep[index++] = 0;
                    }
                    
                    [str appendString:@"\n"];
                    
                    // [fileHandle seekToEndOfFile];
                    if ( fileHandle )
                        [fileHandle writeData:[str dataUsingEncoding:NSUTF8StringEncoding]];
                    
                    NSString * subStr = [str substringFromIndex:[str rangeOfString:@","].location+1];
                    if ( wordHandle )
                        [wordHandle writeData:[subStr dataUsingEncoding:NSUTF8StringEncoding]];
                    
                    if ( points != NULL )
                        free( (void *)points );
                    INK_DeleteSelectedStrokes( inkData, TRUE );
                    if ( wordHandle )
                        [wordHandle closeFile];
                }

                INK_FreeData( inkData );
                TapCloseFile( hTap );

                if ( fileHandle )
                    [fileHandle closeFile];
                
                if ( ! bResult )
                {
                    [[NSFileManager defaultManager] removeItemAtPath:tapFileName error:nil];
                    [[NSFileManager defaultManager] removeItemAtPath:csvFile error:nil];
                    printf( "Error while writing to TAP; file %s deleted\n", tapFileName.UTF8String );
                }

                time = [NSDate timeIntervalSinceReferenceDate] - time;
                if ( self.verbose )
                {
                    printf( "Convertion to TAP finished in %.5f sec.\n", time );
                }
                result = true;
            }
            else
            {
                printf( "Unable to create TAP File: %s\n", tapFileName.UTF8String );
            }
        
        }
        else
        {
            printf( "Skipping %s file while convering to TAP: words %ld, detected words %ld, recognized words %ld, average probability %ld\n", self.fileName.UTF8String, count, words, alt, aveProb );
        }
    }
    return result;
}

@end


static BOOL WriteTap( _HTAP hTap, INK_DATA_PTR ink, p_CHAR text, BOOL bTapFirstEntry )
{
    if (bTapFirstEntry)
    {
        TapSetComment(hTap, VER_STRING);
    }
    else
    {
        TapWriteNewPage(hTap);
        TapWriteNewWord(hTap);
    }
    
    BOOL bResult = TRUE;
    CGPoint *   points = NULL;
    int count = INK_StrokeCount(ink, FALSE);
    for ( int i = 0; i < count; i++ )
    {
        int nPoints = INK_GetStroke( ink, i, &points, NULL, NULL );
        if ( nPoints > 0 )
        {
            p_TAPPOINT  lpTapPoints = (p_TAPPOINT) malloc(sizeof( _TAPPOINT )*nPoints );
            if ( lpTapPoints != NULL )
            {
                _TAPSTROKE  tStroke = {0};
                tStroke.nPoints = nPoints;
                for ( int j = 0; j < nPoints; j++ )
                {
                    lpTapPoints[j].x = (short)points[j].x;
                    lpTapPoints[j].y = (short)points[j].y;
                }
                tStroke.pPoints = lpTapPoints;
                if ( RC_TAP_OK != TapWriteStroke(hTap, &tStroke) )
                    bResult = FALSE;
                free( (void *)lpTapPoints );
            }
            if ( RC_TAP_OK != TapSetText(hTap, text) )
            {
                bResult = FALSE;
                break;
            }
        }
    }
    if ( points != NULL )
        free( (void *)points );
    return bResult;
} /* ttcWriteTap */

