/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2017 PhatWare (r) Corp. All rights reserved.
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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <atomic>

#ifdef MAC_OS_X
#include <xlocale.h>
#include <mach/mach.h>
#include <libkern/OSAtomic.h>
#elif defined(_WIN32)
#define strcasecmp _stricmp
#endif // MAC_OS_X

#include "InternalTypes.h"
#include "RecognizerWrapper.h"
#include "RecognizerApi.h"
#include "PhatCalc.h"
#include "WordLrnFile.h"
#include "WordFile.h"
#include "reco.h"

#include "bastypes.h"
#include "InkData.h"
#include "zctype.h"

#include "pydict.h"

#include "wordlist_ger.h"
#include "wordlist_frn.h"
#include "wordlist_spn.h"
#include "wordlist_pbr.h"
#include "wordlist_dan.h"
#include "wordlist_dut.h"
#include "wordlist_enu.h"
#include "wordlist_prt.h"
#include "wordlist_itl.h"

#ifndef _WIN32
#include <pthread.h>
#define INTERNAL_RECO_THREAD	1
#endif // _WIN32

#define DEFAULT_TIMEOUT         20

// word corrector for unknown languages
static const char * g_szDefWords[] = {
    "phatware", "PhatWare",
};

// NOTE: DO NOT CHANGE THE COPYRIGHT NOTICE. Changing the copyright notice below will violate WritePad source code license
static const char * g_szDescription = "WritePad(r) Multilingual Handwriting Recognition Engine. Copyright (c) 2001-2017 Stan Miasnikov, PhatWare(r) Corp. All rights reserved.";

#define DEFWORDCNT	2

//#ifndef MAC_OS_X
//
//inline BOOL OSAtomicTestAndSet( int bit, UInt32 * val )
//{
//    BOOL res = (0 != ((*val) & (0x1 << bit)));
//    (*val) |= (0x1 << bit);
//    return res;
//}
//
//inline BOOL OSAtomicTestAndClear( int bit, UInt32 * val )
//{
//    BOOL res = (0 != ((*val) & (0x1 << bit)));
//    (*val) &= ~(0x1 << bit);
//    return res;
//}
//
//#endif // MAC_OS_X

static const char * g_pszCapWords[] = {
    ("Monday"),
    ("Tuesday"),
    ("Wednesday"),
    ("Thursday"),
    ("Sunday"),
    ("Saturday"),
    ("Friday"),
    ("December"),
    ("January"),
    ("February"),
    ("March"),
    ("April"),
    // "May",       // do not always capitalize word "may"...
    ("June"),
    ("July"),
    ("August"),
    ("September"),
    ("October"),
    ("November"),
    ("PhatWare")
};

static const char * g_szDefUserWords[] = {
    "",
};

// list of words added to dictionary when switching to Internet mode
static const char * g_szInternetWords[] = {
    "www",
    "www.",
    "com",
    "net",
    "org",
    "mobi",
    ".com",
    ".net",
    ".org",
    ".mobi",
    ".co",
    ".uk",
    "gov",
    "tv",
    ".tv",
    ".gov",
    ".nz",
    ".de",
    ".au",
    ".fr",
    ".ca",
    "edu",
    ".edu",
    "ca",
    "fr",
    "de",
    "uk",
    "nz",
    "au",
    "ru",
    ".ru",
    "wolframalpha",
    "wolframalpha.com",
    "http",
    "http:",
    "https",
    "https:",
    "https://",
    "http://",
    "ftp",
    "ftp:",
    "ftp://",
    "//",
    "/",
    "@",
    "&",
    "%",
    ".",
    "web",
    "google",
    "yahoo",
    "youtube",
    "www.google.com",
    "www.yahoo.com",
    "www.youtube.com",
    "www.bing.com",
    "www.facebook.com",
    "www.myspace.com",
    "www.apple.com",
    "www.witter.com",
    "www.linkedin.com",
    "www.bing.com",
    "google.com",
    "yahoo.com",
    "youtube.com",
    "bing.com",
    "facebook.com",
    "myspace.com",
    "apple.com",
    "witter.com",
    "linkedin.com",
    "bing.com",
    "bing",
    "maps",
    "email",
    "myspace",
    "wiki",
    "wikipedia",
    "msn",
    "baidu",
    "facebook",
    "qq",
    "rapidshare",
    "ebay",
    "twitter",
    "linkedin",
    "amazon",
    "aol",
    "hi5",
    "blogger",
    "blog",
    "blogspot",
    "cnn",
    "cnet"
};

static UCHR g_szCalcDelims[] = { '.', ',' };
static CUCHR g_space[2] = { ' ', '\0' };

#define LANG_ID				1

#define USERWORD_CNT		(sizeof(g_szDefUserWords)/sizeof(g_szDefUserWords[0]))
#define CAPWORD_CNT			(sizeof(g_pszCapWords)/sizeof(g_pszCapWords[0]))
#define INTERNETWORD_CNT	(sizeof(g_szInternetWords)/sizeof(g_szInternetWords[0]))


#define RECO_DEFAULT_FLAGS	(FLAG_USERDICT | FLAG_MAINDICT | FLAG_ANALYZER | FLAG_SPELLIGNORENUM | FLAG_SPELLIGNOREUPPER | FLAG_CORRECTOR | FLAG_ENABLECALC | FLAG_NOSINGLELETSPACE | FLAG_SMOOTHSTROKES)

#define	AssertNoError(inMessage, inHandler)			\
if(result != noErr)									\
{													\
/*printf("%s: %d\n", inMessage, (int)result);*/		\
goto inHandler;                                     \
}

#define AssertNoOALError(inMessage, inHandler)		\
if((result = alGetError()) != AL_NO_ERROR)			\
{													\
/*printf("%s: %x\n", inMessage, (int)result);*/		\
goto inHandler;                                     \
}

#define TRACE_BREAK         (-1)
#define RECQUEUE_MAX_ELEMS  50
#define PIX_OFFSET			1.0
#define	MAX_PIXEL_VALUE		8000
#define STROKE_BUFFER_SIZE	1024

/* -------------------------------------------------------------------------------- */
/* TraceToRec                                                               	    */
/* -------------------------------------------------------------------------------- */

static int TraceToRec( int in_numpts, LPPOINTS in_pt, p_RECO_point_type out_pt )
{
    float	xFltPrv, yFltPrv, xPrv, yPrv;
    int		m_nPointsRec = 1, NumFilteredPoints;
    float	xToRec, yToRec;
    register int i, j; //, x, y;
    
    xPrv = xFltPrv =(in_pt[0].x*2.0f);
    yPrv = yFltPrv = (in_pt[0].y*2.0f);
    out_pt[0].x = (short)xPrv;
	out_pt[0].y = (short)yPrv;
    for( i = 1; i < in_numpts; i++ )
    {
        xToRec = in_pt[i].x*2.0f;
        yToRec = in_pt[i].y*2.0f;
        if ( i == 1 )
            NumFilteredPoints=1;
        else
            NumFilteredPoints=N_SEGS;
        for ( j = 1; j <= NumFilteredPoints; j++ ) 
        {
            float xi = (xToRec - xPrv) * j / N_SEGS + xPrv;
            float yi = (yToRec - yPrv) * j / N_SEGS + yPrv;
            float xf = (xi - xFltPrv) / N_SEGS + xFltPrv;
            float yf = (yi - yFltPrv) / N_SEGS + yFltPrv;
			if ( fabsf( xf - xFltPrv ) > 1.0f || fabsf( yf - yFltPrv ) > 1.0f )
            {
                xFltPrv = xf; 
                yFltPrv = yf; 
				out_pt[m_nPointsRec].x = (short)xFltPrv;
				out_pt[m_nPointsRec++].y = (short)yFltPrv;
            } 
        }
        xPrv = xToRec;
        yPrv = yToRec;
    }
    out_pt[m_nPointsRec].x  = (short)(in_pt[in_numpts-1].x*2.0f);
    out_pt[m_nPointsRec++].y= (short)(in_pt[in_numpts-1].y*2.0f);
    // add brake to the end
    out_pt[m_nPointsRec].x = 0;
    out_pt[m_nPointsRec++].y = TRACE_BREAK;
    return m_nPointsRec;
}	/* end of TraceToRec */

/* -------------------------------------------------------------------------------- */
/* filter_trajectory                                                                */
/* -------------------------------------------------------------------------------- */
static int FilterTrajectory( int innumpts, LPPOINTS instrkin, p_RECO_point_type * iostrkout, BOOL bSmooth )
{
    if ( bSmooth )
    {
        int	outbuflen = innumpts * (N_SEGS + 2);
        
        *iostrkout = (p_RECO_point_type)malloc( sizeof( RECO_point_type ) *(outbuflen+2) );
        if ( NULL == *iostrkout )
            return 0;
        
        return TraceToRec( innumpts, instrkin, *iostrkout );
    }
    else
    {
        p_RECO_point_type out_pt = (p_RECO_point_type)malloc( sizeof( RECO_point_type ) *(innumpts+2) );
        if ( NULL == out_pt )
            return 0;
        int i = 0;
        for( i = 0; i < innumpts; i++ )
        {
            out_pt[i].x = (short)(instrkin[i].x);
            out_pt[i].y = (short)(instrkin[i].y);
        }
        out_pt[i].x = 0;
        out_pt[i].y = TRACE_BREAK;
        *iostrkout = out_pt;
        return innumpts;
    }
}

#ifdef INTERNAL_RECO_THREAD
//==================================================================================================
//	Threading functions
//==================================================================================================
class	OpenALThread
{
    // returns the thread's priority as it was last set by the API
#define OpenALThread_SET_PRIORITY				0
    // returns the thread's priority as it was last scheduled by the Kernel
#define OpenALThread_SCHEDULED_PRIORITY			1
    
    //	Types
public:
    typedef void*			(*ThreadRoutine)(void* inParameter);
    
    //	Constants
public:
    enum
    {
        kMinThreadPriority = 1,
        kMaxThreadPriority = 63,
        kDefaultThreadPriority = 40
    };
    
    //	Construction/Destruction
public:
    OpenALThread(ThreadRoutine inThreadRoutine, void* inParameter)
    :	mPThreadID(0),
    mSpawningThreadPriority(getScheduledPriority(pthread_self(), OpenALThread_SET_PRIORITY)),
    mThreadRoutine(inThreadRoutine),
    mThreadParameter(inParameter),
    mPriority(kDefaultThreadPriority),
    mFixedPriority(false),
    mAutoDelete(true) { }
    
    ~OpenALThread() { }
    
    //	Properties
    BOOL IsRunning() const { return 0 != mPThreadID; }
    void SetAutoDelete(BOOL b) { mAutoDelete = b; }
    
#ifdef MAC_OS_X			
    void Cancel() const { if ( IsRunning() ) pthread_cancel( mPThread ); }
#endif // MAC_OS_X
    
    void SetPriority( int inPriority, BOOL inFixedPriority)
    {
#ifdef MAC_OS_X		
        OSStatus result = noErr;
        mPriority = inPriority;
        mFixedPriority = inFixedPriority;
        if(mPThreadID != 0)
        {
            if (mFixedPriority)
            {
                thread_extended_policy_data_t		theFixedPolicy;
                theFixedPolicy.timeshare = false;	// set to true for a non-fixed thread
                result  = thread_policy_set(pthread_mach_thread_np(mPThread), THREAD_EXTENDED_POLICY, (thread_policy_t)&theFixedPolicy, THREAD_EXTENDED_POLICY_COUNT);
                if (result) 
                {
                    // printf("OpenALThread::SetPriority: failed to set the fixed-priority policy");
                    return;
                }
            }
            // We keep a reference to the spawning thread's priority around (initialized in the constructor), 
            // and set the importance of the child thread relative to the spawning thread's priority.
            thread_precedence_policy_data_t		thePrecedencePolicy;
            
            thePrecedencePolicy.importance = mPriority - mSpawningThreadPriority;
            result =thread_policy_set(pthread_mach_thread_np(mPThread), THREAD_PRECEDENCE_POLICY, (thread_policy_t)&thePrecedencePolicy, THREAD_PRECEDENCE_POLICY_COUNT);
            if (result)
            {
                // printf("OpenALThread::SetPriority: failed to set the precedence policy");
                return;
            }
        } 
#endif // MAC_OS_X		
    }
    
    //	Actions
    void Start()
    {
        if(mPThreadID != 0)
        {
            // printf("OpenALThread::Start: can't start because the thread is already running\n");
            return;
        }
        
        OSStatus			result;
        pthread_attr_t		theThreadAttributes;
        
        result = pthread_attr_init(&theThreadAttributes);
            AssertNoError("Error initializing thread", end);
        
        result = pthread_attr_setdetachstate(&theThreadAttributes, PTHREAD_CREATE_DETACHED);
            AssertNoError("Error setting thread detach state", end);
        
        mPThreadID = pthread_create(&mPThread, &theThreadAttributes, (ThreadRoutine)OpenALThread::Entry, this);
            AssertNoError("Error creating thread", end);
        
        pthread_attr_destroy(&theThreadAttributes);
            AssertNoError("Error destroying thread attributes", end);
    end:
        return;
    }
    
    //	Implementation
protected:
    static void* Entry(OpenALThread* inOpenALThread)
    {
        void* theAnswer = NULL;
        
        // let's system assing priority
        // inOpenALThread->SetPriority(inOpenALThread->mPriority, inOpenALThread->mFixedPriority);
        
        if(inOpenALThread->mThreadRoutine != NULL)
        {
            theAnswer = inOpenALThread->mThreadRoutine(inOpenALThread->mThreadParameter);
        }
        
        inOpenALThread->mPThreadID = 0;
        if (inOpenALThread->mAutoDelete)
            delete inOpenALThread;
        return theAnswer;
    }
    
    static int getScheduledPriority( pthread_t inThread, int inPriorityKind)
    {
#ifdef MAC_OS_X
        thread_basic_info_data_t			threadInfo;
        policy_info_data_t					thePolicyInfo;
        unsigned int						count;
        
        if (inThread == NULL)
            return 0;
        
        // get basic info
        count = THREAD_BASIC_INFO_COUNT;
        thread_info (pthread_mach_thread_np (inThread) , THREAD_BASIC_INFO, (thread_info_t)&threadInfo, &count);
        
        switch (threadInfo.policy)
        {
            case POLICY_TIMESHARE:
                count = POLICY_TIMESHARE_INFO_COUNT;
                thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_TIMESHARE_INFO, (thread_info_t)&(thePolicyInfo.ts), &count);
                if (inPriorityKind == OpenALThread_SCHEDULED_PRIORITY)
                {
                    return thePolicyInfo.ts.cur_priority;
                }
                return thePolicyInfo.ts.base_priority;
                break;
                
            case POLICY_FIFO:
                count = POLICY_FIFO_INFO_COUNT;
                thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_FIFO_INFO, (thread_info_t)&(thePolicyInfo.fifo), &count);
                if ( (thePolicyInfo.fifo.depressed) && (inPriorityKind == OpenALThread_SCHEDULED_PRIORITY) )
                {
                    return thePolicyInfo.fifo.depress_priority;
                }
                return thePolicyInfo.fifo.base_priority;
                break;
                
            case POLICY_RR:
                count = POLICY_RR_INFO_COUNT;
                thread_info(pthread_mach_thread_np (inThread), THREAD_SCHED_RR_INFO, (thread_info_t)&(thePolicyInfo.rr), &count);
                if ( (thePolicyInfo.rr.depressed) && (inPriorityKind == OpenALThread_SCHEDULED_PRIORITY) ) 
                {
                    return thePolicyInfo.rr.depress_priority;
                }
                return thePolicyInfo.rr.base_priority;
                break;
        }
#endif // MAC_OS_X
        return 0;
    }
    
    pthread_t       mPThread;
    int				mPThreadID;
    int				mSpawningThreadPriority;
    ThreadRoutine	mThreadRoutine;
    void*			mThreadParameter;
    int				mPriority;
    BOOL            mFixedPriority;
    BOOL            mAutoDelete;		// delete self when thread terminates
};

#endif // INTERNAL_RECO_THREAD

