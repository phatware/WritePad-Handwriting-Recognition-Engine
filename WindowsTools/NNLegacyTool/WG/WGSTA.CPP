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

#define STRICT
#define _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#ifdef _PENWIN
#include <penwin.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgsta.h"

/* local functions */
int CheckMenu(void);
int CheckWinMail(void);

BOOL    StatusRecognizing;
BOOL    StatusDebugWndOpened;
BOOL    StatusFirstWordFromRec;
BOOL    StatusMail;
BOOL    StatusOnLineInkInitRec;
BOOL    StatusWaitTransmition;
BOOL    StatusSlowDraw;
BOOL    StatusZoomed;
BOOL    StatusZoomAllowed;

/**********************************************************************/
int FAR staInit(void)
{
	StatusMail = CheckWinMail();
	staSetStatus(ST_RECOGNIZING, MAKELONG(FALSE, 0));
	staSetStatus(ST_DEBUGWNDOPENED, MAKELONG(FALSE, 0));
	staSetStatus(ST_FIRSTWORDFROMREC, MAKELONG(FALSE, 0));
	staSetStatus(ST_MAIL, MAKELONG(StatusMail, 0));
	staSetStatus(ST_ONLINEINITREC, MAKELONG(FALSE, 0));
	staSetStatus(ST_WAIT_TRANSMITION, MAKELONG(TRUE, 0));
	staSetStatus(ST_SLOW_DRAW, MAKELONG(FALSE, 0));
	staSetStatus(ST_ZOOM, MAKELONG(FALSE, 0));
	staSetStatus(ST_ZOOM_ALLOWED, MAKELONG(FALSE, 0));
	return 0;
} /* end of staInit */

/**********************************************************************/

DWORD FAR staSetStatus(int Request, LONG lParam)
{
	if (Request == ST_RECOGNIZING)
	{
		StatusRecognizing = (BOOL) lParam;
	}
	else
		if (Request == ST_DEBUGWNDOPENED)
		{
			StatusDebugWndOpened = (BOOL) lParam;
		}
		else
			if (Request == ST_FIRSTWORDFROMREC)
			{
				StatusFirstWordFromRec = (BOOL) lParam;
			}
			else
				if (Request == ST_ONLINEINITREC)
				{
					StatusOnLineInkInitRec = (BOOL) lParam;
				}
				else
					if (Request == ST_WAIT_TRANSMITION)
					{
						StatusWaitTransmition = (BOOL) lParam;
					}
					else
						if (Request == ST_SLOW_DRAW)
						{
							StatusSlowDraw = (BOOL) lParam;
						}
						else
							if (Request == ST_ZOOM)
							{
								StatusZoomed = (BOOL) lParam;
							}
							else
								if (Request == ST_ZOOM_ALLOWED)
								{
									StatusZoomAllowed = (BOOL) lParam;
								}
								else
									if (Request == ST_MAIL)
									{
										if ((BOOL) lParam == FALSE)
										{
											StatusMail = (BOOL) lParam;
										}
										else
										{
											StatusMail = CheckWinMail();
										}
									}
	CheckMenu();
	return 0;
} /* end of staSetStatus */

/**********************************************************************/

DWORD FAR staGetStatus(int Request, LONG lParam)
{
	if (Request == ST_RECOGNIZING)
	{
		*((BOOL FAR *)lParam) = StatusRecognizing;
	}
	else
		if (Request == ST_DEBUGWNDOPENED)
		{
			*((BOOL FAR *)lParam) = StatusDebugWndOpened;
		}
		else
			if (Request == ST_FIRSTWORDFROMREC)
			{
				*((BOOL FAR *)lParam) = StatusFirstWordFromRec;
			}
			else
				if (Request == ST_ONLINEINITREC)
				{
					*((BOOL FAR *)lParam) = StatusOnLineInkInitRec;
				}
				else
					if (Request == ST_MAIL)
					{
						*((BOOL FAR *)lParam) = StatusMail;
					}
					else
						if (Request == ST_WAIT_TRANSMITION)
						{
							*((BOOL FAR *)lParam) = StatusWaitTransmition;
						}
						else
							if (Request == ST_SLOW_DRAW)
							{
								*((BOOL FAR *)lParam) = StatusSlowDraw;
							}
							else
								if (Request == ST_ZOOM)
								{
									*((BOOL FAR *)lParam) = StatusZoomed;
								}
								else
									if (Request == ST_ZOOM_ALLOWED)
									{
										*((BOOL FAR *)lParam) = StatusZoomAllowed;
									}
	return 0;
} /* end of staGetStatus */
/**********************************************************************/

