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

#include "hwr_file.h"
#include "hwr_sys.h"
#include "hwr_swap.h"

#include "ams_mg.h"

#ifndef PEGASUS

#if PS_VOC

#include "xrwdict.h"
#include "elk.h"
#include "vocutilp.h"

#ifndef LOAD_RESOURCES_FROM_ROM
#define LOAD_RESOURCES_FROM_ROM 0
#endif

#if !LOAD_RESOURCES_FROM_ROM

/* common define for voc_load and dat_load */
#define BREAK_ON_ERROR ret = 1; BLOCK_EXIT;
#define BREAK_IF_ERROR if( ret == 1) BLOCK_EXIT

/* ************************************************************************** */
/* *          Vocabulary loading routine                                    * */
/* ************************************************************************** */
_SHORT  voc_load(p_UCHAR fname, _VOID _PTR _PTR vp)
{
	vocptr_type _PTR vocptr = _NULL;
	_HMEM            hvp = _NULL;
	p_CHAR           ptr;

#if HWR_SYSTEM != MACINTOSH
	_CHAR             fnbuffer[MAX_VOCPATH];
#endif /* HWR_SYSTEM */

	/* check parameters */
	if (fname == _NULL || vp == _NULL)
	{
		goto err;
	}
	if (*fname == 0)
	{
		goto err;
	}

	/* bigor 09/14/94 search and remove any filename extension */
	ptr = (p_CHAR) fname;
	if ((ptr = HWRStrrChr((_STR) ptr, (_SHORT)'\\')) == _NULL)
	{
		ptr = (p_CHAR) fname;
	}
	if ((ptr = HWRStrChr((_STR) ptr, (_SHORT)'.')) != _NULL)
	{
		*ptr = 0;
	}
	else
	{
		ptr = (p_CHAR) fname;
		while (*ptr != 0)
		{
			ptr++;
		}
	}

	/* allocate memory for voc_ptr structure */
	if ((hvp = HWRMemoryAllocHandle((_ULONG)sizeof(vocptr_type))) == _NULL)
	{
		goto err;
	}

	vocptr = (vocptr_type _PTR)HWRMemoryLockHandle(hvp);
	HWRMemSet((p_CHAR) vocptr, 0, (_WORD)sizeof(vocptr_type));

	/* store vocptr handle in the vocptr structure */
	vocptr->hvoc = hvp;

	/* store fname without extension in vocptr structure */
	HWRStrCpy((_STR) vocptr->vocname, (_STR) fname);

#if HWR_SYSTEM == MACINTOSH
	HWRStrCat((_STR)vocptr->vocname, ".res");

	vocptr->hvoc_dir = D_DopenResourceFile((_STR)vocptr->vocname,1);
	vocptr->hvoc_inv = D_DopenResourceFile((_STR)vocptr->vocname,2);
#else
	/* copy fname into internal buffer to form full filename */
	HWRStrCpy((_STR) &fnbuffer[0], (_STR) vocptr->vocname);
#if VER_PALK_DICT
	HWRStrCat((_STR)&fnbuffer[0], ".vcb");
#else
	HWRStrCat((_STR) &fnbuffer[0], ".voc");
#endif


	if (ElkLoadDict((p_UCHAR) fnbuffer, &vocptr->hvoc_dir) == ELK_ERR)
	{
		goto err;
	}

#endif /* HWR_SYSTEM == MACINTOSH */

	*vp = vocptr;

	return 0;
err:
	if (hvp != _NULL) if (vocptr != _NULL)
		{
			HWRMemoryUnlockHandle(hvp);
		}
	HWRMemoryFreeHandle(hvp);
	*vp = _NULL;
	return 1;
}

/* ************************************************************************* */
/*        Save PS_voc file                                                   */
/* ************************************************************************* */
_SHORT  voc_save(p_CHAR fname, vocptr_type _PTR vp)
{
	vocptr_type _PTR vocptr = _NULL;
	p_CHAR          ptr;
	_CHAR           name[128];

	vocptr = (vocptr_type _PTR)vp;

	HWRStrCpy(name, (_STR) fname);
	if ((ptr = HWRStrrChr(name, (_SHORT)'.')) != _NULL)
	{
		*ptr = 0;
	}

#if HWR_SYSTEM == MACINTOSH
	HWRStrCat((_STR)vocptr->vocname, ".res");

	vocptr->hvoc_dir = D_DsaveResourceFile((_STR)vocptr->vocname,1);
	vocptr->hvoc_inv = D_DsaveResourceFile((_STR)vocptr->vocname,2);
#else

#if VER_PALK_DICT
	HWRStrCat(name, ".vcb");
#else
	HWRStrCat(name, ".voc");
#endif

	if (ElkSaveDict((p_UCHAR) name, vocptr->hvoc_dir))
	{
		goto err;
	}

#endif /* HWR_SYSTEM == MACINTOSH */

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*       Add word to PS_vocabulary                                           */
/* ************************************************************************* */
_INT tst_word(p_UCHAR inp_word, p_UCHAR stat, vocptr_type _PTR vp)
{
	_UCHAR  attr, status = XRWD_MIDWORD;

#if PG_DEBUG // Temp until we could ...
	if (ElkCheckWord(inp_word, &status, &attr, vp->hvoc_dir))
	{
		goto err;
	}
#endif

	if (status == XRWD_MIDWORD)
	{
		goto err;    // It was just prefix!
	}

	*stat = attr;

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*       Add word to PS_vocabulary                                           */
/* ************************************************************************* */
_SHORT  add_word(p_UCHAR inp_word, _SHORT stat, vocptr_type _PTR vp)
{
	_UCHAR  attr = (_UCHAR) (stat & 0xff);

#if PG_DEBUG // Temp until we could ...
	if (ElkAddWord(inp_word, attr, &vp->hvoc_dir))
	{
		goto err;
	}
#endif

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*       Delete word from the PS vocabulary                                  */
/* ************************************************************************* */
_SHORT  del_word(p_UCHAR inp_word, vocptr_type _PTR vp)
{
	// Not implemented now ...

	UNUSED(inp_word);
	UNUSED(vp);
	return 1;
}

/* ************************************************************************* */
/*      Unload PS vocabularies                                               */
/* ************************************************************************* */
_SHORT  voc_unload(_VOID _PTR _PTR vp)
{
	vocptr_type _PTR vocptr;

	vocptr = (vocptr_type *) (*vp);

	if (vocptr != _NULL)
	{
		ElkFreeDict(&vocptr->hvoc_dir);

		HWRMemoryUnlockHandle(vocptr->hvoc);
		HWRMemoryFreeHandle(vocptr->hvoc);
		*vp = _NULL;
	}

	return 0;
}
#endif /* LOAD_RESOURCES_FROM_ROM */

/* ************************************************************************* */
/*      Lock   PS vocabularies                                               */
/* ************************************************************************* */
_BOOL  LockVocabularies(p_VOID vp)
{
	UNUSED(vp);
	return 0;
}

/* ************************************************************************* */
/*      Unload PS vocabularies                                               */
/* ************************************************************************* */
_BOOL  UnlockVocabularies(p_VOID vp)
{
	UNUSED(vp);
	return 0;
}

#endif /* PS_VOC */

#endif // Pegasus
