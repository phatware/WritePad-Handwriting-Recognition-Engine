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

#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "InkWrapper.h"
#include "RecognizerWrapper.h"

#include <iostream>

#define MAX_PATH	300

#define SEGMENT2            2
#define SEGMENT3            3
#define SEGMENT4            4

#define SEGMENT_DIST_1      3
#define SEGMENT_DIST_2      6
#define SEGMENT_DIST_3      12

static const char *  USER_STATISTICS = "WritePad_Stat.lrn";
static const char *  USER_CORRECTOR	=  "WritePad_Corr.cwl";
static const char *  USER_DICTIONARY = "WritePad_User.dct";

static RECOGNIZER_PTR _recognizer = NULL;

static CGPoint _lastPoint = {0};
static CGPoint _previousLocation = {0};
static int strokeLen = 0;
static int nStroke = 0;

static char _szPath[MAX_PATH] = "";

using namespace std;

static void recognizeCSVFile( const char * buffer );
static int  recognizerInit( int nLanguage, const char * path );
static void freeRecognizer( void );


int main(int argc, const char * argv[])
{
	cout << "WritePad Recognizer Test" << endl; // prints WritePad Recognizer Test

	if ( argc < 2 )
	{
		cout << "Usage: writepadreco <filename>" << endl;
		return 0;
	}


	cout << " Starting Task" << endl;

    // read the entire file into a buffer
	char *  buffer = NULL;
	const char  * filename = argv[1];
    FILE * file = fopen( filename, "r" );
    if ( file != NULL )
    {
    	fseek( file, 0, SEEK_END );
    	size_t len = ftell( file );
    	fseek( file, 0, SEEK_SET );
    	if ( len > 1 )
    	{
    		buffer = (char *)malloc( len+4 );
    		if( buffer != NULL )
    		{
    			if ( fread( buffer, sizeof( char ), len, file ) < 1 )
    			{
    				free( (void *)buffer );
    				buffer = NULL;
    			}
    		}
    	}
    	fclose( file );
    }

    if ( buffer != NULL )
    {
    	int res = recognizerInit( LANGUAGE_ENGLISH, NULL );
    	cout << "recognizerInit: FALGS " << res << endl;
    	recognizeCSVFile( buffer );
    	freeRecognizer();
    	cout << endl << "Task has been completed!" << endl << endl;
    }
    else
    {
    	cout << "File " << filename << " did not load. Try another file." << endl;
    }
    free( (void *)buffer );

	return 0;
}


static int AddPixelsX( CGFloat x, CGFloat y, INK_DATA_PTR iData, BOOL bLastPoint )
// this method called from inkCollectorThread
{
    CGFloat	xNew, yNew, x1, y1;
    CGFloat nSeg = SEGMENT3;
    int     i, nPoints = 0;
    CGFloat dx, dy;

    if  ( strokeLen < 1 )
    {
        INK_AddPixelToStroke( iData, nStroke, x, y, 127 );
        _lastPoint.x = x;
        _lastPoint.y = y;
        _previousLocation = _lastPoint;
        strokeLen = 1;
        return  1;
    }

    dx = fabs( x - _lastPoint.x );
    dy = fabs( y - _lastPoint.y );

    if  ( dx + dy < 2.0f )
        return 0;

    if ( dx + dy > 100.0f * SEGMENT_DIST_2 )
        return 0;

    if  ( (dx + dy) < SEGMENT_DIST_1 )
    {
        INK_AddPixelToStroke( iData, nStroke, x, y, 127 );
        _lastPoint.x = x;
        _lastPoint.y = y;
        _previousLocation = _lastPoint;
        strokeLen++;
        return  1;
    }

    if ( (dx + dy) < SEGMENT_DIST_2 )
    {
        nSeg = SEGMENT2;
    }
    else if ( (dx + dy) >= SEGMENT_DIST_3 )
    {
        nSeg = SEGMENT4;
    }
    else
    {
        nSeg = SEGMENT3;
    }
    for ( i = 1;  i < (int)nSeg;  i++ )
    {
        x1 = _previousLocation.x + ((x - _previousLocation.x)*i ) / nSeg;  //the point "to look at"
        y1 = _previousLocation.y + ((y - _previousLocation.y)*i ) / nSeg;  //the point "to look at"

        xNew = _lastPoint.x + (x1 - _lastPoint.x) / nSeg;
        yNew = _lastPoint.y + (y1 - _lastPoint.y) / nSeg;

        if ( xNew != _lastPoint.x || yNew != _lastPoint.y )
        {
            INK_AddPixelToStroke( iData, nStroke, xNew, yNew, 127 );
            _lastPoint.x = xNew;
            _lastPoint.y = yNew;
            strokeLen++;
            nPoints++;
        }
    }

    if ( bLastPoint )
    {
		// add last point
        if ( x != _lastPoint.x || y != _lastPoint.y )
        {
            INK_AddPixelToStroke( iData, nStroke, x, y, 127 );
            _lastPoint.x = x;
            _lastPoint.y = y;
            strokeLen++;
            nPoints++;
        }
    }

	_previousLocation.x = x;
    _previousLocation.y = y;
    return nPoints;
}

