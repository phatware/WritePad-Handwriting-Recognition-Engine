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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <dos.h>

#include "ams_mg.h"

#include "tapfile.h"

static char FileMark [] =
{ "This file is saved by MS-DOS version of GRAFY program." };

_SHORT write_word(FILE _PTR tabf, _ULONG SavOffset, _ULONG CurrentOffset,
                  p_SHORT pX, p_SHORT pY, _SHORT ii);

/* ************************************************************************** */
/* *  Append new tab data to disk                                           * */
/* ************************************************************************** */

#define LOTWORDS        -1
#define MAX_FILE_WORDS 250

_SHORT  WriteTapData(char *name, p_SHORT pX, p_SHORT pY, _SHORT ii)
{
	FILE        _PTR tabf = _NULL;
	_ULONG        CurrentOffset, SavOffset, PageOffset;
	TAPHEADER     TapHeader;
	TAPPOINTINFO  PointInfo;
	TAPPAGE       TapPage;


	if ((tabf = fopen(name, "rb+a")) == NULL)
	{
		if ((tabf = fopen(name, "wb")) == NULL)
		{
			goto err;
		}

		/*  Now filling the TAP-file header.  */
		CurrentOffset = sizeof(TAPHEADER);
		TapHeader.size = sizeof(TAPHEADER);
		strcpy(TapHeader.label, "TAP");
		TapHeader.version = 0x0100;
		TapHeader.commentSize = strlen(FileMark) + 1;
		TapHeader.commentOffset = CurrentOffset;
		CurrentOffset += TapHeader.commentSize;
		TapHeader.hResolution = 0;
		TapHeader.vResolution = 0;
		TapHeader.xAxisOrientation = 1;
		TapHeader.yAxisOrientation = -1;
		TapHeader.samplingRate = 0;
		TapHeader.samplingDist = 0;
		TapHeader.pointSampleSize = sizeof(PS_point_type);
		TapHeader.nPointInfo = 2;
		TapHeader.pointInfoOffset = CurrentOffset;
		CurrentOffset += sizeof(TAPPOINTINFO)*TapHeader.nPointInfo;
		TapHeader.nPages = 1;
		TapHeader.firstPageOffset = CurrentOffset;

		/*  Writing the file header.  */
		if (fwrite(&TapHeader, sizeof(TAPHEADER), 1, tabf) != 1)
		{
			goto err;
		}

		/*  Writing the info string.  */
		if (fwrite(FileMark, strlen(FileMark) + 1, 1, tabf) != 1)
		{
			goto err;
		}

		/*  Filling the X PointInfo structure.  */
		PointInfo.type = 1;
		PointInfo.blockSize = 2;
		PointInfo.minValue = 0;
		PointInfo.maxValue = 32767;

		/*  Writing it.  */
		if (fwrite(&PointInfo, sizeof(TAPPOINTINFO), 1, tabf) != 1)
		{
			goto err;
		}

		/*  Filling the Y PointInfo structure.  */
		PointInfo.type = 2;

		/*  Writing it.  */
		if (fwrite(&PointInfo, sizeof(TAPPOINTINFO), 1, tabf) != 1)
		{
			goto err;
		}

		/*  Filling the page header.  */
		TapPage.size = sizeof(TAPPAGE);
		TapPage.nextPageOffset = -1L;
		TapPage.pageType = 1; /*  Text page.  */
		TapPage.nWords = 1;
		CurrentOffset += TapPage.size;
		TapPage.firstWordOffset = CurrentOffset;

		/*  Writing the page header.  */
		if (fwrite(&TapPage, sizeof(TAPPAGE), 1, tabf) != 1)
		{
			goto err;
		}

		SavOffset = -1L;
		if (write_word(tabf, SavOffset, CurrentOffset, pX, pY, ii) != SUCCESS)
		{
			goto err;
		}
	}
	else
	{
		fseek(tabf, 0, SEEK_END);
		if (ftell(tabf) == -1l)
		{
			goto err;
		}
		if (ftell(tabf) > 50000l)
		{
			goto err_lotwords;
		}

		fseek(tabf, 0, SEEK_SET);

		if (fread(&TapHeader, sizeof(TAPHEADER), 1, tabf) != 1)
		{
			goto err;
		}
		PageOffset = TapHeader.firstPageOffset;
		fseek(tabf, PageOffset, SEEK_SET);
		if (fread(&TapPage, sizeof(TAPPAGE), 1, tabf) != 1)
		{
			goto err;
		}

		if (TapPage.nWords >= MAX_FILE_WORDS)
		{
			goto err_lotwords;
		}

		TapPage.nWords++;
		fseek(tabf, 0, SEEK_END);
		CurrentOffset = ftell(tabf);
		SavOffset = TapPage.firstWordOffset;
		TapPage.firstWordOffset = CurrentOffset;
		fseek(tabf, PageOffset, SEEK_SET);
		if (fwrite(&TapPage, sizeof(TAPPAGE), 1, tabf) != 1)
		{
			goto err;
		}
		if (write_word(tabf, SavOffset, CurrentOffset, pX, pY, ii) != SUCCESS)
		{
			goto err;
		}
	}
	fclose(tabf);
	return SUCCESS;
err:
	{
		char str[80];
		sprintf(str, "Error during writing\n- %s -", sys_errlist[errno]);
		//  err_msg(str);
	}
	if (tabf)
	{
		fclose(tabf);
	}
	return UNSUCCESS;
err_lotwords:
	if (tabf)
	{
		fclose(tabf);
	}
	return LOTWORDS;
}

