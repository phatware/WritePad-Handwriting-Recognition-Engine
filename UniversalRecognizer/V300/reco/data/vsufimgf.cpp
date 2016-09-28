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

#if 1
_ULONG img_vocsuff[1] = {0}; // No suff dict for french for now  
#else
// ****  DICT body   ********************************************************
_ULONG img_vocsuff[118] =  
 {  
0x00020100, 0x00000000, 0x000001C4, 0x00000000, 0x000001C4, 0x000000D4, 0x000000E2, 0x000000FA, 
0x00000114, 0x0000012D, 0x0000013B, 0x00000142, 0x00000147, 0x0000014C, 0x00000151, 0x00000156, 
0x0000015B, 0x00000160, 0x00000165, 0x0000016A, 0x0000016F, 0x00000174, 0x00000179, 0x0000017E, 
0x00000183, 0x00000188, 0x0000018D, 0x00000192, 0x00000197, 0x0000019C, 0x000001A1, 0x000001A6, 
0x000001AB, 0x000001B0, 0x000001B5, 0x000001BA, 0x000001BF, 0x00400040, 0x00400040, 0x00400040, 
0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 
0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00000005, 0x65071000, 0x73726E6D, 
0x00057A74, 0xC0000000, 0x65011073, 0x05907440, 0x6F696561, 0x0110B073, 0x0005B0E9, 0x10000000, 
0x02906E01, 0x02107369, 0x02107A6E, 0x01106F65, 0x73C0B06E, 0x00000005, 0x90744000, 0x74736503, 
0xB07440B0, 0x01107A40, 0x7302106E, 0x00000574, 0x01100000, 0x40B0B06E, 0x05B0B073, 0x00000000, 
0x00057440, 0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 
0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 
0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 
0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 
0x00000000, 0x00000249, 0x00680520, 0x00680520, 0x00050000 }; 

#endif


// **************************************************************************
// *    END OF ALL                                                          *
// **************************************************************************
