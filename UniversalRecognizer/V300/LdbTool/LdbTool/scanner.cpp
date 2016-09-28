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

#include <stdio.h>
#include <string.h>
#include "mytypes.h"
#include "table.h"
#include "scanner.h"

#define INBUFSIZE 256

typedef struct tagScaner
{
    FILE *file;
    char buf[INBUFSIZE];
    int  cnt;
    int  len;
} Scaner, *PScaner;

static Scaner scaner;

Bool ScnInit(FILE *inFile)
{
    scaner.file   = inFile;
    scaner.cnt    = 0;
    scaner.len    = 0;
    return True;
} /* ScnInit */

Bool ScnRead(int mode, PSymbol pSym)
{
    char *buf;
    int   n, len, cnt;
    int   type = 0;
    long  val;
    PNode pNode;
    
    val = 0L;
    pNode = Null;
    
    do {
        /* Read next line if nesessary */
        if (scaner.cnt >= scaner.len)
        {
            if (fgets(scaner.buf, INBUFSIZE, scaner.file) == NULL)
                return False;
            scaner.len = (int)strlen( scaner.buf );
            scaner.cnt = 0;
        }
        cnt = scaner.cnt;
        buf = scaner.buf;
        /* Find next blank separated token */
        while (cnt < scaner.len && (buf[cnt] == ' ' || buf[cnt] == '\t' || buf[cnt] == '\r') && buf[cnt] != '\n')
            cnt++;
        scaner.cnt = cnt;
    } while (scaner.cnt >= scaner.len);
    /* Find the token type */
    if (buf[cnt] == '\n')
    {
        type = ScnTypeEol;
        len = 1;
    }
    else if (buf[cnt] == ';' && cnt+1 < scaner.len && buf[cnt+1] == ';')
    {
        type = ScnTypeEol;
        len = scaner.len - cnt;
    }
    else
    {
        len = 0;
        while (cnt + len < scaner.len &&
               buf[cnt + len] != ' ' &&
               buf[cnt + len] != '\t' &&
               buf[cnt + len] != '\r' &&
               buf[cnt + len] != '\n')
        {
            len++;
        }
        switch (mode)
        {
            case ScnModeName:
                if (!TblLookup(&(scaner.buf[cnt]), len, TblModeInsert, &pNode))
                    return False;
                type = ScnTypeName;
                break;
                
            case ScnModeChar:
                type = ScnTypeChar;
                if (buf[cnt] == '\\')
                {
                    val = 0L; n = 1;
                    while (buf[cnt + n] >= '0' && buf[cnt + n] <= '9')
                    {
                        val = val * 10 + (long) buf[cnt + n] - (long) '0';
                        n++;
                    }
                }
                else
                {
                    val = (long) buf[cnt];
                }
                break;
                
            case ScnModeNumber:
                type = ScnTypeNumber;
                val = 0L; n = 0;
                while (buf[cnt + n] >= '0' && buf[cnt + n] <= '9')
                {
                    val = val * 10 + (long) buf[cnt + n] - (long) '0';
                    n++;
                }
                break;
                
            case ScnModeSkip:
                type = ScnTypeEol;
                len = scaner.len - cnt;
                break;
        }
    }
    
    /* Fill the answer */
    pSym->type  = type;
    pSym->pNode = pNode;
    pSym->val   = val;
    
    /* Advance the current point */
    scaner.cnt = cnt + len;
    
    return True;
} /* ScnRead */

void ScnDone(void)
{
} /* ScnDone */
