/*****************************************************************************
 * arch/xtensa/src/esp32/rom/esp32_spiflash.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 *****************************************************************************/

#ifndef _ROM_SPI_FLASH_H_
#define _ROM_SPI_FLASH_H_

/*****************************************************************************
 * Included Files
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* spi_flash_apis, spi flash operation related apis */

/*****************************************************************************
 *                            Note
 *****************************************************************************
 * 1. ESP32 chip have 4 SPI slave/master, however, SPI0 is
 *    used as an SPI master to access Flash and ext-SRAM by
 *    Cache module. It will support Decryto read for Flash,
 *    read/write for ext-SRAM. And SPI1 is also used as an
 *    SPI master for Flash read/write and ext-SRAM read/write.
 *    It will support Encrypto write for Flash.
 * 2. As an SPI master, SPI support Highest clock to 80M,
 *    however, Flash with 80M Clock should be configured
 *    for different Flash chips. If you want to use 80M
 *    clock We should use the SPI that is certified by
 *    Espressif. However, the certification is not started
 *    at the time, so please use 40M clock at the moment.
 * 3. SPI Flash can use 2 lines or 4 lines mode. If you
 *    use 2 lines mode, you can save two pad SPIHD and
 *    SPIWP for gpio. ESP32 support configured SPI pad for
 *    Flash, the configuration is stored in efuse and flash.
 *    However, the configurations of pads should be certified
 *    by Espressif. If you use this function, please use 40M
 *    clock at the moment.
 * 4. ESP32 support to use Common SPI command to configure
 *    Flash to QIO mode, if you failed to configure with fix
 *    command. With Common SPI Command, ESP32 can also provide
 *    a way to use same Common SPI command groups on different
 *    Flash chips.
 * 5. This functions are not protected by packeting, Please use the
 *****************************************************************************/

/*****************************************************************************
 * Pre-processor Definitions
 *****************************************************************************/

#define PERIPHS_SPI_FLASH_CMD                 SPI_CMD_REG(1)
#define PERIPHS_SPI_FLASH_ADDR                SPI_ADDR_REG(1)
#define PERIPHS_SPI_FLASH_CTRL                SPI_CTRL_REG(1)
#define PERIPHS_SPI_FLASH_CTRL1               SPI_CTRL1_REG(1)
#define PERIPHS_SPI_FLASH_STATUS              SPI_RD_STATUS_REG(1)
#define PERIPHS_SPI_FLASH_USRREG              SPI_USER_REG(1)
#define PERIPHS_SPI_FLASH_USRREG1             SPI_USER1_REG(1)
#define PERIPHS_SPI_FLASH_USRREG2             SPI_USER2_REG(1)
#define PERIPHS_SPI_FLASH_C0                  SPI_W0_REG(1)
#define PERIPHS_SPI_FLASH_C1                  SPI_W1_REG(1)
#define PERIPHS_SPI_FLASH_C2                  SPI_W2_REG(1)
#define PERIPHS_SPI_FLASH_C3                  SPI_W3_REG(1)
#define PERIPHS_SPI_FLASH_C4                  SPI_W4_REG(1)
#define PERIPHS_SPI_FLASH_C5                  SPI_W5_REG(1)
#define PERIPHS_SPI_FLASH_C6                  SPI_W6_REG(1)
#define PERIPHS_SPI_FLASH_C7                  SPI_W7_REG(1)
#define PERIPHS_SPI_FLASH_TX_CRC              SPI_TX_CRC_REG(1)

#define SPI0_R_QIO_DUMMY_CYCLELEN             3
#define SPI0_R_QIO_ADDR_BITSLEN               31
#define SPI0_R_FAST_DUMMY_CYCLELEN            7
#define SPI0_R_DIO_DUMMY_CYCLELEN             1
#define SPI0_R_DIO_ADDR_BITSLEN               27
#define SPI0_R_FAST_ADDR_BITSLEN              23
#define SPI0_R_SIO_ADDR_BITSLEN               23

