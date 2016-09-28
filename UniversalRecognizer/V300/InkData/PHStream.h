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

#ifndef PHSTREAM_H
#define PHSTREAM_H

#include <stdio.h>
#include "PHTemplArray.h"
#include "recotypes.h"

#define MEMSTR_ALLOC_GRAN	4096

class CPHStream
{
public:
	CPHStream(){}
	virtual ~CPHStream(){}

	virtual BOOL Write( void *pBytes, long size ) = 0;
	virtual BOOL Read( void *pBytes, long size ) = 0;
	virtual long  SetPos( long iOffset, int iOrigin ) = 0;
    virtual long  GetSize( void ) = 0;
};

class CPHFileStream : public CPHStream
{
public:
	CPHFileStream();
	CPHFileStream( FILE * pFile );
	virtual ~CPHFileStream();

	virtual BOOL Write( void *pBytes, long size );
	virtual BOOL Read( void *pBytes, long size );
	virtual long  SetPos( long iOffset, int iOrigin );
    virtual long  GetSize( void );

	void SetHandle( FILE * pFile );
	void Close( void );

private:
	FILE *	m_pFile;
	BOOL	m_bCloseOnDelete;
};

class CPHMemStream : public CPHStream
{
public:
	CPHMemStream();
	CPHMemStream( LPBYTE pBytes, long iSize );
	virtual ~CPHMemStream();

	virtual BOOL Write( void *pBytes, long size );
	virtual BOOL Read( void *pBytes, long size );
	virtual long  SetPos( long iOffset, int iOrigin );
    virtual long GetSize( void ) { return m_nUsed; }

	void FreeData();
	void SetData( LPBYTE pBytes, int size, BOOL bCopy = false );

private:
	LPBYTE	m_pData;
	long	m_nAllocated;
	long	m_nUsed;
	long	m_nCurPos;
	BOOL	m_bCloseOnDelete;
	BOOL	m_bReadOnly;
};

#endif // PHSTREAM_H


