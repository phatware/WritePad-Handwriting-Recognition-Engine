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
#include "wg_stuff.h"

#include "ws.h"

#include "rec-main.h"
#include "recutil.h"
#include "main_utl.h"
#include "xr_names.h"

#include "xrword.h"
#include "postcalc.h"

/* *************************************************************** */
/* ****** Word segmentation support routines ********************* */
/* *************************************************************** */

/* *************************************************************** */
/* *            Create strokes index array                       * */
/* *************************************************************** */
_INT CreateInkInfo(p_VOID ink, _INT npoints, p_VOID _PTR ink_info)
{
	_INT i, j;
	_INT num_strokes;
	p_PS_point_type trace = (p_PS_point_type) ink;
	p_ink_info_type sa;

	sa = (p_ink_info_type) HWRMemoryAlloc(sizeof(ink_info_type));

	if (trace == _NULL || npoints < 3 || sa == _NULL)
	{
		goto err;
	}

	num_strokes = 0;
	for (i = 1, j = 1; i < npoints; i++)
	{
		if (trace[i].y < 0)
		{
			if (num_strokes >= WS_MAX_STROKES)
			{
				goto err;
			}

			sa->strokes[num_strokes].start = (_SHORT) j;
			sa->strokes[num_strokes].len = (_SHORT) ((i + 1) - j);

			num_strokes++;
			j = i + 1;
		}
	}

	sa->ink = (p_VOID) trace;
	sa->num_strokes = num_strokes;

	*ink_info = (p_VOID) (sa);

	return num_strokes;
err:
	if (sa)
	{
		HWRMemoryFree(sa);
	}
	return 0;
}

/* *************************************************************** */
/* *            Delete strokes index array                       * */
/* *************************************************************** */
_INT DestroyInkInfo(p_VOID _PTR ink_info)
{
	p_ink_info_type sa;

	if (ink_info == _NULL)
	{
		goto err;
	}

	sa = (p_ink_info_type) (*ink_info);
	if (sa)
	{
		HWRMemoryFree(sa);
	}

	return 0;
err:
	return 1;
}

/* *************************************************************** */
/* *           Get stroke lenght in points                       * */
/* *************************************************************** */
_INT GetInkStrokeLen(_INT n, p_VOID ink_info)
{
	p_ink_info_type sa = (p_ink_info_type) ink_info;

	if (sa == _NULL)
	{
		goto err;
	}
	if (n < 0 || n >= sa->num_strokes)
	{
		goto err;
	}

	return sa->strokes[n].len;
err:
	return 0;
}
/* *************************************************************** */
/* *           Get pointer to stroke points                      * */
/* *************************************************************** */
p_VOID GetInkStrokePtr(_INT n, p_VOID ink_info)
{
	p_ink_info_type sa = (p_ink_info_type) ink_info;
	p_PS_point_type trace;

	if (sa == _NULL)
	{
		goto err;
	}
	if (n < 0 || n >= sa->num_strokes)
	{
		goto err;
	}

	trace = (p_PS_point_type) (sa->ink);

	return (p_VOID) &(trace[sa->strokes[n].start]);
err:
	return _NULL;
}

/* *************************************************************** */
/* *           Get stroke points and stroke len                  * */
/* *************************************************************** */
_INT GetInkStrokeCopy(_INT n, p_VOID place_for_stroke, p_VOID ink_info)
{
	_INT i;
	_INT len;
	p_ink_info_type sa = (p_ink_info_type) ink_info;
	p_PS_point_type trace, ptr, pp;

	if (sa == _NULL)
	{
		goto err;
	}
	if (place_for_stroke == _NULL)
	{
		goto err;
	}
	if (n < 0 || n >= sa->num_strokes)
	{
		goto err;
	}

	trace = (p_PS_point_type) (sa->ink);

	len = sa->strokes[n].len;
	ptr = &(trace[sa->strokes[n].start]);
	pp = (p_PS_point_type) (place_for_stroke);

	for (i = 0; i < len; i++, ptr++, pp++)
	{
		*pp = *ptr;
	}

	return len;
err:
	return 0;
}

