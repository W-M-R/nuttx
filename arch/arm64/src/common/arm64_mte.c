/****************************************************************************
 * arch/arm64/src/common/arm64_mte.c
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
#include <arch/limits.h>
#include <nuttx/sched.h>

#include "arm64_arch.h"

/***************************************************************************
 * Pre-processor Definitions
 ***************************************************************************/

#ifdef ARM64_ASM_ARCH
#define ARM64_ASM_PREAMBLE ".arch " ARM64_ASM_ARCH "\n"
#else
#define ARM64_ASM_PREAMBLE
#endif

#define MTE_GRANULE_MASK	(~(MTE_GRANULE_SIZE - 1))
#define MTE_GRANULES_PER_PAGE	(PAGE_SIZE / MTE_GRANULE_SIZE)
#define MTE_TAG_SHIFT		56
#define MTE_TAG_SIZE		4
#define MTE_TAG_MASK		GENMASK((MTE_TAG_SHIFT + (MTE_TAG_SIZE - 1)), MTE_TAG_SHIFT)
#define MTE_PAGE_TAG_STORAGE	(MTE_GRANULES_PER_PAGE * MTE_TAG_SIZE / 8)

#define __MTE_PREAMBLE		ARM64_ASM_PREAMBLE ".arch_extension memtag\n"

static inline uint64_t sign_extend64(uint64_t value, int index)
{
	uint8_t shift = 63 - index;
	return (uint64_t)(value << shift) >> shift;
}

#define __tag_shifted(tag)	((uint64_t)(tag) << 56)
#define __tag_reset(addr)	__untagged_addr(addr)
#define __tag_get(addr)		(uint8_t)((uint64_t)(addr) >> 56)

static inline const void *__tag_set(const void *addr, uint8_t tag)
{
	uint64_t __addr = (uint64_t)addr & ~__tag_shifted(0xff);
	return (const void *)(__addr | __tag_shifted(tag));
}

/***************************************************************************
 * Private Functions
 ***************************************************************************/

int arm64_mte_is_support(void) {
    int mte_supported;

    __asm__ volatile (
        "mrs %0, ID_AA64PFR1_EL1\n" // 读取 ID_AA64PFR1_EL1 寄存器
        "ubfx %0, %0, #16, #4\n"    // 提取 MTE 支持字段 [19:16]
        : "=r" (mte_supported)       // 输出到 mte_supported 变量
        :
        : "memory"                   // 声明内存被修改
    );

    return (mte_supported != 0);      // 返回 1 (支持) 或 0 (不支持)
}

void sctlrel1_set(uint64_t value)
{
  uint64_t val;
  val = read_sysreg(sctlr_el1);
  write_sysreg(val | value, sctlr_el1);
}

void sctlrel1_clear(uint64_t value)
{
  uint64_t val;
  val = read_sysreg(sctlr_el1);
  write_sysreg(val & ~(value), sctlr_el1);
}

/*
 * Memory types available.
 *
 * IMPORTANT: MT_NORMAL must be index 0 since vm_get_page_prot() may 'or' in
 *	      the MT_NORMAL_TAGGED memory type for PROT_MTE mappings. Note
 *	      that protection_map[] only contains MT_NORMAL attributes.
 */
#define MT_NORMAL		0
#define MT_NORMAL_TAGGED	1
#define MT_NORMAL_NC		2
#define MT_DEVICE_nGnRnE	3
#define MT_DEVICE_nGnRE		4

/* MAIR_ELx memory attributes (used by Linux) */
#define MAIR_ATTR_DEVICE_nGnRnE		0x00
#define MAIR_ATTR_DEVICE_nGnRE		0x04
#define MAIR_ATTR_NORMAL_NC		0x44
#define MAIR_ATTR_NORMAL_TAGGED		0xf0
#define MAIR_ATTR_NORMAL		0xff
#define MAIR_ATTR_MASK			0xff

/* Position the attr at the correct index */
#define MAIR_ATTRIDX(attr, idx)		((attr) << ((idx) * 8))

void mte_cpu_setup(void)
{
  sctlrel1_clear(SCTLR_EL1_ATA | SCTLR_EL1_ATA0);

	/* Normal Tagged memory type at the corresponding MAIR index */
	modify_sysreg(MAIR_ATTRIDX(MAIR_ATTR_NORMAL_TAGGED,
				               MT_NORMAL_TAGGED),
	              MAIR_ATTRIDX(MAIR_ATTR_MASK, MT_NORMAL_TAGGED),
			      mair_el1);
}