class CRecognizerWrapper
{
public:
    typedef struct
        {
            p_RECO_point_type pTrace;
            UInt32      cTrace;
        } ASYNC_RECOG_PARAMS, * LPASYNC_RECOG_PARAMS;	
    
public:
    CRecognizerWrapper()
    {
        m_bNewSession = true;
        m_RecCtx = NULL;
        m_hUserDict = NULL;
        m_hMainDict = NULL;
        m_hAltDict = NULL;
        m_hInternetDict = NULL;
        m_bInitialized = false;
        m_nRecMode = RECMODE_GENERAL;
        m_dwFlags = RECO_DEFAULT_FLAGS;
        m_bRunThread = true;
        m_pWeights = NULL;
        m_pNStrokes = NULL;
        m_pLastRes = NULL;
        m_pResString = NULL;
        m_pWordLrn = NULL;
        m_pWordList = NULL;
        m_nCustomPunctuations = NULL;
        m_nCustomNumbers = NULL;
		m_lastWord = NULL;
        m_cRecQueue = 0;
        memset( letter_shapes, 0, sizeof( letter_shapes ) );
        // m_bResultsReady = 0;
        // m_bRunRecognizer = 0;

        std::atomic_fetch_and( &m_bResultsReady, 0);
        std::atomic_fetch_or( &m_bRunRecognizer, 1);
        // OSAtomicTestAndClear( 1, &m_bResultsReady );
        // OSAtomicTestAndSet( 1, &m_bRunRecognizer );

#ifdef INTERNAL_RECO_THREAD

        m_Thread = NULL;
        // m_bNewStroke = 0;
        std::atomic_fetch_and( &m_bNewStroke, 0);
        // OSAtomicTestAndClear( 1, &m_bNewStroke );

#ifndef MAC_OS_X
        m_condStroke = PTHREAD_COND_INITIALIZER;
        m_condResult = PTHREAD_COND_INITIALIZER;
        m_mutexReco = PTHREAD_MUTEX_INITIALIZER;		
        m_mutexResult = PTHREAD_MUTEX_INITIALIZER;
        m_mutexAsyncReco = PTHREAD_MUTEX_INITIALIZER;
        m_mutexQueue = PTHREAD_MUTEX_INITIALIZER;
#endif // MAC_OS_X

#endif // INTERNAL_RECO_THREAD
    }
        
    ~CRecognizerWrapper()
    {
        Uninitialize();
    }

#ifdef INTERNAL_RECO_THREAD
    
	static int RecCallback( void * pParam )
    {
        CRecognizerWrapper * pThis = (CRecognizerWrapper *)pParam;
        if ( NULL == pThis )
            return 0;
        // recognizer callback
        pthread_mutex_lock( &pThis->m_mutexQueue );
        int     iQ = pThis->m_cRecQueue;
        pthread_mutex_unlock( &pThis->m_mutexQueue );
        if ( iQ > 0 && pThis->m_pRecQueue[0].pTrace != NULL && pThis->m_pRecQueue[0].cTrace != 0 ) 
            return 1;
        return 0;
    }
    
    int WaitForResult( const struct timespec * timeout )
    {
        pthread_mutex_lock( &m_mutexResult );
        // pthread_mutex_lock( &m_mutexReco );
        // pthread_mutex_unlock( &m_mutexReco );
        int iResult = 0;
        if ( ! std::atomic_fetch_and( &m_bResultsReady, 0) )
        {
            if ( timeout != NULL )
                iResult = pthread_cond_timedwait( &m_condResult, &m_mutexResult, timeout );
            else
                iResult = pthread_cond_wait( &m_condResult, &m_mutexResult );
        }
        
        pthread_mutex_unlock( &m_mutexResult );
        return iResult;
    }
    
            
    void AsynchReset()
    {
        pthread_mutex_lock( &m_mutexReco );
        pthread_mutex_lock( &m_mutexQueue );
        m_bNewSession = true;
        std::atomic_fetch_and( &m_bResultsReady, 0);
        std::atomic_fetch_and( &m_bNewStroke, 0);
        // OSAtomicTestAndClear( 1, &m_bResultsReady );
        // OSAtomicTestAndClear( 1, &m_bNewStroke );
        FreeResults();
        pthread_mutex_unlock( &m_mutexQueue );
        pthread_mutex_unlock( &m_mutexReco );
    }
    
    BOOL AsynchPutStroke( p_RECO_point_type pTrace, UInt32 cTrace )
    {
        if ( m_Thread == NULL )
            return false;
        
        struct timespec   time;
        time.tv_sec = 0;
        time.tv_nsec = 50000000;
        
        pthread_mutex_lock( &m_mutexQueue );
        if ( m_cRecQueue > RECQUEUE_MAX_ELEMS - 1 )
        {
            for ( int i = 0; i < 5; i++ )
            {
                pthread_mutex_unlock( &m_mutexQueue );
                pthread_mutex_lock( &m_mutexReco );
                pthread_mutex_unlock( &m_mutexReco );
                pthread_mutex_lock( &m_mutexQueue );
                if ( m_cRecQueue <= RECQUEUE_MAX_ELEMS - 1 )
                    break;
                nanosleep( &time, NULL );		// 50 ms
            }
            if ( m_cRecQueue > RECQUEUE_MAX_ELEMS-1 )
            {
                pthread_mutex_unlock( &m_mutexQueue );
                return false;
            }
        }
        int index = m_cRecQueue;
        m_pRecQueue[index].pTrace = pTrace;
        m_pRecQueue[index].cTrace = cTrace;
        m_cRecQueue++;
        std::atomic_fetch_and( &m_bNewStroke, 1);
        // OSAtomicTestAndSet( 1, &m_bNewStroke );
        pthread_cond_signal( &m_condStroke );
        pthread_mutex_unlock( &m_mutexQueue );
        return true;
    }
    
    BOOL RecQueueGet( p_RECO_point_type *ppTrace, UInt32 *pcTrace )
    {
        pthread_mutex_lock( &m_mutexQueue );
        if ( m_cRecQueue <= 0 )
        {
            pthread_mutex_unlock( &m_mutexQueue );
            return false;
        }
        *ppTrace = m_pRecQueue[0].pTrace;
        *pcTrace = m_pRecQueue[0].cTrace;
        for ( register int i = 0; i < m_cRecQueue-1; i++ )
            memcpy( &m_pRecQueue[i], &m_pRecQueue[i+1], sizeof(m_pRecQueue[i]) );
        m_cRecQueue--;
        if ( m_cRecQueue <= 0 )
        {
            // reset condition
            // pthread_cond_signal( &m_condStroke );
            // ResetEvent( m_hQueueEvent );
        }
        pthread_mutex_unlock( &m_mutexQueue );
        return true;
    }

	static void* RecognizeProc(void *args)
    {
        
        CRecognizerWrapper * pThis = (CRecognizerWrapper *)args;
        p_RECO_point_type    pTrace = NULL;
        UInt32         cTrace = 0;
        
        while( pThis->m_bRunThread )
        {
            pthread_mutex_lock( &pThis->m_mutexReco );
            //pThis->m_Thread->SetPriority( OpenALThread::kDefaultThreadPriority, false );
            if ( ! std::atomic_fetch_and( &pThis->m_bNewStroke, 0) )
            // if ( ! OSAtomicTestAndClear( 1, &pThis->m_bNewStroke ) )
            {
                pthread_cond_wait( &pThis->m_condStroke, &pThis->m_mutexReco );		/// ??? does not wait, mutex?
            }
            //pThis->m_Thread->SetPriority( OpenALThread::kMinThreadPriority, false );
            if ( pThis->m_bRunThread )
            {
                while ( pThis->RecQueueGet( &pTrace, &cTrace ) )
                {
                    if ( pTrace != NULL && cTrace > 0 )
                    {
                        pThis->RecAddWordsStroke( pTrace, cTrace );
                        // pThis->RecognizedStrokes();
                        free( (void *) pTrace );
                    }
                    else
                    {
                        // last stroke
                        pThis->RecAddWordsStroke( NULL, 0 );
                        //pThis->m_Thread->SetPriority( OpenALThread::kDefaultThreadPriority, false );
                        if ( cTrace == (UInt32)(-1) )
                        {
                            // terminate the recognition thread without the result
                            pThis->RecEndOfStrokes();
                        }
                        else if( pThis->RecEndOfStrokes() && pThis->RecGetWordsList() )
                        {
                            pThis->ResultsReady( true );
                        }
                        else
                        {
                            pThis->ResultsReady( false );
                        }
                        //pThis->m_Thread->SetPriority( OpenALThread::kMinThreadPriority, false );
                    }
                }
            }
#ifdef _WIN32
            if (pThis->m_mutexReco != NULL)
            {
                pthread_mutex_unlock( &pThis->m_mutexReco );
            }
#else
            pthread_mutex_unlock( &pThis->m_mutexReco );
#endif
        }
        // delete pThis->m_Thread;
        pThis->m_Thread = NULL;
        return NULL;
    }		

#else

	static int RecCallback( void * pParam )
    {
        CRecognizerWrapper * pThis = (CRecognizerWrapper *)pParam;
        if ( NULL == pThis )
            return 0;
        return (pThis->m_cRecQueue > 0) ? 1 : 0;
    }

    void SynchReset()
    {
        std::atomic_fetch_and( &m_bResultsReady, 0);
        // OSAtomicTestAndClear( 1, &m_bResultsReady );
        FreeResults();
    }

	BOOL SynchPutStroke( p_RECO_point_type pTrace, int cTrace )
	{
		if ( pTrace != NULL && cTrace > 0 )
		{
			RecAddWordsStroke( pTrace, cTrace );
			// pThis->RecognizedStrokes();
			// free( (void *) pTrace );
			return true;
		}
		else
		{
			// last stroke
			RecAddWordsStroke( NULL, 0 );
			//pThis->m_Thread->SetPriority( OpenALThread::kDefaultThreadPriority, false );
			if ( cTrace < 0 )
			{
				// terminate the recognition thread without the result
				RecEndOfStrokes();
			}
			else if( RecEndOfStrokes() && RecGetWordsList() )
			{
				ResultsReady( false );
				return true;
			}
			else
			{
				ResultsReady( true );
			}
		}
		return false;
	}
    
#endif // INTERNAL_RECO_THREAD

	void ResultsReady( BOOL bReady )
    {
        // the calling thread must wait for this event until recogntion is 
        // complete...
#ifdef INTERNAL_RECO_THREAD
        pthread_mutex_lock( &m_mutexResult );
#endif // INTERNAL_RECO_THREAD
        std::atomic_fetch_or( &m_bResultsReady, 1);
        // OSAtomicTestAndSet( 1, &m_bResultsReady );
#ifdef INTERNAL_RECO_THREAD
        pthread_cond_signal( &m_condResult );
        pthread_mutex_unlock( &m_mutexResult );
#endif // INTERNAL_RECO_THREAD
    }
    

    BOOL RecAddWordsStroke( p_RECO_point_type lpPoints, UInt32 nPoints )
    {
        if ( NULL == m_RecCtx )
            return false;
        
        if ( m_bNewSession )
        {
            RECO_control_type    ctrl; // = {0, 0, NULL, NULL, NULL};
            
            memset( &ctrl, 0, sizeof( RECO_control_type ) );
            
            ctrl.flags = HW_RECFL_TTSEG; // | PEG_RECFL_NSEG;
            
            if ( 0 != (m_dwFlags & FLAG_SINGLEWORDONLY) )
                ctrl.flags |= HW_RECFL_NSEG;
            
            if ( 0 != (m_dwFlags & FLAG_SEPLET) ) 
                ctrl.flags |= HW_RECFL_SEPLET;
            
#ifdef LANG_INTERNATIONAL
            if ( 0 != (m_dwFlags & FLAG_INTERNATIONAL) ) 
                ctrl.flags |= HW_RECFL_INTL_CS;
#endif // LANG_INTERNATIONAL
            
            if ( 0 != (m_dwFlags & FLAG_USECUSTOMPUNCT) && (m_nRecMode != RECMODE_CUSTOM) )
            {
                ctrl.flags |= HW_RECFL_CUSTOM_WITH_ALPHA;
                ctrl.customPunct = m_nCustomPunctuations;
            }
            
            RECOHDICT hUserDict = m_hUserDict;
            // ctrl.flags |=  (HW_RECFL_PURE | HW_RECFL_ALPHAONLY); 
            if ( m_nRecMode == RECMODE_CAPS )  
            {
                ctrl.flags |= HW_RECFL_CAPSONLY;
            }
            else if ( m_nRecMode == RECMODE_NUM ) 
            {
                ctrl.flags |= HW_RECFL_NUMONLY;
            }
            else if ( m_nRecMode == RECMODE_WWW ) 
            {
                if ( NULL != m_hInternetDict )
                    hUserDict = m_hInternetDict;
                // ctrl.flags |= HW_RECFL_CUSTOM_WITH_ALPHA;
                // ctrl.customPunct = m_nCustomPunctuations;
            }
            else if ( m_nRecMode == RECMODE_NUMBERSPURE )
            {
                ctrl.flags |= (HW_RECFL_NUMONLY | HW_RECFL_PURE);
            }
            else if ( m_nRecMode == RECMODE_CUSTOM )
            {
                ctrl.flags |= HW_RECFL_CUSTOM;
                ctrl.customNumbers = m_nCustomNumbers;
                ctrl.customPunct = m_nCustomPunctuations;
            }
            else if ( m_nRecMode == RECMODE_ALPHAONLY )
            {
                ctrl.flags |= (HW_RECFL_PURE | HW_RECFL_ALPHAONLY);
            }
            
            if ( 0 != (m_dwFlags & (FLAG_USERDICT | FLAG_MAINDICT)) ) 
            {
                if ( 0 != (m_dwFlags & FLAG_MAINDICT) && m_hMainDict )
                    ctrl.h_main_dict = m_hMainDict;
                if ( 0 != (m_dwFlags & FLAG_USERDICT) && hUserDict )
                    ctrl.h_user_dict = hUserDict;
                if ( 0 != (m_dwFlags & FLAG_ONLYDICT) && (hUserDict || m_hMainDict) && (m_nRecMode == RECMODE_GENERAL) )
                    ctrl.flags |= HW_RECFL_DICTONLY;
            }
            
            if ( 0 != (m_dwFlags & FLAG_STATICSEGMENT) )
                ctrl.flags |= HW_RECFL_STATICSEG;
            
            ctrl.sp_vs_q = 14;		// used to be 9
            ctrl.ICB_param = (void *)this;
            ctrl.InfoCallBack = (info_func_type)RecCallback;
            
            RecoCloseSession( m_RecCtx );
            RecoOpenSession( &ctrl, m_RecCtx );
            m_bNewSession = false;
        }
        
        // must be done before...
        // lpPoints[nPoints].y = TRACE_BREAK;
        // lpPoints[nPoints].x = 0;
        /*
        for ( int i = 0; i < nPoints; i++ )
        {
            short x = lpPoints[i].x;
            short y = lpPoints[i].y;
            printf( "x=%d, y=%d\n", x, y );
        }
        */
        
        if ( RecoRecognize( (int)nPoints, lpPoints, m_RecCtx ) == 0 )
            return true;
        return false;
    }
    
    BOOL RecognizeSymbol( p_RECO_point_type lpPoints, int nPoints, p_RECO_baseline_type baseline )
    {
        if ( NULL == m_RecCtx )
            return false;
        RECO_control_type    ctrl; // = {0, 0, NULL, NULL, NULL};
        
        memset( &ctrl, 0, sizeof( RECO_control_type ) );
        
        ctrl.flags = HW_RECFL_NSEG | HW_RECFL_SEPLET;
                
#ifdef LANG_INTERNATIONAL
        if ( 0 != (m_dwFlags & FLAG_INTERNATIONAL) )
            ctrl.flags |= HW_RECFL_INTL_CS;
#endif // LANG_INTERNATIONAL
        
        if ( 0 != (m_dwFlags & FLAG_USECUSTOMPUNCT) && (m_nRecMode != RECMODE_CUSTOM) )
        {
            ctrl.flags |= HW_RECFL_CUSTOM_WITH_ALPHA;
            ctrl.customPunct = m_nCustomPunctuations;
        }
        
        RECOHDICT hUserDict = m_hUserDict;
        // ctrl.flags |=  (HW_RECFL_PURE | HW_RECFL_ALPHAONLY);
        if ( m_nRecMode == RECMODE_CAPS )
        {
            ctrl.flags |= HW_RECFL_CAPSONLY;
        }
        else if ( m_nRecMode == RECMODE_NUM )
        {
            ctrl.flags |= HW_RECFL_NUMONLY;
        }
        else if ( m_nRecMode == RECMODE_NUMBERSPURE )
        {
            ctrl.flags |= (HW_RECFL_NUMONLY | HW_RECFL_PURE);
        }
        else if ( m_nRecMode == RECMODE_CUSTOM )
        {
            ctrl.flags |= HW_RECFL_CUSTOM;
            ctrl.customNumbers = m_nCustomNumbers;
            ctrl.customPunct = m_nCustomPunctuations;
        }
        else if ( m_nRecMode == RECMODE_ALPHAONLY )
        {
            ctrl.flags |= (HW_RECFL_PURE | HW_RECFL_ALPHAONLY);
        }
        
        if ( 0 != (m_dwFlags & (FLAG_USERDICT | FLAG_MAINDICT)) )
        {
            if ( 0 != (m_dwFlags & FLAG_MAINDICT) && m_hMainDict )
                ctrl.h_main_dict = m_hMainDict;
            if ( 0 != (m_dwFlags & FLAG_USERDICT) && hUserDict )
                ctrl.h_user_dict = hUserDict;
            if ( 0 != (m_dwFlags & FLAG_ONLYDICT) && (hUserDict || m_hMainDict) )
                ctrl.flags |= HW_RECFL_DICTONLY;
        }
        
        if ( 0 != (m_dwFlags & FLAG_STATICSEGMENT) )
            ctrl.flags |= HW_RECFL_STATICSEG;
        
        ctrl.sp_vs_q = 14;		// used to be 9
        ctrl.ICB_param = (void *)this;
        ctrl.InfoCallBack = (info_func_type)RecCallback;
        
        RecoCloseSession( m_RecCtx );
        RecoOpenSession( &ctrl, m_RecCtx );
        
        BOOL result = (0 == RecoRecognizeSymbol( nPoints, lpPoints, baseline, m_RecCtx ));
        if ( result )
            RecGetWordsList();
        
        RecoCloseSession( m_RecCtx );
        m_bNewSession = true;
        return result;
    }
    
