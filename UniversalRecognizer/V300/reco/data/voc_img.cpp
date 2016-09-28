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

// **************************************************************************
// *    VOC file as C file                                                  *
// **************************************************************************

#include "ams_mg.h"

#if defined INTERNAL_DICT && !defined LSTRIP
#ifdef HW_FULL
#if defined LANG_FRENCH
#include "vocimgff.cpp"
#elif defined LANG_ITALIAN
#include "vocimgff.cpp"
#elif defined LANG_GERMAN
#include "vocimgfg.cpp"
#elif defined(LANG_NORW) || defined(LANG_DAN) || defined(LANG_SPANISH) || defined(LANG_PORTUGUESE) || defined(LANG_PORTUGUESEB) || defined(LANG_DUTCH) || defined(LANG_SWED)
#include "voc_imgf.cpp"
#elif defined LANG_INTERNATIONAL
#include "voc_imgf.cpp"
#else
#include "voc_imgf.cpp"
#endif
#else
#if defined LANG_FRENCH
#include "vocimgcf.cpp"
#elif defined LANG_ITALIAN
#include "vocimgcf.cpp"
#elif defined LANG_GERMAN
#include "vocimcg.cpp"
#elif defined(LANG_NORW) || defined(LANG_DAN) || defined(LANG_SPANISH) || defined(LANG_PORTUGUESE) || defined(LANG_PORTUGUESEB) || defined(LANG_DUTCH) || defined(LANG_SWED)
#include "voc_imgc.cpp"
#elif defined LANG_INTERNATIONAL
#include "voc_imgc.cpp"
#else
#include "voc_imgc.cpp"
#endif
#endif
#endif

// **************************************************************************
// *    END OF ALL                                                          *
// **************************************************************************
