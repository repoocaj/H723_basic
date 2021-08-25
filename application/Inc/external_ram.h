#ifndef __X_EXTERNAL_RAM_H
#define __X_EXTERNAL_RAM_H

#include <stdint.h>

#include "main.h"
#include "app.h"

#include "stm32h7xx_hal_ospi.h"

/**
 * @brief S27KS0643 memory parameters. The range of addresses from
 * S27KS0643_START_ADDRESS to S27KS0643_END_ADDRESS can be directly accessed
 * once OctoSPI-related initialization has completed, and external_ram_init()
 * has been called.
 */
#define S27KS0643_SIZE              0x800000
#define S27KS0643_WORD_SIZE         4
#define S27KS0643_START_ADDRESS     OCTOSPI1_BASE
#define S27KS0643_END_ADDRESS       S27KS0643_START_ADDRESS + S27KS0643_SIZE - 1

/**
  * @brief Initializes external RAM.
  *
  * @retval None
  */
void external_ram_init(OSPI_HandleTypeDef * hospi);

/**
  * @brief Tests if the external RAM is connected and configured correctly. Not
  * intended to check the integrity of the RAM part.
  * @retval RET_SUCCESS           if test passed.
  *         RET_ERROR_INTERNAL    if test failed.
  */
ret_t external_ram_test_connection(void);

/**
  * @brief Write to external RAM. This is a blocking call.
  * @param address: Starting RAM address.
  * @param data: Pointer to data to write.
  * @param size: Number of bytes.
  * @retval Instance of ret_t.
  */
ret_t external_ram_write(uint32_t address, void * data, __size_t size);

/**
  * @brief Read from external RAM. This is a blocking call.
  * @param address: Starting RAM address.
  * @param data: Pointer to read destination.
  * @param size: Number of bytes.
  * @retval Instance of ret_t.
  */
ret_t external_ram_read(uint32_t address, void * data, __size_t size);

#endif // __X_EXTERNAL_RAM_H
