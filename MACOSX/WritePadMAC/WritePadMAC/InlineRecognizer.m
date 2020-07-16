/* ************************************************************************************* */
/* *    PhatWare WritePad SDK                                                          * */
/* *    Copyright (c) 1997-2019 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
 *
 * WritePad Input Panel Sample
 *
 * Unauthorized distribution of this code is prohibited. For more information
 * refer to the End User Software License Agreement provided with this
 * software.
 *
 * This source code is distributed and supported by PhatWare Corp.
 * http://www.phatware.com
 *
 * THIS SAMPLE CODE CAN BE USED  AS A REFERENCE AND, IN ITS BINARY FORM,
 * IN THE USER'S PROJECT WHICH IS INTEGRATED WITH THE WRITEPAD SDK.
 * ANY OTHER USE OF THIS CODE IS PROHIBITED.
 *
 * THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 * AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 * FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL PHATWARE CORP.
 * BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
 * INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS
 * OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT PHATWARE CORP.
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * US Government Users Restricted Rights
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in EULA and in FAR 52.227.19(c)(2) or subparagraph
 * (c)(1)(ii) of the Rights in Technical Data and Computer Software
 * clause at DFARS 252.227-7013 and/or in similar or successor
 * clauses in the FAR or the DOD or NASA FAR Supplement.
 * Unpublished-- rights reserved under the copyright laws of the
 * United States.  Contractor/manufacturer is PhatWare Corp.
 * 10414 W. Highway 2, Ste 4-121 Spokane, WA 99224
 *
 * ************************************************************************************* */

#import "InlineRecognizer.h"
#import "OptionKeys.h"
#import "LanguageManager.h"
#import "RecognizerManager.h"
#import "InkWrapper.h"
#import "RecognizerWrapper.h"
#import "InkDataManager.h"

#define MAX_QUEUE_SIZE        512

@interface InlineRecognizer ()
{
    CGStroke			ptStroke;
    int					strokeLen;
    int					strokeMemLen;
    Boolean				_firstTouch;
    CGPoint				_previousLocation;
    Boolean				autoRecognize;
    Boolean				backgroundReco;
    NSTimer *			_timerRecognizer;
    NSTimeInterval		recognitionDelay;
    GESTURE_TYPE		gesturesEnabledIfEmpty;
    GESTURE_TYPE		gesturesEnabledIfData;
    Boolean				_bAddStroke;
    CGTracePoint		_inkQueue[MAX_QUEUE_SIZE];
    int					_inkQueueGet, _inkQueuePut;
    NSCondition		*	_inkQueueCondition;
    Boolean				_runInkThread;
    Boolean				_bAsyncInkCollector;
    NSLock *			_inkLock;
    BOOL                _useAsyncRecognizer;
    InkDataManager *     inkData;
}

- (void) killRecoTimer;
- (BOOL) enableAsyncInk:(BOOL)bEnable;
- (int)  addPointPoint:(CGTracePoint)point;
- (void) addPointToQueue:(CGTracePoint)point;
- (void) processEndOfStroke:(BOOL)fromThread;

@property (nonatomic, copy ) NSString * currentResult;
@property (nonatomic, strong ) NSTimer * timerTouchAndHold;

@end

@interface RecognizerManager()

@property (nonatomic, readonly) RECOGNIZER_PTR recognizer;

@end

@interface InkDataManager()

@property (nonatomic, readonly) INK_DATA_PTR inkData;

@end

#define STROKE_FILTER_TIMEOUT        1.0
#define STROKE_FILTER_DISTANCE        200
#define DEFAULT_STROKE_LEN          150
#define DEFAULT_RECODELAY           1.2
#define DEFAULT_BACKGESTURELEN      150
#define DEFAULT_TOUCHANDHOLDDELAY   0.8
#define TAP_SPACIAL_SENSITIVITY     8


#define GLOBAL_RECO_HANDLE     ([[RecognizerManager sharedManager] recognizer])

@implementation InlineRecognizer

@synthesize ptStroke = ptStroke;
@synthesize strokeLen = strokeLen;

