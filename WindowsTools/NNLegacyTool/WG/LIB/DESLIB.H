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


#ifndef _DESLIB_H
#define _DESLIB_H

#ifndef _UINT
#define _UINT unsigned int
#endif
#if 0
typedef struct
{
	_CHAR    id_string[4];           // ID - must be ALIS
	_SHORT   num_entries;            // Number of words in file
	_CHAR    creator_signature[40];  // Some descr string
	_LONG    reserved[10];           // For future use
} aliases_file_hdr_type, *p_aliases_file_hdr_type;
#endif
/* *************************************************************** */
#define MAX_TAP_WORDS       80L
#define MAX_ALIASES_MEM     64000u                /* Max memory for aliases in batch */

#define DES_OPEN  1
#define DES_PUT   2
#define DES_GET   3
#define DES_CLOSE 4
#define DES_COUNT 5

#define TAP_OPEN  1
#define TAP_CLOSE 4
#define TAP_COUNT 5
#define TAP_TRACE 6
#define TAP_CMP   7

_ULONG _des_file(_INT command, _ULONG param1, _ULONG param2);
_ULONG _tap_file(_UINT command, _ULONG param1, _ULONG param2, _UINT n);

/* *************************************************************** */
#endif /* _DESLIB_H */
