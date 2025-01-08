/****************************************************************************
 * arch/arm64/include/mte.h
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

#ifndef ___ARCH_ARM64_SRC_COMMON_ARM64_MTE_H
#define ___ARCH_ARM64_SRC_COMMON_ARM64_MTE_H

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/* Initialize MTE settings and enable memory tagging */

void arm64_mte_init(void);

/* Enable MTE by setting the TCF1 bit in SCTLR_EL1 */

void arm64_mte_enable(void);

/* Disable MTE by clearing the TCF1 bit in SCTLR_EL1 */

void arm64_mte_disable(void);

/* Set memory tags for a given memory range */

void arm64_mte_set_tag(const void *addr, size_t size);

/* Get a random label based on the address through the mte register */

uint8_t arm64_mte_get_random_tag(const void *addr);

/* Get the address without label */

FAR void *arm64_mte_get_untagged_addr(const void *addr);

/* Get the address with label */

FAR void *arm64_mte_get_tagged_addr(const void *addr, uint8_t tag);

#endif /* ___ARCH_ARM64_SRC_COMMON_ARM64_MTE_H */
