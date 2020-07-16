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

@property (nonatomic, readonly) INK_DATA_PTR inkData;

@end

@implementation InkDataManager

@synthesize inkData = _inkData;

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

- (int) strokeCount
{
    return INK_StrokeCount( _inkData, FALSE );
}

- (BOOL) deleteLastStroke:(int)stroke
{
    return INK_DeleteStroke( _inkData, stroke );
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

- (NSArray<NSValue *> *) getStrokePoints:(int)nStroke
{
    float       fWidth = 1.0;
    COLORREF    coloref = 0;
    CGStroke    points = NULL;
    int len  = INK_GetStrokeP( _inkData, nStroke, &points, &fWidth, &coloref );
    if ( len > 0 && points != NULL )
    {
        NSMutableArray * arr = [NSMutableArray arrayWithCapacity:len];
        for ( int i = 0; i < len; i++ )
        {
            arr[i] = [NSValue valueWithPoint:points[i].pt];
        }
        free( (void *)points );
        return [NSArray arrayWithArray:arr];
    }
    return nil;
}

- (void) eraseAll
{
    INK_Erase( _inkData );
}

- (void) enableUndo:(BOOL)enable
{
    INK_EnableUndo( _inkData, enable);
}

- (CGRect) getDataRect:(BOOL)selectedOnly
{
    CGRect rect = CGRectZero;
    INK_GetDataRect( _inkData, &rect, selectedOnly );
    return rect;
}

- (CGRect) getStrokeRect:(int)stroke
{
    CGRect rect = CGRectZero;
    INK_GetStrokeRect( _inkData, -1, &rect, TRUE );
    return rect;
}

// Internal function

- (INK_DATA_PTR) dataPtr
{
    return _inkData;
}


@end
