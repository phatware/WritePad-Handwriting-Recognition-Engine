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

#ifndef  NO_HWR_INCLUDED
#define  NO_HWR_INCLUDED

#ifndef _INC_WINDOWS
#include <windows.h>
#endif
#ifndef _INC_WINDOWSX
#include <windowsx.h>
#endif
#include <bastypes.h>
/* if you need not defines for HWR-functions set comment for next line:
 */
#define NO_HWR



#ifdef  NO_HWR

#include <memory.h>


typedef HFILE _HFILE;

/* Origin constants for seek (parameter wOrigin in function HWRFileSeek) */
#define HWR_FILE_SEEK_SET 0
#define HWR_FILE_SEEK_CUR 1
#define HWR_FILE_SEEK_END 2

/* EOF constants */
#define HWR_FILE_EOF   ((_WORD)(_INT)-1)
#define HWR_TEXT_EOF   ((_WORD)(_INT)-1)

#define HWRFileWrite(hf, buf, cb) _lwrite(hf, buf, cb)
#define HWRFileSeek(hf, off, org) (_llseek(hf, off, org) == HFILE_ERROR ? _FALSE : _TRUE)
#define HWRFileTell(hf) _llseek(hf, 0, 1)

#else
//#include "hwr_sys.h"
//#include "hwr_file.h"
#endif /* NO_HWR */




#endif /* NO_HWR_INCLUDED */