- (id) init
{	
	if((self = [super init]))
	{		
		strokeLen = 0;
		strokeMemLen = DEFAULT_STROKE_LEN * sizeof( CGTracePoint );
		ptStroke = malloc( strokeMemLen );
        inkData = [[InkDataManager alloc] init];
        [inkData enableUndo:false];
		recognitionDelay = DEFAULT_RECODELAY;
		_timerRecognizer = nil;
		gesturesEnabledIfEmpty = GEST_NONE;
		gesturesEnabledIfData = GEST_NONE;
				
		_bAddStroke = YES;
		backgroundReco = YES;
		_firstTouch = NO;
		_bAsyncInkCollector = NO;
		_inkQueueCondition = [[NSCondition alloc] init];
		_inkLock = [[NSLock alloc] init];

        self.currentResult = nil;
        self.timerTouchAndHold = nil;
        
        [self enableAsyncInk:YES];
        [self reloadOptions];
        // NOTE: do not need async recognizer if kEditOptionsShowSuggestions is not YES
        _useAsyncRecognizer = YES; // [[NSUserDefaults standardUserDefaults] boolForKey:kEditOptionsShowSuggestions];
		// Init shorctus
    }
	return self;
}

- (NSArray<NSValue *> *) getStrokePoints:(int)nStroke
{
    return [inkData getStrokePoints:nStroke];
}

- (int) strokeCount
{
    return [inkData strokeCount];
}

#pragma mark AsyncInkCollector

- (BOOL) enableAsyncInk:(BOOL)bEnable
{
	// can't disable async ink if async reco is enabled
	if ( (!bEnable) && _bAsyncInkCollector )
	{
		// terminate ink thread
		if ( ! [_inkLock tryLock] )
		{
			_runInkThread = NO;
            CGTracePoint point;
            point.pt = CGPointMake( 0,0 );
            point.pressure = DEFAULT_PRESSURE;
			[self addPointToQueue:point];
			[_inkLock lock];
		}
		[_inkLock unlock];
		_bAsyncInkCollector = NO;
	}
	else if ( bEnable && (!_bAsyncInkCollector)  )
	{
		_runInkThread = YES;
		[NSThread detachNewThreadSelector:@selector(inkCollectorThread:) toTarget:self 
							   withObject:nil];		
		_bAsyncInkCollector = YES;
	}
	_inkQueueGet = _inkQueuePut = 0;
	_bAddStroke = YES;
	return _bAsyncInkCollector;
}


- (void)inkCollectorThread:(id)anObj
{
    @autoreleasepool {
        
        [_inkLock lock];
        while( _runInkThread )
        {
            [_inkQueueCondition lock];
            while ( _inkQueueGet == _inkQueuePut )
            {
                [_inkQueueCondition wait];
            }
            [_inkQueueCondition unlock];
            
            if ( ! _runInkThread )
            {
                // [_inkQueueCondition unlock];
                break;
            }
            
            register int iGet = _inkQueueGet, iPut = _inkQueuePut;
            int nAdded = 0;
            
            while( (iGet = _inkQueueGet) != iPut )
            {
                if ( iGet > iPut )
                {
                    while ( iGet < MAX_QUEUE_SIZE )
                    {
                        nAdded += [self addPointPoint:_inkQueue[iGet]];
                        iGet++;
                    }
                    iGet = 0;
                }
                while ( iGet < iPut )
                {
                    nAdded += [self addPointPoint:_inkQueue[iGet]];
                    iGet++;
                }
                _inkQueueGet = iPut;
            }
        }
        
        [_inkLock unlock];
    }
}

-(void)addPointToQueue:(CGTracePoint)point
{
	[_inkQueueCondition lock]; 
	
	int iPut = _inkQueuePut;
	_inkQueue[iPut] = point;
	iPut++;
	if ( iPut >= MAX_QUEUE_SIZE )
		iPut = 0;
	_inkQueuePut = iPut;
	[_inkQueueCondition broadcast];
	
	[_inkQueueCondition unlock]; 
}

- (int)addPointPoint:(CGTracePoint)point
{
    // this method called from inkCollectorThread
	int nAdded = 0;
	if ( point.pt.y == -1 )
	{
		[self processEndOfStroke:YES];
	}
	else
	{
        nAdded += [self AddPixels:point IsLastPoint:FALSE];
	}
	return nAdded;	
}