    BOOL RecEndOfStrokes()
    {
        BOOL bRes = false;
        
        if ( m_RecCtx != NULL )
        {
            bRes = !RecoCloseSession( m_RecCtx );
            m_bNewSession = true;
        }
        return bRes;
    }
    
    
    int IsDicts()
    {
        int result = 0;
        if ( NULL != m_hMainDict )
            result |= FLAG_MAINDICT;
        if ( NULL != m_hUserDict )
            result |= FLAG_USERDICT;
        if ( NULL != m_hAltDict )
            result |= FLAG_ALTDICT;
        return result;
    }
    
    void EnablePhatCalc( BOOL bEnable ) { if ( bEnable ) m_dwFlags |= FLAG_ENABLECALC; else m_dwFlags &= ~FLAG_ENABLECALC; }
    BOOL IsPhatCalcEnabled() const { return (0 != (m_dwFlags & FLAG_ENABLECALC)); }
    
    BOOL TestWord( UCHR * pszWord, int nFlags ) const
    {
        if ( NULL == m_hMainDict && NULL == m_hUserDict )
            return false;
        
        if ( 0 != (nFlags & HW_SPELL_IGNORENUM) )
        {
            if ( WordHasNumbers( pszWord ) )
                return true;			
        }
        if ( 0 != (nFlags & HW_SPELL_IGNOREUPPER) )
        {
            int len  = 0;
            if ( IsWordUpper( pszWord, len ) )
                return true;
        }
        RECOHDICT hMainDict = m_hMainDict;
        RECOHDICT hUserDict = NULL;
        if ( 0 != (nFlags & HW_SPELL_USERDICT) )
            hUserDict = m_hUserDict;
        if ( m_nRecMode == RECMODE_WWW && NULL != m_hInternetDict )
            hUserDict = m_hInternetDict;
        if ( 0 !=(nFlags & HW_SPELL_USEALTDICT) )
            hMainDict = m_hAltDict;
        
        return (0 == RecoCheckWordInDicts( pszWord, hMainDict, hUserDict ));
    }
    
    BOOL AddUserWord( UCHR * pszWord )
    {
        if ( NULL == m_hUserDict )
            return false;
        if ( PYDICT_NOERR == RecoAddWordToDict( pszWord, &m_hUserDict ) )
        {
            if ( m_nRecMode == RECMODE_WWW && NULL != m_hInternetDict )
                RecoAddWordToDict( pszWord, &m_hInternetDict );
            return true;
        }
        return false;
    }
    
    int SpellCheckWord( UCHR *pszWord, UCHR *pszAnswer, int cbSize, int nFlags )
    {
        RECOHDICT hUserDict = (0==(nFlags & HW_SPELL_USERDICT)) ? NULL : m_hUserDict;
        if ( m_nRecMode == RECMODE_WWW && NULL != m_hInternetDict )
            hUserDict = m_hInternetDict;
        RECOHDICT hMainDict = (nFlags & HW_SPELL_USEALTDICT) ? m_hAltDict : m_hMainDict;
        
        if ( 0 != (nFlags & HW_SPELL_IGNORENUM) )
        {
            if ( WordHasNumbers( pszWord ) )
                return true;			
        }
        if ( 0 != (nFlags & HW_SPELL_IGNOREUPPER) )
        {
            int len  = 0;
            if ( IsWordUpper( pszWord, len ) )
                return true;
        }
        
        return ((NULL == hMainDict) ? 0 : 
                RecoSpellCheckWord( pszWord, pszAnswer, cbSize, hMainDict, hUserDict, (nFlags & HW_SPELL_LIST) ));
    }
    
    BOOL Initialize( const char * pMainDict, const char * pUserDict, int language )
    {
        if ( m_bInitialized )
            Uninitialize();
        if ( ! RecInit( language ) )
            return false;
        m_cRecQueue = 0;
        // load dictionaries only if needed 
        DictsInit( pMainDict, pUserDict );
        return (m_bInitialized = true);
    }
    
    int GetLanguageID() const
    {
        if ( m_bInitialized )
            return RecoGetRecLangID( m_RecCtx );
        return 0;
    }
    
    BOOL RecInit( int language )
    {
        if ( m_bInitialized )
            return true;	// already initialized

        if ( (m_RecCtx = RecoCreateContext( language )) == NULL )
        {
            return false;
        }

        RecSetDefaultShapes();
        
        // initialize async recognition here
#ifdef INTERNAL_RECO_THREAD
        pthread_mutexattr_t		theMutexAttributes;
        pthread_condattr_t		theCondAttributres;
        OSStatus	result;
                
        result = pthread_mutexattr_init(&theMutexAttributes);
        AssertNoError("Error: pthread_mutexattr_init", end);
        result = pthread_mutex_init( &m_mutexQueue, &theMutexAttributes );
        AssertNoError("Error: pthread_mutex_init:m_mutexQueue", end);
        result = pthread_mutex_init( &m_mutexReco, &theMutexAttributes );
        AssertNoError("Error: pthread_mutex_init:m_mutexReco", end);
        result = pthread_mutex_init( &m_mutexAsyncReco, &theMutexAttributes );		
        AssertNoError("Error: pthread_mutex_init:m_mutexAsyncReco", end);
        result = pthread_mutex_init( &m_mutexResult, &theMutexAttributes );
        AssertNoError("Error: pthread_mutex_init:m_mutexResult", end);

#if defined(MAC_OS_X) || defined(_WIN32)
        result = pthread_condattr_init( &theCondAttributres );
        AssertNoError("Error: pthread_condattr_init", end);
#endif // MAC_OS_X
        result = pthread_cond_init( &m_condStroke, &theCondAttributres );
        AssertNoError("Error: pthread_cond_init:m_condStroke", end);
        result = pthread_cond_init( &m_condResult, &theCondAttributres );
        AssertNoError("Error: pthread_cond_init:m_condResult", end);
#if defined(MAC_OS_X) || defined(_WIN32)
        result = pthread_condattr_destroy( &theCondAttributres );
        AssertNoError("Error: pthread_condattr_destroy", end);		
#endif // MAC_OS_X
        result = pthread_mutexattr_destroy( &theMutexAttributes );
        AssertNoError("Error: pthread_mutexattr_destroy", end);
        
        // for loops we need to spawn a new thread					
        m_bRunThread = true;
        m_Thread = new OpenALThread( RecognizeProc, (void*)this );
        m_Thread->Start();
        m_bInitialized = true;

end:
#else
        m_bInitialized = true;
#endif // INTERNAL_RECO_THREAD
        return m_bInitialized;
    }
    
    int DictToData( char ** ppData, int nDictType )
    {
        int		result = 0;
        *ppData = NULL;
        int		len = GetDictLength( nDictType );
        if ( len < 1 )
            return 0;
        char * pData = (char *)malloc( len+2 );
        if ( kDictionaryType_User == nDictType )
        {
            if ( PYDICT_NOERR == RecoSaveDict( pData, m_hUserDict ) )
                result = len;
        }
        else if ( kDictionaryType_Alternative == nDictType )
        {
            if ( PYDICT_NOERR == RecoSaveDict( pData, m_hAltDict ) )
                result = len;
        }
        else
        {
            if ( PYDICT_NOERR == RecoSaveDict( pData, m_hMainDict ) )
                result = len;
        }
        if ( result < 1 && pData != NULL )
            free( (void *)pData );
        else
            *ppData = pData;
        return result;
    }
    
    bool DictFromData( const char * pData, int nDataLen, int nDictType )
    {
        // Unused nDataLen
        if ( kDictionaryType_User == nDictType )
        {
            if ( m_hUserDict != NULL )
                RecoFreeDict( &m_hUserDict );
            m_hUserDict = NULL;
            RecoLoadDict( pData, &m_hUserDict );
            return (NULL != m_hUserDict);			
        }
        else if ( kDictionaryType_Alternative == nDictType )
        {
            if ( m_hAltDict != NULL )
                RecoFreeDict( &m_hAltDict );
            m_hAltDict = NULL;
            RecoLoadDict( pData, &m_hAltDict );
            return (NULL != m_hAltDict);			
        }
        else
        {
            if ( m_hMainDict != NULL )
                RecoFreeDict( &m_hMainDict );
            m_hMainDict = NULL;
            RecoLoadDict( pData, &m_hMainDict );
            return (NULL != m_hMainDict);
        }
    }
        
    BOOL RecGetWordsList()
    {
        USHORT  * pWeights = NULL;
        int	*	  pNStrokes = NULL;
        UCHR   *  pRes = RecBuildResList( &pWeights, &pNStrokes );
        if ( pRes == NULL )
            return false;
        SaveLastRes( pRes, pWeights, pNStrokes );
        return true;
    }
    
    void FreeResults()
    {
        std::atomic_fetch_and( &m_bResultsReady, 0);
        // OSAtomicTestAndClear( 1, &m_bResultsReady );
        if ( NULL != m_pWeights )
            delete [] m_pWeights;		
        m_pWeights = NULL;
        if ( NULL != m_pNStrokes )
            delete [] m_pNStrokes;
        m_pNStrokes = NULL;
        if ( NULL != m_pLastRes )
            delete [] m_pLastRes;
        m_pLastRes = NULL;
        if ( NULL != m_pResString )
            delete [] m_pResString;
        m_pResString = NULL;
    }
    
    int GetResColumnCount() const
    {
        if ( m_pLastRes == NULL )
            return 0;
        
        UCHR *	pRecRes = m_pLastRes;
        int		nCols = 0;

        while ( *pRecRes )
        {
            if ( *pRecRes == PM_LISTSEP )
            {  
                nCols++;
            }
            pRecRes++;
        }
        return nCols;
    }
        
    int GetResRowCount( int nColumn ) const
    {
        if ( m_pLastRes == NULL )
            return 0;
        
        UCHR *	pRecRes = m_pLastRes;
        int		nCols = 0, nRows = 1;
        
        while ( *pRecRes )
        {
            if ( *pRecRes == PM_LISTSEP )
            {  
                nCols++;
                if ( nCols > nColumn )
                    break;
            }
            else if ( nCols == nColumn &&  *pRecRes == PM_ALTSEP )
            {
                nRows++;
            }
            pRecRes++;
        }
        return nRows;
    }
    
    int GetResNStrokes( int nColumn, int nRow ) const
    {
        if (  m_pNStrokes == NULL )
            return 0;
        
        int *	pn = m_pNStrokes;
        int		nCols = 0, nRows = 0;
        int		nResult = 0;
        
        while ( *pn )
        {
            if ( (int)(*pn) == (int)PM_NUMSEPARATOR )
            {  
                nCols++;
                if ( nCols > nColumn )
                    break;
            }
            else if ( nCols == nColumn )
            {
                if ( nRows == nRow )
                {
                    nResult = *pn;
                    break;
                }
                nRows++;
                if ( nRows > nRow )
                    break;
            }
            pn++;
        }
        return nResult;
    }

    USHORT GetResWeight( int nColumn, int nRow ) const
    {
        if (  m_pWeights == NULL )
            return 0;
        
        USHORT *pw = m_pWeights;
        int		nCols = 0, nRows = 0;
        USHORT	nResult = 0;
        
        while ( *pw )
        {
            if ( *pw == PM_LISTSEP )
            {  
                nCols++;
                if ( nCols > nColumn )
                    break;
            }
            else if ( nCols == nColumn )
            {
                if ( nRows == nRow )
                {
                    nResult = *pw;
                    break;
                }
                nRows++;
                if ( nRows > nRow )
                    break;
            }
            pw++;
        }
        return nResult;
    }
    
    const UCHR * GetResWord( int nColumn, int nRow )
    {
        if ( m_pLastRes == NULL )
            return NULL;

        UCHR *	pRecRes = m_pLastRes;
        int		nCols = 0, nRows = 0;
        UCHR    szBuff[HW_MAXWORDLEN] = {0};
        UCHR *	pBuff = szBuff;
        
        while ( *pRecRes )
        {
            if ( *pRecRes == PM_LISTSEP )
            {  
                nCols++;
                if ( nCols > nColumn )
                    break;
            }
            else if ( nCols == nColumn && *pRecRes == PM_ALTSEP )
            {
                nRows++;
                if ( nRows > nRow )
                    break;
            }
            else if ( nCols == nColumn && nRows == nRow )
                *pBuff++ = *pRecRes;
            pRecRes++;
        }
        *pBuff = 0;
        if ( _STRLEN( szBuff ) > 0 )
        {
			if ( NULL != m_lastWord )
				free( (void *)m_lastWord );
			m_lastWord = _STRDUP( szBuff );
            return m_lastWord;
        }
        return NULL;
    }
    
    void SaveLastRes( UCHR * pRecRes, USHORT * pWeights, int * pNStrokes )
    {
        FreeResults();
        m_pLastRes = pRecRes;
        m_pWeights = pWeights;
        m_pNStrokes = pNStrokes;

        if ( NULL != m_pLastRes )
        {
            UCHR *	pRecRes = m_pLastRes;
            int 	len = 0,  i = 0;
            UCHR    szBuff[HW_MAXWORDLEN] = {0};
            
            len = _STRLEN( m_pLastRes ) + HW_MAXWORDLEN;
            m_pResString = new UCHR[len];
            if ( NULL == m_pResString )
                return;
            *m_pResString = 0;
            
            while ( *pRecRes )
            {
                if ( *pRecRes == PM_ALTSEP || *pRecRes == PM_LISTSEP )
                {                
                    szBuff[i] = 0;
                    if ( 0 != szBuff[0] )
                    {
                        if ( *m_pResString != 0 && 0 == (m_dwFlags & FLAG_NOSPACE) )
                            _STRCAT( m_pResString, g_space );
                        _STRCAT( m_pResString, szBuff );
                        if ( _STRLEN( m_pResString ) >= len )
                            break;  // to avoid buffer overflow
                    }
                    i = 0;
                    // skip all other alternatives
                    while( *pRecRes && *pRecRes != PM_LISTSEP )
                        pRecRes++;
                    if ( *pRecRes )
                        pRecRes++;
                }
                else
                {
                    szBuff[i++] = *pRecRes++;
                }
            }
            
            len = _STRLEN( m_pResString );
            if ( len == 1 && 0 != (m_dwFlags & FLAG_NOSINGLELETSPACE) )
            {
                return;
            }
            if ( m_nRecMode != RECMODE_WWW && 0 == (m_dwFlags & FLAG_NOSPACE) )
                _STRCAT( m_pResString, g_space );
            Calculator( m_pResString );
        }
    }
    
