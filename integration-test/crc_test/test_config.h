/*
 * Copyright (c) 2023-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Include logging header files and define logging macros in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for TEST.
 * 3. Include the header file "logging_stack.h", if logging is enabled for TEST.
 */

#include "logging_levels.h"

/* Logging configuration for the test. */
#define LIBRARY_LOG_NAME    "CRC_TEST
#define LIBRARY_LOG_LEVEL    LOG_DEBUG
#include "logging_stack.h"

/******** End of logging configuration ************/


#endif /* ifndef TEST_CONFIG_H */
