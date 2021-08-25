/**
  ******************************************************************************
  * File Name          : debug.c
  * Description        : This file implements an API for setting/clearing debug
  *                      output pins.
  */

#include <stdbool.h>

#define LOG_MODULE_NAME         debug
#define LOG_LEVEL               LOG_LEVEL_DEBUG

#include "stm32h7xx_hal.h"

#include "log.h"
#include "debug.h"

#include "main.h"

/**@brief   Set to 1 to enable some basic testing of the debug interface.
 *
 * The testing happens when the module is initialized.  The output needs to be
 * verified by a human with a logical analyzer.
 */
#define UNIT_TEST               1

/**@brief   Structure used to hold the information about the GPIO pins and
 *          ports used for debugging.
 */
typedef struct
{
    bool available;
    GPIO_InitTypeDef init;
    GPIO_TypeDef *port;
} debug_gpio_t;


/**@brief   An array of the values used to initialize and write to a particular
 *          debug pin.
 */
static debug_gpio_t m_debug_pin[DEBUG_PIN_End] =
{
    [DEBUG_PIN_1] =
    {
        .available = true,
        .init =
        {
            .Pin = DBG1_Pin,
            .Mode = GPIO_MODE_OUTPUT_PP,
            .Pull = GPIO_PULLDOWN,
            .Speed = GPIO_SPEED_FREQ_LOW,
        },
        .port = DBG1_GPIO_Port,
    },
    [DEBUG_PIN_2] =
    {
        .available = true,
        .init =
        {
            .Pin = DBG2_Pin,
            .Mode = GPIO_MODE_OUTPUT_PP,
            .Pull = GPIO_PULLDOWN,
            .Speed = GPIO_SPEED_FREQ_LOW,
        },
        .port = DBG2_GPIO_Port,
    },
    [DEBUG_PIN_3] =
    {
        .available = true,
        .init =
        {
            .Pin = DBG3_Pin,
            .Mode = GPIO_MODE_OUTPUT_PP,
            .Pull = GPIO_PULLDOWN,
            .Speed = GPIO_SPEED_FREQ_LOW,
        },
        .port = DBG3_GPIO_Port,
    },
    [DEBUG_PIN_4] =
    {
        .available = true,
        .init =
        {
            .Pin = DBG4_Pin,
            .Mode = GPIO_MODE_OUTPUT_PP,
            .Pull = GPIO_PULLDOWN,
            .Speed = GPIO_SPEED_FREQ_LOW,
        },
        .port = DBG4_GPIO_Port,
    },
};


/**@brief   Set to true when the module has successfully initialized.
 */
static bool m_initialized = false;


#if ENABLE_DEBUG
/**@brief   Run the module unit tests is UNIT_TEST is non-zero.
 */
static void _unit_test(void)
{
#if UNIT_TEST
    int i;

    LOG_TEST("Each debug pin should produce two 2 ms pulses that are 1 ms apart\n");

    // First pulse is via debug_toggle()

    // Set high
    for (i = DEBUG_PIN_Start; i < DEBUG_PIN_End; i++)
    {
        debug_toggle(i);
    }

    // Delay
    HAL_Delay(2);

    // Set low
    for (i = DEBUG_PIN_Start; i < DEBUG_PIN_End; i++)
    {
        debug_toggle(i);
    }

    // Delay
    HAL_Delay(1);

    // Second pulse is via debug_set() followed by debug_clear()

    // Set high
    for (i = DEBUG_PIN_Start; i < DEBUG_PIN_End; i++)
    {
        debug_set(i);
    }

    // Delay
    HAL_Delay(2);

    // Set low
    for (i = DEBUG_PIN_Start; i < DEBUG_PIN_End; i++)
    {
        debug_clear(i);
    }
#endif
}
#endif

void debug_init(void)
{
#if ENABLE_DEBUG
    int i;

    // Initialize all of the outputs and set them low
    for (i = DEBUG_PIN_Start; i < DEBUG_PIN_End; i++)
    {
        if (m_debug_pin[i].available)
        {
            LOG_DEBUG("DEBUG_PIN_%d enabled: port 0x%08x, pin 0x%04x\n",
                i+1,
                m_debug_pin[i].port,
                m_debug_pin[i].init.Pin
            );
            HAL_GPIO_Init(m_debug_pin[i].port, &m_debug_pin[i].init);
            HAL_GPIO_WritePin(
                m_debug_pin[i].port,
                m_debug_pin[i].init.Pin,
                GPIO_PIN_RESET
            );
        }
        else
        {
            LOG_DEBUG("DEBUG_PIN_%d disabled\n", i+1);
        }
    }

    m_initialized = true;
    LOG_INFO("Initialized\n");

    _unit_test();
#else
    LOG_WARNING("Debug output disabled\n");
#endif
}


#if ENABLE_DEBUG
/**@brief   Test if the module has been initialized and if the given pin is a valid debug pin.
 *
 * @param[in]   pin     A member of the debug_pin_t enumeration that indicates
 *                      the pin to test.
 *
 * @return  True if both conditions are true, false otherwise.
 */
static bool _is_valid(debug_pin_t pin)
{
    bool valid = true;

    if (!m_initialized)
    {
        LOG_ERROR("Module not initialized\n");
        valid = false;
    }

    if (m_debug_pin[pin].available)
    {
    if ((pin < DEBUG_PIN_Start) || (pin >= DEBUG_PIN_End))
    {
        LOG_ERROR("Invalid debug pin %d, valid values are %d-%d\n",
            pin,
            DEBUG_PIN_Start,
            DEBUG_PIN_End - 1
        );
        valid = false;
    }
    }
    else
    {
        valid = false;
    }

    return valid;
}
#endif

void debug_set(debug_pin_t pin)
{
#if ENABLE_DEBUG
    if (_is_valid(pin))
    {
        HAL_GPIO_WritePin(
            m_debug_pin[pin].port,
            m_debug_pin[pin].init.Pin,
            GPIO_PIN_SET
        );
    }
#endif
}


void debug_clear(debug_pin_t pin)
{
#if ENABLE_DEBUG
    if (_is_valid(pin))
    {
        HAL_GPIO_WritePin(
            m_debug_pin[pin].port,
            m_debug_pin[pin].init.Pin,
            GPIO_PIN_RESET
        );
    }
#endif
}


void debug_toggle(debug_pin_t pin)
{
#if ENABLE_DEBUG
    if (_is_valid(pin))
    {
        HAL_GPIO_TogglePin(m_debug_pin[pin].port, m_debug_pin[pin].init.Pin);
    }
#endif
}


void _delay(int count)
{
    for (int i = 0; i < count; i++)
    {
    }
}

void debug_pulse(debug_pin_t pin, uint8_t width, uint8_t reps)
{
#if ENABLE_DEBUG
    if (_is_valid(pin))
    {
        while (reps)
        {
            HAL_GPIO_TogglePin(m_debug_pin[pin].port, m_debug_pin[pin].init.Pin);

            _delay(width);
            HAL_GPIO_TogglePin(m_debug_pin[pin].port, m_debug_pin[pin].init.Pin);

            reps--;
            if (reps)
            {
                _delay(width);
            }
        }
    }
#endif
}

/* vim: set tabstop=8 expandtab shiftwidth=4 softtabstop=4 : */
