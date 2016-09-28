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
_ULONG img_vocpref[126] =
{
	0x00020100, 0x00000000, 0x000001E6, 0x00000000, 0x000001E6, 0x000000D4, 0x000000E6, 0x0000010F,
	0x00000132, 0x00000149, 0x00000158, 0x00000162, 0x00000169, 0x0000016E, 0x00000173, 0x00000178,
	0x0000017D, 0x00000182, 0x00000187, 0x0000018C, 0x00000191, 0x00000196, 0x0000019B, 0x000001A0,
	0x000001A5, 0x000001AA, 0x000001AF, 0x000001B4, 0x000001B9, 0x000001BE, 0x000001C3, 0x000001C8,
	0x000001CD, 0x000001D2, 0x000001D7, 0x000001DC, 0x000001E1, 0x00400040, 0x00400040, 0x00400040,
	0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040,
	0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00400040, 0x00000005, 0x610B1000, 0x6C6A6463,
	0x71706E6D, 0x00057473, 0x10000000, 0x27407501, 0x65270210, 0x75270210, 0x6F270210, 0x03102740,
	0x10E96F27, 0x01106F01, 0x27021075, 0x05274075, 0x00000000, 0xB0740110, 0xB06D0110, 0xB0730110,
	0xB0720110, 0x406E0110, 0x7301106F, 0x10B02740, 0x00057001, 0x40000000, 0x6901106F, 0x10710110,
	0x2D407301, 0x01107440, 0x00000565, 0x2D400000, 0x10750110, 0x72407101, 0x00000005, 0x10274000,
	0x00057501, 0x40000000, 0x00000527, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500,
	0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000,
	0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000,
	0x05000000, 0x00000000, 0x00000005, 0x00000500, 0x00050000, 0x05000000, 0x00000000, 0x00000005,
	0x00000500, 0x00000000, 0x00002225, 0x00416064, 0x00416064
};

// **************************************************************************
// *    END OF ALL                                                          *
// **************************************************************************