#define SPI1_R_QIO_DUMMY_CYCLELEN             3
#define SPI1_R_QIO_ADDR_BITSLEN               31
#define SPI1_R_FAST_DUMMY_CYCLELEN            7
#define SPI1_R_DIO_DUMMY_CYCLELEN             3
#define SPI1_R_DIO_ADDR_BITSLEN               31
#define SPI1_R_FAST_ADDR_BITSLEN              23
#define SPI1_R_SIO_ADDR_BITSLEN               23

#define ESP_ROM_SPIFLASH_W_SIO_ADDR_BITSLEN   23

#define ESP_ROM_SPIFLASH_TWO_BYTE_STATUS_EN   SPI_WRSR_2B

/* SPI address register */

#define ESP_ROM_SPIFLASH_BYTES_LEN            24
#define ESP_ROM_SPIFLASH_BUFF_BYTE_WRITE_NUM  32
#define ESP_ROM_SPIFLASH_BUFF_BYTE_READ_NUM   64
#define ESP_ROM_SPIFLASH_BUFF_BYTE_READ_BITS  0x3f

/* SPI status register */

#define ESP_ROM_SPIFLASH_BUSY_FLAG            BIT0
#define ESP_ROM_SPIFLASH_WRENABLE_FLAG        BIT1
#define ESP_ROM_SPIFLASH_BP0                  BIT2
#define ESP_ROM_SPIFLASH_BP1                  BIT3
#define ESP_ROM_SPIFLASH_BP2                  BIT4
#define ESP_ROM_SPIFLASH_WR_PROTECT           (ESP_ROM_SPIFLASH_BP0|\
                                               ESP_ROM_SPIFLASH_BP1|\
                                               ESP_ROM_SPIFLASH_BP2)
#define ESP_ROM_SPIFLASH_QE                   BIT9

/* Extra dummy for flash read */

#define ESP_ROM_SPIFLASH_DUMMY_LEN_PLUS_20M   0
#define ESP_ROM_SPIFLASH_DUMMY_LEN_PLUS_40M   1
#define ESP_ROM_SPIFLASH_DUMMY_LEN_PLUS_80M   2

#define FLASH_ID_GD25LQ32C  0xC86016

/*****************************************************************************
 * Public Types
 *****************************************************************************/

typedef enum
{
    ESP_ROM_SPIFLASH_QIO_MODE = 0,
    ESP_ROM_SPIFLASH_QOUT_MODE,
    ESP_ROM_SPIFLASH_DIO_MODE,
    ESP_ROM_SPIFLASH_DOUT_MODE,
    ESP_ROM_SPIFLASH_FASTRD_MODE,
    ESP_ROM_SPIFLASH_SLOWRD_MODE
} esp_rom_spiflash_read_mode_t;

typedef enum
{
    ESP_ROM_SPIFLASH_RESULT_OK,
    ESP_ROM_SPIFLASH_RESULT_ERR,
    ESP_ROM_SPIFLASH_RESULT_TIMEOUT
} esp_rom_spiflash_result_t;

typedef struct
{
    uint32_t device_id;
    uint32_t chip_size;    /* chip size in bytes */
    uint32_t block_size;
    uint32_t sector_size;
    uint32_t page_size;
    uint32_t status_mask;
} esp32_spiflash_chip_t;

typedef struct
{
    uint8_t  data_length;
    uint8_t  read_cmd0;
    uint8_t  read_cmd1;
    uint8_t  write_cmd;
    uint16_t data_mask;
    uint16_t data;
} esp_rom_spiflash_common_cmd_t;

/*****************************************************************************
 * Public Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 * Name: esp_rom_spiflash_fix_dummylen
 *
 * Description:
 *   Fix the bug in SPI hardware communication with Flash/Ext-SRAM in High
 *   Speed.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint8_t spi: 0 for SPI0(Cache Access), 1 for SPI1(Flash read/write).
 *
 *   uint8_t freqdiv: Pll is 80M, 4 for 20M, 3 for 26.7M, 2 for 40M,
 *                    1 for 80M.
 *
 * Returned Value:
 *   None
 *
 *****************************************************************************/

