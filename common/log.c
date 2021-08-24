/**
  ******************************************************************************
  * File Name          : log.c
  * Description        : This file implements an API for a simple logging
  *                      interface using the UART or RTT.
  */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define LOG_MODULE_NAME         log
#define LOG_LEVEL               LOG_LEVEL_INFO

#include "log.h"

#include "debug.h"

/**@brief   Set to 1 to use the SEGGER RTT output channel for log messages.
 *          Set to 0 to use ST-LINK UART.
 */
#define USE_RTT                 1

#include "cmsis_os.h"

#if USE_RTT
    #define LOG_BACKEND_RTT     1
#else
    #define LOG_BACKEND_UART    1
#endif

#if LOG_BACKEND_RTT
    #include "SEGGER_RTT.h"

    /**@brief   The RTT buffer ID used to write log messages to.
     */
    #define RTT_TERMINAL_ID     0
#endif

#if LOG_BACKEND_UART
    #include "usart.h"
#endif


/**@brief   Set to 1 to enable some basic testing of the log interface.
 *
 * The testing happens when the module is initialized.  The output needs to be
 * verified by a human since the app isn't reading it's own log output.
 */
#define UNIT_TEST               0

#ifndef BUILD_UT
/**@brief   Handle for the log task. */
osThreadId_t m_log_task_handle;

/**@brief   The attributes for the log task. */
const osThreadAttr_t m_log_task_attributes =
{
    .name = "log",
    .priority = (osPriority_t)osPriorityHigh,
    .stack_size = 512 * 4,
};

/**@brief   Handle for the log queue used to store messages. */
osMessageQueueId_t m_log_queue_handle;

/**@brief   The attributes for the log queue. */
const osMessageQueueAttr_t m_log_queue_attributes =
{
    .name = "log",
};
#endif

/**@brief   An array of names for the log levels.
 *
 * This must match the order of the values of log_level_t enumeration.
 *
 * The last entry is used as a catch all in case an out-of-range value is
 * passed.
 */
static const char *m_level_str[] =
{
    "Debug",
    "Info",
    "Warn",
    "Error",
    "Critical",

    "Test",

    "Unknown",
};

/**@brief   The maximum number of bytes that can be stored in a log message.
 *
 * Longer messages will be truncated.
 */
#define MAX_LOG_ENTRY           120

/**@brief   The maximum number of log entries that can be stored.
 */
#define LOG_QUEUE_EVENTS        48

/**@brief   The largest area the module will do a complete hex dump on, in
 *          bytes.
 */
#define MAX_HEX_BYTES           64

/**@brief   If the hex dump area is larger than MAX_HEX_BYTES, this is the
 *          number of bytes that will be printed from the start of the area.
 */
#define START_HEX_BYTES         32

/**@brief   If the hex dump area is larger than MAX_HEX_BYTES, this is the
 *          number of bytes that will be printed from the end of the area.
 */
#define END_HEX_BYTES           32

/**@brief   A structure that will hold a single log entry.
 */
typedef struct
{
    log_level_t level;              /**< The log level of the message. */
    const char *module;             /**< The module that created the message. */
    bool raw;                       /**< True if a raw message, false otherwise. */
    char buffer[MAX_LOG_ENTRY];     /**< Buffer containing the message. */
} log_entry_t;

/**@brief   The maximum number of log entries that can be stored.
 */
#define LOG_QUEUE_ITEM_SIZE     sizeof(log_entry_t)

/**@brief   Set to true when the module has successfully initialized.
 */
static bool m_initialized = false;


/**@brief   Internal function used to write a string to the output device.
 *
 * @param   str[in]     A pointer to the buffer of characters to write.
 * @param   len[in]     The number of characters to write.
 */
static void _output_str(const char *str, uint8_t len)
{
#if LOG_BACKEND_RTT
    (void)len; // unused

    SEGGER_RTT_WriteString(RTT_TERMINAL_ID, str);
#endif
#if LOG_BACKEND_UART
    MX_USART2_Write((unsigned char *)str, len);
#endif
#if LOG_BACKEND_STDIO
    printf("%s", str);
#endif
}


