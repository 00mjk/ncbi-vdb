#ifndef _h_diagnose_diagnose_
#define _h_diagnose_diagnose_

/*===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 */

#ifndef _h_diagnose_extern_
#include <diagnose/extern.h>
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h> /* rc_t */
#endif

#include <stdarg.h> /* va_list */


#ifdef __cplusplus
extern "C" {
#endif


struct KConfig;
struct KNSManager;
struct VFSManager;


typedef struct KDiagnose KDiagnose;
typedef struct KDiagnoseError KDiagnoseError;
typedef struct KDiagnoseTest KDiagnoseTest;


DIAGNOSE_EXTERN rc_t CC KDiagnoseMakeExt ( KDiagnose ** test,
    struct KConfig * kfg, struct KNSManager * kmg, struct VFSManager * vmgr );
DIAGNOSE_EXTERN rc_t CC KDiagnoseAddRef ( const KDiagnose * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseRelease ( const KDiagnose * self );


#define KVERBOSITY_NONE  -3
#define KVERBOSITY_ERROR -2
#define KVERBOSITY_INFO  -1
#define KVERBOSITY_MAX    0
DIAGNOSE_EXTERN rc_t CC KDiagnoseSetVerbosity ( KDiagnose * self,
    int verbosity );

DIAGNOSE_EXTERN rc_t CC KDiagnoseLogHandlerSet ( KDiagnose * self,
        rc_t ( CC * logger ) ( int verbosity,
                            unsigned type, /* TBD */
                            const char * fmt, va_list args )
    );

DIAGNOSE_EXTERN rc_t CC KDiagnoseLogHandlerSetKOutMsg ( KDiagnose * self );


/*
 * TestHandlerSet: set a test callback
 * the callback is called when each test is started or finished
 *
 * state - state of the test
 * test - test being executed.
 *      test remains valid when KDiagnose is valid;
 *      KDiagnoseTestAddRef if you want to keep it after KDiagnose is released
 */
typedef enum {
    eKDTS_NotStarted,/* test not started yet */
    eKDTS_Started,   /* test started */
    eKDTS_Succeed,   /* test finished successfully */
    eKDTS_Failed,    /* test finished with failure */
    eKDTS_Skipped,   /* test execution was skipped */
    eKDTS_Warning,   /* test finished successfully but has a warning for user */
    eKDTS_Paused,    /* KDiagnosePause was called */
    eKDTS_Resumed,   /* KDiagnoseResume was called */
    eKDTS_Canceled,  /* KDiagnoseCancel was called */
} EKDiagTestState;
/* test is NULL when state is one of:
 *                                    eKDTS_Paused
 *                                    eKDTS_Resumed
 *                                    eKDTS_Canceled
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestHandlerSet ( KDiagnose * self,
        void ( CC * callback )
             ( EKDiagTestState state, const KDiagnoseTest * test )
    );


#define DIAGNOSE_ALL     0
#define DIAGNOSE_FAIL    1
#define DIAGNOSE_CONFIG  2
#define DIAGNOSE_NETWORK 4
/* Run
 *
 * tests is combination of DIAGNOSE_* values
 *
 * When "tests | DIAGNOSE_FAIL != 0" - KDiagnoseRun will return non-0 rc
 * When KDiagnoseCancel is called - KDiagnoseRun will return rcCanceled
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseRun ( KDiagnose * self, uint64_t tests );

DIAGNOSE_EXTERN rc_t CC KDiagnosePause  ( KDiagnose * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseResume ( KDiagnose * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseCancel ( KDiagnose * self );


DIAGNOSE_EXTERN rc_t CC KDiagnoseGetErrorCount ( const KDiagnose * self,
    uint32_t * count );
DIAGNOSE_EXTERN rc_t CC KDiagnoseGetError ( const KDiagnose * self,
    uint32_t idx, const KDiagnoseError ** error );


DIAGNOSE_EXTERN rc_t CC KDiagnoseErrorAddRef ( const KDiagnoseError * self );
DIAGNOSE_EXTERN rc_t CC KDiagnoseErrorRelease ( const KDiagnoseError * self );

/* GetMsg:
 * Get Error Message.
 * Returned string remains valid while "self" is valid
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseErrorGetMsg ( const KDiagnoseError * self,
                                               const char ** message );


DIAGNOSE_EXTERN rc_t CC KDiagnoseGetTests ( const KDiagnose * self,
                                            const KDiagnoseTest ** test );

/* Level
 *
 * Get test level in tests hierarchy.
 * 0 is the highest level
 * tests of 'level 1' are run from test of 'level 0' etc
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestLevel ( const KDiagnoseTest * self,
                                             uint32_t * level );

/* Number
 *
 * Get hiererchical number of test inside of est level in tests hierarchy.
 * E.g., 0.2.2
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestNumber ( const KDiagnoseTest * self,
                                              const char ** number );

/* Name
 *
 * Get test name.
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestName ( const KDiagnoseTest * self,
                                            const char ** name );

/* Message
 *
 * Get test message (is set when test is finished)
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestMessage ( const KDiagnoseTest * self,
                                               const char ** message );

/* Message
 *
 * Get test state ( changes during test execution )
 */
DIAGNOSE_EXTERN rc_t CC KDiagnoseTestState ( const KDiagnoseTest * self,
                                             EKDiagTestState * state );

DIAGNOSE_EXTERN rc_t CC KDiagnoseTestNext ( const KDiagnoseTest * self,
                                            const KDiagnoseTest ** test );

DIAGNOSE_EXTERN rc_t CC KDiagnoseTestChild ( const KDiagnoseTest * self, 
                               uint32_t idx, const KDiagnoseTest ** test );


#ifdef __cplusplus
}
#endif

/******************************************************************************/

#endif /* _h_diagnose_diagnose_ */
