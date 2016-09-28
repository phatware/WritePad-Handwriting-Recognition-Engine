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

#include <string.h>
#include <stdlib.h>
#include "mytypes.h"
#include "table.h"

#define NSSTORESIZE 16384
#define NNODES      2048 /* GIT - was 512, changed because of Apple's huge LDBs */

typedef struct tagTable {
    int   slen;
    int   sptr;
    char *sstore;
    int   nlen;
    int   nptr;
    PNode nstore;
} Table, *PTable;

static Table table;

Bool TblInit(void)
{
    table.sstore = (char *) malloc(NSSTORESIZE);
    if (table.sstore == Null) return False;
    table.nstore = (PNode) malloc( NNODES*sizeof(Node) );
    if (table.nstore == Null)
    {
        free(table.sstore);
        return False;
    }
    table.sptr = 0;
    table.slen = NSSTORESIZE;
    table.nptr = 0;
    table.nlen = NNODES;
    table.nstore[0].left  = -1;
    table.nstore[0].right = -1;
    table.nstore[0].str   = table.sstore;
    table.nstore[0].type  = TblTypeUnspec;
    table.nstore[0].p     = Null;
    table.sstore[0] = '\0';
    return True;
} /* TblInit */

void TblDone(void)
{
    if (table.sstore != Null)
    {
        free(table.sstore);
        table.sstore = Null;
    }
    if (table.nstore != Null)
    {
        free(table.nstore);
        table.nstore = Null;
    }
} /* TblDone */

Bool TblLookup(char *str, int len, int mode, PNode *ppNode)
{
    PNode nstore;
    char *sstore;
    int   pnode, pprevnode;
    int   res;
    char  buf[256];
    
    nstore = table.nstore;
    sstore = table.sstore;
    strncpy(buf, str, len);
    buf[len] = '\0';
    
    /* Find string in name table */
    pnode  = 0; pprevnode = -1;
    while ((res = strcmp(buf,nstore[pnode].str)) != 0)
    {
        pprevnode = pnode;
        if (res < 0)
            pnode = nstore[pnode].left;
        else
            pnode = nstore[pnode].right;
        if (pnode == -1)
            break;
    }
    
    if (pnode != -1)
    {
        *ppNode = &nstore[pnode];
        return True;
    }
    
    if (mode == TblModeLookup)
        return False; /* No insert */
    /* Insert new node */
    /* Check string storage */
    if (table.sptr + len + 1 >= table.slen)
        return False;
    if (table.nptr == 0)
    {
        pnode = 0; table.nptr++;
    }
    else
    {
        /* Check node storage */
        if (table.nptr >= table.nlen)
            return False;
        pnode = table.nptr; table.nptr++;
        if (res < 0)
        {
            nstore[pprevnode].left = pnode;
        }
        else
        {
            nstore[pprevnode].right = pnode;
        }
    }
    nstore[pnode].left  = -1;
    nstore[pnode].right = -1;
    nstore[pnode].str   = &(sstore[table.sptr]);
    nstore[pnode].type  = TblTypeUnspec;
    nstore[pnode].p     = Null;
    strncpy(&(sstore[table.sptr]), str, len);
    sstore[table.sptr+len] = '\0';
    table.sptr += len + 1;
    *ppNode = &nstore[pnode];
    return True;
} /* TblLookup */