static void _process_log_entry(log_entry_t * p_msg)
{
    const char * spacer = ": ";

    if (!p_msg->raw)
    {
        // normal log format: LEVEL: MODULE: buffer

        // LEVEL
        if (p_msg->level >= LOG_LEVEL_End)
        {
            p_msg->level = LOG_LEVEL_End;
        }

        _output_str(
            m_level_str[p_msg->level],
            strlen(m_level_str[p_msg->level])
        );
        _output_str(spacer, strlen(spacer));

        // MODULE
        _output_str(p_msg->module, strlen(p_msg->module));
        _output_str(spacer, strlen(spacer));
    }

    // buffer
    _output_str(p_msg->buffer, strlen(p_msg->buffer));
}


static void _process_log(bool emergency)
{
    bool done = false;
    osStatus_t ret;

    while (!done)
    {
        log_entry_t msg;

        if (emergency)
        {
            ret = osMessageQueueGet(m_log_queue_handle, &msg, NULL, 0);
        }
        else
        {
            ret = osMessageQueueGet(m_log_queue_handle, &msg, NULL, osWaitForever);
        }
        switch (ret)
        {
        case osOK:
            _process_log_entry(&msg);
            if (0 == osMessageQueueGetCount(m_log_queue_handle))
            {
                done = true;
            }
            break;

        case osErrorParameter:
            debug_pulse(DEBUG_PIN_4, 0, 1);
            done = true;
            break;
        case osErrorResource:
            debug_pulse(DEBUG_PIN_4, 0, 2);
            done = true;
            break;
        case osErrorTimeout:
            debug_pulse(DEBUG_PIN_4, 0, 3);
            done = true;
            break;
        default:
            debug_pulse(DEBUG_PIN_4, 0, 4);
            done = true;
            break;
        }
    }
}


/**@brief   Function to test if an OS call was successful.
 *
 * If it wasn't, it directly dumps a message out to the RTT or UART.
 * This is intended to only be used internally in this module.
 * All other modules should use the logging module to report errors.
 *
 * @param   ret[in]             The OS return code to test.
 * @param   description[in]     A short description of the OS call.
 *
 * @return  The function returns true if the OS call was successful, false
 *          otherwise.
 */
static bool log_os_success(osStatus_t ret, const char *description)
{
    if (osOK != ret)
    {
        char buffer[MAX_LOG_ENTRY];
        sprintf(buffer, "!! Log Error, message dump:\n");
        _output_str(buffer, strlen(buffer));
        _process_log(true);
        sprintf(buffer, "!! Log Error: %s returned %d\n", description, ret);
        _output_str(buffer, strlen(buffer));
        return false;
    }
    return true;
}

void log_dump(void)
{
    _process_log(true);
}

/**@brief   FreeRTOS task that empties the queue to the output device.
 */
static void log_task(void * argumnet)
{
    for (;;)
    {
        _process_log(false);
    }
}


/**@brief   Run the module unit tests is UNIT_TEST is non-zero.
 *
 *  This unit test is different in that we don't use the LOG_TEST macro as
 *  we're testing the log macros themselves.
 */
static void _unit_test(void)
{
#if UNIT_TEST
    // This message should not log since the module log level is set to
    // LOG_LEVEL_INFO
    LOG_DEBUG("!!! You shouldn't see this message !!!\n");

    // These messages should appear in the log
    LOG_INFO("Info\n");
    LOG_WARNING("Warning\n");
    LOG_ERROR("Error\n");
    LOG_CRITICAL("Critical\n");

    LOG_INFO("%d items %s\n", 7, "loaded");
    LOG_WARNING(
        "Truncated message: %s",
        "0123456789 "
        "0123456789 "
        "0123456789 "
        "0123456789 "
        "0123456789 "
        "0123456789 "
        "0123456789 "
        "0123456789 "
        "0123456789 "
    );

    char test[80];

    for (int i = 0; i < 80; i++)
    {
        test[i] = i;
    }

    LOG_HEX_INFO("Test data", test, 20, 16);
    LOG_RAW_HEX_INFO(NULL, test, 20, 8);
    LOG_RAW_HEX_INFO("Too large of an area", test, 80, 16);
#endif
}


