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

#include "ams_mg.h"
#include "hwr_sys.h"

#include "xr_names.h"
#include "bitmapco.h"

// ---------------------- Defines ----------------------------------------------

#define GBM_WINGS               5

#define GBM_ONESHIFT           16
#define GBM_ONESIZE    (1 << GBM_ONESHIFT)
#define GBM_QNUM  ((GBM_XSIZE/GBM_QSX) * (GBM_YSIZE/GBM_QSY))

#define BMGABS(x) (((x) < 0) ? (-(x)) : (x))

/* *************************************************************************** */
/* *     Get BitMap given trace and XrData Range                             * */
/* *************************************************************************** */
_INT GetSnnBitMap(_INT st, _INT len, p_xrdata_type xrdata, _TRACE trace, p_UCHAR coeff, p_RECT rect, Part_of_letter _PTR parts)
{
_INT     i, j, n;
_UCHAR   map[GBM_XSIZE + 2][GBM_YSIZE + 2];
_INT     box_minx, box_miny, box_maxx, box_maxy;
_INT     dx, dy;
_INT     cx, cy;
_INT     fp, ep;
_INT     x, y, px, py; //, pp;
_INT     xscale, yscale;
_INT     st_point, en_point;
_UCHAR   cmarker, pmarker;
p_UCHAR  pcf;

if (len < 1)
{
	goto err;
}

dx = rect->right - rect->left;
dy = rect->bottom - rect->top;
cx = rect->left + dx / 2;
cy = rect->top + dy / 2;

if (dy == 0 && dx == 0)
{
	goto err;
}

HWRMemSet(map, 0, sizeof(map));

xscale = yscale = 0;
if (dx)
{
	xscale = (GBM_ONESIZE * GBM_XSIZE) / (dx + dx / 4);
}
if (dy)
{
	yscale = (GBM_ONESIZE * GBM_YSIZE) / (dy + dy / 4);
}
if (xscale == 0)
{
	xscale = yscale;
}
if (yscale == 0)
{
	yscale = xscale;
}

if (xscale > yscale)
{
	xscale = yscale;    // Make uniform scaling
}
else
{
	yscale = xscale;
}

if (xscale == 0)
{
	goto err;
}

i = (GBM_ONESIZE * GBM_XSIZE) / (2 * xscale);
box_minx = cx - i;
box_maxx = cx + i;
i = (GBM_ONESIZE * GBM_YSIZE) / (2 * yscale);
box_miny = cy - i;
box_maxy = cy + i;

// ----------------------- Get trajectory subset for processing --------------

st_point = (*xrdata->xrd)[st].begpoint;
en_point = (*xrdata->xrd)[st + len - 1].endpoint;

i = st - GBM_WINGS;
if (i < 0)
{
	i = 0;
}
n = st + len + GBM_WINGS;
if (n > xrdata->len)
{
	n = xrdata->len;
}

fp = ep = (*xrdata->xrd)[i].begpoint;
for (; i < n; i++)
{
	if (fp >(*xrdata->xrd)[i].begpoint)
	{
		fp = (*xrdata->xrd)[i].begpoint;
	}
	if (ep < (*xrdata->xrd)[i].endpoint)
	{
		ep = (*xrdata->xrd)[i].endpoint;
	}
}

for (i = fp, px = py = 0; i < ep; i++)
{
	x = trace[i].x;
	y = trace[i].y;
	if (y < 0)
	{
		px = 0;
		continue;
	}
	if (x < box_minx || x >= box_maxx || y < box_miny || y >= box_maxy)
	{
		px = 0;
		continue;
	}

	y = (((y - box_miny) * yscale + GBM_ONESIZE / 2) >> GBM_ONESHIFT);
	x = (((x - box_minx) * xscale + GBM_ONESIZE / 2) >> GBM_ONESHIFT);

	if (x == px && y == py)
	{
		continue;
	}
	if (x < 0 || x >= GBM_XSIZE || y < 0 || y >= GBM_YSIZE)
	{
		px = 0;
		continue;
	}

	if (px > 0) // After first point was inited
	{
		_INT len;
		_INT sx, sy, xx, yy, xxx, yyy, pxxx, pyyy;

		dx = BMGABS(x - px);
		dy = BMGABS(y - py);
		len = (dx > dy) ? dx : dy;
		sx = ((x - px) * 256) / len;
		sy = ((y - py) * 256) / len;

		for (j = 0, cmarker = pmarker = 0x40; j < MAX_PARTS_IN_LETTER && parts[j].iend; j++)
			if (i >= parts[j].ibeg && i <= parts[j].iend)
			{
				cmarker = 0xC0;
				pmarker = 0x80;
				break;
			}

		for (j = pxxx = pyyy = 0, xx = yy = 256; j <= len; j++, xx += sx, yy += sy)
		{
			xxx = px + (xx >> 8);
			yyy = py + (yy >> 8);
			if (pxxx == xxx && pyyy == yyy)
			{
				continue;
			}

			map[xxx][yyy] |= cmarker;
			map[xxx + 1][yyy] |= pmarker;
			map[xxx - 1][yyy] |= pmarker;
			map[xxx][yyy + 1] |= pmarker;
			map[xxx][yyy - 1] |= pmarker;
		}
	}

	px = x;
	py = y;
}

// ----------------------- Output coeff string in proper order ---------------

for (i = 0, pcf = coeff; i < GBM_QNUM; i++)
{
	px = (i * GBM_XSIZE / GBM_QSX) % GBM_XSIZE;
	py = (i / GBM_QSX) * (GBM_YSIZE / GBM_QSY);
	for (j = 0; j < (GBM_YSIZE / GBM_QSY); j++)
	{
		for (n = 0; n < (GBM_XSIZE / GBM_QSX); n++)
		{
			*(pcf++) = map[px + n + 1][py + j + 1];
		}
	}
}


return 0;
err:
return 1;
}