_SHORT write_word(FILE _PTR tabf, _ULONG SavOffset, _ULONG CurrentOffset,
                  p_SHORT pX, p_SHORT pY, _SHORT n_points)
{
	TAPWORD       TapWord;
	TAPSTROKE     TapStroke;
	_BOOL         bPenUp, bFirstPenUp, bBeginningOfStroke;
	_SHORT        nPenDownPoints, StrokeInWord;
	_SHORT        j;
	_ULONG        CurOffset = 0l;

	fseek(tabf, CurrentOffset, SEEK_SET);
	/*  Filling the word header.  */
	TapWord.size = sizeof(TAPWORD);
	TapWord.unicode = 0;
	TapWord.nTextSize = 0;
	TapWord.textOffset = 0L;
	TapWord.constraintsOffset = -1L;
	TapWord.language = 0;
	TapWord.orientation = 1;  /*  Normal script.  */
	TapWord.paperOffset = -1L;
	TapWord.firstCharOffset = -1L;
	CurrentOffset += sizeof(TAPWORD);
	TapWord.firstStrokeOffset = CurrentOffset;

	/*  Cycling along points to determine number of strokes and points. */
	TapWord.nStrokes = 0;
	bPenUp = _FALSE;
	bFirstPenUp = _FALSE;
	nPenDownPoints = 0;
	for (j = 0; j < n_points; j++)
	{
		if (pY[j] == -1 || j == 0 || j == n_points - 1)
		{
			if (bPenUp)
			{
				continue;    /*  Multiple pen ups.  */
			}
			if (!bFirstPenUp)
			{
				/*  First pen up encountered.  */
				bFirstPenUp = _TRUE;
				bPenUp = _TRUE;
				continue;
			}

			/*  Pen up after pen down.  */
			bPenUp = _TRUE;
			TapWord.nStrokes++;
		}
		else
		{
			if (!bFirstPenUp)
			{
				continue;    /*  Points before first pen up.  */
			}

			/*  Normal points.  */
			bPenUp = _FALSE;
			nPenDownPoints++;
		}
	}

	/*   Now we can calculate the size of points' data for the word  */
	/* and fill the nextWordOff field (if needed).                   */
	TapWord.nextWordOffset = SavOffset;

	/*  Now writing the word header.  */
	if (fwrite(&TapWord, sizeof(TAPWORD), 1, tabf) != 1)
	{
		goto err;
	}

	/*  Now writing the strokes.  */
	bPenUp = _FALSE;
	bFirstPenUp = _FALSE;
	bBeginningOfStroke = _TRUE;
	StrokeInWord = 0;
	for (j = 0; j < n_points; j++)
	{
		if (pY[j] == -1 || j == 0 || j == n_points - 1)
		{
			bBeginningOfStroke = _TRUE;
			if (bPenUp)
			{
				continue;    /*  Multiple pen ups.  */
			}
			if (!bFirstPenUp)
			{
				/*  First pen up encountered.  */
				bFirstPenUp = _TRUE;
				bPenUp = _TRUE;
				continue;
			}

			/*  Pen up after pen down.  */
			bPenUp = _TRUE;

			if (StrokeInWord <= TapWord.nStrokes)
			{
				TapStroke.nextStrokeOffset = CurOffset;
			}
			else
			{
				TapStroke.nextStrokeOffset = -1L;
			}

			/*  Writing the complete stroke header  */
			fseek(tabf, CurrentOffset, SEEK_SET);
			if (fwrite(&TapStroke, 8, 1, tabf) != 1)
			{
				goto err;
			}
			fseek(tabf, CurOffset, SEEK_SET);

			CurrentOffset = CurOffset;
		}
		else
		{
			if (!bFirstPenUp)
			{
				continue;    /*  Points before first pen up.  */
			}

			/*  Normal points.  */
			if (bBeginningOfStroke)
			{
				if (fwrite(&TapStroke, 8, 1, tabf) != 1)
				{
					goto err;    /*  Writing an empty header just as a placeholder.  */
				}

				bBeginningOfStroke = _FALSE;
				CurOffset = CurrentOffset + 8;
				TapStroke.nSamples = 0;
				StrokeInWord++;
			}

			bPenUp = _FALSE;
			CurOffset += 4;
			TapStroke.nSamples++;

			if (fwrite(&pX[j], 2, 1, tabf) != 1)
			{
				goto err;
			}
			if (fwrite(&pY[j], 2, 1, tabf) != 1)
			{
				goto err;
			}
		}
	}
	return SUCCESS;
err:
	return UNSUCCESS;

}
