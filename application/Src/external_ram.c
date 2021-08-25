#include <stdbool.h>
#include <assert.h>

#include "utils.h"
#include "external_ram.h"
#include "debug.h"

#define LOG_MODULE_NAME         external_ram
#define LOG_LEVEL               LOG_LEVEL_DEBUG

#include "log.h"

#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

/**
 * @brief   Set to 1 to enable memory testing of the external RAM.
 *
 * Set to 0 for production as it will overwrite the contents of RAM.
 */
#define UNIT_TEST               1

/**
 * @brief S27KS0643 PSRAM command codes.
 * @{
 */
#define INSTRUCTION_READ                0xEE
#define INSTRUCTION_WRITE               0xDE
#define INSTRUCTION_RESET               0x99
#define INSTRUCTION_RESET_ENABLE        0x66
#define INSTRUCTION_READ_ID             0x9F
/**
 * @}
 */

/**
 * @brief S27KS0643 PSRAM configuration.
 * @{
 */
#define DUMMY_CLOCK_CYCLES              7
/**
 * @}
 */

/**@brief   The attributes for the test task. */
const osThreadAttr_t m_test_task_attributes =
{
    .name = "ext_ram_test",
    .priority = osPriorityNormal,
    .stack_size = 256 * 4
};

OSPI_HandleTypeDef * ospiHandle;
OSPI_RegularCmdTypeDef sCommand;

/**
 * @brief Set to true when the module has successfully initialized.
 */
static bool m_initialized = false;

typedef struct
{
    __IO uint32_t offset;
    const char * name;
} stm32_reg_t;

static stm32_reg_t m_regs[] =
{
    {0x000,     "CR"},
    {0x008,     "DCR1"},
    {0x00C,     "DCR2"},
    {0x010,     "DCR3"},
    {0x014,     "DCR4"},
    {0x100,     "CCR"},
    {0x108,     "TCR"},
    {0x180,     "WCCR"},
    {0x188,     "WTCR"},

    {0x020,     "SR"},

    {0x040,     "DLR"},
    {0x048,     "AR"},
    {0x050,     "DR"},
    {0x110,     "IR"},
    {0x120,     "ABR"},
    {0x190,     "WIR"},
    {0x1A0,     "WABR"},

    {0x3F0,     "HWCFGR"},
    {0x3F4,     "VER"},
    {0x3F8,     "ID"},
    {0x3FC,     "MID"},

    {0x0,       NULL},
};

static void _dump_regs(const char *descript)
{
    uint32_t *p = (uint32_t *)ospiHandle->Instance;

    LOG_DEBUG("%s\n", descript);

    for (uint8_t i = 0; m_regs[i].name != NULL; i++)
    {
        uint32_t *addr = p + (m_regs[i].offset / 4);
        LOG_DEBUG(" % 6s: 0x%08x (0x%08x)\n", m_regs[i].name, *addr, addr);
    }
}

#if UNIT_TEST
/**
 * @brief   This task runs a continous memory test.
 *
 * @param argument: Not used
 *
 * @retval None
 */
static void _test_thread(void *argument)
{
    static uint8_t buffer[10];
    static uint8_t value = 2;
#if 0
    // TODO: Able to do writes of multiple sizes; generates error if doing a read
    LOG_INFO("Starting write 1...\n");
    external_ram_write(S27KS0643_START_ADDRESS, &value, 1);
    LOG_INFO("Starting write 2...\n");
    external_ram_write(S27KS0643_START_ADDRESS, &value, sizeof(buffer));
    LOG_INFO("Starting read 1...\n");
    external_ram_read(S27KS0643_START_ADDRESS, buffer, 1);
    LOG_INFO("All OSPI operations will now fail, after the failed read.\n");
    LOG_INFO("Starting write 3...\n");
    external_ram_write(S27KS0643_START_ADDRESS, &value, 1);

    // external_ram_test_connection();
#endif
    _dump_regs("Before write");
    debug_set(DEBUG_PIN_1);
    external_ram_write(S27KS0643_START_ADDRESS, &value, 1);
    debug_clear(DEBUG_PIN_1);
    _dump_regs("After write");

    /* Infinite loop */
    for(;;)
    {
        // Write tests
        debug_set(DEBUG_PIN_1);

        osDelay(500);

        // Read tests
        debug_clear(DEBUG_PIN_1);

        osDelay(500);
    }

    // In case we accidentally exit from task loop
    osThreadTerminate(NULL);
}
#endif

