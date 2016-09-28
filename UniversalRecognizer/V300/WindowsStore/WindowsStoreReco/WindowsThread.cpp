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

#include <windows.h>
#include <windowsx.h>
#include <WinBase.h>

class RecoWindowsThread
{
public:
	RecoWindowsThread() 
	{
		m_dwThreadID = 0;
		m_hThread = NULL;
		InitializeCriticalSectionEx(  &m_mutexReco, 1000, 0 );
		InitializeCriticalSectionEx(  &m_mutexResult, 1000, 0 );
		InitializeCriticalSectionEx(  &m_mutexAsyncReco, 1000, 0 );
		InitializeCriticalSectionEx(  &m_mutexQueue, 1000, 0 );
	}
	~RecoWindowsThread()
	{
		DeleteCriticalSection( &m_mutexReco );
		DeleteCriticalSection( &m_mutexAsyncReco );
		DeleteCriticalSection( &m_mutexResult );
		DeleteCriticalSection( &m_mutexQueue );
	}

	BOOL StartThread( LPTHREAD_START_ROUTINE function, LPVOID param )
	{
		m_dwThreadID = 0;
		m_hThread = ::CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)function, param, 0, &m_dwThreadID );	
		::SetThreadPriority( m_hThread, THREAD_PRIORITY_NORMAL );

		return TRUE;
	}


public:
	DWORD					m_dwThreadID;
	HANDLE					m_hThread;
    CRITICAL_SECTION		m_mutexReco;		
    CRITICAL_SECTION		m_mutexResult;
    CRITICAL_SECTION		m_mutexAsyncReco;
    CRITICAL_SECTION		m_mutexQueue;
};