    BOOL Calculator( UCHR * strResult )
    {
        size_t		len = _STRLEN( strResult );
        if ( 0 != (m_dwFlags & FLAG_ENABLECALC) && len > 3 ) 
        {
            if ( (strResult[len-2] == '=' || strResult[len-1] == '=') )
            {
                // Support calc
                FLOAT	fVal = 0.0;
                
#ifdef MAC_OS_X
                struct lconv * loc = localeconv();
                g_szCalcDelims[0] = loc->decimal_point[0];
                g_szCalcDelims[1] = loc->thousands_sep[0];
#endif // MAC_OS_X
                if ( ::PhatCalc( strResult, g_szCalcDelims, &fVal ) )
                {
					len--;
					while ( len > 0 && strResult[len] == ' ' )
					{
						strResult[len] = 0;
						len--;
					}
                    
					if ( fabs(fVal - (long)fVal) < 0.001 )
                    {
                        _ITOSTR( (long)fVal, &strResult[len+1] );
                    }
                    else
                    {
                        _FTOSTR( &strResult[len+1], fVal );
                        for ( size_t i = _STRLEN( strResult )-1; i > 0; i-- )
                        {
                            if ( strResult[i] == '0' )
                                strResult[i] = 0;
                            else
                                break;
                        }
                    }
                    if ( m_nRecMode != RECMODE_WWW && 0 == (m_dwFlags & FLAG_NOSPACE) )
                    {
                        // add space
                        _STRCAT( strResult, g_space );
                    }
                    return true;
                }
            }
        }
        return false;
    }
    
    
    BOOL RecognizeStroke( CGStroke pStroke, UInt32 nStrokeCnt )
    {
        if ( pStroke == NULL || nStrokeCnt < 1 )
		{
#ifdef INTERNAL_RECO_THREAD
            return AsynchPutStroke( NULL, nStrokeCnt );
#else
			return SynchPutStroke( NULL, nStrokeCnt );
#endif // INTERNAL_RECO_THREAD
		}
        else if ( nStrokeCnt > MAX_TRACE_LENGTH )
            return true;
        
        BOOL        bResult = false;
        BOOL		bSkipStroke = false;
        
        LPPOINTS	stroke = new POINTS[nStrokeCnt+1];
        if ( NULL == stroke )
            return bResult;
        
		for (register UInt32 i = 0; i < nStrokeCnt; i++)
        {
            // break very long strokes
            if ( ((PIX_OFFSET+pStroke[i].pt.x) < 0 || (PIX_OFFSET+pStroke[i].pt.y) < 0 || 
                  pStroke[i].pt.y > MAX_PIXEL_VALUE || pStroke[i].pt.x > MAX_PIXEL_VALUE) ) 
                // && pStroke[i].y != TRACE_BREAK )
            {
                bSkipStroke = true;
                break;
            }
            stroke[i].x = (float)(PIX_OFFSET+pStroke[i].pt.x);
            stroke[i].y = (float)(PIX_OFFSET+pStroke[i].pt.y);
        }
        if ( bSkipStroke )
        {
            bResult = true;
        }
        else
        {
            p_RECO_point_type    pTrace = NULL;
            BOOL    bSmooth = (m_dwFlags & FLAG_SMOOTHSTROKES) ? 1 : 0;
            if ( (nStrokeCnt = ::FilterTrajectory( (int)nStrokeCnt, stroke, &pTrace, bSmooth )) > 0 && pTrace )
            {
#ifdef INTERNAL_RECO_THREAD
				if ( !(bResult = AsynchPutStroke( pTrace, nStrokeCnt )) )
				{
                    free( (void *)pTrace );
				}
#else
				bResult = SynchPutStroke( pTrace, nStrokeCnt );
				free( (void *)pTrace );
#endif
            }
        }
        delete [] stroke;
        return bResult;
    }
    
    
    static BOOL RecGetID(p_RECO_ID_type pID)
    {
        return RecoGetRecID( pID );
    }
    
    void RecClose()
    {
        // CloseAsynchRec();
        
        if ( NULL != m_RecCtx )
        {
            RecoCloseContext( m_RecCtx );
            m_RecCtx = NULL;
        }
    }
        
    
    void Uninitialize()
    {
#ifdef INTERNAL_RECO_THREAD
        if ( NULL != m_Thread )
        {
            // terminate the thread
            m_bRunThread = false;
            pthread_mutex_lock( &m_mutexReco );
            std::atomic_fetch_or( &m_bNewStroke, 1);
            // OSAtomicTestAndSet( 1, &m_bNewStroke );
            pthread_cond_signal( &m_condStroke );
            pthread_mutex_unlock( &m_mutexReco );
            
            struct timespec   time;
            time.tv_sec = 0;
            time.tv_nsec = 5000000;
            int i = 0;	// wait for thread to terminate, but avoid infinite loop
            while ( NULL != m_Thread && i++ < 20 )
                nanosleep( &time, NULL );
        }
                
        pthread_mutex_destroy( &m_mutexReco );
        pthread_mutex_destroy( &m_mutexQueue );
        pthread_mutex_destroy( &m_mutexResult );
        pthread_mutex_destroy( &m_mutexAsyncReco );
        pthread_cond_destroy( &m_condStroke );
        pthread_cond_destroy( &m_condResult );
#endif // INTERNAL_RECO_THREAD

        RecClose();
        DictsClose();
        FreeResults();
        FreeAltDict();

		if ( m_lastWord != NULL )
			free( (void *)m_lastWord );
        if ( m_pWordLrn != NULL )
            delete m_pWordLrn;
        m_pWordLrn = NULL;
        if ( m_pWordList != NULL )
            delete m_pWordList;
        m_pWordList = NULL;
		m_lastWord = NULL;
        m_bInitialized = false;
        SetCustomChars( NULL, NULL );
    }

    void SetCustomChars( const UCHR * pCustomNum, const UCHR  * pCustPunct )
    {
        if ( m_nCustomPunctuations != NULL )
            free( (void *)m_nCustomPunctuations );
        m_nCustomPunctuations = NULL;
        if ( pCustPunct != NULL)
            m_nCustomPunctuations = HWRStrDupA( pCustPunct );
        
        if ( m_nCustomNumbers != NULL )
            free( (void *)m_nCustomNumbers );
        m_nCustomNumbers = NULL;
        if ( pCustomNum != NULL)
            m_nCustomNumbers = HWRStrDupA( pCustomNum );
    }		
    
    
    void DictsInit( const char * pMainDict, const char * pUserDict )
    {
        DictsClose();
        
        if ( NULL != pMainDict && 0 != *pMainDict )
            LoadDictionary( pMainDict, &m_hMainDict );
                    
        if ( NULL != pUserDict && 0 != *pUserDict )
            LoadDictionary( pUserDict, &m_hUserDict, true );
    }
    
    bool LoadAltDict( const char * pAltDict )
    {
        FreeAltDict();
        if ( NULL != pAltDict && 0 != *pAltDict )
            LoadDictionary( pAltDict, &m_hAltDict );
        return (NULL != m_hAltDict);
    }
    
    void FreeAltDict()
    {
        if ( m_hAltDict != NULL )
            RecoFreeDict( &m_hAltDict );
        m_hAltDict = NULL;
    }

    static int AddUserWordsCallback( const UCHR * szWord, void * pParam )
    {
        CRecognizerWrapper *  pThis = (CRecognizerWrapper *)pParam;
        if ( PYDICT_NOERR == RecoAddWordToDict( szWord, &pThis->m_hInternetDict ) )
        {
            return 1;
        }
        return 0;
    }
    
    int CreateInternetDictionary()
    {
        int nWordCnt = 0;
        // make sure dictionary is not loaded
        FreeInternetDictionary();
        
        // load internet words then user dictionary
        if ( (0 == RecoLoadDict( NULL, &m_hInternetDict )) && m_hInternetDict != NULL )
        {
            // add default words to user dictionary
            for ( UInt32 i = 0; i < INTERNETWORD_CNT; i++ )
            {
                if ( PYDICT_NOERR == RecoAddWordToDictA( g_szInternetWords[i], &m_hInternetDict ) )
                    nWordCnt++;
            }
        
            if ( nWordCnt > 0 && NULL != m_hUserDict )
            {
                // add words from user to internet dictionary
                nWordCnt += DictEnumWords( m_hUserDict, AddUserWordsCallback, this );
            }
            if ( nWordCnt < 1 )
                FreeInternetDictionary();
        }
        return nWordCnt;
    }
    
    void FreeInternetDictionary()
    {
        if ( m_hInternetDict != NULL )
        {
            RecoFreeDict( &m_hInternetDict );
            m_hInternetDict = NULL;
        }
    }
    
    BOOL HasDictChanged( int dictType )
    {
        int nLen = 0;
        if ( dictType == kDictionaryType_Main )
            return RecoGetDictStatus( &nLen, m_hMainDict );
        else if ( dictType == kDictionaryType_Alternative )
            return RecoGetDictStatus( &nLen, m_hAltDict );
        else
            return RecoGetDictStatus( &nLen, m_hUserDict );
    }

    int GetDictLength( int dictType )
    {
        int nLen = 0;
        if ( dictType == kDictionaryType_Main )
            RecoGetDictStatus( &nLen, m_hMainDict );
        else if ( dictType == kDictionaryType_Alternative )
            RecoGetDictStatus( &nLen, m_hAltDict );
        else
            RecoGetDictStatus( &nLen, m_hUserDict );
        return nLen;
    }
    
    BOOL NewUserDict()
    {
        if ( m_hUserDict != NULL )
            RecoFreeDict( &m_hUserDict );
        m_hUserDict = NULL;
        // create new empty dictionary
        return (0 == RecoLoadDict( NULL, &m_hUserDict ));
    }

#if 0 // defined(MAC_OS_X)

    BOOL SaveUserDict( const char * pName )
    {
        if ( m_hUserDict == NULL )
            return true;
        int		nLen = 0;
        if ( ! RecoGetDictStatus( &nLen, m_hUserDict ) && nLen < 1 )
            return false;
        FILE *	file = fopen( pName, "w+" );
        if ( file == NULL )
            return false;

        // allocate buffer and load dictionary data
        BOOL	bRes = false;
        char *	pBuff = (char *)malloc( nLen + 8 );
        if ( NULL != pBuff )
        {
            if ( 0 == RecoSaveDict( pBuff, m_hUserDict ) )
            {
                if ( fwrite( pBuff, 1, nLen, file ) == nLen )
                    bRes = true;
            }
            free( (void*) pBuff );
        }
        fclose( file );
        if ( ! bRes )
            remove( pName );
        return bRes;
    }
    
    BOOL LoadDictionary( const char * pName, RECOHDICT * phDict, BOOL bUserDict = false )
    {
        // load dictionary
        BOOL	res = false;
        FILE *	file = fopen( pName, "r" );
        if ( file != NULL )
        {
            // get dictionary file length
            fseek( file, 0, SEEK_END );
            long nLen = ftell( file );
            fseek( file, 0, SEEK_SET );
            
            // allocate buffer and load dictionary data
            char * pBuff = (char *)malloc( nLen + 8 );
            if ( NULL != pBuff )
            {
                if ( fread( pBuff, 1, nLen, file ) == nLen )
                {
                    // convert to dictionary handle
                    res = (0 == RecoLoadDict( pBuff, phDict ));
                }
                free( (void *)pBuff );
            }
            fclose( file );
        }
        else if ( bUserDict )
        {
            // create new user dictionary
            res = (0 == RecoLoadDict( NULL, phDict ));
            if ( res && phDict != NULL )
            {
                // add default words to user dictionary
                for ( UInt32 i = 0; i < USERWORD_CNT; i++ )
                {
                    if ( *g_szDefUserWords[i] != 0 )
                    {
                        RecoAddWordToDictA( g_szDefUserWords[i], phDict );
                    }
                }
            }
        }
        return res;
    }
    
#else // MAC_OS_X
    
    BOOL SaveUserDict( const char * pName )
    {
        BOOL bRes = (PYDICT_NOERR == RecoSaveDictToFile( pName, m_hUserDict ));
        
        return bRes;
    }
    
    BOOL LoadDictionary( const char * pName, RECOHDICT * phDict, BOOL bUserDict = false )
    {
        BOOL    res = (PYDICT_NOERR == RecoLoadDictFromFile( pName, phDict ));
        if ( res )
        {
            
        }
        else if ( bUserDict )
        {
            // create new user dictionary
            res = (0 == RecoLoadDict( NULL, phDict ));
            if ( res && phDict != NULL )
            {
                // add default words to user dictionary
                for ( UInt32 i = 0; i < USERWORD_CNT; i++ )
                {
                    if ( *g_szDefUserWords[i] != 0 )
                    {
                        RecoAddWordToDictA( g_szDefUserWords[i], phDict );
                    }
                }
            }
        }
        return res;
    }
    
#endif // MAC_OS_X
    
    void DictsClose()
    {
        if ( m_hMainDict != NULL )
            RecoFreeDict( &m_hMainDict );
        if ( m_hUserDict != NULL )
            RecoFreeDict( &m_hUserDict );
        if ( m_hInternetDict != NULL )
            RecoFreeDict( &m_hInternetDict );
        m_hMainDict = NULL;
        m_hUserDict = NULL;
        m_hInternetDict = NULL;
    }
    
    BOOL ResetUserDictionary( const char * pName, BOOL bCreateNew )
    {
        if ( m_hUserDict != NULL )
            RecoFreeDict( &m_hUserDict );
        m_hUserDict = NULL;
        // create new user dictionary
        if ( bCreateNew )
        {		
            BOOL res = (0 == RecoLoadDict( NULL, &m_hUserDict ));
            if ( res && m_hUserDict != NULL && pName != NULL )
            {
                // add default words to user dictionary
                for ( UInt32 i = 0; i < USERWORD_CNT; i++ )
                {
                    if ( *g_szDefUserWords[i] != 0 )
                    {
                        RecoAddWordToDictA( g_szDefUserWords[i], &m_hUserDict );
                    }
                }
                return SaveUserDict( pName );
            }
            return res;
        }
        return LoadDictionary( pName, &m_hUserDict, true );
    }
    
    static int ExportUserWordsCallback( const UCHR * szWord, void * pParam )
    {
        FILE * file = (FILE *)pParam;
#ifdef HW_RECINT_UNICODE
        char * word = new char[_STRLEN( szWord )+2];
        UNICODEtoStr( word, szWord, _STRLEN( szWord )+1 );
#else
        char * word = (char *)szWord;
#endif // HW_RECINT_UNICODE
        int result = (fwrite( word, 1, HWRStrLen( word ), file ) > 0 && fwrite( "\n", 1, 1, file ) > 0) ? 1 : 0;
#ifdef HW_RECINT_UNICODE
        delete [] word;
#endif // HW_RECINT_UNICODE
        return result;
    }
    
    BOOL ImportUserDictionary( const char * pFileName )
    {
        FILE *	file = fopen( pFileName, "r" );
        if ( file == NULL )
            return false;
        
        // load file into the memory
        fseek( file, 0, SEEK_END );
        long nLen = ftell( file );
        fseek( file, 0, SEEK_SET );
        
        LPBYTE  pBuff = (LPBYTE)malloc( nLen+8 );
        if ( NULL == pBuff )
        {
            fclose( file );
            // SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return false;
        }       
        if ( (long)fread( pBuff, 1, nLen, file ) < nLen )
        {
            delete [] pBuff;
            fclose( file );
            return false;
        }
        fclose( file );		

        if ( m_hUserDict != NULL )
            RecoFreeDict( &m_hUserDict );
        m_hUserDict = NULL;
    
        BOOL res = (0 == RecoLoadDict( NULL, &m_hUserDict ));
        char szWord[HW_MAXWORDLEN] = "";
        
        if ( res && m_hUserDict != NULL )
        {
            // add words to user dictionary
            for ( long i = 0, k = 0; i < nLen; i++ )
            {
                if ( isspace( pBuff[i] ) || pBuff[i] == '\n' || pBuff[i] == '\r' || pBuff[i] == '\t' )
                {
                    if ( k > 0 )
                    {
                        szWord[k] = 0;
                        RecoAddWordToDictA( szWord, &m_hUserDict );
                        k = 0;
                    }
                }
                else if ( k < HW_MAXWORDLEN-1 )
                {
                    szWord[k++] = pBuff[i];
                }
            }
        }
        free( (void *)pBuff );
        return res;
    }
    
    BOOL ExportUserDictionary( const char * pFileName )
    {
        if ( m_hUserDict == NULL )
            return false;
        FILE *	file = fopen( pFileName, "w+" );
        if ( file == NULL )
            return false;

        BOOL bResult = (EnumUserWords(  ExportUserWordsCallback, file ) > 0);
        fclose( file );
        if ( ! bResult )
            remove( pFileName );
        return bResult;
    }
    
    int EnumUserWords( PRECO_ONGOTWORD callback, void * pParam )
    {
        if ( NULL == m_hUserDict )
            return 0;
        return DictEnumWords( m_hUserDict, callback, pParam );
    }
        
    const UCHR * GetResult() const 
    {
        return m_pResString;
    }
    
    int GetStrokeIDs( int word, int alt, const int ** strokes_arr )
    {
        *strokes_arr = NULL;
        if ( word < RecoGetAnswers( HW_NUM_ANSWERS, 0, 0, m_RecCtx ).value && alt < RecoGetAnswers( HW_NUM_ALTS, word, 0, m_RecCtx).value )
        {
            int nStrokes = RecoGetAnswers( HW_ALT_NSTR, word, alt, m_RecCtx ).value;
            if ( nStrokes > 0 )
                *strokes_arr = RecoGetAnswers( HW_ALT_STROKES, word, alt, m_RecCtx ).array;
            return nStrokes;
        }
        return -1;
    }
        
