/****************************************************************************
 * arch/arm/src/armv7-a/arm_hwdebug.c
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/arch.h>

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <debug.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Breakpoint */

#define ARM_BREAKPOINT_EXECUTE   0

/* Watchpoints */

#define ARM_BREAKPOINT_LOAD      1
#define ARM_BREAKPOINT_STORE     2

/* Privilege Levels */

#define ARM_BREAKPOINT_PRIV      1
#define ARM_BREAKPOINT_USER      2

/* Lengths */

#define ARM_BREAKPOINT_LEN_1     0x1
#define ARM_BREAKPOINT_LEN_2     0x3
#define ARM_BREAKPOINT_LEN_4     0xf
#define ARM_BREAKPOINT_LEN_8     0xff

/* Limits */

#define ARM_MAX_BRP              16
#define ARM_MAX_WRP              16

/* DSCR monitor/halting bits. */

#define ARM_DSCR_HDBGEN          (1 << 14)
#define ARM_DSCR_MDBGEN          (1 << 15)

/* opcode2 numbers for the co-processor instructions. */

#define ARM_OP2_BVR              4
#define ARM_OP2_BCR              5
#define ARM_OP2_WVR              6
#define ARM_OP2_WCR              7

/* Base register numbers for the debug registers. */

#define ARM_BASE_BVR             64
#define ARM_BASE_BCR             80
#define ARM_BASE_WVR             96
#define ARM_BASE_WCR             112

#define ARM_ADDBRP_EVENT         0
#define ARM_ADDWRP_EVENT         1

#define ARM_DSCR_MOE(x)            ((x >> 2) & 0xf)
#define ARM_ENTRY_BREAKPOINT       0x1
#define ARM_ENTRY_ASYNC_WATCHPOINT 0x2
#define ARM_ENTRY_CFI_BREAKPOINT   0x3
#define ARM_ENTRY_SYNC_WATCHPOINT  0xa

/* Accessor macros for the debug registers. */