-(void) strokeGestureInTread:(NSArray *)arr
// This method is called when a strokeGestureTread selector from main thread is called.
{
	GESTURE_TYPE	gesture = (GESTURE_TYPE)[(NSNumber *)[arr objectAtIndex:0] intValue];
	UInt32			nStrokeCount = [(NSNumber *)[arr objectAtIndex:1] unsignedIntValue];
	
	if ( gesture == GEST_LOOP && nStrokeCount > 0 )
	{
		// check if this is a correct 
        CGRect rData = [inkData getDataRect:FALSE];
		if ( !CGRectIsEmpty(rData) )
		{
			CGFloat left, right;
			CGFloat bottom, top;
			left = right =  ptStroke[0].pt.x;
			bottom = top =  ptStroke[0].pt.y;
			for( register int i = 1; i < strokeLen; i++ )
			{
				left = MIN( ptStroke[i].pt.x, left );
				right = MAX( ptStroke[i].pt.x, right );
				top =  MIN( ptStroke[i].pt.y, top );
				bottom =  MAX( ptStroke[i].pt.y, bottom );
			}
			CGFloat dx = rData.size.width/8;
			CGFloat dy = rData.size.height/8;
			if ( left < (rData.origin.x + dx) && top < (rData.origin.y + dy) && right > (rData.origin.x + rData.size.width - dx) &&
				bottom > (rData.origin.y + rData.size.height - dy) )
			{
				// TODO: recognize shortcut and get name and see if it matches...
			}
		}
	}
	else
	{
		if ([self.delegate respondsToSelector:@selector(InkCollectorRecognizedGesture:withGesture:isEmpty:)])
        {
			_bAddStroke = [self.delegate InkCollectorRecognizedGesture:self withGesture:gesture isEmpty:(nStrokeCount == 0)];
        }
    }
	if ( ! _bAddStroke )
		strokeLen = 0;
}

#pragma mark ReloadOptions

- (void)reloadOptions
{
	NSUserDefaults*	defaults = [NSUserDefaults standardUserDefaults];

    [self stopAsyncRecoThread];
    recognitionDelay = [defaults doubleForKey:kRecoOptionsTimerDelay];
    if ( 0 == recognitionDelay )
        recognitionDelay = DEFAULT_RECODELAY;
    
    // [self enableAsyncInk:NO];
    _useAsyncRecognizer = [defaults boolForKey:kEditOptionsShowSuggestions];

    // TODO: [self enableGestures:[SysGesturesViewController getEnabledGestues] whenEmpty:YES];
    [self enableGestures:(GEST_RETURN | GEST_CUT | GEST_LOOP | GEST_BACK) whenEmpty:NO];
    
    if ( [self isInkData] && _useAsyncRecognizer )
    {
        [self startAsyncRecoThread];
    }
}

- (BOOL) isInkData
{
	return [inkData strokeCount] > 0 ? YES : NO;
}

- (void) killRecoTimer
{
	if ( nil != _timerRecognizer )
	{
		[_timerRecognizer invalidate];
		_timerRecognizer = nil;
	}
}

-(void) strokeAdded:(NSObject *)object
// This method is called when a strokeAddedInTread selector from main thread is called.
{	
	if ( autoRecognize )
	{
		//Start recognition timer
		[self killRecoTimer];
		_timerRecognizer = [NSTimer scheduledTimerWithTimeInterval:recognitionDelay target:self 
														  selector:@selector(recognizerTimer) userInfo:nil repeats:NO];
	}
}

-(void) recognizeNow
// This method is called when a strokeAddedInTread selector from main thread is called.
{	
	//Start recognition timer
	[self killRecoTimer];
	_timerRecognizer = [NSTimer scheduledTimerWithTimeInterval:0.02 target:self
														  selector:@selector(recognizerTimer) userInfo:nil repeats:NO];
}

