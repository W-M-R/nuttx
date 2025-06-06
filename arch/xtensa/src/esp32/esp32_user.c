/****************************************************************************
 * arch/xtensa/src/esp32/esp32_user.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>

#include <arch/loadstore.h>
#include <arch/xtensa/xtensa_corebits.h>

#include <sys/types.h>
#include <assert.h>
#include <debug.h>

#include "xtensa.h"
#include "esp32_spicache.h"

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef CONFIG_ARCH_USE_TEXT_HEAP
extern uint8_t _siramheap[];
extern uint8_t _eiramheap[];
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

#ifdef CONFIG_ARCH_USE_TEXT_HEAP
#ifdef CONFIG_ENDIAN_BIG
#error not implemented
#endif
#ifndef CONFIG_BUILD_FLAT
#error permission check not implemented
#endif

/****************************************************************************
 * Name: load_uint8
 *
 * Description:
 *   Fetch a byte using 32-bit aligned access.
 *
 ****************************************************************************/

static uint8_t load_uint8(const uint8_t *p)
{
  const uint32_t *aligned;
  uint32_t value;
  unsigned int offset;

  aligned = (const uint32_t *)(((uintptr_t)p) & ~3);
  value = l32i(aligned);
  offset = ((uintptr_t)p) & 3;
  switch (offset)
    {
      case 0:
        return value & 0xff;
      case 1:
        return (value >> 8) & 0xff;
      case 2:
        return (value >> 16) & 0xff;
      case 3:
        return (value >> 24) & 0xff;
    }

  /* not reached */

  PANIC();
}

/****************************************************************************
 * Name: store_uint8
 *
 * Description:
 *   Store a byte using 32-bit aligned access.
 *
 ****************************************************************************/

static void store_uint8(uint8_t *p, uint8_t v)
{
  uint32_t *aligned;
  uint32_t value;
  unsigned int offset;

  aligned = (uint32_t *)(((uintptr_t)p) & ~3);
  value = l32i(aligned);
  offset = ((uintptr_t)p) & 3;
  switch (offset)
    {
      case 0:
        value = (value & 0xffffff00) | v;
        break;
      case 1:
        value = (value & 0xffff00ff) | (v << 8);
        break;
      case 2:
        value = (value & 0xff00ffff) | (v << 16);
        break;
      case 3:
        value = (value & 0x00ffffff) | (v << 24);
        break;
    }

  s32i(aligned, value);
}

/****************************************************************************
 * Name: decode_s8i
 *
 * Description:
 *   Decode S8I instruction using 32-bit aligned access.
 *   Return non-zero on successful decoding.
 *
 ****************************************************************************/

static int decode_s8i(const uint8_t *p, uint8_t *imm8, uint8_t *s,
                      uint8_t *t)
{
  /*  23           16 15   12 11    8 7     4 3     0
   * | imm8          |0 1 0 0| s     | t     |0 0 1 0|
   */

  uint8_t b0 = load_uint8(p);
  uint8_t b1 = load_uint8(p + 1);

  if ((b0 & 0xf) == 2 && (b1 & 0xf0) == 0x40)
    {
      *t = b0 >> 4;
      *s = b1 & 0xf;
      *imm8 = load_uint8(p + 2);
      return 1;
    }

  return 0;
}

/****************************************************************************
 * Name: decode_s16i
 *
 * Description:
 *   Decode S16I instruction using 32-bit aligned access.
 *   Return non-zero on successful decoding.
 *
 ****************************************************************************/

static int decode_s16i(const uint8_t *p, uint8_t *imm8, uint8_t *s,
                       uint8_t *t)
{
  /*  23           16 15   12 11    8 7     4 3     0
   * | imm8          |0 1 0 1| s     | t     |0 0 1 0|
   */

  uint8_t b0 = load_uint8(p);
  uint8_t b1 = load_uint8(p + 1);

  if ((b0 & 0xf) == 2 && (b1 & 0xf0) == 0x50)
    {
      *t = b0 >> 4;
      *s = b1 & 0xf;
      *imm8 = load_uint8(p + 2);
      return 1;
    }

  return 0;
}

/****************************************************************************
 * Name: decode_l8ui
 *
 * Description:
 *   Decode L8UI instruction using 32-bit aligned access.
 *   Return non-zero on successful decoding.
 *
 ****************************************************************************/