void mte_enable_kernel_tcf(void)
{
  sctlrel1_set(SCTLR_EL1_TCF);
  sctlrel1_set(SCTLR_EL1_TCF0);
}

uint8_t mte_read_kernel_tcf(void)
{
  return (uint8_t)(read_sysreg(sctlr_el1) >> 40 & 0xff);
}

static void arm64_set_tco(uint8_t enable)
{
  __asm__ volatile (
    "msr TCO, %0"
    :
    : "r"(enable)
    : "memory"
  );
}

/***************************************************************************
 * Public Functions
 ***************************************************************************/

void arm64_mte_disable_tco(void)
{
	arm64_set_tco(0);
}

void arm64_mte_enable_tco(void)
{
	arm64_set_tco(1);
}

uint8_t mte_get_ptr_tag(void *ptr)
{
	/* Note: The format of KASAN tags is 0xF<x> */

	uint8_t tag = 0xF0 | (uint8_t)(((uint64_t)(ptr)) >> MTE_TAG_SHIFT);

	return tag;
}

/* Get allocation tag for the address. */

uint8_t mte_get_mem_tag(void *addr)
{
	asm(__MTE_PREAMBLE "ldg %0, [%0]"
		: "+r" (addr));

	return mte_get_ptr_tag(addr);
}

uint8_t mte_get_random_tag(void)
{
	void *addr;

	asm(__MTE_PREAMBLE "irg %0, %0"
		: "=r" (addr));

	return mte_get_ptr_tag(addr);
}

uint64_t __stg_post(uint64_t p)
{
	asm volatile(__MTE_PREAMBLE "stg %0, [%0], #16"
		     : "+r"(p)
		     :
		     : "memory");
	return p;
}

uint64_t __stzg_post(uint64_t p)
{
	asm volatile(__MTE_PREAMBLE "stzg %0, [%0], #16"
		     : "+r"(p)
		     :
		     : "memory");
	return p;
}

void __dc_gva(uint64_t p)
{
	asm volatile(__MTE_PREAMBLE "dc gva, %0" : : "r"(p) : "memory");
}

void __dc_gzva(uint64_t p)
{
	asm volatile(__MTE_PREAMBLE "dc gzva, %0" : : "r"(p) : "memory");
}

/*
 * Assign allocation tags for a region of memory based on the pointer tag.
 * Note: The address must be non-NULL and MTE_GRANULE_SIZE aligned and
 * size must be MTE_GRANULE_SIZE aligned.
 */

void mte_set_mem_tag_range(void *addr, size_t size, uint8_t tag,
					 bool init)
{
	uint64_t curr, mask, dczid, dczid_bs, dczid_dzp, end1, end2, end3;

	/* Read DC G(Z)VA block size from the system register. */
	dczid = this_cpu();
	dczid_bs = 4ul << (dczid & 0xf);
	dczid_dzp = (dczid >> 4) & 1;

	curr = (uint64_t)__tag_set(addr, tag);
	mask = dczid_bs - 1;
	/* STG/STZG up to the end of the first block. */
	end1 = curr | mask;
	end3 = curr + size;
	/* DC GVA / GZVA in [end1, end2) */
	end2 = end3 & ~mask;

	/*
	 * The following code uses STG on the first DC GVA block even if the
	 * start address is aligned - it appears to be faster than an alignment
	 * check + conditional branch. Also, if the range size is at least 2 DC
	 * GVA blocks, the first two loops can use post-condition to save one
	 * branch each.
	 */
#define SET_MEMTAG_RANGE(stg_post, dc_gva)		\
	do {						\
		if (!dczid_dzp && size >= 2 * dczid_bs) {\
			do {				\
				curr = stg_post(curr);	\
			} while (curr < end1);		\
							\
			do {				\
				dc_gva(curr);		\
				curr += dczid_bs;	\
			} while (curr < end2);		\
		}					\
							\
		while (curr < end3)			\
			curr = stg_post(curr);		\
	} while (0)

	if (init)
		SET_MEMTAG_RANGE(__stzg_post, __dc_gzva);
	else
		SET_MEMTAG_RANGE(__stg_post, __dc_gva);
#undef SET_MEMTAG_RANGE
}