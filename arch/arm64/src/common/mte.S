/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2020 ARM Ltd.
 */

#include <nuttx/link.h>
#include "arm64_arch.h"

	.arch	armv8.5-a+memtag

/*
 * multitag_transfer_size - set \reg to the block size that is accessed by the
 * LDGM/STGM instructions.
 */
	.macro	multitag_transfer_size, reg, tmp
	mrs	\reg, SYS_GMID_EL1
	ubfx	\reg, \reg, #SYS_GMID_EL1_BS_SHIFT, #SYS_GMID_EL1_BS_SIZE
	mov	\tmp, #4
	lsl	\reg, \tmp, \reg
	.endm

/*
 * mrs_s - Move value from system register to a general-purpose register
 * Replaces the external __emit_inst dependency
 */
	.macro mrs_s, rt, sreg
	mrs \rt, \sreg
	.endm



/*
 * Clear the tags in a page
 *   x0 - address of the page to be cleared
 */
mte_clear_page_tags:
	multitag_transfer_size x1, x2
1:	stgm	xzr, [x0]
	add	x0, x0, x1
	tst	x0, #(MTE_MM_SIZE - 1)
	b.ne	1b
	ret

/*
 * Zero the page and tags at the same time
 *
 * Parameters:
 *	x0 - address to the beginning of the page
 */
mte_zero_clear_page_tags:
	and	x0, x0, #(1 << MTE_TAG_SHIFT) - 1	// clear the tag
	mrs	x1, dczid_el0
	tbnz	x1, #4, 2f	// Branch if DC GZVA is prohibited
	and	w1, w1, #0xf
	mov	x2, #4
	lsl	x1, x2, x1

1:	dc	gzva, x0
	add	x0, x0, x1
	tst	x0, #(MTE_MM_SIZE - 1)
	b.ne	1b
	ret

2:	stz2g	x0, [x0], #(MTE_GRANULE_SIZE * 2)
	tst	x0, #(MTE_MM_SIZE - 1)
	b.ne	2b
	ret

/*
 * Copy the tags from the source page to the destination one
 *   x0 - address of the destination page
 *   x1 - address of the source page
 */
mte_copy_page_tags:
	mov	x2, x0
	mov	x3, x1
	multitag_transfer_size x5, x6
1:	ldgm	x4, [x3]
	stgm	x4, [x2]
	add	x2, x2, x5
	add	x3, x3, x5
	tst	x2, #(MTE_MM_SIZE - 1)
	b.ne	1b
	ret

/*
 * Read tags from a user buffer (one tag per byte) and set the corresponding
 * tags at the given kernel address. Used by PTRACE_POKEMTETAGS.
 *   x0 - kernel address (to)
 *   x1 - user buffer (from)
 *   x2 - number of tags/bytes (n)
 * Returns:
 *   x0 - number of tags read/set
 */
mte_copy_tags_from_user:
	mov	x3, x1
	cbz	x2, 2f
1:
	ldtrb	w4, [x1]		// Direct instruction to load from user space
	lsl	x4, x4, #MTE_TAG_SHIFT
	stg	x4, [x0], #MTE_GRANULE_SIZE
	add	x1, x1, #1
	subs	x2, x2, #1
	b.ne	1b

2:	sub	x0, x1, x3		// update the number of tags set
	ret

/*
 * Get the tags from a kernel address range and write the tag values to the
 * given user buffer (one tag per byte). Used by PTRACE_PEEKMTETAGS.
 *   x0 - user buffer (to)
 *   x1 - kernel address (from)
 *   x2 - number of tags/bytes (n)
 * Returns:
 *   x0 - number of tags read/set
 */
mte_copy_tags_to_user:
	mov	x3, x0
	cbz	x2, 2f
1:
	ldg	x4, [x1]
	ubfx	x4, x4, #MTE_TAG_SHIFT, #MTE_TAG_SIZE
	sttrb	w4, [x0]		// Direct instruction to store to user space
	add	x0, x0, #1
	add	x1, x1, #MTE_GRANULE_SIZE
	subs	x2, x2, #1
	b.ne	1b

2:	sub	x0, x0, x3		// update the number of tags copied
	ret

/*
 * Save the tags in a page
 *   x0 - page address
 *   x1 - tag storage
 */
mte_save_page_tags:
	multitag_transfer_size x7, x5
1:
	mov	x2, #0
2:
	ldgm	x5, [x0]
	orr	x2, x2, x5
	add	x0, x0, x7
	tst	x0, #0xFF		// 16 tag values fit in a register,
	b.ne	2b			// which is 16*16=256 bytes

	str	x2, [x1], #8

	tst	x0, #(MTE_MM_SIZE - 1)
	b.ne	1b
	ret

/*
 * Restore the tags in a page
 *   x0 - page address
 *   x1 - tag storage
 */
mte_restore_page_tags:
	multitag_transfer_size x7, x5
1:
	ldr	x2, [x1], #8
2:
	stgm	x2, [x0]
	add	x0, x0, x7
	tst	x0, #0xFF
	b.ne	2b

	tst	x0, #(MTE_MM_SIZE - 1)
	b.ne	1b
	ret
