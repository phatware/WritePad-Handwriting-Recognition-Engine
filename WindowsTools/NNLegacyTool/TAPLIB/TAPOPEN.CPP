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
#include "main.h"
#include "hwr_tap2.h"
#include "tapxlat.h"

static _INT  _FPREFIX TapClose(_HTAP hTap);

static _HTAP TapOpenRead(_HFILE hFile);
static _HTAP TapOpenWrite(_HFILE hFile);
static _HTAP TapOpenRdWr(_HFILE hFile);
static _HTAP TapOpenTrunc(_HFILE hFile);

static _VOID TapSetDefaultHeader(p_TAPHEADER pHeader);
static _INT isValidHeader(p_TAPHEADER pTapHeader);
static _INT TapCloseRead(p_TAP pTap);
static _INT TapCloseWrite(p_TAP pTap);

_HTAP _FPREFIX TapOpenFile(p_CHAR szBuff, _INT wOpenMode)
{
	_HFILE hFile;

	switch (wOpenMode)
	{
		case TAP_RDONLY:
			hFile = XioOpen(szBuff, XIO_RO);
			if (hFile == XIO_ERROR)
			{
				break;
			}
			return(TapOpenRead(hFile));

		case TAP_WRONLY:
			hFile = XioOpen(szBuff, XIO_RW);
			if (hFile == XIO_ERROR)
			{
				break;
			}
			return TapOpenWrite(hFile); /* append file */

		case TAP_RDWR:
			hFile = XioOpen(szBuff, XIO_RW);
			if (hFile == XIO_ERROR)
			{
				break;
			}
			return TapOpenRdWr(hFile);

		case TAP_TRUNC:
			hFile = XioOpen(szBuff, XIO_RW);
			if (hFile == XIO_ERROR)
			{
				break;
			}
			return TapOpenTrunc(hFile); /* replace old */
	}
	return _NULL;
}

int _FPREFIX TapCloseFile(_HTAP hTap)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_HFILE  hFile = pTap->hFile;

	TapUnlockHandle(hTap);

	TapClose(hTap);
	return(XioClose(hFile));
}


_INT _FPREFIX TapClose(_HTAP hTap)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc = RC_TAP_ERROR;

	switch (pTap->wOpenMode)
	{
		case TAP_RDONLY:
			rc = TapCloseRead(pTap);
			break;
		case TAP_WRONLY:
		case TAP_RDWR:
		case TAP_TRUNC:
			rc = TapCloseWrite(pTap);
			break;
	}
	TapUnlockHandle(hTap);
	return rc;
}

_INT _FPREFIX TapGetInfo(_HTAP hTap, p_TAPINFO pInfo)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc = RC_TAP_ERROR;

	if (!hTap || !pInfo)
	{
		goto err;
	}
	pInfo->version = pTap->TapHeader.version;
	pInfo->hResolution = pTap->TapHeader.hResolution;
	pInfo->vResolution = pTap->TapHeader.vResolution;
	pInfo->xAxisOrientation = pTap->TapHeader.xAxisOrientation;
	pInfo->yAxisOrientation = pTap->TapHeader.yAxisOrientation;
	pInfo->samplingRate = pTap->TapHeader.samplingRate;
	pInfo->samplingDist = pTap->TapHeader.samplingDist;
	rc = RC_TAP_OK;
err:
	TapUnlockHandle(hTap);
	return rc;
}


_INT _FPREFIX TapSetInfo(_HTAP hTap, p_TAPINFO pInfo)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc = RC_TAP_ERROR;

	if (!hTap || !pInfo)
	{
		goto err;
	}
	if (pTap->wOpenMode == TAP_RDONLY)
	{
		goto err;
	}
	/* All other modes allow writing */

	pTap->TapHeader.version = pInfo->version;
	pTap->TapHeader.hResolution = pInfo->hResolution;
	pTap->TapHeader.vResolution = pInfo->vResolution;
	pTap->TapHeader.xAxisOrientation = pInfo->xAxisOrientation;
	pTap->TapHeader.yAxisOrientation = pInfo->yAxisOrientation;
	pTap->TapHeader.samplingRate = pInfo->samplingRate;
	pTap->TapHeader.samplingDist = pInfo->samplingDist;
	rc = RC_TAP_OK;
err:
	TapUnlockHandle(hTap);
	return RC_TAP_OK;
}


_INT _FPREFIX TapGetMaxXY(_HTAP hTap, p_TAPPOINT pTapPoint)
{
	p_TAP pTap = TapLockHandle(hTap);
	pTapPoint->x = (_SHORT) pTap->XPointInfo.maxValue;
	pTapPoint->y = (_SHORT) pTap->YPointInfo.maxValue;
	TapUnlockHandle(hTap);
	return RC_TAP_OK;
}