    UCHR * RecBuildResList( USHORT ** ppWeights, int ** ppNStrokes )
    {
        int       nWords = 0, nAltCnt = 0, nAlts = 0;
        int		  iWeightStart = 0;
        USHORT  * pWeights = NULL;
        int	*	  pNStrokes = NULL;
        UCHR *    pAlt = NULL;
        UCHR *    pRes = NULL;
        register int    iWeightLen = 1;
        register int    i, j;
		size_t	  iWordStart = 0, iBufLen = 1;
        BOOL	  bSuggestedOnly = false;
        BOOL      bReplaceFromDict = false;
        UCHR	  szLearnedAlt[HW_MAXWORDLEN] = {0};
        int		  spellFlags = ((m_dwFlags & FLAG_USERDICT) != 0) ? HW_SPELL_USERDICT : 0;
        if ( 0 != (m_dwFlags & FLAG_SPELLIGNORENUM) )
            spellFlags |= HW_SPELL_IGNORENUM;
        if ( 0 != (m_dwFlags & FLAG_SPELLIGNOREUPPER) )
            spellFlags |= HW_SPELL_IGNOREUPPER;			
        
        // Special processing for the 'Single Word' mode
        nWords = RecoGetAnswers( HW_NUM_ANSWERS, 0, 0, m_RecCtx ).value;
        if ( 0 != (m_dwFlags & FLAG_SINGLEWORDONLY) && nWords == 2 )
            nWords = 1;
            
        for ( i = 0; i < nWords; i++ )
        {
            nAlts = RecoGetAnswers( HW_NUM_ALTS, i, 0, m_RecCtx).value;
            iBufLen += (1 + HW_MAXWORDLEN) * (nAlts + 1);     // for words with ALTSEP
            iBufLen++;  // for LISTSEP
            iWeightLen += (2 * (nAlts+1));
            iWeightLen++;
        }
        iWeightLen++;
        iBufLen++;
        
        pRes = new UCHR[iBufLen];
        if ( pRes == NULL )
            return NULL;
        memset( pRes, 0, sizeof(UCHR) * (iBufLen) );
        pWeights = new USHORT[iWeightLen];
        memset( pWeights, 0, iWeightLen * sizeof( USHORT ) );
        pNStrokes = new int[iWeightLen];
        memset( pNStrokes, 0, iWeightLen * sizeof( int ) );
        
        USHORT  nWeight = 0;	
        int	nStrokes = 0;
        iWeightLen = 0;
        iBufLen = 0;
        
        for ( i = 0; i < nWords; i++ )
        {
            nAltCnt = 0;
            szLearnedAlt[0] = 0;
            bReplaceFromDict = false;
            bSuggestedOnly = false;
            
            nAlts = RecoGetAnswers( HW_NUM_ALTS, i, 0, m_RecCtx).value;
            iWordStart = iBufLen; // remember the position of the word Alts to insert replacement here
            iWeightStart = iWeightLen;
            int		analyze = 0;
            
            for ( j = 0; j < nAlts; j++ )
            {
                pAlt = RecoGetAnswers( HW_ALT_WORD, i, j, m_RecCtx ).word;
                
                if ( NULL == pAlt || 0 == *pAlt )
                    continue;
                
                /* NOTE: this causes problem entering punctuation characters, etc.
                if ( 0 != (m_dwFlags & FLAG_ONLYDICT) && (m_nRecMode == RECMODE_GENERAL) ) 
                {
                    if ( ! TestWord( pAlt, spellFlags )) 
                        continue;
                }
                */
                if ( j == 0 )
                {
                    analyze = (_STRLEN( pAlt ) < 2) ? 2 : TestWord( pAlt, spellFlags );
                }
                
                nWeight = (USHORT)RecoGetAnswers( HW_ALT_WEIGHT, i, j, m_RecCtx ).value;
                nStrokes = RecoGetAnswers( HW_ALT_NSTR, i, j, m_RecCtx ).value;
                                
                if ( m_nRecMode == RECMODE_WWW )
                {
                    // make all lower case for addresses
                    CharLower( pAlt );
                }
                else if ( m_nRecMode == RECMODE_CAPS )
                {
                    // make sure all returned characters are upper-case
                    CharUpper( pAlt );
                }
                else if ( 0 != (m_dwFlags & FLAG_CORRECTOR) ) 
                {
                    if ( ! TestWord( pAlt, spellFlags ) )
                    {
                        // fix caps & month names, if the word is not found in dict.
                        CorrectCaps( pAlt );
                        const UCHR * pNew = CorrectDays( pAlt );
                        if ( pNew != pAlt )
                            pAlt = (UCHR *)pNew;
                    }
                }
                
                // add autocorrector
                if ( 0 != (m_dwFlags & FLAG_CORRECTOR) )
                {
                    if ( j == 0 && 0 == szLearnedAlt[0] )
                    {                
                        if ( ! GetReplaceWord( pAlt, szLearnedAlt, &bSuggestedOnly ) )
                            szLearnedAlt[0] = 0;
                    }
                    else if ( j > 0 && szLearnedAlt[0] != 0 && bSuggestedOnly )
                    {
                        // must use case insensitive compare
                        if ( 0 == _STRCASECMP( szLearnedAlt, pAlt ) )
                            bSuggestedOnly = false;
                    }
                }
                
                if ( szLearnedAlt[0] == 0 && 0 != (m_dwFlags & FLAG_ONLYDICT) && (m_hMainDict || m_hUserDict) && (m_nRecMode == RECMODE_GENERAL) )
                {
                    if ( j == 0 )
                    {
                        if ( ! TestWord( pAlt, spellFlags ) )
                            bReplaceFromDict = true;
                    }
                    else if ( j > 0 && bReplaceFromDict )
                    {
                        if ( TestWord( pAlt, spellFlags ) )
                        {
                            _STRCPY( szLearnedAlt, pAlt );
                            bReplaceFromDict = false;
                        }
                    }
                }
                
                if ( 0 != szLearnedAlt[0] && 0 == _STRCMP( pAlt, szLearnedAlt ) )
                    continue;
            
                _STRCPY( &pRes[iBufLen], pAlt );
                iBufLen += _STRLEN( pAlt ); //for alt
                if ( j < nAlts-1 )
                {
                    pRes[iBufLen] = PM_ALTSEP;
                    iBufLen++; //for PM_ALTSEP
                }
                pNStrokes[iWeightLen] = nStrokes;
                pWeights[iWeightLen] = nWeight;
                iWeightLen++;
                nAltCnt++;
            }
            
            if ( nAltCnt > 0 )
            {
                pRes[iBufLen] = PM_LISTSEP;
                iBufLen++;
                pRes[iBufLen] = 0;
                pWeights[iWeightLen] = PM_LISTSEP; 
                pNStrokes[iWeightLen] = (int)PM_NUMSEPARATOR; 
                iWeightLen++;
                pWeights[iWeightLen] = 0;
                pNStrokes[iWeightLen] = 0;
                
                if ( 0 != (m_dwFlags & FLAG_ANALYZER) && (0 == szLearnedAlt[0] || bSuggestedOnly) && analyze < 2 )
                {
                    if ( analyze )
                        pWeights[iWeightStart] |= RW_DICTIONARYWORD;
                    if ( AnalyzeResult( &pRes[iWordStart], &pWeights[iWeightStart], szLearnedAlt, &nWeight ) < 1 )
                    {
                        szLearnedAlt[0] = 0;
                    }
                    else
                    {
                        if ( 0 != (m_dwFlags & FLAG_CORRECTOR) )
                        {
                            UCHR	  szAlt2[HW_MAXWORDLEN] = {0};
                            if ( GetReplaceWord( szLearnedAlt, szAlt2, &bSuggestedOnly ) )
                                _STRCPY( szLearnedAlt, szAlt2 );
                        }
                        bSuggestedOnly = false;
                    }
                    pWeights[iWeightStart] &= ~RW_DICTIONARYWORD;
                }
                if ( szLearnedAlt[0] != 0 && (! bSuggestedOnly) )
                {
                    size_t     iLen = _STRLEN( szLearnedAlt );
                    
                    // insert a new word
                    memmove( pRes + iWordStart + iLen + 1, pRes + iWordStart, (iBufLen - iWordStart) * sizeof( UCHR ) );
                    _STRCPY( pRes + iWordStart, szLearnedAlt );
                    pRes[iWordStart+iLen] = PM_ALTSEP;
                    iBufLen += (iLen + 1);
                    // offset weights and strokes
                    memmove( pWeights + iWeightStart + 1, pWeights + iWeightStart, (iWeightLen - iWeightStart) * sizeof( USHORT ) );
                    pWeights[iWeightStart] = pWeights[iWeightStart + 1]; // reset mask for the inserted word
                    memmove( pNStrokes + iWeightStart + 1, pNStrokes + iWeightStart, (iWeightLen - iWeightStart) * sizeof( int ) );
                    pNStrokes[iWeightStart] = pNStrokes[iWeightStart + 1]; // reset mask for the inserted word
                    iWeightLen++;
                    nAltCnt++;
                    // note: for now we'll keep the same weight value for the inserted (first) word
                }
            }
        }
        pRes[iBufLen] = 0;
        pWeights[iWeightLen] = 0;
        pNStrokes[iWeightLen] = 0;
        *ppWeights = pWeights;
        *ppNStrokes = pNStrokes;
        return pRes;
    }
    
    void	SetFlags( DWORD dwFlags ) { m_dwFlags = dwFlags; }
    DWORD	GetFlags() const { return m_dwFlags; }
    
    void	SetMode( int nNewMode ) 
    { 
        if ( m_nRecMode != nNewMode )
        {
            // create/destroy internet dictionary as needed
            if ( m_nRecMode == RECMODE_WWW )
                FreeInternetDictionary();
            if ( nNewMode == RECMODE_WWW )
                CreateInternetDictionary();
            m_nRecMode = nNewMode;
            std::atomic_fetch_and( &m_bResultsReady, 0);
            // OSAtomicTestAndClear( 1, &m_bResultsReady );
        }
    } 
    int		GetMode() const { return m_nRecMode; }
    
    BOOL RecSetDefaultShapes()
    {
        if ( NULL == m_RecCtx )
            return false;
        memset( letter_shapes, 0, LRN_WEIGHTSBUFFER_SIZE );
        return (0 == RecoGetSetPicturesWeights( LRN_SETDEFWEIGHTS_OP, letter_shapes, m_RecCtx));
    }

    BOOL RecSetShapes( const unsigned char * pShapes )
    {
        if ( NULL == m_RecCtx )
            return false;
        memcpy( letter_shapes, pShapes, LRN_WEIGHTSBUFFER_SIZE );
        return (0 == RecoGetSetPicturesWeights( LRN_SETCURWEIGHTS_OP, letter_shapes, m_RecCtx));
    }
    
    const unsigned char * RecGetShapes()
    {
        if ( NULL == m_RecCtx )
            return NULL;
        memset( letter_shapes, 0, LRN_WEIGHTSBUFFER_SIZE );
        if ( 0 == RecoGetSetPicturesWeights( LRN_GETCURWEIGHTS_OP, letter_shapes, m_RecCtx) )
        {
            return letter_shapes;
        }
        return NULL;
    }
    
    BOOL InitLearner( const char * pszFileNameOrData, BOOL bFile )
    {
        if ( NULL != m_pWordLrn )
            delete m_pWordLrn;
        
        m_pWordLrn = new CWordLrnFile( GetLanguageID() );
        if ( NULL == m_pWordLrn )
        {
            // SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return false;
        }
        if ( NULL != pszFileNameOrData )
        {
            if ( bFile )
            {
                if ( ! LoadLearnerFile( pszFileNameOrData ) )
                {
                    delete m_pWordLrn;
                    m_pWordLrn = NULL;    // unable to load file; destroy the learner object
                    return false;
                }
            }
            else
            {
                // load from memory
                if ( ! m_pWordLrn->LoadFromMemory( pszFileNameOrData, GetLanguageID() ) )
                {
                    delete m_pWordLrn;
                    m_pWordLrn = NULL;    // unable to load file; destroy the learner object
                    return false;
                }
            }
        }
        return true;
    }
        
    BOOL LoadLearnerFile( const char * pszFileName )
    {
        if ( NULL == m_pWordLrn )
            return false;
        return m_pWordLrn->Load( pszFileName, GetLanguageID() );
    }
    
    BOOL LearnWord( const UCHR * pWord, USHORT nWeight )
    {
        if ( NULL == m_pWordLrn || NULL == pWord || 0 == *pWord || 0 == (m_dwFlags & FLAG_ANALYZER) )
            return false;
        return (m_pWordLrn->AddWord( pWord, (nWeight & RW_WEIGHTMASK), 0 != (nWeight&RW_DICTIONARYWORD) ) >= 0);
    }
    
    BOOL ReplaceWord( const UCHR * pszWord1, USHORT nWeight1, const UCHR * pszWord2, USHORT nWeight2 )
    {
        if ( NULL == m_pWordLrn || NULL == pszWord1 || 0 == *pszWord1 || NULL == pszWord2 || 0 == *pszWord2 || 0 == (m_dwFlags & FLAG_ANALYZER) )
            return false;
        return m_pWordLrn->ReplaceWord( (LPCWSTR)pszWord1, nWeight1, (LPCWSTR)pszWord2, nWeight2 );
    }
    
    void ReleaseLearner( const char * pszFileName )
    {
        if ( NULL != pszFileName && NULL != m_pWordLrn )
            SaveLearner( pszFileName );
        delete m_pWordLrn;
        m_pWordLrn = NULL;
    }
    
    BOOL SaveLearner( const char * pszFileName )
    {
        if ( NULL != m_pWordLrn )
            return m_pWordLrn->Save( pszFileName );
        return false;
    }	
        
    int GetLearnerData( char ** ppData )
    {
        if ( NULL != m_pWordLrn )
            return m_pWordLrn->SaveToMemory( ppData );
        return 0;
    }

    DWORD FindLearnedWord( const UCHR * pszWord )
    {
        if ( NULL == m_pWordLrn )
            return 0;
        CWordLrnFile::CWord * pWord = m_pWordLrn->GetWord( pszWord );
        if ( NULL != pWord )
            return MAKELONG( pWord->m_nCount, pWord->m_nWeight );
        return 0;
    }

    int AnalyzeResult( const UCHR * pszWordList, UCHR * pSortedWordList )
    {
        if ( NULL == m_pWordLrn || NULL == pszWordList || NULL == pSortedWordList )
            return -1;
        return m_pWordLrn->AnalizeResult( pszWordList, NULL, pSortedWordList, NULL );
    }
    
    int AnalyzeResult( const UCHR * pszWordList, USHORT * pnWeights, UCHR * pResult, USHORT * pnWeight = NULL )
    {
        if ( NULL == m_pWordLrn || NULL == pszWordList /*|| NULL == pnWeights*/ || NULL == pResult )
            return -1;
        
        UCHR   wszResult[HW_MAXWORDLEN] = {0};
        int    nResult = m_pWordLrn->AnalizeResult( pszWordList, pnWeights, wszResult, pnWeight );
        
        if ( nResult > 0 )
        {
            _STRCPY( pResult, wszResult );
            CharLower( pResult );
            
            // fix capitalization
            UCHR    szWord[HW_MAXWORDLEN] = {0};
            const UCHR *  pwl = pszWordList;
            int		nuCnt = 0;
            while( *pwl && *pwl != PM_ALTSEP && *pwl != PM_LISTSEP )
                szWord[nuCnt++] = *pwl++;
            szWord[nuCnt] = 0;
            if ( nuCnt > 0 )
            {
                nuCnt = 0;
                if ( IsWordUpper( szWord, nuCnt ) )
                {
                    // make the new word upper case
                    CharUpper( pResult ); 
                }
                else if ( IsUpper( szWord[0] ) )
                {
                    // capitalize the first letter of the new word
                    pResult[0] = toupper( pResult[0] );
                }
            }
        }
        return nResult;
    }
    
    BOOL AllLower( UCHR * pszWord ) const
    {
        int nUpperCnt = 0;
        int		  spellFlags = ((m_dwFlags & FLAG_USERDICT) != 0) ? HW_SPELL_USERDICT : 0;
        if ( 0 != (m_dwFlags & FLAG_SPELLIGNORENUM) )
            spellFlags |= HW_SPELL_IGNORENUM;
        
        IsWordUpper( pszWord, nUpperCnt );
        if ( nUpperCnt < 1 )
            return false;
        CharLower( pszWord );
        return TestWord( pszWord, spellFlags );
    }
    
