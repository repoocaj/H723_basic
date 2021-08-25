#ifndef _APP_H_
#define _APP_H_

/**
 * @brief   An enumeration of the application return code.
 */
typedef enum
{
    RET_SUCCESS = 0,

    RET_ERROR_INTERNAL,
    RET_ERROR_INVALID_PARAM,
    RET_ERROR_FORBIDDEN,

    MAX_RET,
} ret_t;

#endif