_INT _FPREFIX TapSetMaxXY(_HTAP hTap, p_TAPPOINT pTapPoint)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc;

	/* Fill maxValues for X and Y in TAPPOINTINFO */
	pTap->XPointInfo.maxValue = pTapPoint->x;
	pTap->YPointInfo.maxValue = pTapPoint->y;

	/* Write TAPPOINTINFO into the file (just after TAPHEADER) */
	FileSeek(pTap->hFile, TAPHEADERSIZE, SEEK_SET);
	XlatPointInfo(pTap->XPointInfo);
	rc = XioWrite(pTap->hFile, (char _far *)&pTap->XPointInfo, TAPPOINTINFOSIZE);
	XlatPointInfo(pTap->XPointInfo);
	if (rc != TAPPOINTINFOSIZE)
	{
		goto Error;
	}
	XlatPointInfo(pTap->YPointInfo);
	rc = XioWrite(pTap->hFile, (char _far *)&pTap->YPointInfo, TAPPOINTINFOSIZE);
	XlatPointInfo(pTap->YPointInfo);
	if (rc != TAPPOINTINFOSIZE)
	{
		goto Error;
	}

	TapUnlockHandle(hTap);
	return RC_TAP_OK;
Error:
	TapUnlockHandle(hTap);
	return RC_TAP_ERROR;
}

static _HTAP TapOpenRead(_HFILE hFile)
{
	_HTAP   hTap;
	p_TAP   pTap;
	_INT    rt;

	if (hFile == XIO_ERROR)
	{
		return _NULL;
	}

	if ((hTap = (_HTAP) malloc(TAPDATASIZE)) == _NULL)
	{
		return _NULL;
	}
	pTap = TapLockHandle(hTap);
	_fmemset(pTap, 0, TAPDATASIZE);

	pTap->hFile = hFile;
	pTap->wOpenMode = TAP_RDONLY;

	rt = XlatReadHeader(pTap);
	if (rt != RC_TAP_OK || !isValidHeader(&(pTap->TapHeader)))
	{
		TapUnlockHandle(hTap);
		free((void *)hTap);
		return _NULL;
	}
	/* Seek to the first stroke */
	TapSeekToBegin(pTap);
	TapUnlockHandle(hTap);

	return hTap;
}


/*
 * Open TAP file for append, i.e. read the header, seek to last stroke.
 */
static _HTAP TapOpenWrite(_HFILE hFile)
{
	_HTAP   hTap;           /* hTap is a pointer to memory */
	p_TAP   pTap;

	hTap = TapOpenRead(hFile);
	if (hTap == _NULL)      /* this file is not a TAP */
	{
		return TapOpenTrunc(hFile);
	}

	pTap = TapLockHandle(hTap);
	TapSeekToEnd(pTap);
	pTap->wOpenMode = TAP_WRONLY;
	TapUnlockHandle(hTap);

	return hTap;
}


static _HTAP TapOpenRdWr(_HFILE hFile)
{
	_HTAP   hTap;           /* hTap is a pointer to memory */
	p_TAP   pTap;

	hTap = TapOpenRead(hFile);
	if (hTap == _NULL)
	{
		return TapOpenTrunc(hFile);
	}

	pTap = TapLockHandle(hTap);
	pTap->wOpenMode = TAP_RDWR;
	TapUnlockHandle(hTap);

	return hTap;
}


static _HTAP TapOpenTrunc(_HFILE hFile)
{
	_HTAP   hTap;
	p_TAP   pTap;
	_INT    wRet;

	if (hFile == XIO_ERROR)
	{
		return _NULL;
	}

	if ((hTap = (_HTAP) malloc(TAPDATASIZE)) == _NULL)
	{
		return _NULL;
	}
	pTap = TapLockHandle(hTap);
	_fmemset(pTap, 0, TAPDATASIZE);

	TapSetDefaultHeader(&(pTap->TapHeader));

	pTap->hFile = hFile;
	pTap->wOpenMode = TAP_TRUNC;

	if (XlatWriteHeader(pTap) != RC_TAP_OK)
	{
		goto Error;
	}

	/*  Filling the X PointInfo structure.  */
	pTap->XPointInfo.type = 1;
	pTap->XPointInfo.blockSize = 2;
	pTap->XPointInfo.minValue = 0;
	pTap->XPointInfo.maxValue = 32767;

	/*  Writing it.  */
	XlatPointInfo(pTap->XPointInfo);
	wRet = XioWrite(pTap->hFile, (char _far *)&pTap->XPointInfo, TAPPOINTINFOSIZE);
	XlatPointInfo(pTap->XPointInfo);
	if (wRet != TAPPOINTINFOSIZE)
	{
		goto Error;
	}

	/*  Filling the Y PointInfo structure.  */
	pTap->YPointInfo.type = 2;
	pTap->YPointInfo.blockSize = 2;
	pTap->YPointInfo.minValue = 0;
	pTap->YPointInfo.maxValue = 32767;

	/*  Writing it.  */
	XlatPointInfo(pTap->YPointInfo);
	wRet = XioWrite(pTap->hFile, (char _far *)&pTap->YPointInfo, TAPPOINTINFOSIZE);
	XlatPointInfo(pTap->YPointInfo);
	if (wRet != TAPPOINTINFOSIZE)
	{
		goto Error;
	}

	/* Begin the new page. May be it is not needed */
	TapWriteNewPage(hTap);

	return hTap;

Error:
	free( (void *)hTap);
	return _NULL;
}