void log_task_init(void)
{
#if LOG_BACKEND_RTT
    SEGGER_RTT_Init();
#endif

    m_log_task_handle = osThreadNew(log_task, NULL, &m_log_task_attributes);
    if (NULL == m_log_task_handle)
    {
        const char *msg = "ERROR - creating log task\n";
        _output_str(msg, strlen(msg));
        return;
    }

    m_log_queue_handle = osMessageQueueNew(LOG_QUEUE_EVENTS,
                                           LOG_QUEUE_ITEM_SIZE,
                                           &m_log_queue_attributes);
    if (NULL == m_log_queue_handle)
    {
        const char *msg = "ERROR - creating log queue\n";
        _output_str(msg, strlen(msg));
        return;
    }

    m_initialized = true;
    LOG_INFO("Initialized\n");

    _unit_test();
}


void log_entry(
    log_level_t level,
    const char *module,
    bool raw,
    const char *file,
    int line,
    const char *fmt,
    ...
)
{
    if (!m_initialized)
    {
        char *msg = "!! Log module not initialized\n";
        _output_str(msg, strlen(msg));
        return;
    }

    log_entry_t msg =
    {
        .level = level,
        .module = module,
        .raw = raw,
    };
    va_list ap;

    // NOTE: file and line are currently unused, but would be easy to enable
    //       that functionality if desired.
    (void)file; // unused
    (void)line; // unused

    va_start(ap, fmt);
    int written = vsnprintf(msg.buffer, sizeof(msg.buffer), fmt, ap);
    if (written > sizeof(msg.buffer))
    {
        // We truncated the output, add a \n before the null terminator
        msg.buffer[MAX_LOG_ENTRY - 2] = '\n';
    }
    va_end(ap);

    osStatus_t ret = osMessageQueuePut(m_log_queue_handle, &msg, 0U, 0U);
    log_os_success(ret, msg.buffer);
}


/**@brief   Internal function used to add a hex dump of a memory area to the log queue.
 *
 * This function just dumps raw hex bytes into the queue.  Dumping a large area
 * will overload the queue.
 *
 * @param[in] level     The log level of the entry.
 * @param[in] module    The value of LOG_MODULE_NAME of the entry.
 * @param[in] file      The file containing the entry.
 * @param[in] line      The line of the file that the entry starts at.
 * @param[in] p_data    Pointer to data to print.
 * @param[in] length    Amount of data to print in bytes.
 * @param[in] stride    Number of bytes to print before a newline.
 */
static void _internal_dump_hex(
    log_level_t level,
    const char *module,
    bool raw,
    const char *file,
    int line,
    const void * const p_data,
    size_t length,
    uint16_t stride
)
{
    int i;
    bool newline = true;
    uint8_t *p = (uint8_t *) p_data;
    char buffer[MAX_LOG_ENTRY];
    char *output = buffer;

    if (p_data)
    {
        for (i = 0; i < length; i++)
        {
            output += sprintf(output, "%02x ", p[i]);
            newline = false;

            if (((i + 1) % 4) == 0)
            {
                output += sprintf(output, " ");
            }

            if (((i + 1) % stride) == 0)
            {
                log_entry(level, module, raw, file, line, "%s\n", buffer);
                newline = true;
                output = buffer;
            }
        }

        if (!newline)
        {
            log_entry(level, module, raw, file, line, "%s\n", buffer);
        }
    }
}


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
)
{
    if (NULL == p_data)
    {
        LOG_ERROR("NULL buffer\n");
        return;
    }

    if (p_name)
    {
        log_entry(level, module, raw, file, line,
            "%s (%d byte%s)\n",
            p_name,
            length,
            length != 1 ? "s" : ""
        );
    }
    else
    {
        log_entry(level, module, raw, file, line,
            "(%d byte%s)\n",
            length,
            length != 1 ? "s" : ""
        );
    }

    if (length > MAX_HEX_BYTES)
    {
        // print the first START_HEX_BYTES and the last END_HEX_BYTES and
        // ignore the middle otherwise we may run out of memory dumping larger
        // buffers.
        _internal_dump_hex(level, module, raw, file, line,
            p_data, START_HEX_BYTES, stride
        );

        log_entry(level, module, raw, file, line, "...\n");

        _internal_dump_hex(level, module, raw, file, line,
            p_data + length - END_HEX_BYTES, END_HEX_BYTES, stride
        );
    }
    else
    {
        _internal_dump_hex(level, module, raw, file, line, p_data, length, stride);
    }
}


/* vim: set tabstop=8 expandtab shiftwidth=4 softtabstop=4 : */