static const char * getNextToken( const char * buffer, int * pOffset, BOOL * endofrow, BOOL * endoffile )
{
	char ch1, ch = 0;
	BOOL bQuotes = false;
    static char strToken[100];

    *strToken = 0;
	*endofrow = false;
	*endoffile = false;
	while ( (ch = buffer[(*pOffset)++]) )
	{
        if ( bQuotes )
        {
            if ( ch == '\r' )
                continue;
            else if ( ch == '\"' )
            {
				ch1 = buffer[(*pOffset)++];
                if ( ch1 == '\"' )
				{
                    strcat( strToken, "\"" );
				}
                else
				{
					(*pOffset)--;
                    bQuotes = false;
				}
            }
            else
			{
                char sz[2];
                sz[0] = ch; sz[1] = 0;
                strcat( strToken, sz );
			}
        }
        else
        {
            if ( ch == '\r' )
			{
                // ignore \r
			}
            else if ( ch == '\n' )
            {
				// end or row
                *endofrow = true;
                break;
            }
            else if ( ch == '\"' )
                bQuotes = true;
            else if ( ch == ',' )
                break;		// end of column
            else
            {
                char sz[2];
                sz[0] = ch; sz[1] = 0;
                strcat( strToken, sz );
            }
        }
	}
	if ( ch == 0 )
	{
		*endofrow = true;
		*endoffile = true;
	}
	return strToken;
}

static bool recognizeInkData( INK_DATA_PTR inInkData, int iConfig )
{
    const UCHR * recognizedText;

    HWR_Reset( _recognizer );
    unsigned int flags = HWR_GetRecognitionFlags( _recognizer );
    flags &= ~FLAG_SINGLEWORDONLY;
    flags &= ~FLAG_SMOOTHSTROKES;
    // flags |= FLAG_STATICSEGMENT;
    if ( iConfig == 0 )
    {
        flags &= ~FLAG_SEPLET;
        flags &= ~FLAG_ONLYDICT;
        cout << "\n\nRecognition Configuration 1: FLAG_SEPLET - OFF, FLAG_ONLYDICT - OFF\n\n";
    }
    else if ( iConfig == 1 )
    {
        flags |= FLAG_SEPLET;
        flags &= ~FLAG_ONLYDICT;
        cout << "\n\nRecognition Configuration 2: FLAG_SEPLET - ON, FLAG_ONLYDICT - OFF\n\n";
    }
    else if ( iConfig == 2 )
    {
        flags &= ~FLAG_SEPLET;
        flags |= FLAG_ONLYDICT;
        cout << "\n\nRecognition Configuration 3: FLAG_SEPLET - OFF, FLAG_ONLYDICT - ON\n\n";
    }
    else if ( iConfig == 3 )
    {
        flags |= FLAG_SEPLET;
        flags |= FLAG_ONLYDICT;
        cout << "\n\nRecognition Configuration 4: FLAG_SEPLET - ON, FLAG_ONLYDICT - ON\n\n";
    }
    else if ( iConfig == 4 )
    {
        UCHR  custpunct[] = { ',', '.', '!', '\'', '?', '\0' };
        flags |= FLAG_SEPLET;
        flags |= FLAG_ONLYDICT;
        flags |= FLAG_USECUSTOMPUNCT;
        HWR_SetCustomCharset( _recognizer, NULL, custpunct );
        cout << "\n\nRecognition Configuration 5: FLAG_SEPLET - ON, FLAG_ONLYDICT - ON, Custom punctuation.\n\n";
    }
    HWR_SetRecognitionFlags( _recognizer, flags );

	recognizedText = HWR_RecognizeInkData( _recognizer, inInkData, 0, -1, true, false, false, false );
	if (recognizedText == NULL || *recognizedText == 0)
		return false;

	int len2 = 0;
	for ( len2 = 0; len2 < 20000 && recognizedText[len2] != 0; len2++)
		;

	char * string = (char *)malloc( len2 + 2 );
	if ( string != NULL )
	{
		char * p = string;
		for ( int i = 0; i < len2; i++ )
		{
			*p++ = (unsigned char)recognizedText[i];
		}
		*p = 0;

		cout << string << endl;

		free( (void *)string );
	}

    return true;
}


