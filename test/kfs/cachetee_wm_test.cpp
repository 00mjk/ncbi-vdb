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

/**
* Unit tests for cacheteefile
*/

#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#include <ktst/unit_test.hpp>

#include <klib/out.h>
#include <klib/rc.h>

#include <kproc/thread.h>

#include <kfs/defs.h>
#include <kfs/directory.h>
#include <kfs/file.h>
#include <kfs/cacheteefile_wm.h>
#include <kfs/recorder.h>

using namespace std;

#define DATAFILE "org.dat"
#define DATAFILESIZE ( ( 1024 * 1024 ) + 300 )
#define BLOCKSIZE ( 1024 * 16 )

TEST_SUITE( CacheTeeWMTests );

// this is not what we are testing...

/* prototypes of helper functions found in 'cache_tee_helper.cpp' */
rc_t create_random_file( const char * filename, uint64_t file_size );
rc_t remove_file( const char * filename );
rc_t compare_file_content( const KFile * file1, const KFile * file2, uint64_t pos, size_t bytes );
rc_t read_partial( const KFile * src, size_t block_size, uint64_t to_read );
rc_t read_all( const KFile * src, size_t block_size );

// we create a data-file that we want to wrap ....
static rc_t prepare_cachetee_tests( void )
{
    rc_t rc = create_random_file( DATAFILE, DATAFILESIZE );
    return rc;
}

// we remove everything that was created ...
static void finish_cachetee_tests( void )
{
    remove_file( DATAFILE );
}

//////////////////////////////////////////// Test-cases

TEST_CASE( CacheTeeWM_Basic )
{
    KOutMsg( "Test: CacheTeeWM_Basic\n" );
    
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    REQUIRE_RC( KDirectoryMakeCacheTeeWM ( dir, &tee, org, 0, "." ) );

    REQUIRE_RC( KFileRelease( tee ) );
    REQUIRE_RC( KFileRelease( org ) );
    
    //const KFile * cache;
    //REQUIRE( 0 != KDirectoryOpenFileRead( dir, &cache, "%s.wm_cache", DATAFILE ) );
    
    REQUIRE_RC( KDirectoryRelease( dir ) );
}                                 

TEST_CASE( CacheTeeWM_Read )
{
    KOutMsg( "Test: CacheTeeWM_Read\n" );
    
    KDirectory * dir;
    REQUIRE_RC( KDirectoryNativeDir( &dir ) );

    const KFile * org;
    REQUIRE_RC( KDirectoryOpenFileRead( dir, &org, "%s", DATAFILE ) );
    
    const KFile * tee;
    REQUIRE_RC( KDirectoryMakeCacheTeeWM ( dir, &tee, org, 0, "." ) );

    REQUIRE_RC( compare_file_content( org, tee, 0, 100 ) );         // small read at pos zero
    REQUIRE_RC( compare_file_content( org, tee, 10, 100 ) );        // small read at pos 10

    REQUIRE_RC( compare_file_content( org, tee, 0, 100 ) );         // do it again, now from the cache
    REQUIRE_RC( compare_file_content( org, tee, 10, 100 ) );

    REQUIRE_RC( compare_file_content( org, tee, 1024 * BLOCKSIZE, 100 ) );    // small read at block boundary
    REQUIRE_RC( compare_file_content( org, tee, 1024 * BLOCKSIZE, 100 ) );    // do it again, now from cache

    REQUIRE_RC( compare_file_content( org, tee, BLOCKSIZE / 2, BLOCKSIZE ) ); // span 2 blocks
    REQUIRE_RC( compare_file_content( org, tee, BLOCKSIZE / 2, BLOCKSIZE ) ); // do it again, now from cache

    REQUIRE_RC( compare_file_content( org, tee, BLOCKSIZE / 2, BLOCKSIZE * 5 + 100 ) ); // span 5 blocks
    REQUIRE_RC( compare_file_content( org, tee, BLOCKSIZE / 2, BLOCKSIZE * 5 + 100 ) ); // do it again, now from cache

    REQUIRE_RC( compare_file_content( org, tee, 100, 1024 * BLOCKSIZE + 500 ) );  // large read crossing block boundary

    REQUIRE_RC( compare_file_content( org, tee, 200, 1024 * BLOCKSIZE * 3 ) );    // very large read at pos 200
    REQUIRE_RC( compare_file_content( org, tee, 1024 * BLOCKSIZE * 2 + 10, 100 ) );   // small read after block boundary
    REQUIRE_RC( compare_file_content( org, tee, 1024 * BLOCKSIZE * 2 - 10, 100 ) );   // small read crossing block boundary    

    REQUIRE_RC( compare_file_content( org, tee, DATAFILESIZE - 100, 300 ) );    // small read crossing EOF
    REQUIRE_RC( compare_file_content( org, tee, DATAFILESIZE - 100, BLOCKSIZE * 10 ) );    // big read crossing EOF
    REQUIRE_RC( compare_file_content( org, tee, DATAFILESIZE - 10000, 10000 ) );    // big read right at EOF
    
    REQUIRE_RC( KFileRelease( tee ) );    
    REQUIRE_RC( KFileRelease( org ) );
    REQUIRE_RC( KDirectoryRelease( dir ) );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}

rc_t CC UsageSummary ( const char * progname )
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "cachetee-test";

rc_t CC KMain ( int argc, char *argv [] )
{
    srand( time( NULL ) );
    KConfigDisableUserSettings();
    rc_t rc = prepare_cachetee_tests();
    if ( rc == 0 )
    {
        rc = CacheTeeWMTests( argc, argv );
        finish_cachetee_tests();
    }
    KOutMsg( "and the result is: %R\n", rc );
    return rc;
}

}