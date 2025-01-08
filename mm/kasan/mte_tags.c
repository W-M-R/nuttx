/****************************************************************************
 * mm/kasan/mte_tags.c
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

#include <nuttx/mm/mm.h>
#include <nuttx/mm/kasan.h>

#include <arch/mte.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

static void *kasan_set_poison(FAR const void *addr,
                              size_t size,
                              uint8_t tag)
{
  FAR const void *tag_addr;

  /* Get random labels and the addresses after labeling */

  tag_addr = arm64_mte_get_tagged_addr(addr, tag);

  /* Add MTE hardware label to memory block */

  arm64_mte_set_tag(tag_addr, size);

  return (FAR void *)tag_addr;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void kasan_hw_open(void)
{
  arm64_mte_enable();
}

void kasan_hw_close(void)
{
  arm64_mte_disable();
}

FAR void *kasan_reset_tag(FAR const void *addr)
{
  return arm64_mte_get_untagged_addr(addr);
}

void kasan_poison(FAR const void *addr, size_t size)
{
  uint8_t tag = arm64_mte_get_random_tag(addr);

  kasan_set_poison(addr, size, tag);
}

FAR void *kasan_unpoison(FAR const void *addr, size_t size)
{
  uint8_t tag = arm64_mte_get_random_tag(addr);

  return kasan_set_poison(addr, size, tag);
}

void kasan_register(FAR void *addr, FAR size_t *size)
{
  uint8_t tag = arm64_mte_get_random_tag(addr);

  kasan_set_poison(addr, *size, tag);
}

void kasan_unregister(FAR void *addr)
{
}
