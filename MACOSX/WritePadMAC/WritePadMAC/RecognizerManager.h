//
//  RecognizerManager.h
//  WritePadEN
//
//  Created by Stanislav Miasnikov on 6/4/11.
//  Copyright 2011 PhatWare Corp. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>
#import "recotypes.h"
#import "gestures.h"
#import "LanguageManager.h"
#import "InkDataManager.h"

#define MAX_SUGGESTION_COUNT	20


#if HW_RECINT_UNICODE
#define __T(x)      L ##x
#define _STRLEN     wcslen
#define _STRNCMP    wcsncmp
#else
#define __T(x)      x
#define _STRLEN     strlen
#define _STRNCMP    strncmp
#endif

#define kRecognizerDataWord     @"word"
#define kRecognizerDataWords    @"words"
#define kRecognizerDataWeight   @"weight"
#define kRecognizerDataWeights  @"weights"


@interface RecognizedWord : NSObject
{
}

@property (nonatomic, strong, nullable) NSString * word;
@property (nonatomic, assign) NSInteger  prob;
@property (nonatomic, assign) NSInteger  row;
@property (nonatomic, assign) NSInteger  col;
@property (nonatomic, assign) BOOL       isDict;

@end

@interface RecognizerManager : NSObject

@property (nonatomic) BOOL canRealoadRecognizer;

+ (RecognizerManager * _Nonnull) sharedManager;
+ (void) resetRecognizerOptions;
+ (int) calcNextRecognitionMode;

- (void) saveRecognizerDataOfType:(NSInteger)type;
- (void) reloadRecognizerDataOfType:(NSInteger)type;
- (void) resetRecognizerDataOfType:(NSInteger)type;
- (void) reset;
- (void) setMode:(int)mode;
- (BOOL) isEnabled;
- (int) getMode;
- (void) modifyRecoFlags:(NSUInteger)addFlags deleteFlags:(NSUInteger)delFlags;
- (NSString * _Nullable) recognizeInkData:(InkDataManager * _Nonnull)inkData background:(BOOL)backgroundReco async:(BOOL)asyncReco flipY:(BOOL)flip selection:(BOOL)selection;
- (BOOL) isWordInDictionary:(NSString * _Nonnull)chrWord;
- (void) enableCalculator:(BOOL)bEnable;
- (BOOL) addWordToUserDict:(NSString * _Nonnull)strWord save:(BOOL)save filter:(BOOL)filter report:(BOOL)report;
- (BOOL) learnNewWord:(NSString * _Nonnull)strWord weight:(UInt16)weight;
- (BOOL) disable:(BOOL)save;
- (BOOL) enable;
- (BOOL) reloadSettings;
- (BOOL) matchWord:(NSString * _Nonnull)text;
- (int) getWordCount;
- (int) getAltCount:(int)word;
- (BOOL) addStroke:(CGStroke _Nonnull)stroke length:(int)len;
- (BOOL) addPoints:(const CGPoint * _Nonnull)points length:(int)len;
- (BOOL) recognize;
- (NSDictionary * _Nullable) getAllWords:(unsigned int)recoFlags;
- (NSArray * _Nullable) generateWordArray:(NSInteger)suggestionCount spellCheck:(BOOL)spellCheck;
- (NSArray * _Nullable) spellCheckWord:(NSString * _Nonnull)chrWord flags:(int)flags addSpace:(BOOL)bAddSpace skipFirst:(BOOL)skipFirst;
- (BOOL) isDictionaryWord:(NSString * _Nonnull)word;
- (NSString * _Nonnull) calcString:(NSString * _Nonnull)strWord;
- (BOOL) findText:(NSString * _Nonnull)text inInk:(InkDataManager * _Nonnull)inkData startFrom:(int)firstStroke selectedOnly:(BOOL)selected;
- (NSString * _Nonnull) flipCase:(NSString * _Nonnull)word;
- (NSString * _Nonnull) ensureLower:(NSString * _Nonnull)word;
- (void) setFlags:(unsigned int)flags;
- (unsigned int) getFlags;
- (NSString * _Nullable) getResult;
- (BOOL)replaceWord:(NSString * _Nonnull)wordFrom probability1:(USHORT)prob1 wordTo:(NSString * _Nonnull)wordTo probability2:(USHORT)prob2;
- (USHORT) getWeight:(int)word alternative:(int)alt;
- (NSString * _Nonnull) autocorrectedWord:(NSString * _Nonnull)word;
- (BOOL) createUserDictionary;
- (NSArray * _Nonnull) loadUserDictionary;
- (NSArray * _Nullable) spellCheckWord:(NSString * _Nonnull)word;

@end