- (BOOL) recognizeInk:(BOOL)bErase
{
	[self killRecoTimer];
	
    NSMutableString * strResult;
    if ( _useAsyncRecognizer )
    {
        strResult = [self.currentResult mutableCopy];
        self.currentResult = nil;
    }
	else
    {
        if ( (! [self isInkData]) )
            return NO;
        strResult = [[[RecognizerManager sharedManager] recognizeInkData:inkData background:backgroundReco async:NO flipY:YES selection:NO] mutableCopy];
    }
    if ( [strResult length] > 1 && [strResult characterAtIndex:[strResult length] - 1] == ' ' && (![[NSUserDefaults standardUserDefaults] floatForKey:kRecoOptionsAddSpace]) )
    {
        if ( !([strResult length] == 2 && [[NSUserDefaults standardUserDefaults] floatForKey:kRecoOptionsAddSpaceSingleLetter]) )
            [strResult deleteCharactersInRange:NSMakeRange( [strResult length] - 1, 1 )];
    }
	if ( bErase )
		[self empty];
    
	// NSComparisonResult comp = [strResult compare:kEmptyWord options:NSCaseInsensitiveSearch range:NSMakeRange( 0, 5 )]; 
	if ( [strResult rangeOfString:@kEmptyWord].location != NSNotFound || [strResult rangeOfString:@"*Error*"].location != NSNotFound )
	{
		// error...
		return NO;
	}

	if ([self.delegate respondsToSelector:@selector(InkCollectorResultReady:theResult:)])
	{
		[self.delegate InkCollectorResultReady:self theResult:strResult];
	}
    return YES;
}

- (void) enableGestures:(GESTURE_TYPE)gestures whenEmpty:(BOOL)bEmpty
{
	if ( bEmpty )
		gesturesEnabledIfEmpty = gestures;
	else
		gesturesEnabledIfData = gestures;
}

- (BOOL) deleteLastStroke
{
    BOOL bResult = [inkData deleteLastStroke:(-1)];
    
    if ( bResult && _useAsyncRecognizer )
    {
        [self startAsyncRecoThread];
    }
    else if ( bResult && backgroundReco )
	{
		[[RecognizerManager sharedManager] reset];
		if ( [inkData strokeCount] > 0 )
		{
			// restart background recognizer
			HWR_PreRecognizeInkData( GLOBAL_RECO_HANDLE, inkData.inkData, 0, TRUE );
			[self strokeAdded:nil];	// restart recognizer timer
		}
	}
    [self strokeAdded:nil];
    
	return bResult;
}

#pragma mark - Ink Collection support


- (GESTURE_TYPE)recognizeGesture:(GESTURE_TYPE)gestures withStroke:(CGStroke)points withLength:(int)count 
{
	if ( count < 5 )
		return GEST_NONE;
	
    NSUserDefaults * defaults = [NSUserDefaults standardUserDefaults];
	NSInteger iLen = [defaults integerForKey:kRecoOptionsBackstrokeLen];
	if ( iLen == 0 )
	{
		iLen = DEFAULT_BACKGESTURELEN;
		[defaults setInteger:iLen forKey:kRecoOptionsBackstrokeLen];
        [defaults synchronize];
    }
	GESTURE_TYPE type = HWR_CheckGesture( gestures, points, count, 1, (int)iLen );
	return type;
}

// this function is called from secondary thread
-(void) processEndOfStroke:(BOOL)fromThread
{
	GESTURE_TYPE	gesture = GEST_NONE;
    UInt32			nStrokeCount = [inkData strokeCount];
	
	_bAddStroke = YES;
	if ( strokeLen > 5  && nStrokeCount > 0 && gesturesEnabledIfData != GEST_NONE )
	{
		// recognize gesture
        gesture = [self recognizeGesture:gesturesEnabledIfData withStroke:ptStroke withLength:strokeLen];
	}
	else if ( strokeLen > 5 && nStrokeCount == 0 && gesturesEnabledIfEmpty != GEST_NONE )
	{
		// recognize gesture
		gesture = [self recognizeGesture:gesturesEnabledIfEmpty withStroke:ptStroke withLength:strokeLen];
	}
	
	if ( gesture != GEST_NONE )
	{
        NSArray * arr = @[@(gesture), @(nStrokeCount)];
		if ( fromThread )
		{
			// gesture recognized, notify main thread
			[self performSelectorOnMainThread:@selector(strokeGestureInTread:) withObject:arr waitUntilDone:YES];
		}
		else
		{
			[self strokeGestureInTread:arr];
		}
	}		
	
    CGRect rect = CGRectNull;
    
	if ( _bAddStroke )
	{
		if ( (!_useAsyncRecognizer) && backgroundReco && [inkData strokeCount] < 1 )
		{
			[[RecognizerManager sharedManager] reset];
		}
		// call up the app delegate
		if ( [inkData addStroke:ptStroke length:strokeLen width:1.0 color:0] > 0 )
		{
            if ( _useAsyncRecognizer )
            {
                [self startAsyncRecoThread];
            }
			else if ( backgroundReco )
			{
				HWR_RecognizerAddStroke( GLOBAL_RECO_HANDLE, ptStroke, strokeLen );
			}
			if ( fromThread )
			{
				[self performSelectorOnMainThread:@selector(strokeAdded:) withObject:nil waitUntilDone:NO];
			}
			else
			{
				[self strokeAdded:nil];
			}
            rect = [inkData getStrokeRect:(-1)];
		}
	}
	// else
	{
        strokeLen = 0;
	}
}

