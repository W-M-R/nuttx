/****************************************************************************
 * include/nuttx/usb/hub.h
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

#ifndef __INCLUDE_NUTTX_USB_HUB_H
#define __INCLUDE_NUTTX_USB_HUB_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Hub request types */

#define USBHUB_REQ_TYPE_HUB  (USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_DEVICE)
#define USBHUB_REQ_TYPE_PORT (USB_REQ_TYPE_CLASS | USB_REQ_RECIPIENT_OTHER)

/* Hub class requests */

#define USBHUB_REQ_GETSTATUS           USB_REQ_GETSTATUS
#define USBHUB_REQ_CLEARFEATURE        USB_REQ_CLEARFEATURE
#define USBHUB_REQ_SETFEATURE          USB_REQ_SETFEATURE
#define USBHUB_REQ_GETDESCRIPTOR       USB_REQ_GETDESCRIPTOR
#define USBHUB_REQ_SETDESCRIPTOR       USB_REQ_SETDESCRIPTOR
#define USBHUB_REQ_CLEARTTBUFFER       (0x08)
#define USBHUB_REQ_RESETTT             (0x09)
#define USBHUB_REQ_GETTTSTATE          (0x0a)
#define USBHUB_REQ_STOPTT              (0x0b)

/* Hub class features */

#define USBHUB_FEAT_CHUBLOCALPOWER     (0x0)
#define USBHUB_FEAT_CHUBOVERCURRENT    (0x1)

/* Port features */

#define USBHUB_PORT_FEAT_CONNECTION    (0x00)
#define USBHUB_PORT_FEAT_ENABLE        (0x01)
#define USBHUB_PORT_FEAT_SUSPEND       (0x02)
#define USBHUB_PORT_FEAT_OVERCURRENT   (0x03)
#define USBHUB_PORT_FEAT_RESET         (0x04)
#define USBHUB_PORT_FEAT_L1            (0x05)
#define USBHUB_PORT_FEAT_POWER         (0x08)
#define USBHUB_PORT_FEAT_LOWSPEED      (0x09)
#define USBHUB_PORT_FEAT_HIGHSPEED     (0x0a)
#define USBHUB_PORT_FEAT_CCONNECTION   (0x10)
#define USBHUB_PORT_FEAT_CENABLE       (0x11)
#define USBHUB_PORT_FEAT_CSUSPEND      (0x12)
#define USBHUB_PORT_FEAT_COVER_CURRENT (0x13)
#define USBHUB_PORT_FEAT_CRESET        (0x14)
#define USBHUB_PORT_FEAT_TEST          (0x15)
#define USBHUB_PORT_FEAT_INDICATOR     (0x16)
#define USBHUB_PORT_FEAT_CPORTL1       (0x17)

/* Hub characteristics */

#define USBHUB_CHAR_LPSM_SHIFT         (0)       /* Bits 0-1: Logical Power Switching Mode */
#define USBHUB_CHAR_LPSM_MASK          (3 << USBHUB_CHAR_LPSM_SHIFT)
#  define USBHUB_CHAR_LPSM_GANGED      (0 << USBHUB_CHAR_LPSM_SHIFT)
#  define USBHUB_CHAR_LPSM_INDIVIDUAL  (1 << USBHUB_CHAR_LPSM_SHIFT)
#define USBHUB_CHAR_COMPOUND           (1 << 2)  /* Bit 2: Compound device */
#define USBHUB_CHAR_OCPM_SHIFT         (3)       /* Bits 3-4: Over-current Protection Mode */
#define USBHUB_CHAR_OCPM_MASK          (3 << USBHUB_CHAR_OCPM_SHIFT)
#  define USBHUB_CHAR_OCPM_GLOBAL      (0 << USBHUB_CHAR_OCPM_SHIFT)
#  define USBHUB_CHAR_OCPM_INDIVIDUAL  (1 << USBHUB_CHAR_OCPM_SHIFT)
#define USBHUB_CHAR_TTTT_SHIFT         (5)       /* Bits 5-6: TT Think Time */
#define USBHUB_CHAR_TTTT_MASK          (3 << USBHUB_CHAR_TTTT_SHIFT)
#  define USBHUB_CHAR_TTTT_8_BITS      (0 << USBHUB_CHAR_TTTT_SHIFT)
#  define USBHUB_CHAR_TTTT_16_BITS     (1 << USBHUB_CHAR_TTTT_SHIFT)
#  define USBHUB_CHAR_TTTT_24_BITS     (2 << USBHUB_CHAR_TTTT_SHIFT)
#  define USBHUB_CHAR_TTTT_32_BITS     (3 << USBHUB_CHAR_TTTT_SHIFT)
#define USBHUB_CHAR_PORTIND            (1 << 7)  /* Bit 7: Port Indicators Supported */