static void recognizeCSVFile( const char * buffer )
{
    BOOL		endofrow = false;
    BOOL		endoffile = false;
    int         i, offset = 0;
    int     	column = 0;
    CGFloat     minX = 100000, minY = 100000;
    CGFloat     maxX = 0, maxY = 0;
    CGRect      rect;

    INK_DATA_PTR	iData = INK_InitData();

    COLORREF	coloref = 0;
    nStroke = INK_AddEmptyStroke( iData, 3, coloref );
    strokeLen = 0;

    CGPoint pt = {0};
    while ( ! endoffile )
    {
        const char * strToken = getNextToken( buffer, &offset, &endofrow, &endoffile );
        if ( *strToken == 0 )
        {
            break;
        }
        if ( strlen( strToken ) > 0 )
        {
            switch( column )
            {
                case 0 :
                    pt.x = atof( strToken );
                    pt.x /= 2.0;
                    if ( pt.x <= 0 )
                        break;
                    if ( pt.x < minX  )
                        minX = pt.x;
                    if ( pt.x > maxX  )
                        maxX = pt.x;
                    break;

                case 1 :
                    pt.y = atof( strToken );
                    pt.y /= 2.0;
                    if ( pt.y <= 0 )
                        break;
                    if ( pt.y < minY  )
                        minY = pt.y;
                    if ( pt.y > maxY  )
                        maxY = pt.y;
                    break;
            }
        }

        column++;
        if ( endofrow )
        {
            if ( pt.x == 0 || pt.y == 0 )
            {
                // skip this point
            }
            else if ( pt.x < 0 || pt.y < 0 )
            {
                if ( strokeLen > 0 )
                {

                    AddPixelsX( _previousLocation.x, _previousLocation.y, iData, true );
                    nStroke = INK_AddEmptyStroke( iData, 3, coloref );
                    strokeLen = 0;
                }
            }
            else
            {
                //INK_AddPixelToStroke( iData, nStroke, pt.x, pt.y, 127 );
                //strokeLen++;
                AddPixelsX( pt.x, pt.y, iData, false );
            }
            pt.x = pt.y = 0;
            column = 0;
        }
    }


    minX--;
    minY--;
    for ( i = 0; i < INK_StrokeCount( iData, false ); i++ )
    {
        INK_MoveStroke( iData, i, -minX, -minY, &rect, false );
    }

    for ( i = 0; i < 5; i++ )
    {
        if ( ! recognizeInkData( iData, i ) )
        	break;
    }

    INK_FreeData( iData );
}

static int recognizerInit( int nLanguage, const char * path )
{
	char userDict[MAX_PATH];
	char mainDict[MAX_PATH];
	char learner[MAX_PATH];
	char corrector[MAX_PATH];

	userDict[0] = 0;
	learner[0] = 0;
	corrector[0] = 0;
	mainDict[0] = 0;
	if (path != NULL && *path != 0 )
    {
		strcpy(_szPath, path);
		strcat(_szPath, "/");
		strcpy(userDict, _szPath);
		strcpy(learner, _szPath);
		strcpy(corrector, _szPath);
	}
	strcat(userDict, USER_DICTIONARY);
	strcat(learner, USER_STATISTICS);
	strcat(corrector, USER_CORRECTOR);


    if ( ! HWR_IsLanguageSupported( nLanguage ) )
        nLanguage = LANGUAGE_ENGLISH;

    strcpy( mainDict, "Dictionaries/" );
    switch ( nLanguage )
    {
        case LANGUAGE_ENGLISH :
            strcat( mainDict, "English.dct" );
            break;

        case LANGUAGE_GERMAN :
            strcat( mainDict, "German.dct" );
            break;

        case LANGUAGE_FRENCH :
            strcat( mainDict, "French.dct" );
            break;

        case LANGUAGE_ITALIAN :
            strcat( mainDict, "Italian.dct" );
            break;

        case LANGUAGE_SPANISH :
            strcat( mainDict, "Spanish.dct" );
            break;

        case LANGUAGE_SWEDISH :
            strcat( mainDict, "Swedish.dct" );
            break;

        case LANGUAGE_NORWEGIAN :
            strcat( mainDict, "Norwegian.dct" );
            break;

        case LANGUAGE_DUTCH :
            strcat( mainDict, "Dutch.dct" );
            break;

        case LANGUAGE_DANISH :
            strcat( mainDict, "Danish.dct" );
            break;

        case LANGUAGE_PORTUGUESE :
            strcat( mainDict, "Portuguese.dct" );
            break;

        case LANGUAGE_PORTUGUESEB :
            strcat( mainDict, "Brazilian.dct" );
            break;

        case LANGUAGE_FINNISH :
            strcat( mainDict, "Finnish.dct" );
            break;
    }

	int flags = -1;
	_recognizer = HWR_InitRecognizer(mainDict, userDict, learner, corrector, nLanguage, &flags);
	if (NULL == _recognizer)
		return -1;
	return flags;
}

static void freeRecognizer( void )
{
	if (_recognizer != NULL)
    {
        if ( _szPath[0] == 0 )
        {
            HWR_FreeRecognizer(_recognizer, NULL, NULL, NULL );
        }
        else
        {
            char userDict[MAX_PATH];
            char learner[MAX_PATH];
            char corrector[MAX_PATH];

            userDict[0] = 0;
            learner[0] = 0;
            corrector[0] = 0;
            strcpy(userDict, _szPath);
            strcpy(learner, _szPath);
            strcpy(corrector, _szPath);
            strcat(userDict, USER_DICTIONARY);
            strcat(learner, USER_STATISTICS);
            strcat(corrector, USER_CORRECTOR);

            HWR_FreeRecognizer(_recognizer, userDict, learner, corrector);
        }
		_recognizer = NULL;
	}
}


