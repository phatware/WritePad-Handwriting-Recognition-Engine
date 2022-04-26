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
#include "InternalTypes.h"
#include <ctype.h>

static _BOOL  HWRStrEmpty (_CSTR);

/**************************************************************************
 *                                                                         *
 *    strlen.                                                              *
 *                                                                         *
 **************************************************************************/

_WORD HWRStrLen(_CSTR pcString)
{
    _CCHAR *p;
    
    p = pcString+1;
    while(*pcString++ != 0);
    return((_WORD)(pcString-p));
}

_INT HWRStrLenW( CUCHR * pcString)
{
    if ( NULL == pcString )
        return 0;
    CUCHR *p;
    
    p = pcString+1;
    while(*pcString++ != 0);
    return((_INT)(pcString-p));
}


/**************************************************************************
 *                                                                         *
 *    strchr.                                                              *
 *                                                                         *
 **************************************************************************/

_CSTR HWRStrChr( _CSTR zString, _INT iChar)
{
    if ( NULL == zString )
        return _NULL;
    _CSTR p = zString;
    while( *p )
    {
        if ( (unsigned char)(*p) == iChar )
            return p;
        p++;
    }
	return _NULL;
}

_STR HWRStrChr( _STR zString, _INT iChar)
{
    if ( NULL == zString )
        return _NULL;
    _STR p = zString;
    while( *p )
    {
        if ( (unsigned char)(*p) == iChar )
            return p;
        p++;
    }
	return _NULL;
}

CUCHR * HWRStrChrW( CUCHR * zString, CUCHR iChar)
{
    if ( NULL == zString )
        return _NULL;
    CUCHR * p = zString;
    while( *p )
    {
        if ( *p == iChar )
            return p;
        p++;
    }
	return _NULL;
}

/**************************************************************************
 *                                                                         *
 *    strncmp.                                                             *
 *                                                                         *
 **************************************************************************/

_INT HWRStrnCmp( _CSTR zString1, _CSTR zString2, _WORD wNumber)
{
    if(wNumber == 0)
        return 0;
    zString1--;
    zString2--;
    while(*(++zString1) == *(++zString2) && --wNumber != 0 && *zString1 != 0
          && *zString2 != 0) ;
    
    if(wNumber == 0)
        return 0;
    return((_INT)(*zString1 - *zString2));
}

_INT HWRStrnCmpW( CUCHR * zString1, CUCHR * zString2, _INT wNumber)
{
    if(wNumber == 0)
        return 0;
    zString1--;
    zString2--;
    while(*(++zString1) == *(++zString2) && --wNumber != 0 && *zString1 != 0
          && *zString2 != 0) ;
    
    if(wNumber == 0)
        return 0;
    return((_INT)(*zString1 - *zString2));
}


/**************************************************************************
 *                                                                         *
 *    strcpy.                                                              *
 *                                                                         *
 **************************************************************************/


_STR  HWRStrCpy (_STR pcString1, _CSTR pcString2)
{
    _CHAR *p;
    
    p = pcString1;
    while((*pcString1++=*pcString2++) != 0);
    return(p);
}

UCHR * HWRStrCpyW( UCHR * pcString1, CUCHR * pcString2)
{
    UCHR *p;
    
    p = pcString1;
    while((*pcString1++=*pcString2++) != 0);
    return(p);
}


/**************************************************************************
 *                                                                         *
 *    strcat.                                                              *
 *                                                                         *
 **************************************************************************/

_STR HWRStrCat(_STR pcString1, _CSTR pcString2)
{
    _CHAR *p;
    
    p = pcString1-1;
    while(*++p != 0);
    while((*p++=*pcString2++) != 0);
    return(pcString1);
}

UCHR * HWRStrCatW( UCHR *  pcString1, CUCHR *  pcString2)
{
    UCHR *p;
    
    p = pcString1-1;
    while(*++p != 0);
    while((*p++=*pcString2++) != 0);
    return(pcString1);
}

/**************************************************************************
 *                                                                         *
 *    strncat.                                                              *
 *                                                                         *
 **************************************************************************/

_STR  HWRStrnCat (_STR pcString1, _CSTR pcString2, _WORD len)
{
    _CHAR *p;
    
    p = pcString1+len;
    while((*p++=*pcString2++) != 0);
    return(pcString1);
}


/**************************************************************************
 *                                                                         *
 *    strrev.                                                              *
 *                                                                         *
 **************************************************************************/

_STR    HWRStrRev(_STR zString)
{
    _CHAR *p,*p1;
    int i,c;
    
    p = zString-1;
    p1 = zString;
    while(*++p != 0);
    i=((int)(p-p1))>>1;
    while(i-- != 0)
    {
        c=(int)*p1;
        *p1++=*--p;
        *p=(_CHAR)c;
    }
    return zString;
}


/**************************************************************************
 *                                                                         *
 *    strrchr.                                                             *
 *                                                                         *
 **************************************************************************/