static int decode_l8ui(const uint8_t *p, uint8_t *imm8, uint8_t *s,
                       uint8_t *t)
{
  /*  23           16 15   12 11    8 7     4 3     0
   * | imm8          |0 0 0 0| s     | t     |0 0 1 0|
   */

  uint8_t b0 = load_uint8(p);
  uint8_t b1 = load_uint8(p + 1);

  if ((b0 & 0xf) == 2 && (b1 & 0xf0) == 0)
    {
      *t = b0 >> 4;
      *s = b1 & 0xf;
      *imm8 = load_uint8(p + 2);
      return 1;
    }

  return 0;
}

/****************************************************************************
 * Name: decode_l16ui
 *
 * Description:
 *   Decode L16UI instruction using 32-bit aligned access.
 *   Return non-zero on successful decoding.
 *
 ****************************************************************************/

static int decode_l16ui(const uint8_t *p, uint8_t *imm8, uint8_t *s,
                       uint8_t *t)
{
  /*  23           16 15   12 11    8 7     4 3     0
   * | imm8          |0 0 0 1| s     | t     |0 0 1 0|
   */

  uint8_t b0 = load_uint8(p);
  uint8_t b1 = load_uint8(p + 1);

  if ((b0 & 0xf) == 2 && (b1 & 0xf0) == 0x10)
    {
      *t = b0 >> 4;
      *s = b1 & 0xf;
      *imm8 = load_uint8(p + 2);
      return 1;
    }

  return 0;
}

/****************************************************************************
 * Name: decode_l16si
 *
 * Description:
 *   Decode L16SI instruction using 32-bit aligned access.
 *   Return non-zero on successful decoding.
 *
 ****************************************************************************/

static int decode_l16si(const uint8_t *p, uint8_t *imm8, uint8_t *s,
                       uint8_t *t)
{
  /*  23           16 15   12 11    8 7     4 3     0
   * | imm8          |1 0 0 1| s     | t     |0 0 1 0|
   */

  uint8_t b0 = load_uint8(p);
  uint8_t b1 = load_uint8(p + 1);

  if ((b0 & 0xf) == 2 && (b1 & 0xf0) == 0x90)
    {
      *t = b0 >> 4;
      *s = b1 & 0xf;
      *imm8 = load_uint8(p + 2);
      return 1;
    }

  return 0;
}

/****************************************************************************
 * Name: advance_pc
 *
 * Description:
 *   Advance PC register by the given value.
 *
 ****************************************************************************/

static void advance_pc(uint32_t *regs, int diff)
{
  uint32_t nextpc;

  /* Advance to the next instruction. */

  nextpc = regs[REG_PC] + diff;
#if XCHAL_HAVE_LOOPS != 0
  /* See Xtensa ISA 4.3.2.4 Loopback Semantics */

  if (regs[REG_LCOUNT] != 0 && nextpc == regs[REG_LEND])
    {
      regs[REG_LCOUNT]--;
      nextpc = regs[REG_LBEG];
    }

#endif
  regs[REG_PC] = nextpc;
}

#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: xtensa_user
 *
 * Description:
 *   ESP32-specific user exception handler.
 *
 ****************************************************************************/