/* *************************************************************** */
/* *        Create trajectory for next word to recognize         * */
/* *************************************************************** */
_INT GetNextWordInkCopy(_INT flags, p_ws_results_type pwsr, p_VOID place_for_ink, p_VOID ink_info, p_ws_word_info_type wswi)
{
	_INT i, l, m, n, t;
	_INT s_st = 0, wx = 0, wy = 0, dx, dy;
	_INT len;
	_INT combine_carry = 0;
	_INT max_x, loc, end, st, n_str;
	_INT nfs = -1;
	p_PS_point_type pp;
	word_strokes_type(_PTR w_str)[WS_MAX_WORDS];

	if (ink_info == _NULL)
	{
		goto err;
	}
	if (place_for_ink == _NULL)
	{
		goto err;
	}
	if (pwsr == _NULL)
	{
		goto err;
	}

	w_str = pwsr->pwsa;
	pp = (p_PS_point_type) (place_for_ink);
	len = 1;
	n_str = 0;

	for (m = 0; m < pwsr->num_words && m < WS_MAX_WORDS - 1; m++)
	{
		if ((*w_str)[m].flags & WS_FL_PROCESSED)
		{
			continue;
		}

		if ((flags & RM_COMBINE_CARRY) && ((*w_str)[m].flags & WS_FL_CARRYDASH))
		{
			if (m == pwsr->num_words - 1)
			{
				continue;    // If there is no words on  the next line up to now
			}
			combine_carry = 1;
		}

		(*w_str)[m].flags |= WS_FL_PROCESSED;

		for (n = 0; n < (*w_str)[m].num_strokes; n++)
		{
			if (n == 0 && ((*w_str)[m].flags & WS_FL_NL_GESTURE))
			{
				continue;    // NewLine gesture destroy
			}

			l = (*w_str)[m].first_stroke_index;
			l = pwsr->stroke_index[l + n];
			t = GetInkStrokeCopy(l, (p_VOID) &(pp[len]), ink_info);
			if (t == 0)
			{
				goto err;
			}
			len += t;
			n_str++;
			pp[len - 1].x = (_SHORT) n_str;

			if (nfs < 0)
			{
				nfs = l;    // For des-file compatibility
			}
		}

		wswi->nword = m;
		wswi->flags = (*w_str)[m].flags;
		wswi->slant = (*w_str)[m].slope;
		wswi->wstep = (*w_str)[m].writing_step;
		SetStrokeSureValuesWS((combine_carry == 2), m, pwsr, wswi);

		if (combine_carry == 2) // Transform trajectory
		{
			dx = wx - (*w_str)[m].word_x_st;
			dy = wy - (*w_str)[m].word_mid_line;

			for (i = s_st; i < len; i++) // Glue second part
			{
				if (pp[i].y < 0)
				{
					continue;
				}

				pp[i].x += (_SHORT) dx;
				pp[i].y += (_SHORT) dy;
			}
		}

		if (combine_carry == 1) // Remember positions for first part
		{
			max_x = 0;
			loc = 0;
			for (i = 1; i < len; i++) // remove dash
			{
				if (pp[i].y < 0)
				{
					continue;
				}
				if (pp[i].x > max_x)
				{
					max_x = pp[i].x;
					loc = i;
				}
			}

			for (i = loc; i < len; i++)
				if (pp[i].y < 0)
				{
					end = i + 1;
					break;
				}

			for (i = loc; i > 0; i--)
				if (pp[i].y < 0)
				{
					st = i + 1;
					break;
				}

			HWRMemCpy(&(pp[st]), &(pp[end]), (len - end)*sizeof(pp[0]));

			len -= (end - st);

			wx = (*w_str)[m].word_x_end; // + (*w_str)[m].writing_step;
			wy = (*w_str)[m].word_mid_line;
			s_st = len;
			combine_carry = 2;
		}
		else
		{
			if (n_str > 0)
			{
				break;    // If got something for cur word
			}
		}
	}

	pp[0].x = (_SHORT) nfs;  // Debug value for DES-file
	pp[0].y = -1;

	pp[len - 1].x = (_SHORT) (n_str);  // Debug value for DES-file
	return (len == 1) ? 0 : len;
err:
	return 0;
}