#define CP14_GET(n, m, op2) \
  ({ \
    uint32_t _val; \
    asm volatile("mrc p14, 0, %0, " #n "," #m ", " #op2 : "=r" (_val)); \
    _val; \
  })

#define CP14_SET(n, m, op2, val) \
  ({ \
    asm volatile("mcr p14, 0, %0, " #n "," #m ", " #op2 : : "r" (val)); \
  })

#define CP14_GET_REG_CASE(op2, m, val) \
  case ((op2 << 4) + m):               \
    val = CP14_GET(c0, c ## m, op2);   \
    break

#define CP14_SET_REG_CASE(op2, m, val) \
  case ((op2 << 4) + m):               \
    CP14_SET(c0, c ## m, op2, val);    \
    break

#define CP14_GET_REG_CASES(op2, val) \
  CP14_GET_REG_CASE(op2, 0, val);    \
  CP14_GET_REG_CASE(op2, 1, val);    \
  CP14_GET_REG_CASE(op2, 2, val);    \
  CP14_GET_REG_CASE(op2, 3, val);    \
  CP14_GET_REG_CASE(op2, 4, val);    \
  CP14_GET_REG_CASE(op2, 5, val);    \
  CP14_GET_REG_CASE(op2, 6, val);    \
  CP14_GET_REG_CASE(op2, 7, val);    \
  CP14_GET_REG_CASE(op2, 8, val);    \
  CP14_GET_REG_CASE(op2, 9, val);    \
  CP14_GET_REG_CASE(op2, 10, val);   \
  CP14_GET_REG_CASE(op2, 11, val);   \
  CP14_GET_REG_CASE(op2, 12, val);   \
  CP14_GET_REG_CASE(op2, 13, val);   \
  CP14_GET_REG_CASE(op2, 14, val);   \
  CP14_GET_REG_CASE(op2, 15, val)

#define CP14_SET_REG_CASES(op2, val) \
  CP14_SET_REG_CASE(op2, 0, val);    \
  CP14_SET_REG_CASE(op2, 1, val);    \
  CP14_SET_REG_CASE(op2, 2, val);    \
  CP14_SET_REG_CASE(op2, 3, val);    \
  CP14_SET_REG_CASE(op2, 4, val);    \
  CP14_SET_REG_CASE(op2, 5, val);    \
  CP14_SET_REG_CASE(op2, 6, val);    \
  CP14_SET_REG_CASE(op2, 7, val);    \
  CP14_SET_REG_CASE(op2, 8, val);    \
  CP14_SET_REG_CASE(op2, 9, val);    \
  CP14_SET_REG_CASE(op2, 10, val);   \
  CP14_SET_REG_CASE(op2, 11, val);   \
  CP14_SET_REG_CASE(op2, 12, val);   \
  CP14_SET_REG_CASE(op2, 13, val);   \
  CP14_SET_REG_CASE(op2, 14, val);   \
  CP14_SET_REG_CASE(op2, 15, val)

#define RCP14_DIDR()    CP14_GET(c0, c0, 0)
#define RCP14_DSCR()    CP14_GET(c0, c1, 0)
#define RCP14_SCREXT()  CP14_GET(c0, c2, 2)
#define WCP14_SCREXT(v) CP14_SET(c0, c2, 2, v)

#define MASK_ADDR(addr) ((uint32_t)addr & ~0x3)

/****************************************************************************
 * Private Type
 ****************************************************************************/

struct arm_hw_breakpoint_ctrl
{
  uint32_t __reserved  : 9,
  mismatch  : 1,
            : 9,
  len       : 8,
  type      : 2,
  privilege : 2,
  enabled   : 1;
};

struct arm_hw_breakpoint
{
  int type;
  uint32_t addr;
  size_t size;
  debug_callback_t callback;
  void *arg;
  struct arm_hw_breakpoint_ctrl ctrl;
};

struct arm_breakpoint_slot
{
  /* Breakpoint currently in use for each BRP, WRP */

  struct arm_hw_breakpoint brps[ARM_MAX_BRP];
  struct arm_hw_breakpoint wrps[ARM_MAX_WRP];
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct arm_breakpoint_slot g_cpu_bp_slot[CONFIG_SMP_NCPUS];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static uint32_t arm_read_wb_reg(int n)
{
  uint32_t val = 0;

  switch (n)
    {
      CP14_GET_REG_CASES(ARM_OP2_BVR, val);
      CP14_GET_REG_CASES(ARM_OP2_BCR, val);
      CP14_GET_REG_CASES(ARM_OP2_WVR, val);
      CP14_GET_REG_CASES(ARM_OP2_WCR, val);
      default:
        binfo("attempt to read from unknown breakpoint register %d\n", n);
    }

  return val;
}

static void arm_write_wb_reg(int n, uint32_t val)
{
  switch (n)
    {
      CP14_SET_REG_CASES(ARM_OP2_BVR, val);
      CP14_SET_REG_CASES(ARM_OP2_BCR, val);
      CP14_SET_REG_CASES(ARM_OP2_WVR, val);
      CP14_SET_REG_CASES(ARM_OP2_WCR, val);
      default:
        binfo("attempt to write to unknown breakpoint register %d\n", n);
    }
}

static uint32_t arm_encode_ctrl_reg(struct arm_hw_breakpoint_ctrl *ctrl)
{
  return (ctrl->mismatch << 22) | (ctrl->len << 5) | (ctrl->type << 3) |
         (ctrl->privilege << 1) | ctrl->enabled;
}

/* Check if 8-bit byte-address select is available.
 * This clobbers WRP 0.
 */

static uint8_t arm_get_max_wp_len(void)
{
  uint8_t size = 4;
  uint32_t ctrl_reg;
  struct arm_hw_breakpoint_ctrl ctrl =
    {
      0
    };

  ctrl.len = ARM_BREAKPOINT_LEN_8;
  ctrl_reg = arm_encode_ctrl_reg(&ctrl);

  arm_write_wb_reg(ARM_BASE_WVR, 0);
  arm_write_wb_reg(ARM_BASE_WCR, ctrl_reg);
  if ((arm_read_wb_reg(ARM_BASE_WCR) & ctrl_reg) == ctrl_reg)
    {
      size = 8;
    }

  return size;
}

/* Determine number of usable WRPs available. */

static int arm_get_num_wrps(void)
{
  return ((RCP14_DIDR() >> 28) & 0xf) + 1;
}

/* Determine number of usable BRPs available. */

static int arm_get_num_brps(void)
{
  return ((RCP14_DIDR() >> 24) & 0xf) + 1;
}

/* This function attempts to enable the monitor mode on an ARM processor.
 * Monitor mode is a debugging mode that allows the debugger to access
 * and modify processor registers for more in-depth debugging.
 */

static int arm_enable_monitor_mode(void)
{
  /* If monitor mode is already enabled, just return. */

  if (RCP14_DSCR() & ARM_DSCR_MDBGEN)
    {
      return 0;
    }

  WCP14_SCREXT(RCP14_DSCR() | ARM_DSCR_MDBGEN);

  /* Check that the write made it through. */

  if (!(RCP14_DSCR() & ARM_DSCR_MDBGEN))
    {
      return -EPERM;
    }

  return 0;
}

static struct arm_hw_breakpoint
*get_bp_by_addr(int event, uint32_t addr, int *index)
{
  struct arm_hw_breakpoint *p;
  int size;
  int cpu;
  int i;

  cpu = this_cpu();
  if (event == ARM_ADDBRP_EVENT)
    {
      p = g_cpu_bp_slot[cpu].brps;
      size = arm_get_num_brps();
    }
  else
    {
      p = g_cpu_bp_slot[cpu].wrps;
      size = arm_get_num_wrps();
    }

  for (i = 0; i < size; i++)
    {
      if (p[i].addr == addr)
        {
          *index = i;
          return &p[i];
        }
    }

  return NULL;
}

static struct arm_hw_breakpoint *get_empty_bp(int event, int *index)
{
  struct arm_hw_breakpoint *p;
  int size;
  int cpu;
  int i;

  cpu = this_cpu();
  if (event == ARM_ADDBRP_EVENT)
    {
      p = g_cpu_bp_slot[cpu].brps;
      size = arm_get_num_brps();
    }
  else
    {
      p = g_cpu_bp_slot[cpu].wrps;
      size = arm_get_num_wrps();
    }

  for (i = 0; i < size; i++)
    {
      if (!p[i].ctrl.enabled)
        {
          *index = i;
          return &p[i];
        }
    }

  return NULL;
}

static int add_debugpoint(int event, struct arm_hw_breakpoint *bp)
{
  struct arm_hw_breakpoint *p;
  int index;

  p = get_empty_bp(event, &index);
  if (!p)
    {
      return -1;
    }

  memcpy(p, bp, sizeof(struct arm_hw_breakpoint));

  binfo("CPU[%d] add %s[%d] at %p size %d\n",
        this_cpu(),
        (event == ARM_ADDBRP_EVENT) ? "Breakpoint" : "Watchpoint",
        index, bp->addr,
        ((bp->ctrl.len == ARM_BREAKPOINT_LEN_1) ? 1 :
         (bp->ctrl.len == ARM_BREAKPOINT_LEN_2) ? 2 :
         (bp->ctrl.len == ARM_BREAKPOINT_LEN_4) ? 4 : 8));

  return index;
}

static int remove_debugpoint(int event, struct arm_hw_breakpoint *bp)
{
  struct arm_hw_breakpoint *p;
  int index;

  p = get_bp_by_addr(event, bp->addr, &index);
  if (!p)
    {
      return -1;
    }

  memset(p, 0, sizeof(struct arm_hw_breakpoint));

  binfo("CPU[%d] remove %s[%d] at %p size %d\n",
        this_cpu(),
        (event == ARM_ADDBRP_EVENT) ? "Breakpoint" : "Watchpoint",
        index, bp->addr,
        ((bp->ctrl.len == ARM_BREAKPOINT_LEN_1) ? 1 :
         (bp->ctrl.len == ARM_BREAKPOINT_LEN_2) ? 2 :
         (bp->ctrl.len == ARM_BREAKPOINT_LEN_4) ? 4 : 8));

  return index;
}

/* Install a perf counter breakpoint. */

static int arm_install_hw_breakpoint(struct arm_hw_breakpoint *bp)
{
  int ctrl_base;
  int val_base;
  int index;

  bp->ctrl.enabled = 1;

  if (bp->ctrl.type == ARM_BREAKPOINT_EXECUTE)
    {
      /* Breakpoint for code execution */

      index = add_debugpoint(ARM_ADDBRP_EVENT, bp);
      ctrl_base = ARM_BASE_BCR;
      val_base = ARM_BASE_BVR;
    }
  else
    {
      /* Watchpoint for memory access */

      index = add_debugpoint(ARM_ADDWRP_EVENT, bp);
      ctrl_base = ARM_BASE_WCR;
      val_base = ARM_BASE_WVR;
    }

  if (index < 0)
    {
      return -EINVAL;
    }

  arm_write_wb_reg(val_base + index, bp->addr);
  arm_write_wb_reg(ctrl_base + index, arm_encode_ctrl_reg(&bp->ctrl));

  return 0;
}

/* Uninstall a perf counter breakpoint. */

static int arm_uninstall_hw_breakpoint(struct arm_hw_breakpoint *bp)
{
  int index;
  int base;

  if (bp->ctrl.type == ARM_BREAKPOINT_EXECUTE)
    {
      /* Breakpoint */

      index = remove_debugpoint(ARM_ADDBRP_EVENT, bp);
      base = ARM_BASE_BCR;
    }
  else
    {
      /* Watchpoint */

      index = remove_debugpoint(ARM_ADDWRP_EVENT, bp);
      base = ARM_BASE_WCR;
    }

  if (index < 0)
    {
      return -EINVAL;
    }

  arm_write_wb_reg(base + index, 0);
  return 0;
}

static int arm_build_bp_info(struct arm_hw_breakpoint *bp, void *addr,
                             size_t size, int type)
{
  memset(bp, 0, sizeof(struct arm_hw_breakpoint));

  bp->addr = MASK_ADDR(addr);
  bp->ctrl.privilege = ARM_BREAKPOINT_PRIV | ARM_BREAKPOINT_USER;

  switch (type)
    {
      case DEBUGPOINT_WATCHPOINT_RO:
        bp->ctrl.type = ARM_BREAKPOINT_LOAD;
        break;

      case DEBUGPOINT_WATCHPOINT_WO:
        bp->ctrl.type = ARM_BREAKPOINT_STORE;
        break;

      case DEBUGPOINT_WATCHPOINT_RW:
        bp->ctrl.type = ARM_BREAKPOINT_LOAD | ARM_BREAKPOINT_STORE;
        break;

      case DEBUGPOINT_BREAKPOINT:
      case DEBUGPOINT_STEPPOINT:
        bp->ctrl.type = ARM_BREAKPOINT_EXECUTE;
        break;

      default:
        return -EINVAL;
    }

  switch (size)
    {
      case 1:
        bp->ctrl.len = ARM_BREAKPOINT_LEN_1;
        break;

      case 2:
        bp->ctrl.len = ARM_BREAKPOINT_LEN_2;
        break;

      case 4:
        bp->ctrl.len = ARM_BREAKPOINT_LEN_4;
        break;

      case 8:
      default:
        bp->ctrl.len = ARM_BREAKPOINT_LEN_8;
        break;
    }

  return 0;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void arm_hw_breakpoint_pending(uint32_t addr, uint32_t *regs)
{
  const struct arm_hw_breakpoint *bp;
  int index;

  binfo("hw_breakpoint_pending: 0x%08x\n", addr);

  switch (ARM_DSCR_MOE(RCP14_DSCR()))
    {
      case ARM_ENTRY_BREAKPOINT:
      case ARM_ENTRY_CFI_BREAKPOINT:
        bp = get_bp_by_addr(ARM_ADDBRP_EVENT, MASK_ADDR(addr), &index);
        bp->callback(bp->type, (void *)bp->addr, bp->size, bp->arg);
        break;
      case ARM_ENTRY_ASYNC_WATCHPOINT:
      case ARM_ENTRY_SYNC_WATCHPOINT:
        bp = get_bp_by_addr(ARM_ADDWRP_EVENT, MASK_ADDR(addr), &index);
        bp->callback(bp->type, (void *)bp->addr, bp->size, bp->arg);
        break;
      default:
        break;
    }
}
/****************************************************************************
 * Name: arm_enable_dbgmonitor
 *
 * Description:
 *   This function enables the debug monitor exception.
 *
 ****************************************************************************/

int arm_enable_dbgmonitor(void)
{
  int ret;

  /* Determine how many BRPs/WRPs are available.
   * And work out the maximum supported watchpoint length.
   */

  binfo("found %d breakpoint and %d watchpoint registers.\n"
        "maximum watchpoint size is %u bytes.\n",
        arm_get_num_brps(), arm_get_num_wrps(), arm_get_max_wp_len());

  ret = arm_enable_monitor_mode();
  if (ret)
    {
      binfo("Failed to enable monitor mode on CPU %d.\n", this_cpu());
    }

  return ret;
}

/****************************************************************************
 * Name: up_debugpoint_add
 *
 * Description:
 *   Add a debugpoint.
 *
 * Input Parameters:
 *   type     - The debugpoint type. optional value:
 *              DEBUGPOINT_WATCHPOINT_RO - Read only watchpoint.
 *              DEBUGPOINT_WATCHPOINT_WO - Write only watchpoint.
 *              DEBUGPOINT_WATCHPOINT_RW - Read and write watchpoint.
 *              DEBUGPOINT_BREAKPOINT    - Breakpoint.
 *              DEBUGPOINT_STEPPOINT     - Single step.
 *   addr     - The address to be debugged.
 *   size     - The watchpoint size. only for watchpoint.
 *   callback - The callback function when debugpoint triggered.
 *              if NULL, the debugpoint will be removed.
 *   arg      - The argument of callback function.
 *
 * Returned Value:
 *  Zero on success; a negated errno value on failure
 *
 ****************************************************************************/

int up_debugpoint_add(int type, void *addr, size_t size,
                      debug_callback_t callback, void *arg)
{
  struct arm_hw_breakpoint bp;

  if (arm_build_bp_info(&bp, addr, size, type))
    {
      binfo("Failed to build breakpoint info\n");
      return -EINVAL;
    }

  bp.type = type;
  bp.addr = (uint32_t)addr;
  bp.size = size;
  bp.callback = callback;
  bp.arg = arg;

  return arm_install_hw_breakpoint(&bp);
}

/****************************************************************************
 * Name: up_debugpoint_remove
 *
 * Description:
 *   Remove a debugpoint.
 *
 * Input Parameters:
 *   type     - The debugpoint type. optional value:
 *              DEBUGPOINT_WATCHPOINT_RO - Read only watchpoint.
 *              DEBUGPOINT_WATCHPOINT_WO - Write only watchpoint.
 *              DEBUGPOINT_WATCHPOINT_RW - Read and write watchpoint.
 *              DEBUGPOINT_BREAKPOINT    - Breakpoint.
 *              DEBUGPOINT_STEPPOINT     - Single step.
 *   addr     - The address to be debugged.
 *   size     - The watchpoint size. only for watchpoint.
 *
 * Returned Value:
 *  Zero on success; a negated errno value on failure
 *
 ****************************************************************************/

int up_debugpoint_remove(int type, void *addr, size_t size)
{
  struct arm_hw_breakpoint bp;

  if (arm_build_bp_info(&bp, addr, size, type))
    {
      binfo("Failed to build breakpoint info\n");
      return -EINVAL;
    }

  return arm_uninstall_hw_breakpoint(&bp);
}
