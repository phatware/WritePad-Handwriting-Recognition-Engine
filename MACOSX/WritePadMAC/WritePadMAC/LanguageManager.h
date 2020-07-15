//
//  LanguageManager.h
//  WritePad
//
//  Created by Stanislav Miasnikov on 6/4/11.
//  Copyright 2011 PhatWare Corp. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>
#ifdef PERSISTENT_DATA
#import "WritePadPersistentData.h"
#endif // PERSISTENT_DATA

typedef enum
{
	WPLanguageUnknown = 0,
	WPLanguageEnglishUS = 1,
	WPLanguageEnglishUK,
    WPLanguageGerman,
	WPLanguageFrench,
	WPLanguageSpanish,
    WPLanguagePortuguese,
    WPLanguageBrazilian,
	WPLanguageItalian,
    WPLanguageDutch,
	WPLanguageDanish,
    WPLanguageSwedish,
	WPLanguageNorwegian,
	WPLanguageFinnish,
    WPLanguageIndonesian,
	WPLanguageMedicalUS,
	WPLanguageMedicalUK,
} WPLanguage;

#define USERDATA_DICTIONARY		0x0004
#define USERDATA_AUTOCORRECTOR	0x0001
#define USERDATA_LEARNER		0x0002
#define USERDATA_ALL			0x00FF


@interface LanguageManager : NSObject

+ (LanguageManager * _Nonnull) sharedManager;

- (NSString * _Nonnull) languageName:(WPLanguage)wplanguage;
- (NSString * _Nonnull) mainDictionaryPath;
- (NSString * _Nonnull) languageCode;
#ifdef PERSISTENT_DATA
- (NSString * _Nonnull) infoPasteboardName;
#endif // PERSISTENT_DATA
- (int) getLanguageID;
- (void) changeCurrentLanguageID:(int)languageID;
- (void) changeCurrentLanguage:(WPLanguage)language;
- (BOOL) spellCheckerEnabled;
- (NSArray *  _Nonnull) supportedLanguages;
- (NSString * _Nullable) userFilePathOfType:(NSInteger)type;
- (WPLanguage) systemLanguage;
- (NSString *  _Nonnull) shortLanguageName;
- (WPLanguage) languageIDFromLanguageCode:(int)languageID;
- (int) getLanguageIDWithLanguage:(WPLanguage)wpLanguage;
- (BOOL) isLanguageSupported:(WPLanguage)language;

@property (nonatomic,readonly) WPLanguage	currentLanguage;

#ifdef PERSISTENT_DATA
@property (nonatomic, retain, readonly, nonnull) WritePadPersistentData * sharedUserData;
#endif // PERSISTENT_DATA

@end