void esp_rom_spiflash_fix_dummylen(uint8_t spi, uint8_t freqdiv);

/*****************************************************************************
 * Name: esp_rom_spiflash_select_qiomode
 *
 * Description:
 *   Select SPI Flash to QIO mode when WP pad is read from Flash.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint8_t wp_gpio_num: WP gpio number.
 *
 *   uint32_t ishspi: 0 for spi, 1 for hspi, flash pad decided by strapping
 *                    else, bit[5:0] spiclk, bit[11:6] spiq, bit[17:12] spid,
 *                    bit[23:18] spics0, bit[29:24] spihd
 *
 * Returned Value:
 *   None
 *****************************************************************************/

void esp_rom_spiflash_select_qiomode(uint8_t wp_gpio_num,
                                     uint32_t ishspi);

/*****************************************************************************
 * Name: esp_rom_spiflash_set_drvs
 *
 * Description:
 *   Set SPI Flash pad drivers.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint8_t wp_gpio_num: WP gpio number.
 *
 *   uint32_t ishspi: 0 for spi, 1 for hspi, flash pad decided by strapping
 *                    else, bit[5:0] spiclk, bit[11:6] spiq, bit[17:12] spid,
 *                    bit[23:18] spics0, bit[29:24] spihd
 *
 *   uint8_t *drvs:   drvs[0]-bit[3:0] for cpiclk, bit[7:4] for spiq,
 *                    drvs[1]-bit[3:0] for spid, drvs[1]-bit[7:4] for spid
 *                    drvs[2]-bit[3:0] for spihd, drvs[2]-bit[7:4] for spiwp.
 *                    Values usually read from flash by rom code, function
 *                    usually callde by rom code.
 *                    if value with bit(3) set, the value is valid, bit[2:0]
 *                    is the real value.
 *
 * Returned Value:
 *   None
 *
 *****************************************************************************/

void esp_rom_spiflash_set_drvs(uint8_t wp_gpio_num,
                               uint32_t ishspi,
                               uint8_t *drvs);

/*****************************************************************************
 * Name: esp_rom_spiflash_select_padsfunc
 *
 * Description:
 *   Select SPI Flash function for pads.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t ishspi: 0 for spi, 1 for hspi, flash pad decided by strapping
 *                    else, bit[5:0] spiclk, bit[11:6] spiq, bit[17:12] spid,
 *                    bit[23:18] spics0, bit[29:24] spihd
 *
 * Returned Value:
 *   None
 *
 *****************************************************************************/

void esp_rom_spiflash_select_padsfunc(uint32_t ishspi);

/*****************************************************************************
 * Name: esp_rom_spiflash_attach
 *
 * Description:
 *   SPI Flash init, clock divisor is 4, use 1 line Slow read mode.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t ishspi: 0 for spi, 1 for hspi, flash pad decided by strapping
 *                    else, bit[5:0] spiclk, bit[11:6] spiq, bit[17:12] spid,
 *                    bit[23:18] spics0, bit[29:24] spihd
 *
 *   uint8_t legacy: In legacy mode, more SPI command is used in line.
 *
 * Returned Value:
 *   None
 *
 *****************************************************************************/

void esp_rom_spiflash_attach(uint32_t ishspi, bool legacy);

