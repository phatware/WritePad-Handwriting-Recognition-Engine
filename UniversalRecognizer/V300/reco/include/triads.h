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

#ifndef TRIADS_H_INCLUDED
#define TRIADS_H_INCLUDED

#include "ams_mg.h"
#include "hwr_sys.h"

/* -------------------------- Defines for Triads ----------------------------- */

#define TR_NUMSYMBOLS     40

#define TR_OBJTYPE      "Tr#3"                  /* Object type of current TR */
#define TR_VER          "1.00"                  /* Object type of current TR */
#define TR_ID_LEN          4                    /* Length of ID string in TR */
#define TR_FNAME_LEN      128                   /* Pathname of dti len        */

#define TR_NUMINBYTE       4                    /* Number of triad values in one byte */
#define TR_GETVALUE(ptrd, a, b, c)  (((*(ptrd))[(a)][(b)][(c)/4] >> (((c)%4)*2)) & 0x03) /* Get freq value from triads */

RECO_DATA_EXTERNAL _UCHAR triads_mapping_ge[256];
//Ant: ?WHAT? #error Check coding -- it is MAC now! 
RECO_DATA_EXTERNAL _UCHAR triads_mapping_da_no_se[256];
RECO_DATA_EXTERNAL _UCHAR triads_mapping_it_fr_po[256];
RECO_DATA_EXTERNAL _UCHAR triads_mapping_default[256];

/* -------------------------- Types definitions ------------------------------ */

typedef _UCHAR  triad_el_type[TR_NUMSYMBOLS/TR_NUMINBYTE];  /* There are *some* tr in byte ! */
typedef triad_el_type triad_type[TR_NUMSYMBOLS][TR_NUMSYMBOLS];
typedef triad_type _PTR p_triad_type;

typedef struct
{                                 /* Header of Triads file      */
    _CHAR  object_type[TR_ID_LEN];   /* Type file (now Tr#2)       */
    _CHAR  type[TR_ID_LEN];          /* Type of TR (Eng., )        */
    _CHAR  version[TR_ID_LEN];       /* Version number             */
    
    _ULONG tr_offset;                /* File offset of TR part    */
    _ULONG tr_len;                   /* Length of TR part of data */
    _ULONG tr_chsum;                 /* Checksum of TR part       */
} tr_header_type, _PTR p_tr_header_type;

typedef struct
{                                 /* TR memory descriptor       */
    _CHAR  tr_fname[TR_FNAME_LEN];   /* File name of loaded TR     */
    _CHAR  object_type[TR_ID_LEN];   /* Type file (now Tr#1)       */
    _CHAR  type[TR_ID_LEN];          /* Type of TR (Eng., ..)      */
    _CHAR  version[TR_ID_LEN];       /* Version number             */
    
    _HANDLE h_tr;                     /* Handle of tr memory       */
    p_UCHAR p_tr;                     /* Pointer to locked tr mem  */
    _ULONG tr_chsum;                 /* CheckSum of tr memory     */
} tr_descr_type, _PTR p_tr_descr_type;

/* -------------------------- Prototypes ------------------------------------- */

_INT  triads_load(p_CHAR tradsname, _INT what_to_load, p_VOID _PTR tp);
_INT  triads_unload(p_VOID _PTR tp);
_INT  triads_save(p_CHAR fname, _INT what_to_save, p_VOID tp);
_INT  triads_lock(p_VOID tp);
_INT  triads_unlock(p_VOID tp);

_UCHAR triads_get_mapping(int lang, int index);

RECO_DATA p_tr_descr_type triads_get_header(_INT lang);  
RECO_DATA p_ULONG triads_get_body(_INT lang);


#endif //TRIADS_H_INCLUDED

