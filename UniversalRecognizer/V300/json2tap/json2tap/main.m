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
//  main.m
//  json2tap
//
//  Created by Stanislav Miasnikov on 2/28/16.
//  Copyright © 2016 PhatWare Corp. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <stdlib.h>
#import "WordList.h"

static BOOL processInkData( NSString * filepath, NSString * name, NSString * dictPath, BOOL verbose, BOOL singleWord, BOOL csvFile, BOOL wordsFile );
static BOOL isSupportedFile( NSString * name );
static int enumFiles( NSURL * folderURL, NSMutableArray * files, BOOL recursive );

float overallCharsProb = 0.0;
int   overallCharsCount = 0;
float overallNumbersProb = 0.0;
int   overallNumbersCount = 0;
float overallPunktProb = 0.0;
int   overallPunktCount = 0;


int main(int argc, const char * argv[])
{
    @autoreleasepool
    {
        NSString *  filePath = nil;
        NSString *  fileName = nil;
        NSString *  dictPath = nil;
        BOOL        recursive = false;
        BOOL        verbose = false;
        BOOL        singleWord = false;
        BOOL        csvFile = false;
        BOOL        wordsFile = false;
        
        printf( "WritePad SDK JSON to TAP converter. JSON must be in format of the iOS Handwriting Sampling Wizard.\n" );
        printf( "Copyright (c) 2016 PhatWare Corp. All rights reserved.\n\n" );
        
        
        if ( argc < 3 )
            goto invalidArguments;
        
        for ( int i = 1; i < argc; i++ )
        {
            NSString * arg = [NSString stringWithUTF8String:argv[i]];
            if ( [arg caseInsensitiveCompare:@"-p"] == NSOrderedSame )
            {
                // file directory
                if ( fileName != nil || i >= argc - 1 )
                    goto invalidArguments;
                filePath = [NSString stringWithUTF8String:argv[++i]];
            }
            else if ( [arg caseInsensitiveCompare:@"-f"] == NSOrderedSame )
            {
                // file directory
                if ( filePath != nil || i >= argc - 1 )
                    goto invalidArguments;
                fileName = [NSString stringWithUTF8String:argv[++i]];
            }
            else if ( [arg caseInsensitiveCompare:@"-d"] == NSOrderedSame )
            {
                // file directory
                if ( i >= argc - 1 )
                    goto invalidArguments;
                dictPath = [NSString stringWithUTF8String:argv[++i]];
            }
            else if ( [arg caseInsensitiveCompare:@"-r"] == NSOrderedSame )
            {
                recursive = true;
            }
            else if ( [arg caseInsensitiveCompare:@"-v"] == NSOrderedSame )
            {
                verbose = true;
            }
            else if ( [arg caseInsensitiveCompare:@"-1"] == NSOrderedSame )
            {
                singleWord = true;
            }
            else if ( [arg caseInsensitiveCompare:@"-c"] == NSOrderedSame )
            {
                csvFile = true;
            }
            else if ( [arg caseInsensitiveCompare:@"-w"] == NSOrderedSame )
            {
                wordsFile = true;
            }
            else
            {
                goto invalidArguments;
            }
        }
        
        if ( fileName == nil && filePath == nil )
        {
invalidArguments:
            printf( "Invalid arguments. Usage:\n"
                   "json2tap -f <filename> | -p <dirpath> [-r] [-v] [-c] [-w] [-1] [-d <dictionary folder path>]\n\n");
            return -1;
        }

        if ( fileName != nil )
        {
            NSString * name = fileName;
            NSArray * components = fileName.pathComponents;
            if ( components.count >= 2 )
            {
                name = [NSString stringWithFormat:@"%@.%@", components[components.count-1],
                        components[components.count-2]];
            }
            processInkData( fileName, name, dictPath, verbose, singleWord, csvFile, wordsFile );
        }
        else if ( filePath != nil )
        {
            NSMutableArray * files = [NSMutableArray array];
            NSURL * folderURL = [NSURL URLWithString:filePath];
            enumFiles(folderURL, files, recursive);
            
            if ( files.count < 1 )
            {
                printf( "Folder %s does not contain any JSON files.\n", filePath.UTF8String );
                return -1;
            }
            else
            {
                printf("Found %lu files; running handwriting recognition tests\n", (unsigned long)files.count);
            }
            
            NSInteger   count = 0;
            NSInteger   converted = 0;
            for ( NSDictionary * fd in files )
            {
                NSURL * path = fd[@"path"];
                if ( processInkData( [path path], fd[@"name"], dictPath, verbose, singleWord, csvFile, wordsFile ) )
                {
                    converted++;
                }
                count++;
                if ( files.count > 1 )
                {
                    NSInteger percent = (count * 100)/files.count;
                    printf( "\r%ld File of %ld File(s) (%ld%%) complete        ", (long)count, (long)files.count, (long)percent );
                    if ( verbose )
                    {
                        printf("\n");
                    }
                    else
                    {
                        fflush(stdout);
                    }
                }
                if ( overallCharsCount > 0 && overallPunktProb > 0 && overallNumbersCount > 0 && (count % 10) == 0 )
                {
                    printf( "\nRecognizer quality so far. Characters: %f Numbers: %f Punktuation: %f",
                           overallCharsProb/overallCharsCount,
                           overallNumbersProb/overallNumbersCount,
                           overallPunktProb/overallPunktCount
                           );
                }
            }
            printf( "\nFinished: %ld JSON files processed, %ld files converted to TAP\n", count, converted );
            if ( overallCharsProb > 0 && overallPunktProb > 0 && overallNumbersCount > 0 )
            {
                printf( "Overall Recognizer quality. Characters: %f Numbers: %f Punktuation: %f\n",
                       overallCharsProb/overallCharsCount,
                       overallNumbersProb/overallNumbersCount,
                       overallPunktProb/overallPunktCount
                       );
            }
        }
    }
    return 0;
}

