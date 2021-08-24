/**
  ******************************************************************************
  * File Name          : log.h
  * Description        : This file provides an API for a simple logging
  *                      interface using the UART.
  *
  * This module can be used by a source file by:
  *
  *  - Set the identifier LOG_MODULE_NAME to a value used to ID the module.
  *
  *  - Set the identifier LOG_LEVEL to a value that matches the log_level_t
  *    enumeration of the messages to log.  That is, if you want to see only
  *    LOG_LEVEL_INFO messages or higher, set the value to 1.
  *
  *  - include log.h after setting the identifiers.
  */

#ifndef __X_LOG_H
#define __X_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

// Only try to define LOG_MODULE_NAME and LOG_LEVEL if the file including this
// one has not disabled them.
#ifndef LOG_MACROS_ONLY

#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME     app
#endif  // LOG_MODULE_NAME

#ifndef LOG_LEVEL
#define LOG_LEVEL           LOG_LEVEL_WARNING
#endif  // LOG_LEVEL

#endif  // LOG_MACROS_ONLY

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    LOG_LEVEL_Start = 0,

    LOG_LEVEL_DEBUG = LOG_LEVEL_Start,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_CRITICAL,

    LOG_LEVEL_TEST,             /**< Test is the highest value so that test
                                  *  code will always output to the log if it's
                                  *  enabled.
                                  */

    LOG_LEVEL_End,
} log_level_t;


#define __STRINGIFY__(value)    #value

/**@brief   Converts a macro into a character constant.
 */
#define STRINGIFY(value)        __STRINGIFY__(value)

/**@brief   Log a message if the module has LOG_LEVEL set to LOG_LEVEL_DEBUG or
 *          lower.
 */
#define LOG_DEBUG(...)                                                          \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_DEBUG, STRINGIFY(LOG_MODULE_NAME), false, __VA_ARGS__)

/**@brief   Log a message if the module has LOG_LEVEL set to LOG_LEVEL_INFO or
 *          lower.
 */
#define LOG_INFO(...)                                                           \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_INFO, STRINGIFY(LOG_MODULE_NAME), false, __VA_ARGS__)

/**@brief   Log a message if the module has LOG_LEVEL set to LOG_LEVEL_WARNING
 *          or lower.
 */
#define LOG_WARNING(...)                                                        \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_WARNING, STRINGIFY(LOG_MODULE_NAME), false, __VA_ARGS__)

/**@brief   Log a message if the module has LOG_LEVEL set to LOG_LEVEL_ERROR or
 *          lower.
 */
#define LOG_ERROR(...)                                                          \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_ERROR, STRINGIFY(LOG_MODULE_NAME), false, __VA_ARGS__)

/**@brief   Log a message if the module has LOG_LEVEL set to LOG_LEVEL_CRITICAL or
 *          lower.
 */
#define LOG_CRITICAL(...)                                                       \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_CRITICAL, STRINGIFY(LOG_MODULE_NAME), false, __VA_ARGS__)

/**@brief   Log a message if the module has LOG_LEVEL set to LOG_LEVEL_TEST or
 *          lower.
 *
 *          We don't implement LOG_RAW_TEST macros as we always want test
 *          output to be obvious in the log.
 */
#define LOG_TEST(...)                                                           \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_TEST, STRINGIFY(LOG_MODULE_NAME), false, __VA_ARGS__)

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_DEBUG or lower.
 */
#define LOG_RAW_DEBUG(...)                                                      \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_DEBUG, STRINGIFY(LOG_MODULE_NAME), true, __VA_ARGS__)

/**@brief   Log a raw message if the module has LOG_LEVEL set to LOG_LEVEL_INFO
 *          or lower.
 */
#define LOG_RAW_INFO(...)                                                       \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_INFO, STRINGIFY(LOG_MODULE_NAME), true, __VA_ARGS__)

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_WARNING or lower.
 */
#define LOG_RAW_WARNING(...)                                                    \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_WARNING, STRINGIFY(LOG_MODULE_NAME), true, __VA_ARGS__)

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_ERROR or lower.
 */
#define LOG_RAW_ERROR(...)                                                      \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_ERROR, STRINGIFY(LOG_MODULE_NAME), true, __VA_ARGS__)

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_CRITICAL or lower.
 */
#define LOG_RAW_CRITICAL(...)                                                   \
    LOG_INTERNAL(LOG_LEVEL, LOG_LEVEL_CRITICAL, STRINGIFY(LOG_MODULE_NAME), true, __VA_ARGS__)

// Internal macro to test if logging should occur
#define LOG_INTERNAL(MODULE_LEVEL, TRIGGER_LEVEL, MODULE_NAME, RAW, ...)        \
    do {                                                                        \
        if ((MODULE_LEVEL) <= (TRIGGER_LEVEL))                                  \
        {                                                                       \
            log_entry((TRIGGER_LEVEL), (MODULE_NAME), (RAW), __FILE__, __LINE__, __VA_ARGS__); \
        }                                                                       \
    } while (0)

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_DEBUG or lower.
 */
#define LOG_HEX_DEBUG(DESCRIPT, DATA, LENGTH, STRIDE)                           \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_DEBUG, STRINGIFY(LOG_MODULE_NAME), false, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to LOG_LEVEL_INFO
 *          or lower.
 */
#define LOG_HEX_INFO(DESCRIPT, DATA, LENGTH, STRIDE)                            \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_INFO, STRINGIFY(LOG_MODULE_NAME), false, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_WARNING or lower.
 */
