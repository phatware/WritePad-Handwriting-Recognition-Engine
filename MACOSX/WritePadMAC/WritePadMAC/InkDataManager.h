//
//  InkDataManager.h
//  WritePadMAC
//
//  Created by Stan Miasnikov on 7/12/20.
//

#import <Foundation/Foundation.h>
#import "recotypes.h"
#import "gestures.h"

@interface InkDataManager : NSObject

+ (InkDataManager * _Nonnull) sharedInkManager;

- (void) selectAllStrokes:(BOOL)select;
- (void) selectStroke:(BOOL)select atIndex:(int)index;
- (int)  addStroke:(CGStroke _Nonnull)stroke length:(int)length width:(float)width color:(COLORREF)color;
- (void) eraseAll;

@end
