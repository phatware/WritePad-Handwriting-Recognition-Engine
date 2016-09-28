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

#ifndef _RECO_VER_H_INCLUDED
#define _RECO_VER_H_INCLUDED

#include "recodefs.h"

// ------------ Configuration -------------------------------------------
          
// -------------- Rec engine defines -------------------------------------------

#define VER_DTI_COMPRESSED		0  // Include full blown DTI in engine build
#define VER_SNN_COMPRESSED		1  // Include expanded SNN net
#define VER_DICT_RECODICT		1  // Include all dictionary-related functions in the Rec engine (file and spell func)

// ------------------------ Redefines and errors -------------------------------
//--------- Support redifintions - no user serviceable defines inside! ---------
// -----------------------------------------------------------------------------

//------ Rec engine switches ---------------------------------------------------

#if VER_DTI_COMPRESSED
#define DTI_COMPRESSED_ON
#endif

#if !VER_SNN_COMPRESSED
#define SNN_FAT_NET_ON
#endif 

#if VER_DICT_RECODICT && !defined(RECODICT)
#define RECODICT
#endif

#endif // _RECO_VER_H_INCLUDED

