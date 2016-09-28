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
#include "vocutilp.h"

// ****  DICT body   ********************************************************
_ULONG img_vocsuff[98] =
 {
0x00020100, 0x00000000, 0x00000175, 0x00000175, 0x000000D0, 0x000000D8, 0x000000DF, 0x000000E4,
0x000000E9, 0x000000EE, 0x000000F3, 0x000000F8, 0x000000FD, 0x00000102, 0x00000107, 0x0000010C,
0x00000111, 0x00000116, 0x0000011B, 0x00000120, 0x00000125, 0x0000012A, 0x0000012F, 0x00000134,
0x00000139, 0x0000013E, 0x00000143, 0x00000148, 0x0000014D, 0x00000152, 0x00000157, 0x0000015C,
0x00000161, 0x00000166, 0x0000016B, 0x00000170, 0x00400040, 0x00400040, 0x00400040, 0x00400040,
0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040,
0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00000005, 0x27011000, 0x00000005, 0x05734000,
0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500,
0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000,
0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000,
0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005,
0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x77777700, 0x77777777, 0x77777777,
0x77777777 };

// **************************************************************************
// *    END OF ALL                                                          *
// **************************************************************************
