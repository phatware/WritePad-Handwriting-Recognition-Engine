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
#include <math.h>

#include "InternalTypes.h"
#include "PhatCalc.h"
#include "RecognizerApi.h"

#define STATE_START		0
#define STATE_NUM		1
#define MAX_STRINGNAME	200


/* *********************************************************************** */
/* *                 PhatCalc                                                * */
/* *********************************************************************** */

extern "C"
BOOL PhatCalc( const UCHR * str, const UCHR * szDelimeters, FLOAT * num )
{
    int		i, j, k, n, len, add0 = 0;
    int		state, type, ptype;
    FLOAT	neg;
    unsigned char operands[] = { '-','+','*', 0xf7, 0xd7, 'x','X','/',':','=',0 };
    char    digits[] = { '0','1','2','3','4','5','6','7','8','9',0 };
	FLOAT	values[32];
    UCHR	operations[32];
    char 	ns[MAX_STRINGNAME];
    UCHR	last_op; 
    UCHR	cDEC = szDelimeters[0];			// decimal separator
    UCHR	cTHOUSAND = szDelimeters[1];	// thousand separator
    FLOAT	res;
	
    for ( len = 0; len < MAX_STRINGNAME-1 && str[len] != 0; len ++);
	
    if ( str[len-1] != '=' && !(str[len-1] == ' ' && str[len-2] == '=')) 
        goto err; 
	
    state = type = 0; 
    neg = 1.0;
	add0 = 0;
		
    for ( i = j = k = n = 0; i < len; i++ )
    {
        if ( str[i] == ' ' || str[i] == cTHOUSAND || (cTHOUSAND == ',' && str[i] == '\'') )
            continue;
		
        ptype = type;
		
		if ( str[i] == cDEC )
			add0 = 1;
        if ( str[i] == cDEC || HWRStrChr( digits, str[i]) != 0 )
            type = 1;
        else if ( HWRStrChr( (const char *)operands, str[i]) != 0)
            type = 2;
        else
            goto err;
		
        if ( ptype == 1 && type != 1 )
        {
            // float num;
            if ( n == 0 ) 
                goto err;
			
			if ( add0 )
				ns[n++] = '0';
            ns[n] = 0;
			
#ifdef _WIN32
			values[j++] = (FLOAT)( neg * strtod( ns, NULL ));
#else
			values[j++] = (FLOAT)(neg * strtof( ns, NULL ));
#endif
			n = 0; 
            neg = 1.0;
			add0 = 0;
        }
		
        switch (state)
        {
            case STATE_START:
                if (type == 1) 
                    ns[n++] = (unsigned char)str[i];
                else if ( type == 2 ) 
                {
                    if ( str[i] != '-' ) 
                        goto err; 
                    else 
                        neg = -1.0;
                }
                state = STATE_NUM;
                break;
				
            case STATE_NUM:
                if (type == 1) 
					ns[n++] = (unsigned char)str[i];
	            else if (type == 2) 
                {
                    operations[k++] = str[i]; 
                    state = STATE_START;
                }
                break;
        }
    }
	
    if ( k < 2 || k != j ) 
        goto err; // Required: two operands and two operations (eg. + and =)
	
    if ( operations[k-1] != '=' ) 
        goto err; // Formula in our sense always ends with =
	
    // --------------- Now let's make calculations ------------------------
	
    for ( i = 0, k = 0; i < j-1; i++ )
    {
        switch ( (unsigned char)operations[i] )
        {
            case 'x' :
			case 'X' :
			case 0xd7 :
            case '*' : 
                values[i+1] = (values[i+1] * values[i]); 
                k = 1; 
                break;
				
            case '/' :
			case 0xf7 :
            case ':' : 
            {
                if ( values[i+1] == 0 ) 
                    goto err; 
                values[i+1] = values[i] / values[i+1]; 
                k = 1; 
                break;
            }
				
            default: 
                continue;
        }
		
        if ( k != 0 )
        {
            values[i] = 0;
            operations[i] = ' ';  
            k = 0;
        }
    }
	
    res = values[0];
    last_op = '+';
	
    for ( i = 0; i < j; i++ )
    {
        if ( i > 0 )
        {
            switch ( last_op )
            {
                case '-': 
                    res = res - values[i]; 
                    break;
                case '+': 
                    res = res + values[i]; 
                    break;
                default: 
                    goto err;
            }
        }
		
        if ( operations[i] != ' ' ) 
            last_op = operations[i];
    }
	
    *num = res;
    return true;
	
err:
    return false;
}
