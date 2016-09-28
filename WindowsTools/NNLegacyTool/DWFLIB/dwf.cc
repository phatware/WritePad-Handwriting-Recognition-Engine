/* ************************************************************************************* */
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: dwf.cc
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

#include "dwf.h"

HDWF DWF_Open(PSTRING pszFilename, long *plCharNum)
{
	HDWF	hFile;
	DWORD	nRead;

	if (!pszFilename || !plCharNum) return FALSE;
	if (!(hFile = Dio_fopen(pszFilename, L"rb"))) return FALSE;

	nRead = fread(plCharNum, 1, sizeof(long), hFile);
	if (nRead != sizeof(long)) return FALSE;

	return hFile;
}

HDWF DWF_Modify(PSTRING pszFilename, long *plCharNum)
{
	HDWF	hFile;
	DWORD	nRead;

	if (!pszFilename || !plCharNum) return FALSE;
	if (!(hFile = Dio_fopen(pszFilename, L"r+b"))) return FALSE;

	nRead = fread(plCharNum, 1, sizeof(long), hFile);
	if (nRead != sizeof(long)) return FALSE;

	return hFile;
}

HDWF DWF_Create(PSTRING pszFilename)
{
	HDWF	hFile;
	DWORD	nWrite;
	long	lCharNum = 0;

	if (!pszFilename) return FALSE;
	if (!(hFile = Dio_fopen(pszFilename, L"w+b"))) return FALSE;

	nWrite = fwrite(&lCharNum, 1, sizeof(long), hFile);
	if (nWrite != sizeof(long)) return FALSE;

	return hFile;

}

BOOL DWF_SetBlockCount(HDWF hFile, long lCharNum)
{
	size_t	nWrite;

	if (!hFile) return FALSE;
	fseek(hFile, 0, SEEK_SET);
	nWrite = fwrite((void *) &lCharNum, 1, sizeof(long), hFile);
	if (nWrite != sizeof(long)) return FALSE;
	return TRUE;
}

void DWF_Close(HDWF hFile)
{
	fclose(hFile);
}

long DWF_Seek(HDWF hFile, long lOffset, int nStartPos)
{
	if (!hFile) return -1;
	if (fseek(hFile, lOffset, nStartPos)) return -1;
	return ftell(hFile);
}

BOOL DWF_Get(HDWF hFile, PDWFBLOCK pudBlock)
{
	int i;
	size_t	nRead;

	memset(pudBlock, 0, sizeof(DWFBLOCK));
	if (!hFile || !pudBlock) return FALSE;
	nRead = fread(&(pudBlock->unCode), 1, sizeof(short), hFile);
	if (nRead != sizeof(short)) return FALSE;

	if (pudBlock->unCode == 0x20)
	{
		nRead = fread(&(pudBlock->nLetters), 1, sizeof(short), hFile);
		if (nRead != sizeof(short)) return FALSE;

		memset(pudBlock->strResult, 0, sizeof(unsigned short)*DWF_MAXSTROKE);
		nRead = fread(pudBlock->strResult, 1, sizeof(unsigned short)*pudBlock->nLetters, hFile);
		if (nRead != sizeof(unsigned short)*pudBlock->nLetters) return FALSE;
	}

	nRead = fread(&(pudBlock->nStrokes), 1, sizeof(short), hFile);
	if (nRead != sizeof(short)) return FALSE;

	if (pudBlock->nStrokes < 0 || pudBlock->nStrokes >= DWF_MAXSTROKE) //okjeong 2008/05/26 [DioInkCollector]
	{
		return FALSE;
	}

	nRead = fread(&(pudBlock->nPoints), 1, sizeof(short), hFile);
	if (nRead != sizeof(short)) return FALSE;

	if (pudBlock->nPoints < 0 || pudBlock->nPoints > DWF_MAXPOINT)
		return FALSE;

	nRead = fread(&(pudBlock->udSegment), 1, sizeof(DWFEXSEGMENT), hFile);
	if (nRead != sizeof(DWFEXSEGMENT)) return FALSE;

	nRead = fread(pudBlock->nIndex, 1, sizeof(short)*(pudBlock->nStrokes + 1), hFile);
	if (nRead != sizeof(short)*(pudBlock->nStrokes + 1)) return FALSE;

	if (pudBlock->nIndex[0] != 0)
		return FALSE;

	for (i = 0; i<pudBlock->nStrokes; i++)
	{
		if (pudBlock->nIndex[i] < 0 || pudBlock->nIndex[i] > DWF_MAXPOINT ||
			pudBlock->nIndex[i] > pudBlock->nIndex[i + 1])
			return FALSE;
	}

	nRead = fread(pudBlock->udCoord, 1, sizeof(DIOPOINT)*pudBlock->nPoints, hFile);
	if (nRead != sizeof(DIOPOINT)*pudBlock->nPoints) return FALSE;

	return TRUE;
}

BOOL DWF_Put(HDWF hFile, PDWFBLOCK pudBlock)
{
	DWORD	nWrite;

	if (!hFile || !pudBlock) return FALSE;
	if (pudBlock->nStrokes > DWF_MAXSTROKE) return FALSE;
	if (pudBlock->nPoints > DWF_MAXPOINT) return FALSE;

	nWrite = fwrite(&(pudBlock->unCode), 1, sizeof(short), hFile);
	if (nWrite != sizeof(short)) return FALSE;

	if (pudBlock->unCode == 0x20)
	{
		nWrite = fwrite(&(pudBlock->nLetters), 1, sizeof(short), hFile);
		if (nWrite != sizeof(short)) return FALSE;

		nWrite = fwrite(pudBlock->strResult, 1, sizeof(unsigned short)*pudBlock->nLetters, hFile);
		if (nWrite != sizeof(unsigned short)*pudBlock->nLetters) return FALSE;
	}

	nWrite = fwrite(&(pudBlock->nStrokes), 1, sizeof(short), hFile);
	if (nWrite != sizeof(short)) return FALSE;

	nWrite = fwrite(&(pudBlock->nPoints), 1, sizeof(short), hFile);
	if (nWrite != sizeof(short)) return FALSE;

	nWrite = fwrite(&(pudBlock->udSegment), 1, sizeof(DWFEXSEGMENT), hFile);
	if (nWrite != sizeof(DWFEXSEGMENT)) return FALSE;

	nWrite = fwrite(pudBlock->nIndex, 1, sizeof(short)*(pudBlock->nStrokes + 1), hFile);
	if (nWrite != sizeof(short)*(pudBlock->nStrokes + 1)) return FALSE;

	nWrite = fwrite(pudBlock->udCoord, 1, sizeof(DIOPOINT)*pudBlock->nPoints, hFile);
	if (nWrite != sizeof(DIOPOINT)*pudBlock->nPoints) return FALSE;

	return TRUE;
}
