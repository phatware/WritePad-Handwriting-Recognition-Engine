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

#ifndef COMPESSEDSTROKE_H
#define COMPESSEDSTROKE_H

#include <string.h>
#include "PHStroke.h"
#include "PHTemplArray.h"

//#define _DEBUG_OUT

//
// interface definition for the compressed ink stroke
// 
/* Stroke format
 1. Header
 2. Line info
 */

/*
 Compressed Ink format:
 <HEADER>
 UCHAR   m_ucVersion[5] - 1.0
 UCHAR   m_ucFlags     - 0x01 - deltas and delta signes are stored 
 in different arrays
 UInt16  m_uStrokes    - size of the comprsssed stroke array
 UCHAR   m_Options     - TBD
 LOGPALETTE m_Palette  - needed for CE, ignored right now
 <Array of comressed strokes>
 };
 */
/*
 Stroke format:
 <HEADER - 8 bytes long><DATA>
 byte 0 - flags
 bit 0 - connect the fisrt point with last one of prev stroke 
 (set -> connect)
 bit 1 - color/width inherited from the prev stroke (set -> yes)
 bit 3,4 - compression type
 00 - old Calligrapher format
 01 - deltas are listed in the header
 10 - deltas are 3-bit values with a preceding sign
 11 - reserved
 bit 5 - straight line (data bytes are simply(short, short)
 (may need a deep forward lookup to determine a stroke split)
 byte 1 -> 4 bits for color index in the palette
 4 bits stroke width
 bytes 3,4 -> number of points in this stroke (unsigned short)
 bytes 5,6 -> X for the first point (short)
 bytes 7,8 -> Y for the first point (short)
 -> if flag indicates that deltas are listed in the header
 this is an array of UINT with two bits per point in the stroke
 minus one, 
 if the flag indicates that deltas are stored in the header
 this value is NULL,  
 Data -  deltas for X and Y, 4 bits each or 3 bit deltas depending on the flag
 if delta more than 15 -> wrap up this stroke and start the new one 
 with the remaining pixels
 */

#define TRACE_FORMAT_VERSION_V100	"1.00"
#define TRACE_FORMAT_VERSION_V101	"1.01" // adds pressure
#define TRACE_FORMAT_VERSION_V102	"1.02" // byte for color/width
#define TRACE_FORMAT_VERSION_V103	"1.03" // pressure and longer width
#define TRACE_FORMAT_VERSION_V104	"1.04" // Z-Order
#define TRACE_FORMAT_VERSION_V105	"1.05" // pressure
#define TRACE_FORMAT_VERSION_V106	"1.06" // float point width

const UCHAR TRACE_FORMAT_DELTA4BIT = 0x01;
const UCHAR TRACE_FORMAT_DELTA3BIT = 0x02;

const UCHAR WIDTH_MASK = 0x0F;
const UCHAR COLOR_MASK = 0xF0;
const UCHAR MAX_COLOR = 16;
const UCHAR MAX_WIDTH = 16;
const UCHAR LOW_MASK = 0x0F;
const UCHAR HIGH_MASK = 0xF0;

const short MAX_DELTA_3BIT = 7;
const short MIN_DELTA_3BIT = -7;

const short MAX_DELTA_4BIT = 15;
const short MIN_DELTA_4BIT = -15;

//defines the filtering box
//pixel inside this box is skipped
const UInt16 TOLERANCE = 1; 

const UInt16 FLAGS_OFFSET = 0;
const UInt16 COLORWIDTH_OFFSET = 1;
const UInt16 POINT_CNT_OFFSET = 2;

const UInt16 MAX_STROKE_POINTS = 65534;

//sign table flags
//point #0

const UCHAR X_SIGN_MASK  = 0x01;
const UCHAR Y_SIGN_MASK  = 0x02;
/*
 //point #1
 const UCHAR X1_PLUS   = 0xF;
 const UCHAR X1_MINUS  = 0xF;
 const UCHAR Y1_PLUS   = 0xF;
 const UCHAR Y1_MINUS  = 0xF;
 
 //point #2
 const UCHAR X2_PLUS   = 0x0;
 const UCHAR X2_MINUS  = 0x1;
 const UCHAR Y2_PLUS   = 0x2;
 const UCHAR Y2_MINUS  = 0x3;
 
 //point #3
 const UCHAR X3_PLUS   = 0x0F;
 const UCHAR X3_MINUS  = 0x1F;
 const UCHAR Y3_PLUS   = 0x2F;
 const UCHAR Y3_MINUS  = 0x3F;
 */

//stroke flag masks
const UCHAR CONNECT_TO_PREV_FLAG = 0x01;
//if set color and width are the same as in prev stroke
const UCHAR COLORWIDTH_PREV_FLAG = 0x02; 