/*****************************************************************************
 * Name: esp_rom_spiflash_read_status
 *
 * Description:
 *   SPI Read Flash status register. We use CMD 0x05 (RDSR).
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   esp32_spiflash_chip_t *spi : The information for Flash, which is exported
 *                                from ld file.
 *
 *   uint32_t *status : The pointer to which to return the Flash status value.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : read OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : read error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : read timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_read_status(esp32_spiflash_chip_t *spi,
                             uint32_t *status);

/*****************************************************************************
 * Name: esp_rom_spiflash_read_statushigh
 *
 * Description:
 *   SPI Read Flash status register bits 8-15. We use CMD 0x35 (RDSR2).
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   esp32_spiflash_chip_t *spi : The information for Flash, which is exported
 *                                from ld file.
 *
 *   uint32_t *status : The pointer to which to return the Flash status value.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : read OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : read error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : read timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_read_statushigh(esp32_spiflash_chip_t *spi,
                                 uint32_t *status);

/*****************************************************************************
 * Name: esp32_spiflash_write_status
 *
 * Description:
 *   Write status to Flash status register.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   esp32_spiflash_chip_t *spi : The information for Flash, which is exported
 *                                from ld file.
 *
 *   uint32_t status_value : Value to .
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : write OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : write error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : write timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp32_spiflash_write_status(esp32_spiflash_chip_t *spi,
                            uint32_t status_value);

/*****************************************************************************
 * Name: esp_rom_spiflash_read_user_cmd
 *
 * Description:
 *   Use a command to Read Flash status register.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   esp32_spiflash_chip_t *spi : The information for Flash, which is exported
 *                                from ld file.
 *
 *   uint32_t*status : The pointer to which to return the Flash status value.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : read OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : read error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : read timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_read_user_cmd(uint32_t *status,
                               uint8_t cmd);

/*****************************************************************************
 * Name: esp_rom_spiflash_config_readmode
 *
 * Description:
 *   Config SPI Flash read mode when init.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameter:
 *   esp_rom_spiflash_read_mode_t mode : QIO/QOUT/DIO/DOUT/FastRD/SlowRD.
 *
 *   This function does not try to set the QIO Enable bit in the status
 *   register, caller is responsible for this.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : config OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : config error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : config timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_config_readmode(esp_rom_spiflash_read_mode_t mode);

/*****************************************************************************
 * Name: esp_rom_spiflash_config_clk
 *
 * Description:
 *   Config SPI Flash clock divisor.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint8_t freqdiv: clock divisor.
 *
 *   uint8_t spi: 0 for SPI0, 1 for SPI1.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : config OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : config error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : config timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_config_clk(uint8_t freqdiv,
                            uint8_t spi);

/*****************************************************************************
 * Name: esp_rom_spiflash_common_cmd
 *
 * Description:
 *   Send CommonCmd to Flash so that is can go into QIO mode, some Flash use
 *   different CMD.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameter:
 *   esp_rom_spiflash_common_cmd_t *cmd : A struct to show the action of a
 *                                        command.
 *
 * Returned Value:
 *   uint16_t  0 : do not send command any more.
 *             1 : go to the next command.
 *             n > 1 : skip (n - 1) commands.
 *
 *****************************************************************************/

uint16_t esp_rom_spiflash_common_cmd(esp_rom_spiflash_common_cmd_t *cmd);