_STR    HWRStrrChr(_STR zString, _INT iChar)
{
    _CHAR *p;
    
    zString--;
    p=_NULL;
    while(*++zString != 0)
    {
        if(((_UCHAR)*zString) == (_UCHAR)iChar)
            p = zString;
    }
    return (p);
}

/**************************************************************************
 *                                                                         *
 *    strncpy.                                                             *
 *                                                                         *
 **************************************************************************/

_STR HWRStrnCpy(_STR zString1, _CSTR zString2, _WORD wNumber)
{
    _CHAR *p;
    
    if(wNumber == 0)
        return(zString1);
    p = zString1-1;
    zString2--;
    while((*++p=*++zString2) != 0 && --wNumber != 0  );
    if(wNumber == 0)
        *++p = 0;
    return(zString1);
}

UCHR *  HWRStrnCpyW( UCHR *  zString1, CUCHR *  zString2, _INT wNumber)
{
    UCHR *p;
    
    if(wNumber == 0)
        return(zString1);
    p = zString1-1;
    zString2--;
    while((*++p=*++zString2) != 0 && --wNumber != 0  );
    if(wNumber == 0)
        *++p = 0;
    return(zString1);
}

/**************************************************************************
 *                                                                         *
 *    strcmp.                                                              *
 *                                                                         *
 **************************************************************************/

_INT  HWRStrCmp (_CSTR zString1, _CSTR zString2)
{
    zString1--;
    zString2--;
    while(*(++zString1) == *(++zString2) && *zString1 != 0 && *zString2 != 0) ;
    
    return((_INT)(*zString1 - *zString2));
}

_INT HWRStrCmpW( CUCHR * zString1, CUCHR * zString2)
{
    zString1--;
    zString2--;
    while(*(++zString1) == *(++zString2) && *zString1 != 0 && *zString2 != 0) ;
    
    return((_INT)(*zString1 - *zString2));
}

_INT HWRStrCaseCmpW( CUCHR * zString1, CUCHR * zString2)
{
    zString1--;
    zString2--;
    while( tolower( *(++zString1) ) == tolower( *(++zString2) ) && *zString1 != 0 && *zString2 != 0) ;
    
    return((_INT)(tolower( *zString1 ) - tolower( *zString2) ));
}

/**************************************************************************
 *                                                                         *
 *    StrEq.                                                               *
 *                                                                         *
 **************************************************************************/

static _BOOL    HWRStrEmpty (_CSTR zString)
{
    if (!zString)
        return _TRUE;
    if (!*zString)
        return _TRUE;
    return _FALSE;
}

_BOOL HWRStrEq(_CSTR zString1, _CSTR zString2)
{
    if (HWRStrEmpty (zString1) && HWRStrEmpty (zString2))
        return _TRUE;
    if (HWRStrCmp (zString1, zString2) == 0)
        return _TRUE;
    else
        return _FALSE;
}


/**************************************************************************
 *                                                                         *
 *    memcpy.                                                              *
 *                                                                         *
 **************************************************************************/
#ifndef HWRMemCpy

p_VOID  HWRMemCpy(p_VOID pcDest, p_VOID pcSrc, _WORD  wNumber)
{
    memmove(pcDest, pcSrc, wNumber);
    return(pcDest);
}

#endif /* HWRMemCpy */

/**************************************************************************
 *                                                                         *
 *    memset.                                                              *
 *                                                                         *
 **************************************************************************/

#ifndef HWRMemSet

p_VOID  HWRMemSet(p_VOID pcDest, _UCHAR ucChar, _WORD  wNumber)
{
    memset(pcDest, ucChar, wNumber);
    return(pcDest);
}
#endif /* HWRMemSet */


/**************************************************************************
 *                                                                         *
 *    stedup.                                                              *
 *                                                                         *
 **************************************************************************/

#ifdef HW_RECINT_UNICODE

UCHR * HWRStrDupW( CUCHR * stringIn )
{
    _INT    length = HWRStrLenW( stringIn );
    if ( length < 1 )
        return NULL;
    UCHR * stringOut = (UCHR *)HWRMemoryAlloc( sizeof( UCHR ) * (length+2) );
    if ( stringOut != NULL )
    {
        HWRStrnCpyW( stringOut, stringIn, length+1 );
    }
    return stringOut;
}

UCHR * HWRStrDupW( const char * stringIn )
{
    _INT    length = HWRStrLen( stringIn );
    if ( length < 1 )
        return NULL;
    UCHR * stringOut = (UCHR *)HWRMemoryAlloc( sizeof( UCHR ) * (length+2) );
    if ( stringOut != NULL )
    {
        StrToUNICODE( stringOut, stringIn, length+1 );
    }
    return stringOut;
}

#endif // HW_RECINT_UNICODE

char * HWRStrDupA( CUCHR * stringIn )
{
    _INT    length = HWRStrLenW( stringIn );
    if ( length < 1 )
        return NULL;
    char * stringOut = (char *)HWRMemoryAlloc( sizeof( char ) * (length+2) );
    if ( stringOut != NULL )
    {
#ifdef HW_RECINT_UNICODE
        UNICODEtoStr( stringOut, stringIn, length+1 );
#else
        HWRStrCpy( stringOut, stringIn );
#endif // HW_RECINT_UNICODE
    }
    return stringOut;
}


