/****************************************************************************
 * arch/arm/src/sama5/sam_isi.c
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

#include <debug.h>

#include "sam_memories.h"
#include "sam_pio.h"
#include "sam_pck.h"
#include "sam_isi.h"

#ifdef CONFIG_SAMA5_ISI

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* The sensor master clock (ISI_MCK) is generated by the Advanced Power
 * Management Controller (APMC) through a Programmable Clock output.
 */

#ifndef CONFIG_ISI_MCKFREQ
#  error "CONFIG_ISI_MCKFREQ must be defined"
#endif

#if defined(CONFIG_ISI_PCK0)
#  define ISI_PCKID PCK0
#elif defined(CONFIG_ISI_PCK1)
#  define ISI_PCKID PCK1
#elif defined(CONFIG_ISI_PCK2)
#  define ISI_PCKID PCK2
#else
#  error "No valid PCK selection"
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/* This structure defines the overall state of the ISI interface */

struct sam_isi_s
{
  uint32_t actual;  /* Actual ISI_MCK frequency */
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* ISI interface state */

static struct sam_isi_s g_isi;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Function: sam_isi_initialize
 *
 * Description:
 *   Initialize the ISI driver.
 *
 * Input Parameters:
 *   None
 *
 * Returned Value:
 *   OK on success; Negated errno on failure.
 *
 * Assumptions:
 *   Called very early in the initialization sequence.
 *
 ****************************************************************************/

int sam_isi_initialize(void)
{
  int ret;

  /* Configure PIO pins for the ISI (outputs) */

  /* Data pins */

  sam_configpio(PIO_ISI_D0);
  sam_configpio(PIO_ISI_D1);
  sam_configpio(PIO_ISI_D2);
  sam_configpio(PIO_ISI_D3);
  sam_configpio(PIO_ISI_D4);
  sam_configpio(PIO_ISI_D5);
  sam_configpio(PIO_ISI_D6);
  sam_configpio(PIO_ISI_D7);
  sam_configpio(PIO_ISI_D8);
  sam_configpio(PIO_ISI_D9);
  sam_configpio(PIO_ISI_D10);
  sam_configpio(PIO_ISI_D11);

  /* Horizontal and vertical sync pins (inputs) */

  sam_configpio(PIO_ISI_HSYNC);
  sam_configpio(PIO_ISI_VSYNC);

  /* Pixel clock input (ISI_PCK, not to be confused with the processor clock
   * (PCK) or the programmable clock (PCK).
   *
   * NOTE: "Several parts of the ISI controller use the pixel clock provided
   * by the image sensor (ISI_PCK). Thus the user must first program the
   * image sensor to provide this clock (ISI_PCK) before programming the
   * Image Sensor Controller."
   */

  sam_configpio(PIO_ISI_PCK);

  /* Configure ISI_MCK programmable clock output.
   *
   * REVISIT:  Might this not be needed before the image sensor is
   * initialized?
   */

  g_isi.actual = sam_pck_configure(ISI_PCKID, PCKSRC_MCK,
                                   CONFIG_ISI_MCKFREQ);
  ginfo("PCK%d frequency=%d actual=%d\n",
        ISI_PCKID, CONFIG_ISI_MCKFREQ, g_isi.actual);

  /* Enable the MCK (output) */

  sam_pck_enable(ISI_PCKID, true);

  /* Configure the pixel clock */
#warning Missing logic

  /* Configure color */
#warning Missing logic

  /* Configure decimation */
#warning Missing logic

  /* Configure DMA */
#warning Missing logic
}

#endif /* CONFIG_SAMA5_ISI */