// Releases resources when they are not longer needed.
- (void) dealloc
{
	[self killRecoTimer];
    
    self.currentResult = nil;
	
	if ( NULL != ptStroke )
		free( ptStroke );	
	ptStroke = NULL;	
}


-(void) empty
{
	[self killRecoTimer];
    [inkData eraseAll];
    self.currentResult = nil;
}

#pragma mark - AddPixelToStroke

#define SEGMENT2            2
#define SEGMENT3            3
#define SEGMENT4            4

#define SEGMENT_DIST_1      3
#define SEGMENT_DIST_2      6
#define SEGMENT_DIST_3      12

- (NSInteger) AddPixels:(CGTracePoint)tracePoint IsLastPoint:(BOOL)bLastPoint
// this method called from inkCollectorThread
{
    CGFloat		xNew, yNew, x1, y1;
    CGFloat		nSeg = SEGMENT3;
    int         pressure = tracePoint.pressure;
    CGFloat     x = tracePoint.pt.x;
    CGFloat     y = tracePoint.pt.y;

	if ( NULL == ptStroke )
		return 0;
	
    if  ( strokeLen < 1 )
    {
        ptStroke[strokeLen].pt.x = _previousLocation.x = x;
        ptStroke[strokeLen].pt.y = _previousLocation.y = y;
        ptStroke[strokeLen].pressure = pressure;
        strokeLen = 1;
        return  1;
    }
	
    CGFloat dx = fabs( x - ptStroke[strokeLen-1].pt.x );
    CGFloat dy = fabs( y - ptStroke[strokeLen-1].pt.y );
	
    if  ( dx + dy < 2.0f )
        return 0;
	
    if ( dx + dy > 100.0f * SEGMENT_DIST_2 )
        return 0;
	
	int nNewLen = (strokeLen + 2 * SEGMENT4 + 1) * sizeof( CGTracePoint );
	if ( nNewLen >= strokeMemLen )
	{
		strokeMemLen += DEFAULT_STROKE_LEN * sizeof( CGTracePoint );
		ptStroke = realloc( ptStroke, strokeMemLen );
		if ( NULL == ptStroke )
			return 0;
	}
    if ( strokeLen == 1 )
    {
        ptStroke[0].pressure = pressure;
    }

    if  ( (dx + dy) < SEGMENT_DIST_1 )
    {
        ptStroke[strokeLen].pt.x = _previousLocation.x = x;
        ptStroke[strokeLen].pt.y = _previousLocation.y = y;
        ptStroke[strokeLen].pressure = pressure;
        strokeLen++;
        return  1;
    }
	
    if ( (dx + dy) < SEGMENT_DIST_2 )
        nSeg = SEGMENT2;
    else if ( (dx + dy) < SEGMENT_DIST_3 )
        nSeg = SEGMENT3;
    else
		nSeg = SEGMENT4;
    int     nPoints = 0;
    for ( register int i = 1;  i < nSeg;  i++ )
    {
        x1 = _previousLocation.x + ((x - _previousLocation.x)*i ) / nSeg;  //the point "to look at"
        y1 = _previousLocation.y + ((y - _previousLocation.y)*i ) / nSeg;  //the point "to look at"
		
        xNew = ptStroke[strokeLen-1].pt.x + (x1 - ptStroke[strokeLen-1].pt.x) / nSeg;
        yNew = ptStroke[strokeLen-1].pt.y + (y1 - ptStroke[strokeLen-1].pt.y) / nSeg;
		
        if ( xNew != ptStroke[strokeLen-1].pt.x || yNew != ptStroke[strokeLen-1].pt.y )
        {
            ptStroke[strokeLen].pt.x = xNew;
            ptStroke[strokeLen].pt.y = yNew;
            ptStroke[strokeLen].pressure = pressure;
            strokeLen++;
            nPoints++;
        }
    }
	
    if ( bLastPoint )
    {
		// add last point
        if ( x != ptStroke[strokeLen-1].pt.x || y != ptStroke[strokeLen-1].pt.y )
        {
            ptStroke[strokeLen].pt.x = x;
            ptStroke[strokeLen].pt.y = y;
            ptStroke[strokeLen].pressure = pressure;
            strokeLen++;
            nPoints++;
        }
    }
	
	_previousLocation.x = x;
    _previousLocation.y = y;
    return nPoints;
}


