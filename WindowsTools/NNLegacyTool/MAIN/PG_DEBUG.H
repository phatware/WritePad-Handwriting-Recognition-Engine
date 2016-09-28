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

#ifndef _PG_DBG_INCLUDE_
#define _PG_DBG_INCLUDE_

/*  Grafy LAB application calls with windows */

/* bios.h definitions */
int bioskey(int cmd);
long biostime(int cmd, long newtime);
/* graphics.h definitions */
#ifndef BGI_DEFS
#define BGI_DEFS
enum COLORS
{
	BLACK,          /* dark colors */
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	BROWN,
	LIGHTGRAY,
	DARKGRAY,           /* light colors */
	LIGHTBLUE,
	LIGHTGREEN,
	LIGHTCYAN,
	LIGHTRED,
	LIGHTMAGENTA,
	YELLOW,
	WHITE
};

enum  BGI_EGA_COLORS
{
	EGA_BLACK = 0,      /* dark colors */
	EGA_BLUE = 1,
	EGA_GREEN = 2,
	EGA_CYAN = 3,
	EGA_RED = 4,
	EGA_MAGENTA = 5,
	EGA_BROWN = 20,
	EGA_LIGHTGRAY = 7,
	EGA_DARKGRAY = 56,     /* light colors */
	EGA_LIGHTBLUE = 57,
	EGA_LIGHTGREEN = 58,
	EGA_LIGHTCYAN = 59,
	EGA_LIGHTRED = 60,
	EGA_LIGHTMAGENTA = 61,
	EGA_YELLOW = 62,
	EGA_WHITE = 63
};

enum line_styles        /* Line styles for get/setlinestyle */
{
	SOLID_LINE = 0,
	DOTTED_LINE = 1,
	CENTER_LINE = 2,
	DASHED_LINE = 3,
	USERBIT_LINE = 4,   /* User defined line style */
};

enum line_widths        /* Line widths for get/setlinestyle */
{
	NORM_WIDTH = 1,
	THICK_WIDTH = 3,
};

enum fill_patterns          /* Fill patterns for get/setfillstyle */
{
	EMPTY_FILL,             /* fills area in background color */
	SOLID_FILL,             /* fills area in solid fill color */
	LINE_FILL,              /* --- fill */
	LTSLASH_FILL,           /* /// fill */
	SLASH_FILL,             /* /// fill with thick lines */
	BKSLASH_FILL,           /* \\\ fill with thick lines */
	LTBKSLASH_FILL,         /* \\\ fill */
	HATCH_FILL,             /* light hatch fill */
	XHATCH_FILL,            /* heavy cross hatch fill */
	INTERLEAVE_FILL,        /* interleaving line fill */
	WIDE_DOT_FILL,          /* Widely spaced dot fill */
	CLOSE_DOT_FILL,         /* Closely spaced dot fill */
	USER_FILL               /* user defined fill */
};
#endif /* BGI_DEFS */

void bar(int left, int top, int right, int bottom);
int getmaxx(void);
int getmaxy(void);
void outtextxy(int x, int y, const char * textstring);
void putpixel(int x, int y, int color);
void setcolor(int color);
void setfillstyle(int pattern, int color);


#if DOS_GRAFY /* bigor 7-Apr-93 */
void draw_init (_SHORT step,_SHORT x1,_SHORT y1,_SHORT x2,_SHORT y2);
#else /* !DOS_GRAFY */
_VOID draw_init(_SHORT step,
                _SHORT x1, _SHORT y1, _SHORT x2, _SHORT y2,
                p_SHORT xp, p_SHORT yp, _SHORT np);
#endif /* DOS_GRAFY */

_SHORT draw_line(_INT xbeg, _INT ybeg, _INT xend, _INT yend,
                 _INT color, _INT style, _INT thickness);
_VOID draw_arc(_INT color, p_SHORT xx, p_SHORT yy,
               _INT ibeg, _INT iend);
