/**
  ******************************************************************************
  * File Name          : debug.h
  * Description        : This file implements an API for setting/clearing debug
  *                      output pins.
  */

#ifndef __X_DEBUG_H
#define __X_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#if NDEBUG
    #ifndef ENABLE_DEBUG
        #define ENABLE_DEBUG        0
    #endif // ENABLE_DEBUG
#else   // NDEBUG
    #ifndef ENABLE_DEBUG
        #define ENABLE_DEBUG        1
    #endif // ENABLE_DEBUG
#endif  // NDEBUG

/**@brief   An enumeration of the configured debug pins.
 *
 * @note    The enumeration contains all of the possible debug pins, but not
 *          all pins are defined for all hardware platforms.
 */
typedef enum
{
    DEBUG_PIN_Start = 0,

    DEBUG_PIN_1 = DEBUG_PIN_Start,
    DEBUG_PIN_2,
    DEBUG_PIN_3,
    DEBUG_PIN_4,

    DEBUG_PIN_End,
} debug_pin_t;


/**@brief   Initialize the debug module.
 */
void debug_init(void);

/**@brief   Set a debug pin high.
 *
 * @param[in]   pin     A member of the debug_pin_t enumeration that indicates
 *                      the pin to set.
 *
 *                      If the value isn't valid or the module isn't
 *                      initialized, an error message will be logged.
 */
void debug_set(debug_pin_t pin);

/**@brief   Set a debug pin low.
 *
 * @param[in]   pin     A member of the debug_pin_t enumeration that indicates
 *                      the pin to clear.
 *
 *                      If the value isn't valid or the module isn't
 *                      initialized, an error message will be logged.
 */
void debug_clear(debug_pin_t pin);

/**@brief   Toggle a debug pin.
 *
 * @param[in]   pin     A member of the debug_pin_t enumeration that indicates
 *                      the pin to toggle.
 *
 *                      If the value isn't valid or the module isn't
 *                      initialized, an error message will be logged.
 */
void debug_toggle(debug_pin_t pin);

/**@brief   Create a pulse on the the debug pin of the given width.
 *
 * @param[in]   pin     A member of the debug_pin_t enumeration that indicates
 *                      the pin to create the pulse on.
 * @param[in]   width   The width of the pulse.  This is the number of time a
 *                      simple for loop will increment a counter in a busy
 *                      wait.
 * @param[in]   reps    The number of pulses to create.  The width is also used
 *                      as the spacing between pulses.
 *
 * @Note    The pulse will be either positive or negative based on the initial
 *          state of the debug pin.
 */
void debug_pulse(debug_pin_t pin, uint8_t width, uint8_t reps);

#ifdef __cplusplus
}
#endif

#endif /* __X_DEBUG_H */

/* vim: set tabstop=8 expandtab shiftwidth=4 softtabstop=4 : */