    BOOL FlipCase( UCHR * pszWord ) const
    {
        int nUpperCnt = 0;
        int		  spellFlags = ((m_dwFlags & FLAG_USERDICT) != 0) ? HW_SPELL_USERDICT : 0;
        if ( 0 != (m_dwFlags & FLAG_SPELLIGNORENUM) )
            spellFlags |= HW_SPELL_IGNORENUM;
        
        IsWordUpper( pszWord, nUpperCnt );
        BOOL upper =  IsUpper( pszWord[0] );
        if ( nUpperCnt > 1 )
        {
            CharLower( pszWord );
        }
        else
        {
            if ( upper )
            {
                pszWord[0] = tolower( pszWord[0] );
            }
            else // if ( IsLower( pszWord[0] ) )
            {
                pszWord[0] = toupper( pszWord[0] );
            }
        }
        return TestWord( pszWord, spellFlags );
    }
    
    BOOL IsWordUpper( const UCHR * pszWord, int & nUpperCnt ) const
    {
        BOOL bRes = true;
        nUpperCnt = 0;
        while( *pszWord )
        {
            if ((! IsUpper( *pszWord )) && (! IsSpace( *pszWord )) )
            {
                bRes = false;
                break;
            }
            else
                nUpperCnt++;		
            pszWord++;
        }
        return bRes;
    }
    
    BOOL WordHasNumbers( const UCHR * pszWord ) const
    {
        while( *pszWord )
        {
            if ( IsDigit(*pszWord) )
                return true;
            pszWord++;
        }
        return false;
    }

    void CharUpper( UCHR * pszWord ) const
    {
        while( *pszWord )
        {
            if ( ! IsUpper( *pszWord ) )
                *pszWord = ToUpper( *pszWord );
            pszWord++;
        }
    }

    void CharLower( UCHR * pszWord ) const 
    {
        while( *pszWord )
        {
            if ( IsUpper( *pszWord ) )
                *pszWord = ToLower( *pszWord );
            pszWord++;
        }
    }
        
    BOOL InitWordList( const char * pszFileNameOrData, BOOL bFile, BOOL bCreateNew = false )
    {
        if ( NULL != m_pWordList )
            return true;

        m_pWordList = new CWordList;
        if ( NULL == m_pWordList )
            return false;
        if ( NULL != pszFileNameOrData )
        {
            if ( bFile )
            {
                if ( ! bCreateNew )
                {
                    if ( m_pWordList->Load( pszFileNameOrData ) )
                        return true;
                }
                if ( CreateDefaultListFile( pszFileNameOrData ) )
                {
                    return true;
                }
            }
            else
            {
                if ( ! bCreateNew )
                {
                    if ( m_pWordList->LoadFromMemory( pszFileNameOrData ) )
                        return true;
                }
                if ( CreateDefaultListFile( NULL ) )
                {
                    return true;
                }
            }
            delete m_pWordList;
            m_pWordList = NULL;
            return false;
        }
        return true;
    }
    
    size_t GetWordListData( char ** ppData )
    {
        if ( NULL == m_pWordList )
            return 0;
        return m_pWordList->SaveToMemory( ppData, LANG_ID );
    }
    
    BOOL LoadWordList( const char * pszFileName )
    {
        if ( NULL == m_pWordList )
            return false;
        return m_pWordList->Load( pszFileName );
    }
    
    BOOL AddWordToWordList( const UCHR * pWord1, const UCHR * pWord2, DWORD dwFlags, BOOL bReplace )
    {
        if ( NULL == m_pWordList || NULL == pWord1 || 0 == *pWord1 || NULL == pWord2 || 0 == *pWord2 )
            return false;
        return (NULL != m_pWordList->AddWord( pWord1, pWord2, bReplace, dwFlags ));
    }
        
    BOOL SaveWordList( const char * pszFileName )
    {
        if ( NULL == m_pWordList )
            return false;
        return m_pWordList->Save( pszFileName );
    }
    
    BOOL ExportWordList( const char * pszFileName )
    {
        if ( NULL == m_pWordList )
            return false;
        return m_pWordList->Export( pszFileName );
    }
    
    BOOL ImportWordList( const char * pszFileName )
    {
        if ( NULL == m_pWordList )
            return false;
        return m_pWordList->Import( pszFileName );
    }
    
    int EnumWordList( RECO_ONGOTWORDLIST callback, void * pParam )
    {
        if ( NULL == m_pWordList )
            return 0;
        return m_pWordList->EnumWordList( callback, pParam );
    }	
        
    BOOL GetReplaceWord( const UCHR * pszWord, UCHR * pszResult, BOOL * pbSuggested )
    {
        if ( NULL == m_pWordList )
            return false;
        DWORD	dwFlags = 0;
        
        const UCHR * pResult = m_pWordList->GetReplaceWord( pszWord, &dwFlags );
        
        if ( 0 != (dwFlags & WCF_DISABLED) )
        {
            return false;	// disabled
        }
        if ( NULL != pResult )
        {
            int		nuCnt = 0;
            _STRCPY( pszResult, pResult );
            if ( IsWordUpper( pszWord, nuCnt ) )
            {
                // make the new word upper case
                CharUpper( pszResult ); 
            }
            else if ( IsUpper( pszWord[0] ) )
            {
                // capitalize the first letter of the new word
                pszResult[0] = toupper( pszResult[0] );
            }
        }
        *pbSuggested = (0 == (dwFlags & WCF_ALWAYS));
        return (NULL != pResult);
    }
    
    BOOL FindWordInWordList( const UCHR * pszWord )
    {
        if ( NULL == m_pWordList )
            return false;
        return (NULL != m_pWordList->GetReplaceWord( pszWord ));
    }
    
    BOOL ReleaseWordList( const char * pszFileName )
    {
        if ( NULL == m_pWordList )
            return false;
        if ( NULL != pszFileName )
        {
            if ( ! m_pWordList->Save( pszFileName ) )
                return false;
        }
        delete m_pWordList;
        m_pWordList = NULL;
        return true;
    }
    
    BOOL EmptyWordList()
    {
        if ( NULL == m_pWordList )
            return false;
        
        m_pWordList->RemoveAll();		
        return true;
    }
    
    BOOL CreateDefaultListFile( const char * pszFileName )
    {
        if ( NULL == m_pWordList )
            return false;
        
        m_pWordList->RemoveAll();		
        m_pWordList->AddWord( "phatwane", "PhatWare" );
        
        UInt32 count = 0;
        switch ( GetLanguageID() ) 
        {
            case LANGUAGE_ENGLISHUK :
                // TODO: add UK-specific word list
            case LANGUAGE_ENGLISH :
                count = DEFWORDCNT_EUS;
                break;
            case LANGUAGE_DANISH :
                count = DEFWORDCNT_DAN;
                break;
            case LANGUAGE_SPANISH :
                count = DEFWORDCNT_SPN;
                break;
            case LANGUAGE_DUTCH :
                count = DEFWORDCNT_DUT;
                break;
            case LANGUAGE_GERMAN :
                count = DEFWORDCNT_GER;
                break;
            case LANGUAGE_FRENCH :
                count = DEFWORDCNT_FRN;
                break;
            case LANGUAGE_PORTUGUESEB :
                count = DEFWORDCNT_BRZ;
                break;
            case LANGUAGE_PORTUGUESE :
                count = DEFWORDCNT_PRT; 
                break;
            case LANGUAGE_ITALIAN :
                count = DEFWORDCNT_ITL;
                break;
            case LANGUAGE_INDONESIAN :
            default:
                count = DEFWORDCNT;
                break;
        }		
        
        for ( UInt32 i = 0; i < count; i += 2 )
        {
            switch ( GetLanguageID() ) 
            {
                case LANGUAGE_ENGLISHUK :
                    // TODO: add UK-specific word list
                case LANGUAGE_ENGLISH :
                    m_pWordList->AddWord( g_szDefWords_EUS[i], g_szDefWords_EUS[i+1] );
                    break;
                case LANGUAGE_DANISH :
                    m_pWordList->AddWord( g_szDefWords_DAN[i], g_szDefWords_DAN[i+1] );
                    break;
                case LANGUAGE_SPANISH :
                    m_pWordList->AddWord( g_szDefWords_SPN[i], g_szDefWords_SPN[i+1] );
                    break;
                case LANGUAGE_DUTCH :
                    m_pWordList->AddWord( g_szDefWords_DUT[i], g_szDefWords_DUT[i+1] );
                    break;
                case LANGUAGE_GERMAN :
                    m_pWordList->AddWord( g_szDefWords_GER[i], g_szDefWords_GER[i+1] );
                    break;
                case LANGUAGE_FRENCH :
                    m_pWordList->AddWord( g_szDefWords_FRN[i], g_szDefWords_FRN[i+1] );
                    break;
                case LANGUAGE_ITALIAN :
                    m_pWordList->AddWord( g_szDefWords_ITL[i], g_szDefWords_ITL[i+1] );
                    break;
                case LANGUAGE_PORTUGUESEB :
                    m_pWordList->AddWord( g_szDefWords_BRZ[i], g_szDefWords_BRZ[i+1] );
                    break;
                case LANGUAGE_PORTUGUESE :
                    m_pWordList->AddWord( g_szDefWords_PRT[i], g_szDefWords_PRT[i+1] );
                    break;
                case LANGUAGE_INDONESIAN :
                default:
                    m_pWordList->AddWord( g_szDefWords[i], g_szDefWords[i+1] );
                    break;
            }
        }
        if ( pszFileName == NULL )
            return true;	// do not save to file, just exit the function
        return m_pWordList->Save( pszFileName, GetLanguageID() );
    }
    
    const UCHR * CorrectDays( UCHR * pszWord )
    {
        for ( UInt32 i = 0; i < CAPWORD_CNT; i++ )
        {
            // must use case insensitive search can't find stricmp
#ifdef HW_RECINT_UNICODE
            UCHR szUnicode[50] = {0};
            StrToUNICODE( szUnicode, g_pszCapWords[i], 49 );
            if ( _STRCASECMP( szUnicode, pszWord ) == 0 )
            {
                _STRCPY( pszWord, szUnicode );
#else
            if ( _STRCASECMP( g_pszCapWords[i], pszWord ) == 0 )
            {
                _STRCPY( pszWord, g_pszCapWords[i] );
#endif
                break;
            }
        }
        return pszWord;
    }
    
    const UCHR * CorrectCaps( UCHR * pszWord )
    {
        if ( (unsigned char)pszWord[0] <= ' ' )
            return pszWord;
        size_t		nlen = _STRLEN( pszWord );
        if ( nlen < 2 )
            return pszWord;
        if ( IsLower( pszWord[0] ) )
        {
            CharLower( pszWord );
            return pszWord;
        }
        int		nuCnt = 0;
        if ( IsWordUpper( pszWord, nuCnt ) )
            return pszWord;
        if ( (IsUpper( pszWord[0] ) && IsUpper( pszWord[1] ) && (unsigned char)pszWord[2] > ' ') ||
            (nlen > 2 && nlen < 8 && nuCnt > 1) || nuCnt > 2 )
        {
            CharLower( pszWord );
            pszWord[0] = (unsigned char)toupper( pszWord[0] );
        }
        return pszWord;
    }
    
    const UCHR * RecognizeInkData( CInkData * pInkData, int nFirstStroke, int nLastStroke, BOOL bAsync, BOOL bFlipY, BOOL bSort, BOOL bSelOnly )
    {
        // make sure the recognizer is enabled...
        if ( bAsync )
        {
            std::atomic_fetch_or( &m_bRunRecognizer, 1);
            // OSAtomicTestAndSet( 1, &m_bRunRecognizer );
        }
        int nCnt = pInkData->StrokesTotal();
        if ( nCnt < 1 )
            return NULL;
        
        if ( nLastStroke > 0 )
            nCnt = min( nLastStroke, nCnt );
        LPSTRRECT pStrokes = pInkData->GetRecoStrokes( nFirstStroke, &nCnt, bSort, bSelOnly );
        if ( pStrokes == NULL )
            return NULL;

#ifdef INTERNAL_RECO_THREAD
        AsynchReset();
#else
		SynchReset();
#endif // INTERNAL_RECO_THREAD
        
        UInt32 nCurrentLen = STROKE_BUFFER_SIZE;
        CGStroke points = (CGStroke)malloc( nCurrentLen * sizeof( CGTracePoint ) );
        if ( points == NULL )
            goto err;
                
        for ( int i = 0; i < nCnt; i++ )
        {
            if ( bAsync )
            {
                if ( !std::atomic_fetch_or( &m_bRunRecognizer, 1) )
                // if ( ! OSAtomicTestAndSet( 1, &m_bRunRecognizer ) )
                    goto err;
            }
            UInt32 nStrokeLen = pInkData->GetStrokePointCnt( (int)pStrokes[i].num );
            if ( nStrokeLen >= nCurrentLen )
            {
                // add more memory if needed
                nCurrentLen = nStrokeLen + STROKE_BUFFER_SIZE;
                points = (CGStroke)realloc( (void *)points, nCurrentLen * sizeof( CGTracePoint ) );
                if ( NULL == points )
                    goto err;
            }
            nStrokeLen = pInkData->ReadOneStroke( (int)pStrokes[i].num, points );
            if ( nStrokeLen > 0 )
            {
                if ( bFlipY )
                {
                    // on iPhone coordinates are up-side-down
                    for ( UInt32 j = 0; j < nStrokeLen; j++ )
                    {
                        // 4096 - this number must be bigger than screen size, otherwise does not matter
                        points[j].pt.y = 4096 - points[j].pt.y;			
                    }				
                }
#ifndef INTERNAL_RECO_THREAD
				m_cRecQueue = (nCnt - i);
#endif // INTERNAL_RECO_THREAD
                if ( ! RecognizeStroke( points, nStrokeLen ) )
                    goto err;
            }
        }
        
#ifndef INTERNAL_RECO_THREAD
		m_cRecQueue = 0;
#endif // INTERNAL_RECO_THREAD
        if ( bAsync )
        {
            if ( ! std::atomic_fetch_or( &m_bRunRecognizer, 1) )
            // if ( ! OSAtomicTestAndSet( 1, &m_bRunRecognizer ) )
                goto err;
        }
        
        if ( ! RecognizeStroke( NULL, 0 ) )
            goto err;
        free( (void *)pStrokes );
        free( (void *)points );
        pStrokes = NULL;
        points = NULL;
#ifdef INTERNAL_RECO_THREAD
        struct timespec wait;
        wait.tv_nsec = 0;
        wait.tv_sec = DEFAULT_TIMEOUT;
        WaitForResult( NULL );
#else

#endif // INTERNAL_RECO_THREAD
        if ( bAsync )
        {
            if ( ! std::atomic_fetch_or( &m_bRunRecognizer, 1) )
            // if ( ! OSAtomicTestAndSet( 1, &m_bRunRecognizer ) )
                return NULL;
        }
        return GetResult();
        
    err:
        if ( NULL != pStrokes )
            free( (void *)pStrokes );
        if ( NULL != points )
            free( (void *)points );
        // RecognizeStroke( NULL, 0 );
#ifdef INTERNAL_RECO_THREAD
        AsynchReset();
#else
		SynchReset();
#endif // INTERNAL_RECO_THREAD
        return NULL;
    }

    BOOL PreRecognizeInkData( CInkData * pInkData, int nDataLen, BOOL bFlipY  )
    {
        int nCnt = pInkData->StrokesTotal();
        if ( nCnt < 1 )
            return false;
        
        if ( nDataLen > 0 )
            nCnt = min( nDataLen, nCnt );
        
#ifdef INTERNAL_RECO_THREAD
        AsynchReset();
#else
		SynchReset();
#endif // INTERNAL_RECO_THREAD
        
        BOOL	result = false;
        UInt32	nCurrentLen = STROKE_BUFFER_SIZE;
        CGStroke points = (CGStroke)malloc( nCurrentLen * sizeof( CGTracePoint ) );
        if ( points == NULL )
            return false;
        
        for ( int i = 0; i < nCnt; i++ )
        {
            UInt32 nStrokeLen = pInkData->GetStrokePointCnt( i );
            if ( nStrokeLen > nCurrentLen )
            {
                // add more memory if needed
                nCurrentLen = nStrokeLen + STROKE_BUFFER_SIZE;
                points = (CGStroke)realloc( (void *)points, nCurrentLen * sizeof( CGTracePoint ) );
                if ( NULL == points )
                    goto err;
            }
            nStrokeLen = pInkData->ReadOneStroke( i, points );
            if ( nStrokeLen > 0 )
            {
                if ( bFlipY )
                {
                    // on iPhone coordinates are up-side-down
                    for ( UInt32 j = 0; j < nStrokeLen; j++ )
                    {
                        // 4096 - this number must be bigger than screen size, otherwise does not matter
                        points[j].pt.y = 4096 - points[j].pt.y;			
                    }				
                }
                if ( ! RecognizeStroke( points, nStrokeLen ) )
                    goto err;
            }
        }
        result = true;
        
    err:
        
        // if ( NULL != points )
        free( (void *)points );
        return result;
    }
        
    void StopAsyncReco()
    {
        std::atomic_fetch_and( &m_bRunRecognizer, 0);
        // OSAtomicTestAndClear( 1, &m_bRunRecognizer );
        ResultsReady( false );
    }
    
#ifdef _WIN32
    void nanosleep(const struct timespec* req, struct timespec* rem)
    {
        //Sleep(50);
    }
#endif
    // BOOL RunRecognizer() const { return m_bRunRecognizer; }

public:
	UCHR	*	 m_lastWord;
    
private:
    RECOCTX      m_RecCtx;
    RECOHDICT    m_hUserDict;
    RECOHDICT    m_hMainDict;
    RECOHDICT	 m_hInternetDict;
    RECOHDICT	 m_hAltDict;
    int			 m_nRecMode;
    BOOL		 m_bNewSession;
    DWORD		 m_dwFlags;
    BOOL		 m_bInitialized;
    BOOL		 m_bRunThread;
    USHORT	*	 m_pWeights;
    int	*		 m_pNStrokes;
    UCHR	*	 m_pLastRes;
    UCHR *		 m_pResString;
    char	*	 m_nCustomPunctuations;
    char	*	 m_nCustomNumbers;

    // recognition thread support
    std::atomic_int		m_bRunRecognizer;
    std::atomic_int		m_bResultsReady;
    int                 m_cRecQueue;

#ifdef INTERNAL_RECO_THREAD
    OpenALThread *		m_Thread;
    pthread_cond_t		m_condStroke;
    pthread_cond_t		m_condResult;
    pthread_mutex_t		m_mutexReco;		
    pthread_mutex_t		m_mutexResult;
    pthread_mutex_t		m_mutexAsyncReco;
    pthread_mutex_t		m_mutexQueue;
    ASYNC_RECOG_PARAMS  m_pRecQueue[RECQUEUE_MAX_ELEMS];
    std::atomic_int     m_bNewStroke;
#endif // INTERNAL_RECO_THREAD
    
    unsigned char		letter_shapes[LRN_WEIGHTSBUFFER_SIZE];
            
    CWordLrnFile *		m_pWordLrn;
    CWordList *			m_pWordList;
};

    
static BOOL _CheckWord( const UCHR * pszWord )
{
    if ( pszWord == NULL || *pszWord == 0 )
        return false;
    int i;
    for ( i = 0; i < _STRLEN( pszWord ) && i  < HW_MAXWORDLEN; i++ )
    {
        if ( pszWord[i] <= ' ' || pszWord[i] >= 0xFF )
            return false;
    }
    if ( i >= HW_MAXWORDLEN )
        return false;
    return true;
}
    
    
extern "C"
RECOGNIZER_PTR HWR_InitRecognizer( const char * inDictionaryMain, const char * inDictionaryCustom, const char * inLearner, const char * inAutoCorrect, int language, int * pFlags )
{
    if ( ! HWR_IsLanguageSupported( language ) )
    {
        return NULL;
    }
    CRecognizerWrapper * pRecognizer = new CRecognizerWrapper;
    if ( pRecognizer && (! pRecognizer->Initialize( inDictionaryMain, inDictionaryCustom, language )) )
    {
        delete pRecognizer;
        return NULL;
    }
    int flags = pRecognizer->IsDicts();
    if ( NULL != inLearner )
    {
        if ( pRecognizer->InitLearner( inLearner, true ) )
            flags |= FLAG_ANALYZER;
    }
    if ( NULL != inAutoCorrect )
    {
        if ( pRecognizer->InitWordList( inAutoCorrect, true ) )
            flags |= FLAG_CORRECTOR;
    }
    if ( pFlags != NULL )
        *pFlags = flags;
    return (RECOGNIZER_PTR)pRecognizer;
}

extern "C"
RECOGNIZER_PTR HWR_InitRecognizerFromMemory( const char * inDictionaryMain, const char * inDictionaryCustom, const char * inLearner, const char * inAutoCorrect, int language, int * pFlags )
{
    if ( ! HWR_IsLanguageSupported( language ) )
    {
        return NULL;
    }
    CRecognizerWrapper * pRecognizer = new CRecognizerWrapper;
    if ( pRecognizer && (! pRecognizer->Initialize( NULL, NULL, language )) )
    {
        delete pRecognizer;
        return NULL;
    }

    // load main and user dictionaries form memory
    if ( NULL != inDictionaryMain )
        pRecognizer->DictFromData( inDictionaryMain, -1, kDictionaryType_Main );
    if ( NULL != inDictionaryCustom )
        pRecognizer->DictFromData( inDictionaryCustom, -1, kDictionaryType_User );
    int flags = pRecognizer->IsDicts();
    // load learner
    if ( NULL != inLearner )
    {
        if ( pRecognizer->InitLearner( inLearner, false ) )
            flags |= FLAG_ANALYZER;
    }
    // load autocorrector
    if ( NULL != inAutoCorrect )
    {
        if ( pRecognizer->InitWordList( inAutoCorrect, false ) )
            flags |= FLAG_CORRECTOR;
    }
    if ( pFlags != NULL )
        *pFlags = flags;
    return (RECOGNIZER_PTR)pRecognizer;
}


extern "C"
BOOL HWR_LoadAlternativeDict( RECOGNIZER_PTR pRecognizer, const char * inDictionaryAlt )
{
    BOOL result = false;
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        result = gpRecognizer->LoadAltDict( inDictionaryAlt );
    }
    return result;
}

