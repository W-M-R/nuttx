/****************************************************************************
 * arch/arm/src/stm32/stm32f10xxf30xx_flash.c
 *
 * SPDX-License-Identifier: Apache-2.0
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
 ****************************************************************************/

/* Provides standard flash access functions, to be used by the  flash mtd
 * driver. The interface is defined in the include/nuttx/progmem.h
 *
 * Requirements during write/erase operations on FLASH:
 *  - HSI must be ON.
 *  - Low Power Modes are not permitted during write/erase
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>
#include <nuttx/mutex.h>

#include <stdbool.h>
#include <assert.h>
#include <errno.h>

#include "stm32_flash.h"
#include "stm32_rcc.h"
#include "stm32_waste.h"
#include "arm_internal.h"

/* Only for the STM32F[1|3]0xx family. */

#if defined(CONFIG_STM32_STM32F10XX) || defined(CONFIG_STM32_STM32F30XX)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define FLASH_KEY1                 0x45670123
#define FLASH_KEY2                 0xcdef89ab
#define FLASH_OPTKEY1              0x08192a3b
#define FLASH_OPTKEY2              0x4c5d6e7f
#define FLASH_ERASEDVALUE          0xffu

#if defined(STM32_FLASH_DUAL_BANK)
/* Bank 0 is 512Kb; Bank 1 is up to 512Kb */

#  define STM32_FLASH_BANK0_NPAGES (512 * 1024 / STM32_FLASH_PAGESIZE)
#  define STM32_FLASH_BANK1_NPAGES (STM32_FLASH_NPAGES - STM32_FLASH_BANK0_NPAGES)
#else
/* Bank 0 is up to 512Kb; Bank 1 is not present */

#  define STM32_FLASH_BANK0_NPAGES STM32_FLASH_NPAGES
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

static mutex_t g_lock = NXMUTEX_INITIALIZER;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static void flash_unlock(uintptr_t base)
{
  while ((getreg32(base + STM32_FLASH_SR_OFFSET) & FLASH_SR_BSY) != 0)
    {
      stm32_waste();
    }

  if ((getreg32(base + STM32_FLASH_CR_OFFSET) & FLASH_CR_LOCK) != 0)
    {
      /* Unlock sequence */

      putreg32(FLASH_KEY1, base + STM32_FLASH_KEYR_OFFSET);
      putreg32(FLASH_KEY2, base + STM32_FLASH_KEYR_OFFSET);
    }
}