_VOID draw_wait(p_VOID specl,  // p_SPECL
                p_SHORT xx, p_SHORT yy, _INT nn,
                p_ULONG lowrastr, p_ULONG hirastr,
                _INT nxrastr);
_VOID  DrawBox(p_RECT pBox, _INT color, _INT style);
_VOID  OpenTextWindow(_INT lines);
_VOID  CloseTextWindow(_VOID);
_INT   printw(const p_CHAR fmt_str, ...);
_INT   SetTalk(_BOOL bOn);
_SHORT scanw(p_CHAR inbuf);
_SHORT brkeyw(p_CHAR outstr);
_VOID  CheckPause(_VOID);
int    SetTesterMark(void);

int    delay(int ticks);
int    sleep(int seconds);

/* avp.h definitions */
extern _SHORT mpr;
extern int look;

#define NOPRINT             0              /* Look definitions for xrw     */
#define WORDPRINT           1              /* Look definitions for xrw     */
#define VOCWPRINT           2              /* Look definitions for xrw     */
#define MATRPRINT           3              /* Look definitions for xrw     */
#define LCUTPRINT           4              /* Look definitions for xrw     */
#define CSTMPRINT           5              /* Look definitions for xrw     */
#define XRLPRINT            6              /* Look definitions for look    */
#define XRCPRINT            7              /* Look definitions for look    */

#define pause delay

#define DES_CUSTOM_MARK   0x0001           /* Bit for tester to search */

_INT   gprintf(_INT x, _INT y, _INT color, _INT spc, const p_CHAR fmt_str, ...);

#if DOS_GRAFY
_SHORT put_xr (xrinp_type xr, _SHORT x, _SHORT y, _SHORT inc);
#else
_SHORT put_xr(xrinp_type xr, _SHORT inc);
#endif

_SHORT WStub(_VOID);

void gotoxy(int x, int y);
void set_timer(void);
void gp_ega(_SHORT x, _SHORT y, _SHORT color);

/* additional functions to collect recognizer data */
_VOID DbgFillGRData(p_VOID p_rwg, p_VOID pRWS, p_VOID pPPDData, _SHORT AfterPP);
_VOID DbgFillXRData(xrdata_type _PTR xrdata);
_VOID DbgFillHLData(rc_type _PTR prc,
                    xrdata_type(_PTR xrdata)[XRINP_SIZE],
                    p_VOID answr, //answ_type (_PTR answr)[NUM_RW],
                    rec_w_type(_PTR rec_words)[NUM_RW]);
_VOID DbgFillRWData(rec_w_type(_PTR rec_words)[NUM_RW]);
_VOID DbgFillAWData(p_VOID answers); //answ_type(_PTR answers)[NUM_RW]);
_VOID DbgFillTraceData(PS_point_type _PTR pts, _SHORT np);
_VOID DbgFillFlTraceData(PS_point_type _PTR pts, _SHORT np);
_VOID DbgFillBorderData(p_USHORT pts, _SHORT np);
_VOID DbgFillGraphData(rc_type _PTR prc, xrdata_type _PTR xrdata, p_VOID p_rwg);

#if USE_CHUNK_PROCESSOR
_VOID DbgFillChunkData(p_VOID pChunkData);
_VOID DbgLetterPos(p_VOID pLetterPos);
#endif /* USE_CHUNK_PROCESSOR */

_VOID DbgGetCMPWord(rec_w_type(_PTR rec_words)[NUM_RW]);

/* New Wave debugging */
_VOID dbgDrawInit(_INT left, _INT top, _INT right, _INT bottom);
_VOID dbgAddTrace(PS_point_type _PTR p_trace, _SHORT num_points);
_VOID dbgColorTrace(_SHORT beg, _SHORT end, _SHORT color);
_VOID dbgAddRaster(p_ULONG lRaster, p_ULONG hRaster, _SHORT width);
_VOID dbgAddBox(_RECT box, _INT col, _INT col1, _INT width);

#endif /* _PG_DBG_INCLUDE_ */
