/****************************************************************************
 * boards/arm/cxd56xx/spresense/include/cxd56_clock.h
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

#ifndef __BOARDS_ARM_CXD56XX_SPRESENSE_INCLUDE_CXD56_CLOCK_H
#define __BOARDS_ARM_CXD56XX_SPRESENSE_INCLUDE_CXD56_CLOCK_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Name: board_clock_initialize
 *
 * Description:
 *   Initialize clock control.
 *   If this API is called and is returned, the clock mode is the highest
 *   clock mode. For example, CPU frequency is 156MHz.
 *
 ****************************************************************************/

void board_clock_initialize(void);

/****************************************************************************
 * Name: board_clock_enable
 *
 * Description:
 *   Enable dynamic clock control.
 *   If this API is called and is returned, the clock mode is the lowest
 *   clock mode. Later, you can control the clock mode dynamically by calling
 *   up_pm_acquire_freqlock() or up_pm_release_freqlock.
 *
 ****************************************************************************/

void board_clock_enable(void);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __BOARDS_ARM_CXD56XX_SPRESENSE_INCLUDE_CXD56_CLOCK_H */