//00 - Old Calligrpaher format
//01 - 4bit per coord
//10 - 3bit per corrd
//11 - reserved
const UCHAR COMPR_4BIT_FLAG = 0x04;
const UCHAR COMPR_3BIT_FLAG = 0x08;
// const UCHAR POINT_ENC_1BYTE = (4 << 1);

const UCHAR NUMPOINTS_1BYTE_FLAG = 0x10; 

const UCHAR COMPR_TYPE_MASK = 0x0C;
const UCHAR CONNECT_DELTA_FLAG = 0x20;
const UCHAR NO_POINTS_FLAG = 0x40;
const UCHAR NOT_REC_FLAG = 0x80;

//Flags for Pressure sequence
const UCHAR PRESSURE_CONST_MASK = 0x01; //no pressures for this stroke
const UCHAR PRESSURE_NORMAL_MASK = 0x02; //regular structure with packed & unpacked entries
//Pressures are stored after each stroke as follows
//If there's less than 4 points in stroke, then pressures follows in bytes
//Otherwise, if NULL_MASK flag is stored in the next byte, then all pressures = 0
//Otherwise we read pressures by words. If PRESSURE_UNPACKED_MASK then pressure is stored in low byte
//Otherwise pressures are stored in deltas, packed 3 in word (5 bits per delta).
const UCHAR PRESSURE_UNPACKED_MASK = 0x00;
const WORD	PRESSURE_PACKED_MASK = 0x8000;
struct PWCompStrokeHeader
{     
    UCHAR  ucFlags;
    UCHAR  ucWidth;
    COLORREF Color;
    UInt16 nPoints;
    short  iX0;
    short  iY0;
};

struct DELTA //can't use POINTS since delta is always positive
{
    short x;
    short y;
};

struct SUBSTROKE_INFO
{
    short  iBasePointIndex;
    short  iPointsCount;
    POINTS basePoint;
};

class PWCompressedStroke
{
public:
    PWCompressedStroke(); 
    PWCompressedStroke(const PWCompressedStroke& CmpStrk ) ;
    PWCompressedStroke(const PWCompressedStroke& , FILE * hFile ); 
    ~PWCompressedStroke();
	
    inline UCHAR GetWidth();
    inline void SetWidth( UCHAR ucWidth);
    inline void SetColor( COLORREF ucColor);
    inline COLORREF GetColor();
	
    int Create( const POINTS& pnt, const PHArray<DELTA *>&  deltaArr );
	int Read( FILE * pFile  );
    int Write( FILE * pFile );
	
    inline BOOL IsColWidthInherited();
    inline BOOL IsConnectedToPrev();
    inline void SetConnectedToPrev( BOOL bFlag);
	
    UCHAR * GetDeltaSignDataPtr() const{ return m_pDeltaSign; }
    UCHAR * GetDeltaDataPtr() const{ return m_pDelta; }
	
    void GetDeltaSign( int iIndex,  short& iSignX, short& iSignY );
    void GetDeltaValue( int iIndex, DELTA& delta  );
	
    PWCompStrokeHeader * GetHeader()  { return &m_Header; }
    
    PWCompressedStroke& operator=( const PWCompressedStroke& );
private:
    //little helpers
    void SetDeltaSign(const DELTA& delta, /*SHORT iDeltaX, SHORT iDeltaY, */int iIndex );
    void SetDeltaValue( DELTA& delta/* SHORT iDeltaX, SHORT iDeltaY*/, int iIndex );
    
    //i want to have only one palce to track these values 
    inline UInt32 GetDeltaSignArrSize(); 
    inline UInt32 GetDeltaArrSize(); 
    inline void SetDeltaSignArrSize(); 
    inline void SetDeltaArrSize(); 
    
    
    //???? would subStroke be better
    void GetChunk( UInt32 uPntCnt, PHArray<DELTA *> deltaArr );
	
private:   
    struct PWCompStrokeHeader m_Header;
    UCHAR *      m_pDeltaSign;
    UCHAR *      m_pDelta;
	
    // HANDLE    m_hFile;
    UInt32       m_DeltaSignArrSize;
    UInt32       m_DeltaArrSize;
    POINTS       m_CurrPoint;
};

#define VER_FLD_LEN             4

const DWORD PWINK_FILE_STAMP =  'CIWP';

// header options
#define FHF_TEXTOBJECTS         0x01
#define FHF_IMAGEOBJECTS        0x02
#define FHF_IMAGEROTATION       0x04
#define FHF_INKZORDER           0x08
#define FHF_INKPRESSURE         0x10
#define FHF_FLOATWIDTH          0x20

struct PHCompTraceHeader
{
	DWORD	m_dwStamp;
	char	m_ucVersion[VER_FLD_LEN+1];	
	UCHAR   m_ucFlags;
	UInt16  m_uStrokes;		// number of strokes
	UCHAR   m_Options;
	UCHAR   m_nColors;		// number of colors
};

#endif