static _INT isValidHeader(p_TAPHEADER pTapHeader)
{
	/* need to change to stricmp(), but no HWR equivalent */
	if (strcmp(pTapHeader->label, "TAP") == 0)
	{
		if (pTapHeader->size == TAPHEADERSIZE)
			// GIT - Apple changed version in corrector files, so...  if (pTapHeader->version == TAP_VERSION)
		{
			return _TRUE;
		}
	}
	return _FALSE;
}


static _INT TapCloseRead(p_TAP pTap)
{
	free( (void *)pTap );
	return RC_TAP_OK;
}


static _INT TapCloseWrite(p_TAP pTap)
{
	/* Write the TAP header */
	if (XlatWriteHeader(pTap) != RC_TAP_OK)
	{
		goto TapCloseError;
	}
	/* Write the TAP page descriptor */
	if (XlatWritePage(pTap) != RC_TAP_OK)
	{
		goto TapCloseError;
	}
	/* Write the TAP word descriptor */
	if (XlatWriteWord(pTap) != RC_TAP_OK)
	{
		goto TapCloseError;
	}

	free( (void *)pTap );
	return RC_TAP_OK;

TapCloseError:
	free( (void *)pTap );
	return RC_TAP_ERROR;
}

_INT _FPREFIX TapGetComment(_HTAP hTap, p_VOID zComment, int iBuffSize, p_INT piCommentSize)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc = RC_TAP_ERROR, nBytes = pTap->TapHeader.commentSize;

	*piCommentSize = nBytes;
	if (pTap->wOpenMode == TAP_WRONLY)
	{
		goto err;
	}
	if (!zComment || iBuffSize < nBytes)
	{
		goto err;
	}
	if (nBytes > 0)
	{
		FileSeek(pTap->hFile, pTap->TapHeader.commentOffset, SEEK_SET);
		rc = XioRead(pTap->hFile, (char*) zComment, nBytes);
		if (iBuffSize > nBytes)
		{
			* (((p_CHAR) zComment) + nBytes) = 0;
		}
		rc = (rc == nBytes) ? RC_TAP_OK : RC_TAP_ERROR;
	}
	else
	{
		*(p_CHAR) zComment = 0;
		rc = RC_TAP_OK;
	}
err:
	TapUnlockHandle(hTap);
	return rc;
}

_INT _FPREFIX TapSetComment(_HTAP hTap, p_VOID zComment)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    i, rc = RC_TAP_ERROR, nBytes = zComment ? (_INT)strlen((char *) zComment) : 0;

	if (pTap->wOpenMode == TAP_RDONLY)
	{
		goto err;
	}
	if (nBytes == 0)
	{
		/* remove existing comment */
		pTap->TapHeader.commentOffset = 0;
		pTap->TapHeader.commentSize = 0;
		/* the header will be written on exit */
		rc = RC_TAP_OK;
	}
	else
	{
		p_CHAR  pBuff = (p_CHAR) malloc(nBytes + 8);
		if (pBuff == _NULL)
		{
			rc = RC_TAP_EMEM;
			goto err;
		}
		_fmemmove(pBuff, zComment, nBytes);
		for (i = nBytes; i < nBytes + 8; i++)
		{
			pBuff[i] = 0;
		}
		nBytes = (nBytes + 4) & ~3; /* align to 4 byte boundary */
		FileSeek(pTap->hFile, 0, SEEK_END);
		pTap->TapHeader.commentOffset = (_ULONG)FileTell(pTap->hFile);
		pTap->TapHeader.commentSize = nBytes;
		rc = XioWrite(pTap->hFile, pBuff, nBytes);
		free( (void *)pBuff);
		rc = (rc == nBytes) ? RC_TAP_OK : RC_TAP_ERROR;
	}
err:
	TapUnlockHandle(hTap);
	return rc;
}


static _VOID TapSetDefaultHeader(p_TAPHEADER pHeader)
{
	pHeader->size = TAPHEADERSIZE;
	pHeader->version = TAP_VERSION;
	pHeader->hResolution = 0;
	pHeader->vResolution = 0;
	pHeader->xAxisOrientation = 1;
	pHeader->yAxisOrientation = (_USHORT) -1;
	pHeader->samplingRate = 0;
	pHeader->samplingDist = 0;
	pHeader->pointSampleSize = TAPPOINTSIZE;
	pHeader->commentSize = 0;
	pHeader->commentOffset = 0;
	pHeader->padding1 = 0;
	pHeader->nPointInfo = 2;
	pHeader->pointInfoOffset = TAPHEADERSIZE;
	pHeader->padding2 = 0;
	pHeader->nPages = 0;
	pHeader->firstPageOffset = (_ULONG) -1;
	strcpy(pHeader->label, "TAP");
}
