/****************************************************************************
 * boards/arm/samd5e5/metro-m4/src/metro-m4.h
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

#ifndef __BOARDS_ARM_SAMD5E5_METRO_M4_SRC_METRO_M4_H
#define __BOARDS_ARM_SAMD5E5_METRO_M4_SRC_METRO_M4_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Metro-M4 GPIOs ***********************************************************/

/* LEDs
 *
 * The Adafruit Metro M4 has four LEDs, but only two are controllable by
 * software:
 *
 *   1. The red LED on the Arduino D13 pin, and
 *   2. A NeoPixel RGB LED.
 *
 * Currently, only the red LED is supported.
 *
 *   ------ ----------------- -----------
 *   SHIELD SAMD5E5           FUNCTION
 *   ------ ----------------- -----------
 *   D13    PA16              GPIO output
 */

#define PORT_RED_LED (PORT_OUTPUT  | PORT_PULL_NONE | PORT_OUTPUT_SET | PORTA | PORT_PIN16)

#define PORT_D8   (PORT_OUTPUT     | PORT_PULL_NONE | PORT_OUTPUT_SET | PORTA | PORT_PIN21)
#define PORT_D9   (PORT_INTERRUPT  | PORT_PULL_NONE | PORT_INT_RISING | PORTA | PORT_PIN20)
#define PORT_D10  (PORT_INPUT      | PORT_PULL_NONE | PORT_INT_CHANGE | PORTA | PORT_PIN18)

/* GPIO pins used by the GPIO Subsystem */

#define BOARD_NGPIOIN     1 /* Amount of GPIO Input pins */
#define BOARD_NGPIOOUT    1 /* Amount of GPIO Output pins */
#define BOARD_NGPIOINT    1 /* Amount of GPIO Input w/ Interruption pins */

#define HAVE_AT24 1

/* AT24 Serial EEPROM */

#define AT24_I2C_BUS   5 /* AT24C256 connected to I2C5 */
#define AT24_MINOR     0

#if !defined(CONFIG_MTD_AT24XX) || !defined(CONFIG_SAMD5E5_SERCOM5_ISI2C)
#  undef HAVE_AT24
#endif

/* Can't support AT24 features if mountpoints are disabled or if we were not
 * asked to mount the AT25 part
 */

#if defined(CONFIG_DISABLE_MOUNTPOINT) || \
   !defined(CONFIG_METRO_M4_AT24_BLOCKMOUNT)
#  undef HAVE_AT24
#endif

/* On-chip Programming Memory */

#if defined(CONFIG_SAMD5E5_PROGMEM) || defined(CONFIG_MTD_PROGMEM)
#define HAVE_PROGMEM_CHARDEV
#endif

/* This is the on-chip progmem memory driver minor number */

#define PROGMEM_MTD_MINOR 0

/****************************************************************************
 * Public Types
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************
 * Public Functions Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: sam_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y && CONFIG_BOARDCTL=y :
 *     Called from the NSH library
 *
 ****************************************************************************/

int sam_bringup(void);

/****************************************************************************
 * Name: sam_led_pminitialize
 *
 * Description:
 *   Register LED power management features.
 *
 ****************************************************************************/

#ifdef CONFIG_PM
void sam_led_pminitialize(void);
#endif

#ifdef CONFIG_METRO_M4_USB_AUTOMOUNT
void sam_automount_initialize(void);
void sam_automount_event(bool inserted);
#endif

#ifdef CONFIG_SAMD5E5_SERCOM5_ISI2C
struct i2c_master_s *g_i2c5_dev;
int metro_m4_i2cdev_initialize(void);
#endif

#ifdef CONFIG_FS_SMARTFS
int sam_smartfs_initialize(void);
#endif

#ifdef CONFIG_BQ27426
int sam_bq27426_initialize(const char *devname);
#endif

#ifdef CONFIG_DEV_GPIO
int sam_gpio_initialize(void);
#endif

#endif /* __ASSEMBLY__ */
#endif /* __BOARDS_ARM_SAMD5E5_METRO_M4_SRC_METRO_M4_H */
