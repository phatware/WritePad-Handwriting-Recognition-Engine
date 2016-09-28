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

#ifndef     CDCTP_INCLUDED
#define     CDCTP_INCLUDED

#define     ENCODER_DEBUG       _TRUE
#define     DECODER_DEBUG       _TRUE

#define      RSCONST       10000   // div const for restore segment (decoder)
#define     _SCALE_M        1024L
#define     _SCALE_D          1L
#define     _Bits10            9
#define     _SHIFT_4           4
#define     _SHIFT_5           5

// Encoder  constants ------------
#define     CDD_SIZE          4096

#define     EOT                  0
#define     LE_CODE         -32768
#define     RE_CODE          32767
#define     MaxSam             128
#define     ReSam                9
#define     MaxOrder             4
#define     MaxItr              30
#define     FIX_PREC            24

// Some Other Constant -----------
#define      LONG_STROKE       0
#define     SHORT_STROKE       1
#define       THE_END          2

#define      FAR_SEG           3
#define     NEAR_SEG           4

#define     LAST_SEG           5
#define     NEXT_SEG           6

//  Encoder Decoder ERROR constant ---
#define      ERR_NO_ERR              0  // no error
#define      ERR_CODE_TABLE          1  // no codetable or not enough
#define      ERR_DCC_ALLOC           2  // not enough memory for decoder structure
#define      ERR_CDC_ALLOC           3  // not enough memory for coder structure
#define      ERR_CDB_OVERFLOW        5  // Internal Buffer Overflow

// Quantization constant - set 2  (screen)
#define     _QvStep_2          (4 * 1876L)
#define     _QvStep_3          (2 * 2133L)
#define     _QvStepE_1         (4 * 2048L)
#define     _QvStepE_2         (4 * 2048L)
#define      MaxLambda         (18119393L  /* 1.08 */)
#define      MaxError          (1324L * 1324L)
#define      MaxDensity        (4096L)

//   Debug  constant
#define      DEBUG_SEG_READY     1
#define      DEBUG_ITR_READY     2
#define      DEBUG_LONG_STROKE   4
#define      DEBUG_LAST_ITR      8

typedef   struct
{
	_SHORT   x;
	_SHORT   y;
} _POINT;

#ifndef    _LPPOINT
#define    _LPPOINT  _POINT _PTR
#endif

#ifndef    p_POINT
#define    p_POINT   _POINT _PTR
#endif

#ifndef    _LPSTR
#define    _LPSTR    p_CHAR
#endif

#ifndef    _BREAK
#define    _BREAK    -1
#endif

typedef  struct
{
	_SHORT   orig_code;   //   source code
	_USHORT  code_len;    //   length of code word
	_ULONG   code_word;   //   code word
}  _CODEWORD, _PTR  p_CODEWORD;

typedef struct
{
	_USHORT    SZ;          //   size in bytes
	_SHORT     N;          //   number of lines in code table
	_SHORT    LB;          //   left border value
	_SHORT    RB;          //   right border value
	_SHORT    RE;          //   right escape
	_SHORT    LE;          //   left escape
	_CODEWORD W[1];
}   _CODETABLE, _PTR  p_CODETABLE;

typedef  struct
{
	_LONG    x;       //   X - coordinate for this point
	_LONG    y;       //   Y - coordinate for this point
	_LONG   dx;       //   distance between prev. and cur. point by X
	_LONG   dy;       //   distance between prev. and cur. point by Y
	_ULONG    s;       //   speed in this point on curve
	_ULONG    r;       //   road  in this point on curve
}  _ORG_P, _PTR _LPORG_P, _PTR  p_ORG_P;

typedef  struct
{
	_LONG    x;       //   X - coordinate for this point
	_LONG    y;       //   Y - coordinate for this point
	_ULONG    s;       //   speed in this point on curve
	_ULONG    r;       //   road  in this point on curve
}  _RPR_P, _PTR _LPRPR_P, _PTR  p_RPR_P;

typedef struct
{
	_LONG    x;       //   X - coordinate for this point
	_LONG    y;       //   Y - coordinate for this point
}  _L_POINT, _PTR  p_L_POINT;