/*****************************************************************************
 * Name: esp_rom_spiflash_unlock
 *
 * Description:
 *   Unlock SPI write protect.
 *
 *   Please do not call this function in SDK.
 *
 * Input Value:
 *   None.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Unlock OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Unlock error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Unlock timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t esp_rom_spiflash_unlock(void);

/*****************************************************************************
 * Name: esp_rom_spiflash_lock
 *
 * Description:
 *   SPI write protect.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameter:
 *   None.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Lock OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Lock error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Lock timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t esp_rom_spiflash_lock(void);

/*****************************************************************************
 * Name: esp_rom_spiflash_config_param
 *
 * Description:
 *   Update SPI Flash parameter.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t deviceId : Device ID read from SPI, the low 32 bit.
 *
 *   uint32_t chip_size : The Flash size.
 *
 *   uint32_t block_size : The Flash block size.
 *
 *   uint32_t sector_size : The Flash sector size.
 *
 *   uint32_t page_size : The Flash page size.
 *
 *   uint32_t status_mask : The Mask used when read status from Flash
 *                          (use single CMD).
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Update OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Update error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Update timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_config_param(uint32_t deviceid,
                              uint32_t chip_size,
                              uint32_t block_size,
                              uint32_t sector_size,
                              uint32_t page_size,
                              uint32_t status_mask);

/*****************************************************************************
 * Name: esp_rom_spiflash_erase_chip
 *
 * Description:
 *   Erase whole flash chip.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameter:
 *   None
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Erase OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Erase error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Erase timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t esp_rom_spiflash_erase_chip(void);

/*****************************************************************************
 * Name: esp_rom_spiflash_erase_block
 *
 * Description:
 *   Erase a 64KB block of flash
 *   Uses SPI flash command D8H.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameter:
 *   uint32_t block_num : Which block to erase.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Erase OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Erase error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Erase timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t esp_rom_spiflash_erase_block(uint32_t block_num);

/*****************************************************************************
 * Name: esp_rom_spiflash_erase_sector
 *
 * Description:
 *   Erase a sector of flash.
 *   Uses SPI flash command 20H.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t sector_num : Which sector to erase.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Erase OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Erase error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Erase timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t esp_rom_spiflash_erase_sector(uint32_t sector_num);

/*****************************************************************************
 * Name: esp_rom_spiflash_erase_area
 *
 * Description:
 *   Erase some sectors.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t start_addr : Start addr to erase, should be sector aligned.
 *
 *   uint32_t area_len : Length to erase, should be sector aligned.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Erase OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Erase error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Erase timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_erase_area(uint32_t start_addr,
                            uint32_t area_len);

/*****************************************************************************
 * Name: esp_rom_spiflash_write
 *
 * Description:
 *  Write Data to Flash, you should Erase it yourself if need.
 *
 *  Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t dest_addr : Address to write, should be 4 bytes aligned.
 *
 *   const uint32_t *src : The pointer to data which is to write.
 *
 *   uint32_t len : Length to write, should be 4 bytes aligned.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Write OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Write error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Write timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_write(uint32_t dest_addr,
                       const uint32_t *src,
                       int32_t len);

/*****************************************************************************
 * Name: esp_rom_spiflash_enable_write
 *
 * Description:
 *  Enable Flash write.
 *
 *  Please do not call this function in SDK.
 *
 * Input Parameters:
 *   esp32_spiflash_chip_t *spi : The information for Flash, which is exported
 *                                from ld file.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Write OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Write error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Write timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_enable_write(esp32_spiflash_chip_t *spi);

/*****************************************************************************
 * Name: esp_rom_spiflash_write_status
 *
 * Description:
 *  Write status to Flash status register.
 *
 *  Please do not call this function in SDK.
 *
 * Input Parameters:
 *   esp32_spiflash_chip_t *spi : The information for Flash, which is exported
 *                                from ld file.
 *
 *   uint32_t status_value : Value to write.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Write OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Write error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Write timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_write_status(esp32_spiflash_chip_t *spi,
                              uint32_t status_value);

/*****************************************************************************
 * Name: esp_rom_spiflash_read
 *
 * Description:
 *   Read Data from Flash, you should Erase it yourself if need.
 *
 *   Please do not call this function in SDK.
 *
 * Input Values:
 *   uint32_t src_addr : Address to read, should be 4 bytes aligned.
 *
 *   uint32_t *dest : The buf to read the data.
 *
 *   uint32_t len : Length to read, should be 4 bytes aligned.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Read OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Read error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Read timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_read(uint32_t src_addr,
                      uint32_t *dest,
                      int32_t len);

/*****************************************************************************
 * Name: esp_rom_spiflash_write_encrypted_enable
 *
 * Description:
 *   SPI1 go into encrypto mode.
 *
 *   Please do not call this function in SDK.
 *
 *****************************************************************************/

void esp_rom_spiflash_write_encrypted_enable(void);

