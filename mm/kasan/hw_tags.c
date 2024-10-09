/****************************************************************************
 * mm/kasan/hw_tags.c
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

#include <nuttx/mm/kasan.h>
#include <nuttx/compiler.h>
#include <nuttx/spinlock.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define KASAN_TAG_SHIFT 56


#define kasan_get_tag(addr) \
  ((uint8_t)((uint64_t)(addr) >> KASAN_TAG_SHIFT))

#define kasan_set_tag(addr, tag) \
  (FAR void *)((((uint64_t)(addr)) & ~((uint64_t)0xff << KASAN_TAG_SHIFT)) | \
               (((uint64_t)(tag)) << KASAN_TAG_SHIFT))

/****************************************************************************
 * Private Types
 ****************************************************************************/

enum kasan_arg {
	KASAN_ARG_DEFAULT,
	KASAN_ARG_OFF,
	KASAN_ARG_ON,
};

enum kasan_arg_mode {
	KASAN_ARG_MODE_DEFAULT,
	KASAN_ARG_MODE_SYNC,
	KASAN_ARG_MODE_ASYNC,
};

enum kasan_arg_stacktrace {
	KASAN_ARG_STACKTRACE_DEFAULT,
	KASAN_ARG_STACKTRACE_OFF,
	KASAN_ARG_STACKTRACE_ON,
};


/****************************************************************************
 * Private Functions
 ****************************************************************************/

static inline_function FAR uint8_t *
kasan_mem_to_shadow(FAR const void *ptr, size_t size)
{
  return NULL;
}

static inline_function bool
kasan_is_poisoned(FAR const void *addr, size_t size)
{
  return false;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

FAR void *kasan_reset_tag(FAR const void *addr)
{
  return (FAR void *)
         (((uint64_t)(addr)) & ~((uint64_t)0xff << KASAN_TAG_SHIFT));
}

void kasan_poison(FAR const void *addr, size_t size)
{
  /* mte_set_mem_tag_range(kasan_reset_tag(addr), size, mte_get_random_tag(), true); */
}

FAR void *kasan_unpoison(FAR const void *addr, size_t size)
{
  /* uint8_t value = mte_get_random_tag();
  mte_set_mem_tag_range(kasan_reset_tag(addr), size, value, false);
  return kasan_set_tag(addr, value); */
  return kasan_set_tag(addr, 0x00);
}

void kasan_register(FAR void *addr, FAR size_t *size)
{
}

void kasan_unregister(FAR void *addr)
{
}