typedef struct
{
	_L_POINT    LEnd;      //   Left  End of Segment
	_L_POINT    REnd;      //   Right End of Segment
	_L_POINT    Sp[4];     //   Spectrum for Segment
}  _SEG, _PTR _LPSEG, _PTR p_SEG;

typedef _VOID(_FPREFIX  *_LPDEBUG)(_ULONG);

typedef struct tag_CDC
{
	_RPR_P          Adata[ReSam];
	_RPR_P          Rdata[ReSam];
	_LONG          Spdata[2][4];
	_LONG          Spdata_D[2][4];
	_USHORT          OdataSam;
	_ORG_P          Odata[MaxSam];
	_RPR_P          CPPos_Cod;
	_USHORT          PrevPen;
	_POINT          PrevPoint;
	_USHORT          ScMul;
	_USHORT          ScDiv;
	_USHORT          ItrNum1;
	_USHORT          ItrNum2;
	_LONG          QvStepE_1;
	_LONG          QvStepE_2;
	_LONG          QvStep2;
	_LONG          QvStep3;
	_ULONG          MaxErr;
	_ULONG          CurErr;
	_LONG          MaxLam;
	_LONG          CurLam;
	_BOOL          fFirstStroke;
	_BOOL          fFirstSeg;
	_BOOL          fNewSeg;
	_SHORT          QvX[4];
	_SHORT          QvY[4];
	p_CODETABLE    pFarDX;
	p_CODETABLE    pFarDY;
	p_CODETABLE    pNearDX;
	p_CODETABLE    pNearDY;
	p_CODETABLE    pSpX2;
	p_CODETABLE    pSpY2;
	p_CODETABLE    pSpX3;
	p_CODETABLE    pSpY3;
	_USHORT          ErrCode;
	_LPDEBUG          lpfDebug;
	_LONG          fDebug;
	_LONG          CoderBufMaxPos;
	_LONG          CoderBufCurPos;            // bits
	_UCHAR          CodedData[CDD_SIZE];
} _CDC, _PTR _LPCDC, _PTR p_CDC;

typedef  struct tag_DCC
{
	_USHORT           ScMul;
	_USHORT           ScDiv;
	_LONG           KvStepE_1;
	_LONG           KvStepE_2;
	_LONG           KvStep2;
	_LONG           KvStep3;
	p_CODETABLE      pFarDX;
	p_CODETABLE      pFarDY;
	p_CODETABLE      pNearDX;
	p_CODETABLE      pNearDY;
	p_CODETABLE      pSpX2;
	p_CODETABLE      pSpY2;
	p_CODETABLE      pSpX3;
	p_CODETABLE      pSpY3;
	_USHORT           ErrCode;
	_L_POINT          CPPos;
	p_VOID           pCodedData;
	_SHORT            PointN;
	_L_POINT          SegmentEnd;
	_BOOL             fNewStroke;
	_BOOL             fLastSeg;
	_SHORT            StrokeFormat;
	_POINT            SegmentArray[66];     // 0-LEnd, 1,...,64, 65 - REnd
	_SHORT            LastPnt;            // should be less then 65
	_SHORT            CurrPnt;            //
	_LONG             DecoderBufEndPos;
	_LONG             DecoderBufCurPos;
	_BOOL             fFirstStroke;
#if   DECODER_DEBUG
	_USHORT            SegmentPoints;
	_LPDEBUG           lpfDebug;
#endif
}  _DCC, _PTR _LPDCC, _PTR p_DCC;

#define   IP_TO_TP(X,scMul,scDiv) (((_LONG)X * scDiv)/scMul)
#define   TP_TO_IP(X,scMul,scDiv) (((_LONG)X * scMul)/scDiv)
#define   BIT_TO_BYTE(X) (((X)%8 == 0) ? ((X)/8) : ((X)/8 + 1))
#define   NEXT_TABLE(pTbl) ((p_CODETABLE)((p_UCHAR) pTbl + pTbl-> SZ))

#endif
