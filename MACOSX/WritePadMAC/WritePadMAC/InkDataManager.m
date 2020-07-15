//
//  InkDataManager.m
//  WritePadMAC
//
//  Created by Stan Miasnikov on 7/12/20.
//

#import "InkDataManager.h"
#import "RecognizerWrapper.h"
#import "InkWrapper.h"
#import "OptionKeys.h"

static InkDataManager *    gManager;


@interface InkDataManager()
{
    INK_DATA_PTR    _inkData;
}

@end


@implementation InkDataManager


+ (InkDataManager * _Nonnull) sharedInkManager
{
    @synchronized(self)
    {
        if ( nil == gManager )
        {
            gManager = [[InkDataManager alloc] init];
        }
    }
    return gManager;
}

- (id) init
{
    self = [super init];
    if (self != nil)
    {
        _inkData = INK_InitData();
        if ( NULL == _inkData )
            return nil;
    }
    return self;
}

- (void) selectAllStrokes:(BOOL)select
{
    INK_SelectAllStrokes( _inkData, select );
}

- (void) selectStroke:(BOOL)select atIndex:(int)index
{
    INK_SelectStroke( _inkData, index, select );
}

- (void) dealloc
{
    if ( _inkData != NULL )
        INK_FreeData( _inkData );
}

- (int) addStroke:(CGStroke)stroke length:(int)length width:(float)width color:(COLORREF)color
{
    return INK_AddStroke( _inkData, stroke, length, width, color );
}

- (void) eraseAll
{
    INK_Erase( _inkData );
}

// Internal function

- (INK_DATA_PTR) dataPtr
{
    return _inkData;
}


@end
