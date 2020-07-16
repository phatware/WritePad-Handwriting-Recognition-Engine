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

#import <Foundation/Foundation.h>
#import "recotypes.h"
#import "gestures.h"
#import "LanguageManager.h"
#import "InkDataManager.h"

@protocol InlineRecognizerProtocol;

@interface InlineRecognizer : NSObject

@property(nonatomic, nullable) id<InlineRecognizerProtocol> delegate;
@property (nonatomic, readonly, nonnull ) CGStroke ptStroke;
@property (nonatomic, readonly ) int strokeLen;

- (void) empty;
- (BOOL) deleteLastStroke;
- (void) enableGestures:(GESTURE_TYPE)gestures whenEmpty:(BOOL)bEmpty;
- (void) recognizeNow;

- (void) processTouchAtLocation:(CGTracePoint)location;
- (void) processMoveToLocation:(CGTracePoint)location;
- (void) processTouchUpAtLocation:(CGTracePoint)location touchCount:(NSInteger)touchCount;
- (void) processCancelAtLocation:(CGTracePoint)location;

- (NSArray<NSValue *> * _Nullable) getStrokePoints:(int)nStroke;
- (int) strokeCount;

@end

@protocol InlineRecognizerProtocol<NSObject>
@optional

- (void) InkCollectorAsyncResultReady:(InlineRecognizer* _Nonnull)inkView theResult:(NSString* _Nonnull)string;
- (void) InkCollectorResultReady:(InlineRecognizer* _Nonnull)inkView theResult:(NSString* _Nonnull)string;
- (BOOL) InkCollectorRecognizedGesture:(InlineRecognizer* _Nonnull)inkView withGesture:(GESTURE_TYPE)gesture isEmpty:(BOOL)bEmpty;

@end