extern "C"
void HWR_FreeRecognizer( RECOGNIZER_PTR pRecognizer, const char * inDictionaryCustom, const char * inLearner, const char * inWordList )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        if ( NULL != inDictionaryCustom && 0 != *inDictionaryCustom )
            gpRecognizer->SaveUserDict( inDictionaryCustom );
        if ( NULL != inLearner && 0 != *inLearner )
            gpRecognizer->ReleaseLearner( inLearner );
        if ( NULL != inWordList && 0 != *inWordList )
            gpRecognizer->ReleaseWordList( inWordList );
        delete gpRecognizer;
        gpRecognizer = NULL;
    }	
}

extern "C"
BOOL HWR_RecognizerAddStroke( RECOGNIZER_PTR pRecognizer, CGStroke pStroke, int nStrokeCnt )
{
    if ( NULL == pRecognizer )
        return false;
    CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
    return gpRecognizer->RecognizeStroke(pStroke, nStrokeCnt );
}

extern "C"
BOOL HWR_Recognize( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL == pRecognizer )
        return false;
    CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
    if ( ! gpRecognizer->RecognizeStroke( NULL, 0 ) )
        return false;
    
#ifdef INTERNAL_RECO_THREAD
    struct timespec wait;
    wait.tv_nsec = 0;
    wait.tv_sec = DEFAULT_TIMEOUT;
    gpRecognizer->WaitForResult( NULL );
#endif // INTERNAL_RECO_THREAD

    return true;
}

extern "C"
const UCHR * HWR_GetResult( RECOGNIZER_PTR pRecognizer )
{
    const UCHR * pResult = NULL;

    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        pResult = gpRecognizer->GetResult();
    }
    return pResult;
}

extern "C"
const UCHR * HWR_GetResultWord( RECOGNIZER_PTR pRecognizer, int nWord, int nAlternative )
{
    const UCHR * pResult = NULL;
    
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        pResult = gpRecognizer->GetResWord( nWord, nAlternative );
    }
    return pResult;
}

extern "C"
USHORT HWR_GetResultWeight( RECOGNIZER_PTR pRecognizer, int nWord, int nAlternative )
{
    USHORT nResult = 0;
    
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nResult = gpRecognizer->GetResWeight( nWord, nAlternative );
    }
    return nResult;
}

extern "C"
int HWR_GetResultStrokesNumber( RECOGNIZER_PTR pRecognizer, int nWord, int nAlternative )
{
    int nResult = 0;
    
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nResult = gpRecognizer->GetResNStrokes( nWord, nAlternative );
    }
    return nResult;
}

extern "C"
int HWR_GetResultWordCount( RECOGNIZER_PTR pRecognizer )
{
    int nWordCnt = 0;
    
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nWordCnt = gpRecognizer->GetResColumnCount();
    }
    return nWordCnt;
}
    
extern "C"
int HWR_GetResultAlternativeCount( RECOGNIZER_PTR pRecognizer, int nWord )
{
    int nAltCnt = 0;
    
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nAltCnt = gpRecognizer->GetResRowCount( nWord );
    }
    return nAltCnt;
}

extern "C"
BOOL HWR_Reset( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
#ifdef INTERNAL_RECO_THREAD
        gpRecognizer->AsynchReset();
#else
		gpRecognizer->SynchReset();
#endif // INTERNAL_RECO_THREAD
        return true;
    }
    return false;
}

extern "C"
BOOL HWR_IsWordInDict( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord )
{
    if ( NULL != pRecognizer && _CheckWord( pszWord ) )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        UCHR	szWord[HW_MAXWORDLEN] = {0};
        _STRNCPY( szWord, pszWord, HW_MAXWORDLEN-1 );
        szWord[HW_MAXWORDLEN-1] = 0;
        DWORD recoFlags = gpRecognizer->GetFlags();
        int	flags = HW_SPELL_CHECK;
        if ( 0 != (recoFlags & FLAG_USERDICT) )
            flags |= HW_SPELL_USERDICT;
        if ( 0 != (recoFlags & FLAG_SPELLIGNORENUM) )
            flags |= HW_SPELL_IGNORENUM;
        if ( 0 != (recoFlags & FLAG_SPELLIGNOREUPPER) )
            flags |= HW_SPELL_IGNOREUPPER;
        if ( 0 != (recoFlags & FLAG_ALTDICT) )
            flags |= HW_SPELL_USEALTDICT;
        return gpRecognizer->TestWord( szWord, flags );
    }
    return false;
}

extern "C"
BOOL HWR_NewUserDict( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->NewUserDict();
    }
    return false;
}

extern "C"
BOOL HWR_SaveUserDict( RECOGNIZER_PTR pRecognizer, const char * inDictionaryCustom )
{
    if ( NULL != pRecognizer && NULL != inDictionaryCustom && 0 != *inDictionaryCustom )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->SaveUserDict( inDictionaryCustom );
    }	
    return false;
}

extern "C"
BOOL HWR_ResetUserDict( RECOGNIZER_PTR pRecognizer, const char * inDictionaryCustom )
{
    if ( NULL != pRecognizer && NULL != inDictionaryCustom && 0 != *inDictionaryCustom )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->ResetUserDictionary( inDictionaryCustom, true );
    }	
    return false;
}

extern "C"
BOOL HWR_ReloadUserDict( RECOGNIZER_PTR pRecognizer, const char * inDictionaryCustom )
{
    if ( NULL != pRecognizer && NULL != inDictionaryCustom && 0 != *inDictionaryCustom )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->ResetUserDictionary( inDictionaryCustom, false );
    }	
    return false;
}

extern "C"
BOOL HWR_ExportUserDictionary( RECOGNIZER_PTR pRecognizer, const char * inExportFile )
{
    if ( NULL != pRecognizer && NULL != inExportFile && 0 != *inExportFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->ExportUserDictionary( inExportFile );
    }	
    return false;
}

extern "C"
BOOL HWR_ImportUserDictionary( RECOGNIZER_PTR pRecognizer, const char * inImportFile )
{
    if ( NULL != pRecognizer && NULL != inImportFile && 0 != *inImportFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->ImportUserDictionary( inImportFile );
    }	
    return false;
}

extern "C"
BOOL HWR_ResetAutoCorrector( RECOGNIZER_PTR pRecognizer, const char * inWordListFile )
{
    if ( NULL != pRecognizer && NULL != inWordListFile && 0 != *inWordListFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->ReleaseWordList( NULL );
        return gpRecognizer->InitWordList( inWordListFile, true, true );
    }	
    return false;
}

extern "C"
BOOL HWR_ReloadAutoCorrector( RECOGNIZER_PTR pRecognizer, const char * inWordListFile )
{
    if ( NULL != pRecognizer && NULL != inWordListFile && 0 != *inWordListFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->ReleaseWordList( NULL );
        // load list word from the file
        return gpRecognizer->InitWordList( inWordListFile, true, false );
    }	
    return false;
}

extern "C"
BOOL HWR_AddUserWordToDict( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord, BOOL filter )
{
    if ( NULL != pRecognizer && _CheckWord( pszWord ) )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        UCHR	szWord[HW_MAXWORDLEN] = {0};
        UCHR	separators[] = { '.',',',';',';',':','\"','?','!','(',')','[',']', '\0' };
        int		k, i = 0;
        while ( _STRCHR( separators, pszWord[i] ) != NULL )
            i++;
        for ( k = 0; i < _STRLEN( pszWord ) && k < (HW_MAXWORDLEN-1); i++ )
        {
            if ( pszWord[i] <= ' ' || pszWord[i] >= 0xFF )
                return false;
            if ( filter && _STRCHR( separators, pszWord[i] ) != NULL )
                continue;
            szWord[k++] = pszWord[i];
        }
        szWord[k] = 0;
        int len = _STRLEN( szWord );
        if ( len < 1 )
            return false;
        return gpRecognizer->AddUserWord( szWord );
    }
    return false;
}


extern "C"
BOOL HWR_ResetLearner( RECOGNIZER_PTR pRecognizer, const char * inLearnerFile )
{
    if ( NULL != pRecognizer && NULL != inLearnerFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->ReleaseLearner( NULL );
        remove( inLearnerFile ); 
        return gpRecognizer->InitLearner( NULL, true );
    }
    return false;
}

extern "C"
BOOL HWR_ReloadLearner( RECOGNIZER_PTR pRecognizer, const char * inLearnerFile )
{
    if ( NULL != pRecognizer && NULL != inLearnerFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->ReleaseLearner( NULL );
        remove( inLearnerFile ); 
        return gpRecognizer->InitLearner( inLearnerFile, true );
    }
    return false;
}

extern "C"
int HWR_SpellCheckWord( RECOGNIZER_PTR pRecognizer, const UCHR *pszWord, UCHR *pszAnswer, int cbSize, int fl )
{
    if ( NULL != pRecognizer && _CheckWord( pszWord ) && NULL != pszAnswer && 0 < cbSize )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        UCHR	szWord[HW_MAXWORDLEN];
        memset( szWord, 0, sizeof( szWord ) );
        _STRNCPY( szWord, pszWord, HW_MAXWORDLEN-1 );
        DWORD recoFlags = gpRecognizer->GetFlags();
        int	flags = fl;
        if ( 0 != (recoFlags & FLAG_USERDICT) )
            flags |= HW_SPELL_USERDICT;
        if ( 0 != (recoFlags & FLAG_SPELLIGNORENUM) )
            flags |= HW_SPELL_IGNORENUM;
        if ( 0 != (recoFlags & FLAG_SPELLIGNOREUPPER) )
            flags |= HW_SPELL_IGNOREUPPER;
        if ( 0 != (recoFlags & FLAG_ALTDICT) )
            flags |= HW_SPELL_USEALTDICT;
        // flags |= fl;
        return gpRecognizer->SpellCheckWord( szWord, pszAnswer, cbSize, flags );
    }
    return (-1);
}

extern "C"
BOOL HWR_AnalyzeWordList( RECOGNIZER_PTR pRecognizer, const UCHR *pszWordList, UCHR *pszResult )
{
    if ( NULL != pRecognizer && NULL != pszWordList && NULL != pszResult )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        if ( gpRecognizer->AnalyzeResult( pszWordList, pszResult ) > 0 )
        {
            return true;
        }
    }
    return false;
}

extern "C"
int HWR_EnumUserWords( RECOGNIZER_PTR pRecognizer, PRECO_ONGOTWORD callback, void * pParam )
{
    int		nTotalWords = 0;
    if ( NULL != pRecognizer && NULL != callback )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nTotalWords = gpRecognizer->EnumUserWords( callback, pParam );
    }
    return nTotalWords;
}

extern "C"
int HWR_EnumWordList( RECOGNIZER_PTR pRecognizer, RECO_ONGOTWORDLIST callback, void * pParam )
{
    int		nTotalWords = 0;
    if ( NULL != pRecognizer && NULL != callback )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nTotalWords = gpRecognizer->EnumWordList( callback, pParam );
    }
    return nTotalWords;
}