#define LOG_HEX_WARNING(DESCRIPT, DATA, LENGTH, STRIDE)                         \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_WARNING, STRINGIFY(LOG_MODULE_NAME), false, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_ERROR or lower.
 */
#define LOG_HEX_ERROR(DESCRIPT, DATA, LENGTH, STRIDE)                           \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_ERROR, STRINGIFY(LOG_MODULE_NAME), false, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_CRITICAL or lower.
 */
#define LOG_HEX_CRITICAL(DESCRIPT, DATA, LENGTH, STRIDE)                        \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_CRITICAL, STRINGIFY(LOG_MODULE_NAME), false, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_TEST or lower.
 *
 *          We don't implement LOG_HEX_RAW_TEST macros as we always want test
 *          output to be obvious in the log.
 */
#define LOG_HEX_TEST(DESCRIPT, DATA, LENGTH, STRIDE)                            \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_TEST, STRINGIFY(LOG_MODULE_NAME), false, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_DEBUG or lower.
 */
#define LOG_RAW_HEX_DEBUG(DESCRIPT, DATA, LENGTH, STRIDE)                       \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_DEBUG, STRINGIFY(LOG_MODULE_NAME), true, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to LOG_LEVEL_INFO
 *          or lower.
 */
#define LOG_RAW_HEX_INFO(DESCRIPT, DATA, LENGTH, STRIDE)                        \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_INFO, STRINGIFY(LOG_MODULE_NAME), true, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_WARNING or lower.
 */
#define LOG_RAW_HEX_WARNING(DESCRIPT, DATA, LENGTH, STRIDE)                     \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_WARNING, STRINGIFY(LOG_MODULE_NAME), true, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_ERROR or lower.
 */
#define LOG_RAW_HEX_ERROR(DESCRIPT, DATA, LENGTH, STRIDE)                       \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_ERROR, STRINGIFY(LOG_MODULE_NAME), true, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

/**@brief   Log a raw message if the module has LOG_LEVEL set to
 *          LOG_LEVEL_CRITICAL or lower.
 */
#define LOG_RAW_HEX_CRITICAL(DESCRIPT, DATA, LENGTH, STRIDE)                    \
    LOG_HEX_INTERNAL(LOG_LEVEL, LOG_LEVEL_CRITICAL, STRINGIFY(LOG_MODULE_NAME), true, (DESCRIPT), (DATA), (LENGTH), (STRIDE))

// Internal macro to test if logging should occur
#define LOG_HEX_INTERNAL(MODULE_LEVEL, TRIGGER_LEVEL, MODULE_NAME, RAW, DESCRIPT, DATA, LENGTH, STRIDE) \
    do {                                                                        \
        if ((MODULE_LEVEL) <= (TRIGGER_LEVEL))                                  \
        {                                                                       \
            log_hex_entry(                                                      \
                (TRIGGER_LEVEL),                                                \
                (MODULE_NAME),                                                  \
                (RAW),                                                          \
                __FILE__,                                                       \
                __LINE__,                                                       \
                (DESCRIPT),                                                     \
                (DATA),                                                         \
                (LENGTH),                                                       \
                (STRIDE)                                                        \
            );                                                                  \
        }                                                                       \
    } while (0)

/**@brief   Initialize the log task and data structures.
 */
void log_task_init(void);

/**@brief   Add an entry to the log queue.
 *
 * NOTE: Users of the module should use the macro's and not call this function directly.
 *
 * @param[in] level     The log level of the entry.
 * @param[in] module    The value of LOG_MODULE_NAME of the entry.
 * @param[in] file      The file containing the entry.
 * @param[in] line      The line of the file that the entry starts at.
 * @param[in] p_name    Description of data being printed, may be NULL.
 * @param[in] fmt       A printf format string to format the remaining arguments.
 * @param[in] ...       A variable list of arguments to use to format the
 *                      output string.
 */
void log_entry(
    log_level_t level,
    const char *module,
    bool raw,
    const char *file,
    int line,
    const char *fmt,
    ...
);

/**@brief   Add a hex dump of a memory area to the log queue.
 *
 * NOTE: Users of the module should use the macro's and not call this function directly.
 *
 * This function will limit the number of bytes output to MAX_HEX_BYTES.  If
 * the requested area is larger than MAX_HEX_BYTES, the first START_HEX_BYTES
 * and the last END_HEX_BYTES will be output.  This is intended to prevent the
 * log queue from filling up from a large hex dump.
 *
 * @param[in] level     The log level of the entry.
 * @param[in] module    The value of LOG_MODULE_NAME of the entry.
 * @param[in] file      The file containing the entry.
 * @param[in] line      The line of the file that the entry starts at.
 * @param[in] p_name    Description of data being printed, may be NULL.
 * @param[in] p_data    Pointer to data to print.
 * @param[in] length    Amount of data to print in bytes.
 * @param[in] stride    Number of bytes to print before a newline.
 */
void log_hex_entry(
    log_level_t level,
    const char *module,
    bool raw,
    const char *file,
    int line,
    const char * p_name,
    const void * const p_data,
    size_t length,
    uint16_t stride
);

/**
 * @brief   Function to dump the current contents of the log to the ouput.
 *
 * This can be used in an assert or error handler to dump the log contents.
 */
void log_dump(void);

#ifdef __cplusplus
}
#endif

#endif /* __X_LOG_H */

/* vim: set tabstop=8 expandtab shiftwidth=4 softtabstop=4 : */