/* Hub status */

#define USBHUB_STAT_LOCALPOWER         (1 << 0)
#define USBHUB_STAT_OVERCURRENT        (1 << 1)

/* Hub status change */

#define USBHUB_STAT_CLOCALPOWER        (1 << 0)
#define USBHUB_STAT_COVERCURRENT       (1 << 1)

/* Hub port status */

#define USBHUB_PORT_STAT_CONNECTION    (1 << 0)
#define USBHUB_PORT_STAT_ENABLE        (1 << 1)
#define USBHUB_PORT_STAT_SUSPEND       (1 << 2)
#define USBHUB_PORT_STAT_OVERCURRENT   (1 << 3)
#define USBHUB_PORT_STAT_RESET         (1 << 4)
#define USBHUB_PORT_STAT_L1            (1 << 5)

#define USBHUB_PORT_STAT_POWER         (1 << 8)
#define USBHUB_PORT_STAT_LOW_SPEED     (1 << 9)
#define USBHUB_PORT_STAT_HIGH_SPEED    (1 << 10)
#define USBHUB_PORT_STAT_TEST          (1 << 11)
#define USBHUB_PORT_STAT_INDICATOR     (1 << 12)

/* Hub port status change */

#define USBHUB_PORT_STAT_CCONNECTION   (1 << 0)
#define USBHUB_PORT_STAT_CENABLE       (1 << 1)
#define USBHUB_PORT_STAT_CSUSPEND      (1 << 2)
#define USBHUB_PORT_STAT_COVERCURRENT  (1 << 3)
#define USBHUB_PORT_STAT_CRESET        (1 << 4)
#define USBHUB_PORT_STAT_CL1           (1 << 5)

/* Hub descriptor type */

#define USB_DESC_TYPE_HUB              (USB_REQ_TYPE_CLASS | USB_CLASS_HUB)

/* Hub max ports */

#define USBHUB_MAX_PORTS               (7)

/****************************************************************************
 * Public Types
 ****************************************************************************/

/* Hub descriptor */

struct usb_hubdesc_s
{
  uint8_t  len;
  uint8_t  type;
  uint8_t  nports;
  uint8_t  characteristics[2];
  uint8_t  pwrondelay;
  uint8_t  ctrlcurrent;
  uint8_t  devattached;
  uint8_t  pwrctrlmask;
};
#define USB_SIZEOF_HUBDESC 9

/* Hub status */

struct usb_hubstatus_s
{
  uint8_t status[2];
  uint8_t change[2];
};
#define USB_SIZEOF_HUBSTS 4

/* Hub port status */

struct usb_portstatus_s
{
  uint8_t status[2];
  uint8_t change[2];
};
#define USB_SIZEOF_PORTSTS 4

/* Hub transaction translator */

struct usb_hubtt_s
{
  /* Hub class that is the transaction translator for device */

  FAR struct usbhost_class_s *class;

  /* Transaction translator think time */

  uint16_t thinktime;
};

#endif /* __INCLUDE_NUTTX_USB_HUB_H */
