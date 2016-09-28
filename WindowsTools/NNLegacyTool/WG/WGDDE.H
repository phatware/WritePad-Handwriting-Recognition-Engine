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

#define BUSY_FLAG     (UINT)0x1
#define TIMEOUT_FLAG  (UINT)0x2
#define CONNECT_FLAG  (UINT)0x4
#define INIT_FLAG     (UINT)0x4
#define EXEC_FLAG     (UINT)0x4
#define STOP_FLAG     (UINT)0x4
#define ALL_FLAGS     BUSY_FLAG|TIMEOUT_FLAG|CONNECT_FLAG|INIT_FLAG|\
EXEC_FLAG|STOP_FLAG

#define TIMEOUT 10000
#define TIMERID 0xf

#define MYGWL_DDEFLAG 0
#define MYGWL_DDETIMER MYGWL_DDEFLAG+sizeof(ULONG)
#define MYGWL_DDESERVER MYGWL_DDETIMER+sizeof(ULONG)

#define MAX_NAME_LEN 128

LRESULT CALLBACK ddeServerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND ddeServerCreateDDEWnd(HINSTANCE hInstance, HWND hwndParent);

HWND ddeServerReloadDDE(HINSTANCE hInstance, HWND hwndParent);
typedef struct                            // Standart XrVoc element type
{
	unsigned char  xr;
	unsigned char  a;
	unsigned char  p;
	unsigned char  h;
} xrvoc_type;

