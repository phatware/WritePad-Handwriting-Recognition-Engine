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

ROM_DATA _ULONG img_vocsuff[53] =  
{  
    0x30312E31, 0x00000001, 0x000000B0, 0x00000010, 0x00000002, 0x00000068, 0x00000000, 0x0000006E, 
    0x00000001, 0x00000074, 0x00000003, 0x0000007A, 0x00000005, 0x00000080, 0x00000007, 0x00000085,
    0x00000008, 0x0000008A, 0x00000009, 0x0000008F, 0x0000000A, 0x00000095, 0x0000000B, 0x0000009A,
    0x0000000C, 0x0000009F, 0x0000000D, 0x000000A5, 0x0000000E, 0x000000AB, 0x0000000F, 0x00000004,
    0x0004C200, 0x85CC0000, 0x00000004, 0x0004E1CB, 0xE5810000, 0x00000004, 0x000004E9, 0x0004EF00,
    0x04F30000, 0x00000000, 0x000004C1, 0x00049000, 0x04810000, 0x00000000, 0x000004C0, 0x04C10000,
    0x85000000, 0x00000010, 0x0201000A, 0x02000C00, 0x9045B0E9
}; 

// **************************************************************************
// *    END OF ALL                                                          *
// **************************************************************************