static void flash_lock(uintptr_t base)
{
  modifyreg32(base + STM32_FLASH_CR_OFFSET, 0, FLASH_CR_LOCK);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int stm32_flash_unlock(void)
{
  int ret;

  ret = nxmutex_lock(&g_lock);
  if (ret < 0)
    {
      return ret;
    }

  flash_unlock(STM32_FLASHIF_BASE);
#if defined(STM32_FLASH_DUAL_BANK)
  flash_unlock(STM32_FLASHIF1_BASE);
#endif
  nxmutex_unlock(&g_lock);

  return ret;
}

int stm32_flash_lock(void)
{
  int ret;

  ret = nxmutex_lock(&g_lock);
  if (ret < 0)
    {
      return ret;
    }

  flash_lock(STM32_FLASHIF_BASE);
#if defined(STM32_FLASH_DUAL_BANK)
  flash_lock(STM32_FLASHIF1_BASE);
#endif
  nxmutex_unlock(&g_lock);

  return ret;
}

size_t up_progmem_pagesize(size_t page)
{
  return STM32_FLASH_PAGESIZE;
}

size_t up_progmem_erasesize(size_t block)
{
  return STM32_FLASH_PAGESIZE;
}

ssize_t up_progmem_getpage(size_t addr)
{
  if (addr >= STM32_FLASH_BASE)
    {
      addr -= STM32_FLASH_BASE;
    }

  if (addr >= STM32_FLASH_SIZE)
    {
      return -EFAULT;
    }

  return addr / STM32_FLASH_PAGESIZE;
}

size_t up_progmem_getaddress(size_t page)
{
  if (page >= STM32_FLASH_NPAGES)
    {
      return SIZE_MAX;
    }

  return page * STM32_FLASH_PAGESIZE + STM32_FLASH_BASE;
}

size_t up_progmem_neraseblocks(void)
{
  return STM32_FLASH_NPAGES;
}

bool up_progmem_isuniform(void)
{
#ifdef STM32_FLASH_PAGESIZE
  return true;
#else
  return false;
#endif
}

ssize_t up_progmem_ispageerased(size_t page)
{
  size_t addr;
  size_t count;
  size_t bwritten = 0;

  if (page >= STM32_FLASH_NPAGES)
    {
      return -EFAULT;
    }

  /* Verify */

  for (addr = up_progmem_getaddress(page), count = up_progmem_pagesize(page);
       count; count--, addr++)
    {
      if (getreg8(addr) != FLASH_ERASEDVALUE)
        {
          bwritten++;
        }
    }

  return bwritten;
}

ssize_t up_progmem_eraseblock(size_t block)
{
  uintptr_t base;
  size_t page_address;
  int ret;

  if (block >= STM32_FLASH_NPAGES)
    {
      return -EFAULT;
    }

#if defined(STM32_FLASH_DUAL_BANK)
  /* Handle paged FLASH */

  if (block >= STM32_FLASH_BANK0_NPAGES)
    {
      base = STM32_FLASHIF1_BASE;
    }
  else
#endif
    {
      base = STM32_FLASHIF_BASE;
    }

  ret = nxmutex_lock(&g_lock);
  if (ret < 0)
    {
      return (ssize_t)ret;
    }

  if ((getreg32(STM32_RCC_CR) & RCC_CR_HSION) == 0)
    {
      nxmutex_unlock(&g_lock);
      return -EPERM;
    }

  /* Get flash ready and begin erasing single page */

  flash_unlock(base);

  modifyreg32(base + STM32_FLASH_CR_OFFSET, 0, FLASH_CR_PER);

  /* Must be valid - page index checked above */

  page_address = up_progmem_getaddress(block);
  putreg32(page_address, base + STM32_FLASH_AR_OFFSET);

  modifyreg32(base + STM32_FLASH_CR_OFFSET, 0, FLASH_CR_STRT);

  while ((getreg32(base + STM32_FLASH_SR_OFFSET) & FLASH_SR_BSY) != 0)
    {
      stm32_waste();
    }

  modifyreg32(base + STM32_FLASH_CR_OFFSET, FLASH_CR_PER, 0);
  nxmutex_unlock(&g_lock);

  /* Verify */

  if (up_progmem_ispageerased(block) == 0)
    {
      return up_progmem_erasesize(block); /* success */
    }
  else
    {
      return -EIO; /* failure */
    }
}

ssize_t up_progmem_write(size_t addr, const void *buf, size_t count)
{
  uintptr_t base;
  uint16_t *hword = (uint16_t *)buf;
  size_t written = count;
  int ret;

#if defined(STM32_FLASH_DUAL_BANK)
  /* Handle paged FLASH */

  if (page >= STM32_FLASH_BANK0_NPAGES)
    {
      base = STM32_FLASHIF1_BASE;
    }
  else
#endif
    {
      base = STM32_FLASHIF_BASE;
    }

  /* STM32 requires half-word access */

  if (count & 1)
    {
      return -EINVAL;
    }

  /* Check for valid address range */

  if (addr >= STM32_FLASH_BASE)
    {
      addr -= STM32_FLASH_BASE;
    }

  if ((addr + count) > STM32_FLASH_SIZE)
    {
      return -EFAULT;
    }

  ret = nxmutex_lock(&g_lock);
  if (ret < 0)
    {
      return (ssize_t)ret;
    }

  if ((getreg32(STM32_RCC_CR) & RCC_CR_HSION) == 0)
    {
      nxmutex_unlock(&g_lock);
      return -EPERM;
    }

  /* Get flash ready and begin flashing */

  flash_unlock(base);

  modifyreg32(base + STM32_FLASH_CR_OFFSET, 0, FLASH_CR_PG);

  for (addr += STM32_FLASH_BASE; count; count -= 2, hword++, addr += 2)
    {
      /* Write half-word and wait to complete */

      putreg16(*hword, addr);

      while ((getreg32(base + STM32_FLASH_SR_OFFSET) & FLASH_SR_BSY) != 0)
        {
          stm32_waste();
        }

      /* Verify */

      if ((getreg32(base + STM32_FLASH_SR_OFFSET) & FLASH_SR_WRPRT_ERR) != 0)
        {
          modifyreg32(base + STM32_FLASH_CR_OFFSET, FLASH_CR_PG, 0);
          nxmutex_unlock(&g_lock);
          return -EROFS;
        }

      if (getreg16(addr) != *hword)
        {
          modifyreg32(base + STM32_FLASH_CR_OFFSET, FLASH_CR_PG, 0);
          nxmutex_unlock(&g_lock);
          return -EIO;
        }
    }

  modifyreg32(base + STM32_FLASH_CR_OFFSET, FLASH_CR_PG, 0);

  nxmutex_unlock(&g_lock);
  return written;
}

uint8_t up_progmem_erasestate(void)
{
  return FLASH_ERASEDVALUE;
}

#endif /* defined(CONFIG_STM32_STM32F10XX) || defined(CONFIG_STM32_STM32F30XX) */