uint32_t *xtensa_user(int exccause, uint32_t *regs)
{
#ifdef CONFIG_ESP32_SPIFLASH
  bool is_cache_reenabled = false;

  if (!spi_flash_cache_enabled())
    {
      is_cache_reenabled = true;

      spi_enable_cache(0);
#  ifdef CONFIG_SMP
      spi_enable_cache(1);
#  endif
    }
#endif /* CONFIG_ESP32_SPIFLASH */

#ifdef CONFIG_ARCH_USE_TEXT_HEAP
  /* Emulate byte access for module text.
   *
   * ESP32 only allows word-aligned accesses to the instruction memory
   * regions.  A non-aligned access raises a LoadStoreErrorCause exception.
   * We catch those exception and emulate byte access here because it's
   * necessary in a few places during dynamic code loading:
   *
   *  - memcpy as a part of read(2) when loading code from a file system.
   *  - relocation needs to inspect and modify text.
   *
   * (thus binfo() is used below)
   */

  if (exccause == EXCCAUSE_LOAD_STORE_ERROR &&
      (uintptr_t)_siramheap <= regs[REG_EXCVADDR] &&
      (uintptr_t)_eiramheap > regs[REG_EXCVADDR])
    {
      uint8_t *pc = (uint8_t *)regs[REG_PC];
      uint8_t imm8;
      uint8_t s;
      uint8_t t;

      binfo("EXCCAUSE_LOAD_STORE_ERROR at %p, pc=%p\n",
            (void *)regs[REG_EXCVADDR],
            pc);

      if (decode_s8i(pc, &imm8, &s, &t))
        {
          binfo("Emulating S8I imm8=%u, s=%u (%p), t=%u (%p)\n",
                (unsigned int)imm8,
                (unsigned int)s,
                (void *)regs[REG_A0 + s],
                (unsigned int)t,
                (void *)regs[REG_A0 + t]);

          DEBUGASSERT(regs[REG_A0 + s] + imm8 == regs[REG_EXCVADDR]);
          store_uint8(((uint8_t *)regs[REG_A0 + s]) + imm8,
                      regs[REG_A0 + t]);
          advance_pc(regs, 3);
          goto return_with_regs;
        }
      else if (decode_s16i(pc, &imm8, &s, &t))
        {
          binfo("Emulating S16I imm8=%u, s=%u (%p), t=%u (%p)\n",
                (unsigned int)imm8,
                (unsigned int)s,
                (void *)regs[REG_A0 + s],
                (unsigned int)t,
                (void *)regs[REG_A0 + t]);

          uintptr_t va = regs[REG_A0 + s] + (imm8 << 1);
          DEBUGASSERT(va == regs[REG_EXCVADDR]);
          store_uint8((uint8_t *)va, regs[REG_A0 + t]);
          store_uint8((uint8_t *)va + 1, regs[REG_A0 + t] >> 8);
          advance_pc(regs, 3);
          goto return_with_regs;
        }
      else if (decode_l8ui(pc, &imm8, &s, &t))
        {
          binfo("Emulating L8UI imm8=%u, s=%u (%p), t=%u (%p)\n",
                (unsigned int)imm8,
                (unsigned int)s,
                (void *)regs[REG_A0 + s],
                (unsigned int)t,
                (void *)regs[REG_A0 + t]);

          DEBUGASSERT(regs[REG_A0 + s] + imm8 == regs[REG_EXCVADDR]);
          regs[REG_A0 + t] = load_uint8(((uint8_t *)regs[REG_A0 + s]) +
                                        imm8);
          advance_pc(regs, 3);
          goto return_with_regs;
        }
      else if (decode_l16si(pc, &imm8, &s, &t))
        {
          binfo("Emulating L16SI imm8=%u, s=%u (%p), t=%u (%p)\n",
                (unsigned int)imm8,
                (unsigned int)s,
                (void *)regs[REG_A0 + s],
                (unsigned int)t,
                (void *)regs[REG_A0 + t]);

          uintptr_t va = regs[REG_A0 + s] + (imm8 << 1);
          DEBUGASSERT(va == regs[REG_EXCVADDR]);
          uint8_t lo = load_uint8((uint8_t *)va);
          uint8_t hi = load_uint8((uint8_t *)va + 1);
          regs[REG_A0 + t] = (int16_t)((hi << 8) | lo);
          advance_pc(regs, 3);
          goto return_with_regs;
        }
      else if (decode_l16ui(pc, &imm8, &s, &t))
        {
          binfo("Emulating L16UI imm8=%u, s=%u (%p), t=%u (%p)\n",
                (unsigned int)imm8,
                (unsigned int)s,
                (void *)regs[REG_A0 + s],
                (unsigned int)t,
                (void *)regs[REG_A0 + t]);

          uintptr_t va = regs[REG_A0 + s] + (imm8 << 1);
          DEBUGASSERT(va == regs[REG_EXCVADDR]);
          uint8_t lo = load_uint8((uint8_t *)va);
          uint8_t hi = load_uint8((uint8_t *)va + 1);
          regs[REG_A0 + t] = (hi << 8) | lo;
          advance_pc(regs, 3);
          goto return_with_regs;
        }

return_with_regs:
#  ifdef CONFIG_ESP32_SPIFLASH
      if (is_cache_reenabled)
        {
          spi_disable_cache(0);
#    ifdef CONFIG_SMP
          spi_disable_cache(1);
#    endif
        }
#  endif /* CONFIG_ESP32_SPIFLASH */

      return regs;
    }

#else
#  ifdef CONFIG_ESP32_SPIFLASH
  UNUSED(is_cache_reenabled);
#  endif
#endif

  /* xtensa_user_panic never returns. */

  xtensa_user_panic(exccause, regs);

  while (1)
    {
    }
}