extern "C"
BOOL HWR_SaveWordList( RECOGNIZER_PTR pRecognizer, const char * inWordListFile )
{
    if ( NULL != pRecognizer && NULL != inWordListFile && 0 != *inWordListFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->SaveWordList( inWordListFile );
    }	
    return false;
}

extern "C"
BOOL HWR_ExportWordList( RECOGNIZER_PTR pRecognizer, const char * inExportFile )
{
    if ( NULL != pRecognizer && NULL != inExportFile && 0 != *inExportFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->ExportWordList( inExportFile );
    }	
    return false;
}

extern "C"
BOOL HWR_ImportWordList( RECOGNIZER_PTR pRecognizer, const char * inImportFile )
{
    if ( NULL != pRecognizer && NULL != inImportFile && 0 != *inImportFile )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->ImportWordList( inImportFile );
    }	
    return false;
}

extern "C"
unsigned int HWR_GetRecognitionFlags( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return (unsigned int)gpRecognizer->GetFlags();
    }
    return FLAG_ERROR;	// error
}

extern "C"
unsigned int HWR_SetRecognitionFlags( RECOGNIZER_PTR pRecognizer, unsigned int newFlags )
{
    unsigned int nOldFlags = FLAG_ERROR;
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nOldFlags = (unsigned int)gpRecognizer->GetFlags();
        gpRecognizer->SetFlags(newFlags);
    }
    return nOldFlags;
}

extern "C"
int  HWR_GetRecognitionMode( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->GetMode();
    }
    return RECMODE_INVALID;
}

extern "C"
int  HWR_SetRecognitionMode( RECOGNIZER_PTR pRecognizer, int nNewMode )
{
    int nOldMode = RECMODE_INVALID;
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        nOldMode = gpRecognizer->GetMode();
        gpRecognizer->SetMode(nNewMode);
        return nOldMode;
    }
    return nOldMode;
}

extern "C"
BOOL HWR_LearnNewWord( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord, USHORT nWeight )
{
    if ( NULL != pRecognizer && _CheckWord( pszWord ) )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        USHORT w = HWR_IsWordInDict( pRecognizer, pszWord ) ? RW_DICTIONARYWORD : 0;
        w |= nWeight;		
        return gpRecognizer->LearnWord( pszWord, w );
    }
    return false;
}

extern "C"
BOOL HWR_SaveLearner( RECOGNIZER_PTR pRecognizer, const char * pszFileName )
{
    if ( NULL != pRecognizer && NULL != pszFileName && 0 != *pszFileName )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->SaveLearner(pszFileName);
    }
    return false;
}

extern "C"
const UCHR * HWR_AutocorrectWord( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord )
{
    if ( NULL != pRecognizer && _CheckWord( pszWord ) )
    {
        UCHR	  szReplacement[HW_MAXWORDLEN];
        BOOL	  bSuggestedOnly = false;
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        szReplacement[0] = 0;
        if ( gpRecognizer->GetReplaceWord( pszWord, szReplacement, &bSuggestedOnly ) && szReplacement[0] != 0 )
        {
			if ( NULL != gpRecognizer->m_lastWord )
				free( (void *)gpRecognizer->m_lastWord );
            gpRecognizer->m_lastWord = _STRDUP( szReplacement );
            return (const UCHR *)gpRecognizer->m_lastWord;
        }
    }
    return NULL;
}

extern "C"
BOOL HWR_AddWordToWordList( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord1, const UCHR * pszWord2, int nFlags, BOOL bReplace )
{
    if ( NULL != pRecognizer && _CheckWord( pszWord1 ) && _CheckWord( pszWord2 ) )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->AddWordToWordList( pszWord1, pszWord2, (DWORD)nFlags, bReplace );
    }
    return false;
}

extern "C"
BOOL HWR_EmptyWordList( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->EmptyWordList();
    }
    return false;
}

extern "C"
BOOL HWR_ReplaceWord( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord1, USHORT nWeight1, const UCHR * pszWord2, USHORT nWeight2 )
{
    if ( NULL != pRecognizer && _CheckWord( pszWord1 ) && _CheckWord( pszWord2 ) )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        USHORT w1 = (_STRLEN( pszWord1 ) < 2 || HWR_IsWordInDict( pRecognizer, pszWord1 )) ? RW_DICTIONARYWORD : 0;
        USHORT w2 = (_STRLEN( pszWord2 ) < 2 || HWR_IsWordInDict( pRecognizer, pszWord2 )) ? RW_DICTIONARYWORD : 0;
        w1 |= nWeight1;
        w2 |= nWeight2;		
        return gpRecognizer->ReplaceWord( pszWord1, w1, pszWord2, w2 );
    }
    return false;
}

extern "C"
BOOL HWR_EnablePhatCalc( RECOGNIZER_PTR pRecognizer, BOOL bEnable )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->EnablePhatCalc( bEnable );
        return true;
    }
    return false;
}

extern "C"
const UCHR * HWR_WordFlipCase( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord )
{
    static UCHR	szWord[HW_MAXWORDLEN] = {0};
    if ( NULL != pRecognizer && NULL != pszWord )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        _STRNCPY( szWord, pszWord, HW_MAXWORDLEN-1 );
        szWord[HW_MAXWORDLEN-1] = 0;
        if ( gpRecognizer->FlipCase( szWord ) )
            return (const UCHR *)(szWord);
    }
    return NULL;
}

extern "C"
const UCHR * HWR_WordEnsureLowerCase( RECOGNIZER_PTR pRecognizer, const UCHR * pszWord )
{
    static UCHR	szWord[HW_MAXWORDLEN] = {0};
    if ( NULL != pRecognizer && NULL != pszWord )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        _STRNCPY( szWord, pszWord, HW_MAXWORDLEN-1 );
        szWord[HW_MAXWORDLEN-1] = 0;
        if ( gpRecognizer->AllLower( szWord ) )
            return (const UCHR *)(szWord);
    }
    return NULL;
}

extern "C"
const UCHR * HWR_CalculateString( RECOGNIZER_PTR pRecognizer, const UCHR * pszString )
{
    if ( NULL != pRecognizer && NULL != pszString )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        size_t len = _STRLEN( pszString );
        if ( len > 3 )	// math string has to be 4 chars or more
        {
			if ( NULL != gpRecognizer->m_lastWord )
				free( (void *)gpRecognizer->m_lastWord );
            gpRecognizer->m_lastWord = (UCHR *)malloc( sizeof( UCHR ) * (len + HW_MAXWORDLEN) );
            if ( NULL != gpRecognizer->m_lastWord )
            {
                _STRCPY( gpRecognizer->m_lastWord, pszString );
                if ( ! gpRecognizer->Calculator( gpRecognizer->m_lastWord ) )
                {
                    free( (void *)gpRecognizer->m_lastWord );
                    gpRecognizer->m_lastWord = NULL;
                }
            }
		    return (const UCHR *)gpRecognizer->m_lastWord;
        }
    }
	return NULL;
}

extern "C"
void HWR_StopAsyncReco( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->StopAsyncReco();
    }
}

extern "C"
BOOL HWR_PreRecognizeInkData( RECOGNIZER_PTR pRecognizer, INK_DATA_PTR pInkData, int nDataLen, BOOL bFlipY  )
{
    if ( NULL == pRecognizer || NULL == pInkData )
    {
        return false;
    }
    CRecognizerWrapper *	gpRecognizer = (CRecognizerWrapper *)pRecognizer;
    CInkData *				gpData = (CInkData *)pInkData;
    return gpRecognizer->PreRecognizeInkData( gpData, nDataLen, bFlipY );
}

    
extern "C"
BOOL HWR_RecognizeSymbol( RECOGNIZER_PTR pRecognizer, INK_DATA_PTR pInkData, int base, int charsize )
{
    if ( NULL == pRecognizer || NULL == pInkData )
    {
        return false;
    }
    CRecognizerWrapper *	gpRecognizer = (CRecognizerWrapper *)pRecognizer;
    CInkData *				gpData = (CInkData *)pInkData;
    RECO_baseline_type      baseline;
    baseline.size = charsize;
    baseline.base = base;

    UInt32 memsize = 2;
    for ( int i = 0; i < gpData->StrokesTotal(); i++ )
    {
        memsize += (gpData->GetStrokePointCnt( i ) + 1);
    }
    CGStroke stroke = (CGStroke)malloc( memsize * sizeof( CGTracePoint ) );
    if ( stroke == NULL )
        return false;
  
    p_RECO_point_type points = (p_RECO_point_type)malloc( memsize * sizeof( RECO_point_type ) );
    if ( points == NULL )
        return false;
    
    
    UInt32 pos = 0;
    for ( int i = 0; i < gpData->StrokesTotal(); i++ )
    {
        UInt32 nStrokeLen = gpData->ReadOneStroke( i, stroke );
        for ( UInt32 j = 0; j < nStrokeLen; j++ )
        {
            points[pos].x = (short)(stroke[j].pt.x * 2.0);
            points[pos].y = (short)(stroke[j].pt.y * 2.0);
            pos++;
        }
        points[pos].x = 0;
        points[pos].y = TRACE_BREAK;
        pos++;
    }
    
    BOOL result = gpRecognizer->RecognizeSymbol( points, pos, &baseline );
    
    
    free( (void *)points );
    free( (void *)stroke );
    
    return result;
    
}
    
extern "C"
const UCHR * HWR_RecognizeInkData( RECOGNIZER_PTR pRecognizer, INK_DATA_PTR pInkData, int nFirstStroke, int nLastStroke, BOOL bAsync, BOOL bFlipY, BOOL bSort, BOOL bSelOnly  )
{
    if ( NULL == pRecognizer || NULL == pInkData )
    {
        return NULL;
    }
    CRecognizerWrapper *	gpRecognizer = (CRecognizerWrapper *)pRecognizer;
    CInkData *				gpData = (CInkData *)pInkData;
    
    const UCHR * result = gpRecognizer->RecognizeInkData( gpData, nFirstStroke, nLastStroke, bAsync, bFlipY, bSort, bSelOnly );
    /*
    if ( result == NULL )
        return (gpRecognizer->RunRecognizer()) ? "**Error**" : "**Cancelled**";
    */
    return result;
} 

extern "C"
BOOL HWR_SetDictionaryData( RECOGNIZER_PTR pRecognizer, const char *pData, int nDictType )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->DictFromData( pData, -1, nDictType );
    }
    return false;
}

extern "C"
BOOL HWR_SetAutocorrectorData( RECOGNIZER_PTR pRecognizer, const char *pData )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->ReleaseWordList( NULL );
        return gpRecognizer->InitWordList( pData, false );
    }
    return false;
}

extern "C"
int HWR_GetStrokeIDs( RECOGNIZER_PTR pRecognizer, int word, int altrnative, const int ** strokes )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->GetStrokeIDs( word, altrnative, strokes );
    }
    return -1;
}


extern "C"
int	HWR_GetAutocorrectorData( RECOGNIZER_PTR pRecognizer, char **ppData )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return (int)gpRecognizer->GetWordListData( ppData );
    }
    return 0;
}

extern "C"
BOOL HWR_SetLearnerData( RECOGNIZER_PTR pRecognizer, const char *pData )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->ReleaseWordList( NULL );
        return gpRecognizer->InitLearner( pData, false );
    }
    return false;
}

extern "C"
int	HWR_GetLearnerData( RECOGNIZER_PTR pRecognizer, char **ppData )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->GetLearnerData( ppData );
    }
    return 0;
}


extern "C"
int	HWR_GetDictionaryData( RECOGNIZER_PTR pRecognizer, char **ppData, int nDictType )
{
    int result = 0;
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        result = gpRecognizer->DictToData( ppData, nDictType );
    }
    return result;	
}

extern "C"
int HWR_GetDictionaryLength( RECOGNIZER_PTR pRecognizer, int nDictType )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->GetDictLength( nDictType );
    }
    return 0;
}

extern "C"
BOOL HWR_HasDictionaryChanged( RECOGNIZER_PTR pRecognizer, int nDictType )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->HasDictChanged( nDictType );
    }
    return false;
}

extern "C"
int HWR_GetLanguageID( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->GetLanguageID();
    }
    return 0;
}

extern "C"
void HWR_SetCustomCharset( RECOGNIZER_PTR pRecognizer, const UCHR * pCustomNum, const UCHR  * pCustPunct )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        gpRecognizer->SetCustomChars( pCustomNum, pCustPunct );
    }
}

extern "C"
BOOL HWR_SetDefaultShapes( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->RecSetDefaultShapes();
    }
    return false;
}

extern "C"
BOOL HWR_SetLetterShapes( RECOGNIZER_PTR pRecognizer, const unsigned char * pShapes )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->RecSetShapes( pShapes );
    }
    return false;
}

extern "C"
const unsigned char * HWR_GetLetterShapes( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        return gpRecognizer->RecGetShapes();
    }
    return NULL;
}

extern "C"
const char * HWR_GetLanguageName( RECOGNIZER_PTR pRecognizer )
{
    if ( NULL != pRecognizer )
    {
        CRecognizerWrapper * gpRecognizer = (CRecognizerWrapper *)pRecognizer;
        int lang = gpRecognizer->GetLanguageID();
        switch( lang )
        {
            case LANGUAGE_ENGLISHUK :
                return "English (UK)";
            case LANGUAGE_ENGLISH :
                return "English";
            case LANGUAGE_FRENCH :
                return "French";
            case LANGUAGE_GERMAN :
                return "German";
            case LANGUAGE_SPANISH :
                return "Spanish";
            case LANGUAGE_ITALIAN :
                return "Italian";
            case LANGUAGE_SWEDISH :
                return "Swedish";
            case LANGUAGE_NORWEGIAN :
                return "Norwegian";
            case LANGUAGE_DUTCH :
                return "Dutch";
            case LANGUAGE_DANISH :
                return "Danish";
            case LANGUAGE_PORTUGUESE :
                return "Portuguese";
            case LANGUAGE_PORTUGUESEB :
                return "Portuguese (BR)";
            case LANGUAGE_FINNISH :
                return "Finnish";
            case LANGUAGE_INDONESIAN :
                return "Indonesian";
        }
    }
    return "Unspecified";
}

extern "C" 
int HWR_GetSupportedLanguages( int ** languages )
{
    static int _languages[] = {
#ifdef LANG_ENGLISHUK
        LANGUAGE_ENGLISHUK,
#endif
#ifdef LANG_ENGLISH
        LANGUAGE_ENGLISH,
#endif
#ifdef LANG_FRENCH
        LANGUAGE_FRENCH,
#endif
#ifdef LANG_GERMAN
        LANGUAGE_GERMAN,
#endif
#ifdef LANG_NORW
        LANGUAGE_NORWEGIAN,
#endif
#ifdef LANG_DAN
        LANGUAGE_DANISH,
#endif
#ifdef LANG_SPANISH
        LANGUAGE_SPANISH,
#endif
#ifdef LANG_SWED
        LANGUAGE_SWEDISH,
#endif
#ifdef LANG_PORTUGUESE
        LANGUAGE_PORTUGUESE,
#endif
#ifdef LANG_PORTUGUESEB
        LANGUAGE_PORTUGUESEB,
#endif
#ifdef LANG_ITALIAN
        LANGUAGE_ITALIAN,
#endif
#ifdef LANG_FINNISH
        LANGUAGE_FINNISH,
#endif
#ifdef LANG_DUTCH
        LANGUAGE_DUTCH,
#endif
#ifdef LANG_INDONESIAN
        LANGUAGE_INDONESIAN,
#endif
    };
    *languages = _languages;
    return sizeof( _languages )/sizeof( _languages[0] ); 
}

extern "C"
BOOL HWR_IsLanguageSupported( int langID )
{
    int *   languages = NULL;
    int     lcount =  HWR_GetSupportedLanguages( &languages );
    for ( int i = 0; i < lcount; i++ )
    {
        if ( languages[i] == langID )
            return true;
    }
    return false;
}


extern "C"
const char * HWR_Description()
{
    return g_szDescription;
}

static RECO_ID_type idType = {0};

extern "C"
const char * HWR_ID()
{
    CRecognizerWrapper::RecGetID( &idType );
    return idType.id_string;
}

extern "C"
int HWR_Capabilities()
{
    CRecognizerWrapper::RecGetID( &idType );
    return idType.capabilities;
}

extern "C"
PHWR_VERSION HWR_Version()
{
    static HWR_VERSION  version = { 5, 6, 910, 0 };
    return &version;
}

RECO_DATA_EXTERNAL _INT dti_set_external_data( _INT lang, p_CHAR data );

extern "C"
BOOL HWR_SetExternalResource( int lang, const void * data )
{
    BOOL result = false;
    if ( dti_set_external_data( (_INT)lang, (p_CHAR)data ) == 1 )
        result = true;
    return result;
}


///
