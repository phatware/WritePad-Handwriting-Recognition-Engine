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

ROM_DATA _ULONG img_vocpref[46] =  
{  
    0x30312E31, 0x00000001, 0x00000081, 0x00000016, 0x00000009, 0x00000028, 0x00000000, 0x0000003C, 
    0x00000001, 0x00000059, 0x0000000A, 0x0000006C, 0x00000012, 0x00000075, 0x00000016, 0x00000004, 
    0x80028002, 0x64636102, 0x706E6C6A, 0xF46D7371, 0x00000004, 0x6530A7F5, 0x0B8002A7, 0xF5271380, 
    0x30A76F30, 0x8001EFC7, 0x7530C502, 0x000004A7, 0x0F800100, 0xF2ED81C6, 0xC0168001, 0xF0F3C230, 
    0x00000004, 0xC630F3E9, 0x000004E5, 0x01C33000, 0x30C40880, 0x000016C1, 0x01000A00, 0x00110007, 
    0x6F726E03, 0x7475712D, 0x0927E96F, 0x05000000, 0x0B800100
}; 

// **************************************************************************
// *    END OF ALL                                                          *
// **************************************************************************