int CheckMenu(void)
{
	UINT    MenuStatus;


	/* Main Menu */
	/* IDM_EXIT       always on */
	/* IDM_PREFERNCES always on */
	if (StatusRecognizing != 0 || StatusWaitTransmition == 0)
	{
		MenuStatus = MF_GRAYED;
	}
	else
	{
		MenuStatus = MF_ENABLED;
	}
	EnableMenuItem(hMainMenu, IDM_INSTALL_REC, MenuStatus);
	EnableMenuItem(hMainMenu, IDM_OPEN_TAP, MenuStatus);
	if (StatusDebugWndOpened != 0 || StatusWaitTransmition == 0)
	{
		MenuStatus = MF_GRAYED;
	}
	else
	{
		MenuStatus = MF_ENABLED;
	}
	EnableMenuItem(hMainMenu, IDM_OPEN_INKOUTPUT, MenuStatus);

	/* TAP Menu */
	/* IDM_EXIT         always on */
	/* IDM_CLOSE_DOC    always on */
	/* IDM_PREFERNCES   always on */
	/* IDM_REC_CONFIG   always on */
	/* IDM_CASCADE_DOC  always on */
	/* IDM_TILE_DOCV    always on */
	/* IDM_TILE_DOCH    always on */
	/* IDM_ARRANGE_DOC  always on */
	/* IDM_CLOSEALL_DOC always on */
	if (StatusMail == 0 || StatusWaitTransmition == 0)
	{
		MenuStatus = MF_GRAYED;
	}
	else
	{
		MenuStatus = MF_ENABLED;
	}
	EnableMenuItem(hTAPMenu, IDM_MAIL_TAP, MenuStatus);
	if (StatusDebugWndOpened != 0 || StatusWaitTransmition == 0)
	{
		MenuStatus = MF_GRAYED;
	}
	else
	{
		MenuStatus = MF_ENABLED;
	}
	EnableMenuItem(hTAPMenu, IDM_OPEN_INKOUTPUT, MenuStatus);
	if (StatusRecognizing != 0 || StatusWaitTransmition == 0)
	{
		MenuStatus = MF_GRAYED;
	}
	else
	{
		MenuStatus = MF_ENABLED;
	}
	EnableMenuItem(hTAPMenu, IDM_INSTALL_REC, MenuStatus);
	EnableMenuItem(hTAPMenu, IDM_OPEN_TAP, MenuStatus);
	EnableMenuItem(hTAPMenu, IDM_REC_RECOGNIZE, MenuStatus);
	EnableMenuItem(hTAPMenu, IDM_REC_UPPER, MenuStatus);
	EnableMenuItem(hTAPMenu, IDM_REC_WORDCUT, MenuStatus);
	EnableMenuItem(hTAPMenu, IDM_REC_SAVEDESFILE, MenuStatus);

	if (StatusZoomAllowed)
	{
		if (StatusZoomed == 0)
		{
			MenuStatus = MF_GRAYED;
		}
		else
		{
			MenuStatus = MF_ENABLED;
		}
		EnableMenuItem(hInkOutputMenu, IDM_NO_ZOOM, MenuStatus);
		EnableMenuItem(hInkOutputMenu, IDM_ZOOM_OUT, MenuStatus);
		EnableMenuItem(hInkOutputMenu, IDM_ZOOM_IN, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hInkOutputMenu, IDM_NO_ZOOM, MF_GRAYED);
		EnableMenuItem(hInkOutputMenu, IDM_ZOOM_OUT, MF_GRAYED);
		EnableMenuItem(hInkOutputMenu, IDM_ZOOM_IN, MF_GRAYED);
	}
	if (StatusDebugWndOpened != 0 || StatusWaitTransmition == 0)
	{
		MenuStatus = MF_GRAYED;
	}
	else
	{
		MenuStatus = MF_ENABLED;
	}
	EnableMenuItem(hTAPMenu, IDM_OPEN_INKOUTPUT, MenuStatus);
	if (StatusRecognizing != 0 || StatusWaitTransmition == 0)
	{
		MenuStatus = MF_GRAYED;
	}
	else
	{
		MenuStatus = MF_ENABLED;
	}
	EnableMenuItem(hInkOutputMenu, IDM_INSTALL_REC, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_OPEN_TAP, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_SAVE_TAP, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_INK_RECORDING, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_REC_RECOGNIZE, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_REC_UPPER, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_EDIT_INIFILE, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_PREV_WORD, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_NEXT_WORD, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_COR_MATRIX, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_WORD_LAYOUT, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_V_FUNCTION, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_V2_FUNCTION, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_XREDIT, MenuStatus);
	EnableMenuItem(hInkOutputMenu, IDM_READ_WACOM, MenuStatus);
	return 0;
} /* end of CheckMenu */

/**********************************************************************/
int CheckWinMail(void)
{
	return TRUE;//GetProfileInt("MAIL", "MAPI", 0);
} /* end of CheckWinMail */