#pragma mark - Main thread callback methods 

- (void) recognizerTimer
{
	[self recognizeInk:YES];
}

- (void)addPointAndDraw:(CGTracePoint)tracePoint IsLastPoint:(BOOL)isLastPoint
{
	int	lenSave = strokeLen-1;	
	if ( lenSave < 0 )
	{
		return;
	}

    CGPoint point = tracePoint.pt;
	// must not contain negative coordinates
	if ( point.x < 0 )
		point.x = 0;
	if ( point.y < 0 )
		point.y = 0;
	
	if ( isLastPoint )
	{
		// make sure last point is not too far
		if ( ABS( ptStroke[lenSave].pt.x - point.x ) > 20 || ABS( ptStroke[lenSave].pt.y - point.y ) > 20 )
		{
			point = ptStroke[lenSave].pt;
		}
	}
    // TODO: if pen pressure is supported, you may change DEFAULT_PRESSURE to actual pressure value,
    // The pressure is assumed to changes between 1 (min) and 255 (mac), 150 considered to be default.
    [self AddPixels:tracePoint IsLastPoint:FALSE];
}

- (void) startTouchAndHoldTimer
{
    [self killTouchAndHoldTimer];
    
    NSTimeInterval timeout = [[NSUserDefaults standardUserDefaults] doubleForKey:kTouchAndHoldTimeout];
    if ( timeout == 0 )
        timeout = DEFAULT_TOUCHANDHOLDDELAY;
    self.timerTouchAndHold = [NSTimer scheduledTimerWithTimeInterval:timeout
                                                              target:self
                                                            selector:@selector(touchAndHoldTimer:)
                                                            userInfo:nil
                                                             repeats:NO];
}

- (void) killTouchAndHoldTimer
{
    if ( nil != self.timerTouchAndHold )
    {
        [self.timerTouchAndHold invalidate];
        self.timerTouchAndHold = nil;
    }	
}

- (void) touchAndHoldTimer:(NSTimer *)timer
{
    [self killTouchAndHoldTimer];
    if ( [inkData strokeCount] < 1 )
    {
        if ( _firstTouch || strokeLen < 5 )
        {
            // show menu
            if ( strokeLen > 0 )
            {
                [self tapAtLocation:ptStroke[0].pt];
                // TODO: may add delegate
            }
        }
        else
        {
            [self selectFrom:ptStroke[0].pt to:ptStroke[strokeLen-1].pt];
        }
        strokeLen = 0;
        _firstTouch = NO;
    }
}

- (void) processTouchAtLocation:(CGTracePoint)location
{
	[self killRecoTimer];
	strokeLen = 0;
    _firstTouch = YES;
    
    if ( [inkData strokeCount] < 1 )
    {
        [self startTouchAndHoldTimer];
    }

	_previousLocation = location.pt;
    
	if ( _bAsyncInkCollector )
	{
		if ( _inkQueueGet == _inkQueuePut )
			_inkQueueGet = _inkQueuePut = 0;
		[self addPointToQueue:location];
	}
	else
	{	
        ptStroke[0].pressure = DEFAULT_PRESSURE;
		ptStroke[0].pt = _previousLocation;
		strokeLen = 1;
	}
}