/*****************************************************************************
 * Name: esp_rom_spiflash_prepare_encrypted_data
 *
 * Description:
 *   Prepare 32 Bytes data to encrpto writing, you should Erase it yourself
 *   if need.
 *
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t flash_addr : Address to write, should be 32 bytes aligned.
 *
 *   uint32_t *data : The pointer to data which is to write.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Prepare OK.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Prepare error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Prepare timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_prepare_encrypted_data(uint32_t flash_addr,
                                        uint32_t *data);

/*****************************************************************************
 * Name: esp_rom_spiflash_write_encrypted_disable
 *
 * Description:
 *   SPI1 go out of encrypto mode.
 *
 *   Please do not call this function in SDK.
 *
 *****************************************************************************/

void esp_rom_spiflash_write_encrypted_disable(void);

/*****************************************************************************
 * Name: esp_rom_spiflash_write_encrypted
 *
 * Description:
 *   Write data to flash with transparent encryption.
 *   Sectors to be written should already be erased.
 *   Please do not call this function in SDK.
 *
 * Input Parameters:
 *   uint32_t flash_addr : Address to write, should be 32 byte aligned.
 *
 *   uint32_t *data : The pointer to data to write. Note, this pointer must
 *                    be 32 bit aligned and the content of the data will be
 *                    modified by the encryption function.
 *
 *   uint32_t len : Length to write, should be 32 bytes aligned.
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Data written successfully.
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Encryption write error.
 *   ESP_ROM_SPIFLASH_RESULT_TIMEOUT : Encrypto write timeout.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t
esp_rom_spiflash_write_encrypted(uint32_t flash_addr,
                                 uint32_t *data,
                                 uint32_t len);

/*****************************************************************************
 * Name: esp_rom_spiflash_wait_idle
 *
 * Description:
 *   Wait until SPI flash write operation is complete
 *
 *   Please do not call this function in SDK.
 *
 *   Reads the Write In Progress bit of the SPI flash status register,
 *   repeats until this bit is zero (indicating write complete).
 *
 * Returned Value:
 *   ESP_ROM_SPIFLASH_RESULT_OK : Write is complete
 *   ESP_ROM_SPIFLASH_RESULT_ERR : Error while reading status.
 *
 *****************************************************************************/

esp_rom_spiflash_result_t esp_rom_spiflash_wait_idle(esp32_spiflash_chip_t
                                                     *spi);

/*****************************************************************************
 * Name: esp_rom_spiflash_select_qio_pins
 *
 * Description:
 *   Enable Quad I/O pin functions
 *
 *   Please do not call this function in SDK.
 *
 *   Sets the HD & WP pin functions for Quad I/O modes, based on the
 *   efuse SPI pin configuration.
 *
 * Input Parameters:
 *   wp_gpio_num - Number of the WP pin to reconfigure for quad I/O.
 *   spiconfig   - Pin configuration, as returned from
 *                 ets_efuse_get_spiconfig().
 *               - If this parameter is 0, default SPI pins are used and
 *                 wp_gpio_num parameter is ignored.
 *               - If this parameter is 1, default HSPI pins are used and
 *                 wp_gpio_num parameter is ignored.
 *               - For other values, this parameter encodes the HD pin number
 *                 and also the CLK pin number. CLK pin selection is used to
 *                 determine if HSPI or SPI peripheral will be used (use HSPI
 *                 if CLK pin is the HSPI clock pin, otherwise use SPI).
 *   Both HD & WP pins are configured via GPIO matrix to map to the selected
 *   peripheral.
 *
 *****************************************************************************/

void esp_rom_spiflash_select_qio_pins(uint8_t wp_gpio_num,
                                      uint32_t spiconfig);

/* Global esp32_spiflash_chip_t structure used by ROM functions */

extern esp32_spiflash_chip_t g_rom_flashchip;

extern uint8_t g_rom_spiflash_dummy_len_plus[];

#ifdef __cplusplus
}
#endif

#endif /* _ROM_SPI_FLASH_H_ */
