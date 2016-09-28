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

#define MAXPORTS        4

#define  WACOM_NAME         "WACOM"
#define  NEWTON_NAME        "NEWTON"
#define  MAX_DEVICE_NAME    7          // max of above names

typedef  struct _PORTSTRUCT
{
	char  DeviceName[MAX_DEVICE_NAME];
	int   PortNumber;
#ifdef _WIN32
	DWORD Rate;
#else
	WORD  Rate;
#endif
	int   Parity;
	int   StopBits;
} PORTSTRUCT_TYPE, FAR *p_PORTSTRUCT;
#define SETPORT(x)      (x->PortNumber)
#define SETRATE(x)      (x->Rate)
#define SETPARITY(x)    (x->Parity)
#define SETSTOPBITS(x)  (x->StopBits)
#define SETNAME(x)      (x->DeviceName)

#define  NWT_WIDTH          (239*8)
#define  NWT_HEIGHT         (335*8)

#define  WACOM_WIDTH        4500
#define  WACOM_HEIGHT       3000

#define  WACOM_FRAME_BEGIN  '#'
#define  WACOM_T_CR         '\r'
#define  WACOM_T_LF         '\n'
#define  WACOM_DBSP_PENUP   '0'
#define  WACOM_DBSP_OUTRANGE '9'

typedef  struct                            // all coordinates are positive
{
	char Begin;
	char Delim1;
	char X1;
	char X2;
	char X3;
	char X4;
	char X5;
	char Delim2;
	char Y1;
	char Y2;
	char Y3;
	char Y4;
	char Y5;
	char Delim3;
	char lo_DBSP;
	char hi_DBSP;
	char CR;
	char LF;
} WACOM_FRAME, *p_WACOM_FRAME;

typedef  struct
{
	BYTE Begin;
	BYTE HIx;
	BYTE LOx;
	BYTE HIy;
	BYTE LOy;
} NEWTON_FRAME, *p_NEWTON_FRAME;

typedef  struct
{
	BYTE Begin;
	WORD x;
	WORD y;
} MOUSE_FRAME, *p_MOUSE_FRAME;

#define  TRACECOLOR         RGB(0, 255, 0)

#define  MOUSE_TIMER_ID     40000
#define  COMM_TIMEOUT       (600)
//#define  COMM_TIMEOUT       (600)
#define  MIN_COM_INPUT      10

#define  PEN_DOWN           0xE9
#define  PEN_UP             0xE8


// we are going to fake the CN_EVENT notifications using another
// thread in Win32

#ifdef _WIN32
#define WM_COMMNOTIFY 0x0044
#define CN_EVENT 0x04
#endif

// data structures
#define MAXBLOCK        sizeof(WACOM_FRAME)*12

typedef struct tagSERIALINFO
{
#ifdef _WIN32
	HANDLE  idComDev;
#else
	int     idComDev ;
#endif
	BYTE    bPort;
	BOOL    fConnected, fUseCNReceive;
	BYTE    bByteSize, bFlowCtrl, bParity, bStopBits;
#ifdef _WIN32
	DWORD   dwBaudRate;
#else
	WORD    wBaudRate;
#endif
#ifdef _WIN32
	HANDLE      hPostEvent, hWatchThread, hWatchEvent;
	HWND        hTermWnd;
	DWORD       dwThreadID;
	OVERLAPPED  osWrite, osRead;
#endif
	char         InputBuffer[MAXBLOCK + sizeof(WACOM_FRAME)];
	int          RemBytes;
} SERIALINFO, FAR *p_SERIALINFO;

// macros ( for easier readability )

#define COMDEV(x)     (x->idComDev)
#define PORT(x)       (x->bPort)
#define CONNECTED(x)  (x->fConnected)
#define BYTESIZE(x)   (x->bByteSize)
#define PARITY(x)     (x->bParity)
#define STOPBITS(x)   (x->bStopBits)
#ifdef _WIN32
#define BAUDRATE(x) (x->dwBaudRate)
#else
#define BAUDRATE(x) (x->wBaudRate)
#endif
#define USECNRECEIVE(x)(x->fUseCNReceive)
#ifdef _WIN32
#define TERMWND(x)  (x->hTermWnd)
#define POSTEVENT(x)(x->hPostEvent)
#define HTHREAD(x)  (x->hWatchThread)
#define THREADID(x) (x->dwThreadID)
#define WRITE_OS(x) (x->osWrite)
#define READ_OS(x)  (x->osRead)
#endif
#define INPUT(x,i) (x->InputBuffer[i])
#define REMAINDER(x) (x->RemBytes)


BOOL FAR  srlClose(void);
int  FAR  srlOpenConnection(HWND hWnd, WORD DeviceID);
BOOL FAR  srlCloseConnection(WORD DeviceID);
BOOL FAR  srlProcessCOMMNotification(HWND hWnd, WPARAM wParam, LPARAM lParam);
BOOL FAR PASCAL srlSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL FAR  srlPortDlg(HWND hWnd, int id);
void FAR  srlReadPortSettings(LPSTR IniName, LPSTR SectionName);
int  FAR  srlAwaitInput(void);

#ifdef _WIN32
DWORD FAR PASCAL CommWatchProc(LPSTR Name);
#endif
void FAR srlGetMouseTrajectory(HWND hWnd, WPARAM wParam, LPARAM lParam);
void FAR srlTimeOut(HWND hWnd);

// defines for serial port setting
#define IDD_PORTCB         0x220
#define IDD_BAUDCB         0x221
#define IDD_DATABITSCB     0x222
#define IDD_PARITYCB       0x223
#define IDD_STOPBITSCB     0x224
#define IDD_DEVICENAME     0x225

#define IDS_COMPREFIX      0x310

#define IDS_BAUD110        0x320
#define IDS_BAUD300        0x321
#define IDS_BAUD600        0x322
#define IDS_BAUD1200       0x323
#define IDS_BAUD2400       0x324
#define IDS_BAUD4800       0x325
#define IDS_BAUD9600       0x326
#define IDS_BAUD14400      0x327
#define IDS_BAUD19200      0x328
#define IDS_BAUD38400      0x329
#define IDS_BAUD56000      0x32A
#define IDS_BAUD128000     0x32B
#define IDS_BAUD256000     0x32C

#define IDS_PARITYNONE     0x330
#define IDS_PARITYEVEN     0x331
#define IDS_PARITYODD      0x332

#define IDS_ONESTOPBIT     0x340
#define IDS_ONE5STOPBITS   0x341
#define IDS_TWOSTOPBITS    0x342
