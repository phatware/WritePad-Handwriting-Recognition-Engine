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

#ifndef PR_PEGREC_P_H_INCLUDED
#define PR_PEGREC_P_H_INCLUDED

/* ------------------------- Defines ---------------------------------------- */

#ifdef PEGREC_DEBUG

#define PRP_01                                                                  \
                                                                                \
  static prp_num_words_in_rec = 0;                                              \
  void PegDebugPrintf(char * format, ...);

#define PRP_02                                                                  \
                                                                                \
  PegDebugPrintf("Got Stroke! %d points\n", npoints);                           \
  if (npoints == 0) PegDebugPrintf("========================\n", npoints);      \
 
#define PRP_03                                                                  \
                                                                                \
PegDebugPrintf("Good tentative word! %d points, numbrer %d\n", len, pri->wswi.nword);

#define PRP_04                                                                  \
                                                                                \
PegDebugPrintf("Validated tentative word! %d points, numbrer %d\n", len, pri->wswi.nword);

#define PRP_05                                                                  \
                                                                                \
PegDebugPrintf("Continue Recognition! %d points, word number %d\n", len, pri->wswi.nword);

#define PRP_06                                                                  \
                                                                                \
PegDebugPrintf("Can't Continue Recognition! %d points, word number %d\n", len, pri->wswi.nword);

#define PRP_07                                                                  \
                                                                                \
if (pri->rc.p_xd_data == 0) prp_num_words_in_rec ++;                            \
PegDebugPrintf("Start Recognition! %d points, word number %d, Global word num: %d\n", len, pri->wswi.nword, prp_num_words_in_rec);

#define PRP_08                                                                  \
                                                                                \
PegDebugPrintf("End Recognition! %d return code.\n", er);

#else // ------------------------------------------

#define PRP_01
#define PRP_02
#define PRP_03
#define PRP_04
#define PRP_05
#define PRP_06
#define PRP_07
#define PRP_08

#endif

/* ------------------------- Structures ------------------------------------- */


/* ------------------------- Prototypes ------------------------------------- */



#endif /* PR_PEGREC_H_INCLUDED */
/* ************************************************************************** */
/* *   Head functions of Pegasus recognizer header end                      * */
/* ************************************************************************** */