- (void) processMoveToLocation:(CGTracePoint)location
{
    if ( _firstTouch )
    {
        UInt32		nStrokeCount = [inkData strokeCount];
        if ( nStrokeCount > 0 )
            _firstTouch = NO;
        else
        {
            CGFloat dx = location.pt.x - _previousLocation.x;
            CGFloat dy = location.pt.y - _previousLocation.y;
            if ( dx*dx + dy*dy > TAP_SPACIAL_SENSITIVITY )
                _firstTouch = NO;
        }
    }
    
    [self killTouchAndHoldTimer];
    if ( ! _firstTouch && [inkData strokeCount] < 1 )
    {
        [self startTouchAndHoldTimer];
    }
    
    if ( _bAsyncInkCollector  )
    {
        [self addPointToQueue:location];
    }
    else if ( (location.pt.y != _previousLocation.y || location.pt.x != _previousLocation.x) && NULL != ptStroke )
    {		
        // if this is the first stroke, re-enable the touch timer
        [self addPointAndDraw:location IsLastPoint:FALSE];
        
    }	
}

- (void) processTouchUpAtLocation:(CGTracePoint)location touchCount:(NSInteger)touchCount
{
	UInt32		nStrokeCount = [inkData strokeCount];

    [self killTouchAndHoldTimer];
	if ( _firstTouch )
	{
		_firstTouch = NO;
		if ( nStrokeCount < 1  )
		{
            if ( strokeLen > 0 )
            {
                strokeLen = 0;
            }
            [self tapAtLocation:location.pt];
			return;
		}
		else
		{
			location.pt.x++;
		}
	}
	if ( nStrokeCount < 1 && strokeLen < 4 )
	{
		if ( strokeLen > 0 )
		{
			strokeLen = 0;
		}
		return;
	}
    
	if ( _bAsyncInkCollector )
	{
		[self addPointToQueue:location];
        location.pt = CGPointMake( 0, -1 );
		[self addPointToQueue:location];
	}
	else
	{
		[self addPointAndDraw:location IsLastPoint:TRUE];
		
		// process the new stroke
		[self processEndOfStroke:NO];
	}
}

- (void) processCancelAtLocation:(CGTracePoint)location
{
	_firstTouch = NO;
	
    // cancel current stroke
    [self killTouchAndHoldTimer];
    strokeLen = 0;
}

- (void) tapAtLocation:(CGPoint)location
{
    // TODO:
}

- (void) selectFrom:(CGPoint)location_start to:(CGPoint)location_end
{
    // TODO:
}

#pragma mark - Asyncronous Recognizer Thread

-(BOOL) startAsyncRecoThread
{
    if ( ! _useAsyncRecognizer )
        return NO;
	// make sure another recognizer thread is not already running
	[self stopAsyncRecoThread];
	
    self.currentResult = nil;
	if ( [[RecognizerManager sharedManager] isEnabled] &&  [self isInkData] )
	{
		[NSThread detachNewThreadSelector:@selector(asyncRecoThread:) toTarget:self withObject:inkData];
        return YES;
	}
    return NO;
}

-(void) stopAsyncRecoThread
{
    if ( _useAsyncRecognizer )
        HWR_StopAsyncReco( GLOBAL_RECO_HANDLE );
}

-(void) showAsyncRecoResult:(NSString *)strResult
{
	if ([self.delegate respondsToSelector:@selector(InkCollectorAsyncResultReady:theResult:)])
	{
		[self.delegate InkCollectorAsyncResultReady:self theResult:strResult];
	}
}

-(void) asyncRecoThread:(id)obj
{
	@autoreleasepool
    {
        [NSThread setThreadPriority:0.1];
        InkDataManager * ink = obj;
        if ( ink != NULL )
        {
            @synchronized( self )
            {
                NSString * text = [[RecognizerManager sharedManager] recognizeInkData:ink background:NO async:YES flipY:YES selection:NO];
                self.currentResult = text;
            }
            if ( [self.currentResult length] > 0 )
                [self performSelectorOnMainThread:@selector(showAsyncRecoResult:) withObject:self.currentResult waitUntilDone:YES];
            // exit thread, recognition completed
        }
    }
}

@end