void external_ram_init(OSPI_HandleTypeDef * hospi)
{
    // Save configuratoin
    ospiHandle = hospi;

    //  (+) Instruction phase : the mode used and if present the size, the instruction
    //      opcode and the DTR mode.
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
    //  (+) Address phase : the mode used and if present the size, the address
    //      value and the DTR mode.
    sCommand.AddressMode = HAL_OSPI_ADDRESS_8_LINES;
    sCommand.AddressSize = HAL_OSPI_ADDRESS_32_BITS;
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_ENABLE;
    //  (+) Alternate-bytes phase : the mode used and if present the size, the
    //      alternate bytes values and the DTR mode.
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
    //  (+) Dummy-cycles phase : the number of dummy cycles (mode used is same as data phase).
    sCommand.DummyCycles = DUMMY_CLOCK_CYCLES;
    //  (+) Data phase : the mode used and if present the number of bytes and the DTR mode.
    sCommand.DataMode = HAL_OSPI_DATA_8_LINES;
    sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_ENABLE;
    //  (+) Data strobe (DQS) mode : the activation (or not) of this mode
    sCommand.DQSMode = HAL_OSPI_DQS_ENABLE;
    //  (+) Sending Instruction Only Once (SIOO) mode : the activation (or not) of this mode.
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;
    //  (+) Operation type : always common configuration
    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;

#if UNIT_TEST
    assert ( osThreadNew(_test_thread, NULL, &m_test_task_attributes) != NULL );
#endif

    // Reset the RAM chip.
    //
    // The datasheet lists 200 nS as the minimum reset pulse width and 200 nS
    // after the pulse has gone back high before starting the clock.  We're
    // waiting 1 mS as that's the minimum delay that the HAL provides.
    HAL_GPIO_WritePin(Ext_RAM_Reset_GPIO_Port, Ext_RAM_Reset_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(Ext_RAM_Reset_GPIO_Port, Ext_RAM_Reset_Pin, GPIO_PIN_SET);
    HAL_Delay(1);

    // Set initialized
    m_initialized = true;
}

ret_t external_ram_id(void)
{
    ret_t ret = RET_SUCCESS;
#if 0
    sCommand.Instruction = INSTRUCTION_READ_ID;

    HAL_StatusTypeDef status = HAL_OSPI_Command(ospiHandle,
                                                &sCommand,
                                                HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    VALIDATE_HANDLE(HAL_OK, status, ospiHandle, RET_ERROR_INTERNAL);

    status = HAL_OSPI_Transmit(ospiHandle, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    VALIDATE_HANDLE(HAL_OK, status, ospiHandle, RET_ERROR_INTERNAL);
#endif
    return ret;
}

ret_t external_ram_write(uint32_t address, void * data, __size_t size)
{
    LOG_INFO("%s\n", __func__);

    sCommand.Address = address;
    sCommand.NbData = size;
    sCommand.Instruction = INSTRUCTION_WRITE;

    ret_t ret = RET_SUCCESS;
    HAL_StatusTypeDef status = HAL_OSPI_Command(ospiHandle,
                                                &sCommand,
                                                HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    VALIDATE_HANDLE(HAL_OK, status, ospiHandle, RET_ERROR_INTERNAL);
    LOG_INFO("%s@%d: %d, state: 0x%02x (%d), errorcode: 0x%02x (%d)\n",
        __func__, __LINE__, status,
        ospiHandle->State, ospiHandle->State,
        ospiHandle->ErrorCode, ospiHandle->ErrorCode);

    status = HAL_OSPI_Transmit(ospiHandle, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    VALIDATE_HANDLE(HAL_OK, status, ospiHandle, RET_ERROR_INTERNAL);

    return ret;
}

ret_t external_ram_read(uint32_t address, void * data, __size_t size)
{
    sCommand.Address = address;
    sCommand.NbData = size;
    sCommand.Instruction = INSTRUCTION_READ;

    ret_t ret = RET_SUCCESS;
    HAL_StatusTypeDef status = HAL_OSPI_Command(ospiHandle,
                                                &sCommand,
                                                HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    VALIDATE_HANDLE(HAL_OK, status, ospiHandle, RET_ERROR_INTERNAL);

    status = HAL_OSPI_Receive(ospiHandle, data, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);
    VALIDATE_HANDLE(HAL_OK, status, ospiHandle, RET_ERROR_INTERNAL);

    return ret;
}

static bool external_ram_test_shift1(void)
{
    uint32_t walking_one = 1;

    for (uint32_t i = 0; i < 32; i++)
    {
        walking_one = walking_one << i;

        external_ram_write(S27KS0643_START_ADDRESS, &walking_one, sizeof(walking_one));
        uint32_t readback;
        external_ram_read(S27KS0643_START_ADDRESS, &readback, sizeof(readback));

        if (walking_one != readback)
        {
            return false;
            LOG_ERROR("Walking one RAM test failed at bit (%d).\n", i);
        }
    }

    return true;
}

/**
  * @brief Walking zeros test. Write 32 bit words to ram where a 0 bit is moved through
  * all 32 positions.
  * @retval true    if value read back matches value written
  *         false   if ram problem
  */
static bool external_ram_test_shift0(void)
{
    uint32_t walking_one = 1;

    for (uint32_t i = 0; i < 32; i++)
    {
        walking_one = walking_one << i;
        uint32_t walking_zero = ~walking_one;

        external_ram_write(S27KS0643_START_ADDRESS, &walking_zero, sizeof(walking_one));
        uint32_t readback;
        external_ram_read(S27KS0643_START_ADDRESS, &readback, sizeof(readback));

        if (walking_zero != readback)
        {
            return false;
            LOG_ERROR("Walking zero RAM test failed at bit (%d).\n", i);
        }
    }

    return true;
}

/**
  * @brief Unique value test. Each 32 bit address is written to with a unique value.
  * @retval true    if value read back matches value written
  *         false   if ram problem
  */
static bool external_ram_test_addresses(void)
{
    uint32_t words_to_write = (S27KS0643_END_ADDRESS - S27KS0643_START_ADDRESS)/S27KS0643_WORD_SIZE;
    uint32_t ram_address = S27KS0643_START_ADDRESS;

    for (uint32_t i = 0; i < words_to_write; i++)
    {
        external_ram_write(ram_address, &i, sizeof(i));
        uint32_t readback;
        external_ram_read(ram_address, &readback, sizeof(readback));

        if (i != readback)
        {
            return false;
            LOG_ERROR("Unique value RAM test failed at address (%d).\n", i);
        }

        ram_address += S27KS0643_WORD_SIZE;
    }

    LOG_INFO("end address: %u\n", ram_address); // 2424307708; 907FFFFC

    return true;
}

ret_t external_ram_test_connection(void)
{
    if (external_ram_test_shift0() &&
        external_ram_test_shift1() &&
        external_ram_test_addresses())
    {
        LOG_INFO("All RAM tests successful.\n");
        return RET_SUCCESS;
    }
    else
    {
        return RET_ERROR_INTERNAL;
    }
}
