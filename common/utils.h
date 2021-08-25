/**
 * @file    utils.h
 *
 * Defines some globally useful functions
 */
#ifndef __X_UTILS_H
#define __X_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Check if a module has been initialized before allowing it to run
 *
 * @param[in]   ret     The value to return if the module isn't initialized.
 */
#if defined(DEBUG)
#define MODULE_INITIALIZED(ret)                                             \
    do                                                                      \
    {                                                                       \
        if (!m_initialized)                                                 \
        {                                                                   \
            LOG_ERROR("%s: module not initialized\n", __func__);            \
            return ret;                                                     \
        }                                                                   \
    } while (0)
#else
#define MODULE_INITIALIZED(ret)
#endif


/**
 * @brief   Log the name of a function if VERBOSE has been defined in that module.
 */
#if defined(DEBUG) && defined(VERBOSE)
#define VERBOSE_ID()                                                        \
    do                                                                      \
    {                                                                       \
        LOG_DEBUG("%s", __func__);                                     	    \
    } while (0)
#else
#define VERBOSE_ID()
#endif

/**
 * @brief   Macro used to test a return code from a function
 *
 * An error will be logged if the actual doesn't match the expected.
 *
 * @param[in]   expected    The return code to expect if everything is OK.
 * @param[in]   actual      The actual code returned from a function.
 */
#define VALIDATE(expected, actual)                                          \
    do                                                                      \
    {                                                                       \
        if ((expected) != (actual))                                         \
        {                                                                   \
            LOG_ERROR("%s@%d: %d\n", __func__, __LINE__, (actual));         \
        }                                                                   \
    } while(0)

/**
 * @brief   Macro used to test a return code from a function
 *
 * An error will be logged if the actual doesn't match the expected.  The macro
 * will then exit the calling function with the given return code.
 *
 * @param[in]   expected    The return code to expect if everything is OK.
 * @param[in]   actual      The actual code returned from a function.
 * @param[in]   ret         The value to return from this function when the
 *                          actual return code doesn't match the expected
 *                          value.
 *
 * @note    If used in a function that returns void, don't pass a 'ret'
 *          parameter:
 *
 *              VALIDATE_EXIT(0, 1, );
 */
#define VALIDATE_EXIT(expected, actual, ret)                                \
    do                                                                      \
    {                                                                       \
        if ((expected) != (actual))                                         \
        {                                                                   \
            LOG_ERROR("%s@%d: %d\n", __func__, __LINE__, (actual));         \
            return ret;                                                     \
        }                                                                   \
    } while(0)

/**
 * @brief   Macro used to test a return code from a STM32 function that uses an
 *          handle.
 *
 * STM32 APIs that have a handle style interface usually have a State and
 * ErrorCode members in the structure that the handle points to.  This can be
 * used to provide more information in the case of an error.
 *
 * If the handle doesn't provide those members, you'll get a compile error
 * trying to use the macro.
 *
 * An error will be logged if the actual doesn't match the expected.  The macro
 * will then exit the calling function with the given return code.
 *
 * @param[in]   expected    The return code to expect if everything is OK.
 * @param[in]   actual      The actual code returned from a function.
 * @param[in]   handle      The handle for the API.
 * @param[in]   ret         The value to return from this function when the
 *                          actual return code doesn't match the expected
 *                          value.
 *
 * @note    If used in a function that returns void, don't pass a 'ret'
 *          parameter:
 *
 *              VALIDATE_EXIT(0, 1, );
 */
#define VALIDATE_HANDLE(expected, actual, handle, ret)                      \
    do                                                                      \
    {                                                                       \
        if ((expected) != (actual))                                         \
        {                                                                   \
            LOG_ERROR("%s@%d: %d, state: 0x%02x (%d), errorcode: 0x%02x (%d)\n",    \
                __func__, __LINE__, (actual),                               \
                (handle)->State, (handle)->State,                           \
                (handle)->ErrorCode, (handle)->ErrorCode);                  \
            return ret;                                                     \
        }                                                                   \
    } while(0)

#define VALIDATE_HANDLE_NOEXIT(expected, actual, handle)                    \
    do                                                                      \
    {                                                                       \
        if ((expected) != (actual))                                         \
        {                                                                   \
            LOG_ERROR("%s@%d: %d, state: 0x%02x (%d), errorcode: 0x%02x (%d)\n",    \
                __func__, __LINE__, (actual),                               \
                (handle)->State, (handle)->State,                           \
                (handle)->ErrorCode, (handle)->ErrorCode);                  \
        }                                                                   \
    } while(0)

/**
 * @brief   Tests if a return code was the expected value.
 *
 * If the values don't match, an error message is logged.  It also sets the
 * boolean variable passed as success to either true or false so that the
 * calling function can react to a failure.
 *
 * @param[in]   expected    The return code to expect if everything is OK.
 * @param[in]   actual      The actual code returned from a function.
 * @param[out]  success     A boolean value that is set to true if the actual
 *                          value matched the expected value, false otherwise.
 */
#define VALIDATE_SUCCESS(expected, actual, success)                         \
    do                                                                      \
    {                                                                       \
        (success) = true;                                                   \
        if ((expected) != (actual))                                         \
        {                                                                   \
            LOG_ERROR("%s@%d: %d\n", __func__, __LINE__, (actual));         \
            (success) = false;                                              \
        }                                                                   \
    } while (0)

#ifdef __cplusplus
}
#endif

#ifndef   MIN
  #define MIN(a, b)         (((a) < (b)) ? (a) : (b))
#endif

#ifndef   MAX
  #define MAX(a, b)         (((a) > (b)) ? (a) : (b))
#endif

#endif  // __X_UTILS_H