static int enumFiles( NSURL * folderURL, NSMutableArray * files, BOOL recursive )
{
    int count = 0;
    @try
    {
        // NSLog( @"Folder Path: %@", folderPath );
        NSArray * keys = @[NSURLIsDirectoryKey, NSURLNameKey, NSURLIsRegularFileKey, NSURLIsHiddenKey,
                           NSURLContentModificationDateKey, NSURLFileSizeKey];
        // NSArray * keys = @[NSURLIsDirectoryKey, NSURLIsHiddenKey];
        NSDirectoryEnumerator *direnum = [[NSFileManager defaultManager] enumeratorAtURL:folderURL includingPropertiesForKeys:keys
                                                                                 options:NSDirectoryEnumerationSkipsSubdirectoryDescendants
                                          //| NSDirectoryEnumerationSkipsHiddenFiles)
                                                                            errorHandler:^(NSURL *url, NSError *error)
                                          {
                                              // Handle the error.
                                              // Return YES if the enumeration should continue after the error.
                                              printf( "Error while enumerating files %s", error.description.UTF8String );
                                              return YES;
                                          }];
        
        [direnum skipDescendants];
        NSURL * pname = nil;
        while ( pname = [direnum nextObject] )
        {
            NSError *   error = nil;
            NSString *  name = nil;
            NSNumber *  hidden = nil;
            NSNumber *  directory = nil;
            if (! [pname getResourceValue:&name forKey:NSURLNameKey error:&error])
            {
                continue;
            }
            [pname getResourceValue:&hidden forKey:NSURLIsHiddenKey error:&error];
            [pname getResourceValue:&directory forKey:NSURLIsDirectoryKey error:&error];
            if ( recursive && [directory boolValue] && (![hidden boolValue]) )
            {
                NSURL * newFolder = [folderURL URLByAppendingPathComponent:name isDirectory:YES];
                count += enumFiles( newFolder, files, recursive );
                continue;
            }
            else if ( [hidden boolValue] )
            {
                continue;
            }
            
            if ( ! isSupportedFile( name ) )
            {
                continue;
            }
            
            NSURL * fileURL = [folderURL URLByAppendingPathComponent:name];
            [files addObject:@{ @"path": fileURL, @"name": name }];
            count++;
        }
    }
    @catch ( NSException * exc )
    {
        printf( "Error while enumerating files %s\n", exc.description.UTF8String );
    }
    return count;
}

static BOOL isSupportedFile( NSString * name )
{
    NSArray * extensions = @[@".json", @".jsn"];
    for ( NSString * ext in extensions )
    {
        NSInteger len1 = [name length];
        NSInteger len2 = [ext length];
        if ( len1 > len2 && [[name substringFromIndex:(len1-len2)] caseInsensitiveCompare:ext] == NSOrderedSame )
            return YES;
    }
    return NO;
}

static BOOL processInkData( NSString * filepath, NSString * name, NSString * dictPath, BOOL verbose, BOOL singleWord, BOOL csvFile, BOOL wordsFile )
{
    WordList * words = nil;
    @autoreleasepool
    {
        NSData * data = [NSData dataWithContentsOfFile:filepath];
        NSError * error = nil;
        id json = [NSJSONSerialization JSONObjectWithData:data options:(NSJSONReadingMutableLeaves) error:&error];
        
        if ( json == nil || error != nil )
        {
            printf( "Unable to parse JSON file %s. Error: %s\n", name.UTF8String, error.description.UTF8String );
            return false;
        }
        
        if ( ![json isKindOfClass:[NSDictionary class]] )
        {
            printf( "Unable to parce JSON file %s. Error: invalid JSON format.\n", name.UTF8String );
            return false;
        }
        
        words = [[WordList alloc] initWithDictionary:(NSDictionary *)json];
        words.fileName = name;
        words.dictPath = dictPath;
        words.filePath = filepath;
        words.verbose = verbose;
        words.singleWord = singleWord;
        words.createWordFile = wordsFile;
        words.createCSVFile = csvFile;
    }
    return [words testRecognitionEngine];
}

