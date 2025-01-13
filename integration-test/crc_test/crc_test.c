/*
 * Copyright (c) 2024-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

/* Standard includes. */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* POSIX includes. */
#include <unistd.h>

/* Include Test Config as the first non-system header. */
#include "test_config.h"

/* OS and Transport header. */
#include "os_posix.h"

/* RTIO API header. */
#include "core_rtio.h"



#include <stdio.h>  
#include <stdlib.h>  
#include <time.h>  
#include <string.h>  


void initRandomSeed()
{
    /* Initialize the random number generator with the current time as the seed */
    srand( (unsigned int)time( NULL ) );
}

char* generateRandomString( uint16_t length )
{
    int i = 0;
    char* str = (char*)malloc( length + 1 ); /* +1 for the null terminator */
    if( str == NULL )
    {
        /* Handle memory allocation failure */
        fprintf( stderr, "Memory allocation failed\n" );
        exit( EXIT_FAILURE );
    }

    /* Initialize the first character as '/' */
    str[ 0 ] = '/';

    /* Generate the rest of the random string */
    for( i = 1; i < length; ++i )
    {
        /* Generate a random number between 0 and 65 (inclusive) */
        int randomNum = rand() % 65;

        /* Map the random number to the allowed character set (letters, digits, and '/')  */
        if( randomNum < 10 )
        { /* Digits 0-9  */
            str[ i ] = '0' + randomNum;
        }
        else if( randomNum < 36 )
        { /* Lowercase letters a-z */
            str[ i ] = 'a' + ( randomNum - 10 );
        }
        else if( randomNum < 62 )
        { /* Uppercase letters A-Z */
            str[ i ] = 'A' + ( randomNum - 36 );
        }
        else
        { /* 63-65 Slash '/' */
            str[ i ] = '/';
        }
    }

    /* Null-terminate the string */
    str[ length ] = '\0';
    return str;
}


#define TEST_URI_NUM 2000
int main()
{
    int i = 0;
    int strLength = 0;
    char* randomStr = NULL;
    uint32_t digest  = 0;

    initRandomSeed();

    for( i = 0; i < TEST_URI_NUM; ++i )
    {
        strLength = 5 + rand() % 123; /* rand 5-128 */
        randomStr = generateRandomString( strLength );
        digest = crc32Ieee( randomStr, strLength );
        printf( "{\"uri\":\"%s\", \"digest\":\"%u\"}\n", randomStr, digest );
        free( randomStr );
    }

    return 0;
}