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

#include <string.h>
#include <fcntl.h>
#if TARGET_OS_MAC
#include <unistd.h>
#else
#include <io.h>
#include <sys\stat.h>
#endif
#include "xio.h"

#define SIZE          sizeof(char)
#define XIO_BUFF_SIZE 1024*SIZE

#if TARGET_OS_MAC
#define _O_BINARY   0x0
#define _O_RDONLY   O_RDONLY
#define _O_RDWR     O_RDWR
#define _O_CREAT    O_CREAT
#define _open       open
#define _close      close
#define _read       read
#define _write      write
#define _lseek      lseek
#else

#endif

static char szBuff[XIO_BUFF_SIZE / SIZE];

XIO  XioOpen(const char _far *lpFile, int nFlag)
{
	_fstrcpy((char _far*)szBuff, lpFile);

	if (nFlag == XIO_RO)
	{
		return((XIO) _open(szBuff, _O_BINARY | _O_RDONLY));
	}
	return((XIO) _open(szBuff, _O_BINARY | _O_RDWR | _O_CREAT /*, _S_IREAD | _S_IWRITE */));
}

int  XioClose(XIO xFile)
{
	return(_close(xFile));
}

int  XioRead(XIO xFile, char _far *lpBuff, unsigned int nCnt)
{
	size_t     nRet = 0;
	size_t     iRet = 0;
	size_t     iCnt = 0;

	while (1)
	{
		iCnt = (nCnt >= XIO_BUFF_SIZE ? XIO_BUFF_SIZE : nCnt);
		iRet = _read(xFile, szBuff, iCnt);

		if (iRet < 0)
		{
			return (int)iRet;
		}
		nRet += iRet;
		_fmemmove(lpBuff, (char _far *)szBuff, iRet);

		if (iRet < iCnt)
		{
			break;
		}
		if (nCnt <= XIO_BUFF_SIZE)
		{
			break;
		}

		nCnt -= XIO_BUFF_SIZE;
		lpBuff += XIO_BUFF_SIZE / SIZE;
	}
	return (int)nRet;
}

int  XioWrite(XIO xFile, char _far *lpBuff, unsigned int nCnt)
{
	size_t nRet = 0;
	size_t iRet;
	size_t iCnt;

	while (1)
	{
		iCnt = (nCnt >= XIO_BUFF_SIZE ? XIO_BUFF_SIZE : nCnt);
		_fmemmove((char _far *)szBuff, lpBuff, iCnt);

		iRet = _write(xFile, szBuff, iCnt);

		if (iRet < 0)
		{
			return (int)iRet;
		}
		nRet += iRet;

		if ( iRet < iCnt )
		{
			break;
		}
		if (nCnt <= XIO_BUFF_SIZE)
		{
			break;
		}

		nCnt -= XIO_BUFF_SIZE;
		lpBuff += XIO_BUFF_SIZE / SIZE;
	}
	return (int)nRet;
}

long XioSeek(XIO xFile, long lOff, int nOrg)
{
	long lPos = _lseek(xFile, lOff, nOrg);

	return(lPos);
}
