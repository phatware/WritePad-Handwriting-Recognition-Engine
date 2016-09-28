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

#include "hwr_sys.h"
#include "ams_mg.h"                           /* Most global definitions     */
#include "zctype.h"
#include "xrwdict.h"

/* ************************************************************************* */
/*       Check and correct word by dictionary or lex DB, case sens or not    */
/* ************************************************************************* */

_INT CheckAns( _INT flags, lex_data_type _PTR vs, fw_buf_type(_PTR fbuf)[XRWD_MAX_LETBUF] )
{

    if ( CheckAnsSym( flags, 0, 0l, vs, fbuf ) )
        goto err;

    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*       Check and correct word by dictionary                                */
/* ************************************************************************* */

_INT CheckAnsSym( _INT flags, _INT dep, _ULONG state, lex_data_type _PTR vs, fw_buf_type(_PTR fbuf)[XRWD_MAX_LETBUF] )
{
    _INT i;
    _UCHAR alts[2] = {0};
    _UCHAR cn[2] = {0};
    _ULONG states[2] = {0};
    _INT nums[2] = {0};
    p_fw_buf_type fb;

    if ( dep >= w_lim - 1 )
        goto err;

    vs->done_let = dep;

    if ( vs->l_sym.sources == XRWD_SRCID_VOC )
    {
        vs->l_sym.sd[XRWD_N_VOC].l_status = (_UCHAR) ((dep == 0) ? XRWD_INIT : XRWD_MIDWORD);
        vs->l_sym.sd[XRWD_N_VOC].state = state;
        GF_VocSymbolSet( vs, fbuf );
    }

    if ( vs->l_sym.sources == XRWD_SRCID_LD )
    {
        vs->l_sym.sd[XRWD_N_LD].l_status = (_UCHAR) ((dep == 0) ? XRWD_INIT : XRWD_MIDWORD);
        vs->l_sym.sd[XRWD_N_LD].state = state;
        GF_LexDbSymbolSet( vs, fbuf );
    }

    alts[0] = vs->realword[dep];
    if ( (flags & 0x01) && IsAlpha( alts[0] ) )
    {
        if ( IsLower( alts[0] ) )
            alts[1] = (_UCHAR) ToUpper( alts[0] );
        else
            alts[1] = (_UCHAR) ToLower( alts[0] );
    }
    else
    {
        alts[1] = 0;
    }

    if ( vs->realword[dep + 1] == 0 ) // Last letter to verify
    {
        for ( i = 0, fb = &((*fbuf)[0]); i < XRWD_MAX_LETBUF && fb->sym != 0; i++, fb++ )
        {
            if ( fb->sym == alts[0] && XRWD_TO_BE_ANSWERED(fb->l_status) )
            {
                vs->realword[dep] = alts[0];
                goto done;
            }
            if ( fb->sym == alts[1] && XRWD_TO_BE_ANSWERED(fb->l_status) )
            {
                vs->realword[dep] = alts[1];
                goto done;
            }
        }
    }
    else
    {
        states[0] = states[1] = 0l;
        nums[0] = nums[1] = -1;
        for ( i = 0, fb = &((*fbuf)[0]); i < XRWD_MAX_LETBUF && fb->sym != 0; i++, fb++ )
        {
            if ( fb->sym == alts[0] )
            {
                nums[0] = i;
                states[0] = fb->state;
                cn[0] = fb->chain_num;
            }
            if ( fb->sym == alts[1] )
            {
                nums[1] = i;
                states[1] = fb->state;
                cn[1] = fb->chain_num;
            }
        }

        for ( i = 0; i < 2 && alts[i] != 0; i++ )
        {
            if ( nums[i] < 0 )
                continue;

            vs->realword[dep] = vs->word[dep] = alts[i];
            if ( vs->l_sym.sources == XRWD_SRCID_LD )
                vs->l_sym.sd[XRWD_N_LD].chain_num = cn[i];
            else
                vs->l_sym.sd[XRWD_N_VOC].chain_num = cn[i];

            if ( CheckAnsSym( flags, dep + 1, states[i], vs, fbuf ) == 0 )
                goto done;
        }
    }

err:
    return 1;
done:
    return 0;
}

/* ************************************************************************* */
/*       Setup proper dictionary handle for given ID                         */
/* ************************************************************************* */

_INT SetupVocHandle( lex_data_type _PTR vs, _INT id )
{

    switch ( id )
    {
        case XRWD_SRCID_VOC :
            if ( (vs->hdict = vs->hmaindict) == _NULL )
                goto err;
            else
                break;
        case XRWD_SRCID_USV :
            if ( (vs->hdict = vs->huserdict) == _NULL )
                goto err;
            else
                break;
        case XRWD_SRCID_PFX :
            if ( (vs->hdict = vs->hprefdict) == _NULL )
                goto err;
            else
                break;
        case XRWD_SRCID_SFX :
            if ( (vs->hdict = vs->hsuffdict) == _NULL )
                goto err;
            else
                break;

        default :
            goto err;
    }

    return 0;
err:
    return 1;
}

