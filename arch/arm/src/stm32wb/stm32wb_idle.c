/****************************************************************************
 * arch/arm/src/stm32wb/stm32wb_idle.c
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
#include <nuttx/irq.h>
#include <nuttx/board.h>
#include <nuttx/power/pm.h>

#include "arm_internal.h"
#include "chip.h"
#include "stm32wb_pm.h"
#include "stm32wb_rcc.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Does the board support an IDLE LED to indicate that the board is in the
 * IDLE state?
 */

#if defined(CONFIG_ARCH_LEDS) && defined(LED_IDLE)
#  define BEGIN_IDLE() board_autoled_on(LED_IDLE)
#  define END_IDLE()   board_autoled_off(LED_IDLE)
#else
#  define BEGIN_IDLE()
#  define END_IDLE()
#endif

#define PM_IDLE_DOMAIN 0 /* Revisit */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_idlepm
 *
 * Description:
 *   Perform IDLE state power management.
 *
 ****************************************************************************/

#ifdef CONFIG_PM
static void up_idlepm(void)
{
  static enum pm_state_e oldstate = PM_NORMAL;
  enum pm_state_e newstate;
  irqstate_t flags;
  int ret;

  /* Decide, which power saving level can be obtained */

  newstate = pm_checkstate(PM_IDLE_DOMAIN);

  /* Check for state changes */

  if (newstate != oldstate)
    {
      flags = enter_critical_section();

      /* Perform board-specific, state-dependent logic here */

      _info("newstate= %d oldstate=%d\n", newstate, oldstate);

      /* Then force the global state change */

      ret = pm_changestate(PM_IDLE_DOMAIN, newstate);
      if (ret < 0)
        {
          /* The new state change failed, revert to the preceding state */

          pm_changestate(PM_IDLE_DOMAIN, oldstate);
        }
      else
        {
          /* Save the new state */

          oldstate = newstate;
        }

      /* MCU-specific power management logic */

      switch (newstate)
        {
          case PM_NORMAL:
            break;

          case PM_IDLE:
            break;

          case PM_STANDBY:

            /* Enter STOP mode */

            BEGIN_IDLE();
            stm32wb_pmstop(true);
            END_IDLE();

            /* Set correct clock again after returning from STOP */

            stm32wb_clockenable();

            /* Inform of all drivers of the new state */

            ret = pm_changestate(PM_IDLE_DOMAIN, PM_NORMAL);
            if (ret >= 0)
              {
                oldstate = PM_NORMAL;
              }

            break;

          case PM_SLEEP:
            stm32wb_pmstandby();
            break;

          default:
            break;
        }

      leave_critical_section(flags);
    }
}
#else
#  define up_idlepm()
#endif

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_idle
 *
 * Description:
 *   up_idle() is the logic that will be executed when there is no other
 *   ready-to-run task.  This is processor idle time and will continue until
 *   some interrupt occurs to cause a context switch from the idle task.
 *
 *   Processing in this state may be processor-specific. e.g., this is where
 *   power management operations might be performed.
 *
 ****************************************************************************/

void up_idle(void)
{
#if defined(CONFIG_SUPPRESS_INTERRUPTS) || defined(CONFIG_SUPPRESS_TIMER_INTS)
  /* If the system is idle and there are no timer interrupts, then process
   * "fake" timer interrupts. Hopefully, something will wake up.
   */

  nxsched_process_timer();
#else

  /* Perform IDLE mode power management */

  up_idlepm();

  /* Sleep until an interrupt occurs to save power. */

#if !(defined(CONFIG_DEBUG_SYMBOLS) && defined(CONFIG_STM32WB_DISABLE_IDLE_SLEEP_DURING_DEBUG))
  BEGIN_IDLE();
  __asm__ volatile ("wfi");
  END_IDLE();
#endif

#endif
}