/**************************************************************************
 *                                                                         *
 *    itoa and ftoa                                                        *
 *                                                                         *
 **************************************************************************/

static void reverse(UCHR s[])
{
    int i, j;
    UCHR c;
    
    for (i = 0, j = _STRLEN(s)-1; i<j; i++, j--)
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void HWRIntToStrW( long n, UCHR s[] )
{
    long sign;
    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    long i = 0;
    do
    {
        /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}

typedef union {
	int L;
	float F;
} LF_t;


static double PRECISION = 0.00001;

/**
 * Double to ASCII
 */

static int log10( float f )
{
    int r = 0;
    while( f > 10 )
    {
        f = f/10;
        r++;
    }
    return r;
}

static float pow10( float f, int p )
{
    float r = f;
    if ( p < 0 )
    {
        r = 1.0;
        for ( int i = p; i < 0; i++ )
        {
            r = r / f;
        }
    }
    else if ( p == 0 )
    {
        return 1.0;
    }
    else
    {
        for ( int i = 1; i < p; i++ )
        {
            r = r * f;
        }
    }
    return r;
}

static int floor( float f)
{
    return (int)f;
}

UCHR * HWRFloatToStrW( UCHR * s, float n )
{
    // handle special cases
    if (n == 0.0)
    {
        s[0] = '0';
        s[1] = '\0';
    }
    else
    {
        int digit, m, m1 = 0;
        UCHR * c = s;
        int neg = (n < 0);
        if (neg)
            n = -n;
        // calculate magnitude
        m = log10(n);
        int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
        if (neg)
            *(c++) = '-';
        // set up for scientific notation
        if (useExp) {
            if (m < 0)
                m -= 1;
            n = n / pow10(10.0, m);
            m1 = m;
            m = 0;
        }
        if (m < 1.0) {
            m = 0;
        }
        // convert the number
        while (n > PRECISION || m >= 0)
        {
            float weight = pow10(10.0, m);
            if (weight > 0)
            {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0)
                *(c++) = '.';
            m--;
        }
        if (useExp)
        {
            // convert the exponent
            int i, j;
            *(c++) = 'e';
            if (m1 > 0)
            {
                *(c++) = '+';
            } else {
                *(c++) = '-';
                m1 = -m1;
            }
            m = 0;
            while (m1 > 0) {
                *(c++) = '0' + m1 % 10;
                m1 /= 10;
                m++;
            }
            c -= m;
            for (i = 0, j = m-1; i<j; i++, j--)
            {
                // swap without temporary
                c[i] ^= c[j];
                c[j] ^= c[i];
                c[i] ^= c[j];
            }
            c += m;
        }
        *(c) = '\0';
    }
    return s;
}

#ifdef HW_RECINT_UNICODE

#define MAX_XU_CONVERTS 5

static int _iHexes[MAX_XU_CONVERTS] = {	0x08a, 0x08c, 0x09a, 0x09c, 0x09f};
static int _iUnicodes[MAX_XU_CONVERTS] = { 352, 338, 353, 339, 376};

/* ************************************************************************** */
/* *   Convert char string to UNICODE                                       * */
/* ************************************************************************** */

int StrToUNICODE(UCHR * tstr, const char * str, int cMax)
{
	int i, j;
    
	for (i = 0; i < cMax-1 && str[i] != 0; i ++)
	{
		tstr[i] = (UCHR)(unsigned char)str[i];
		if((unsigned char)str[i]>=0x08a && (unsigned char)str[i]<=0x09f)
        {
            for (j=0; j<MAX_XU_CONVERTS; j++)
            {
                if((unsigned char)str[i]==_iHexes[j])
                    tstr[i] = _iUnicodes[j];
            }
        }
	}
    
	tstr[i] = 0;
	return i;
}

/* ************************************************************************** */
/* *   Convert UNICODE string to char                                       * */
/* ************************************************************************** */

int UNICODEtoStr(char * str, CUCHR * tstr, int cMax)
{
	int i, j;
    
	for (i = 0; i < cMax && tstr[i] != 0; i ++)
	{
		str[i] = ((unsigned char)tstr[i]);
		if(tstr[i]>=338 && tstr[i]<=376)
        {
            for (j=0; j<MAX_XU_CONVERTS; j++)
            {
                if(tstr[i]==_iUnicodes[j])
                    str[i] = (unsigned char)_iHexes[j];
            }
        }
	}
    
	str[i] = 0;
	return i;
}

/* ************************************************************************** */
/* *   Convert char to UNICODE                                              * */
/* ************************************************************************** */

CUCHR CharToUNICODE(char ch)
{
	int i;
	if((unsigned char)ch>=0x08a && (unsigned char)ch<=0x09f)
    {
        for(i=0; i<MAX_XU_CONVERTS; i++)
        {
            if((unsigned char)ch==_iHexes[i])
                return _iUnicodes[i];
        }
    }
    return (UCHR)(unsigned char)ch;
}

#endif // HW_RECINT_UNICODE

