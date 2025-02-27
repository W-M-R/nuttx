/****************************************************************************
 * crypto/set_key.c
 *
 * SPDX-License-Identifier: SSLeay-standalone
 * SPDX-FileCopyrightText: Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * SPDX-FileCopyrightText: Eric Young (eay@mincom.oz.au).
 *
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by
 *    Eric Young (eay@mincom.oz.au)
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.
 * i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 *
 * set_key.c v 1.4 eay 24/9/91
 * 1.4 Speed up by 400% :-)
 * 1.3 added register declarations.
 * 1.2 unrolled make_key_sched a bit more
 * 1.1 added norm_expand_bits
 * 1.0 First working version
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <strings.h>

#include "des_locl.h"
#include "podd.h"
#include "sk.h"

static int check_parity(FAR des_cblock *key);

int des_check_key;

static int check_parity(FAR des_cblock *key)
{
  int i;

  for (i = 0; i < DES_KEY_SZ; i++)
  {
    if (*key[i] != odd_parity[*key[i]])
      {
        return 0;
      }
  }
  return 1;
}

/* Weak and semi week keys as take from
 * %A D.W. Davies
 * %A W.L. Price
 * %T Security for Computer Networks
 * %I John Wiley & Sons
 * %D 1984
 * Many thanks to smb@ulysses.att.com (Steven Bellovin) for the reference
 * (and actual cblock values).
 */

#define NUM_WEAK_KEY 16
static des_cblock weak_keys[NUM_WEAK_KEY] =
{
  /* weak keys */

  {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
  },
  {
    0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe
  },
  {
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f
  },
  {
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0
  },

  /* semi-weak keys */

  {
    0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe
  },
  {
    0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01, 0xfe, 0x01
  },
  {
    0x1f, 0xe0, 0x1f, 0xe0, 0x0e, 0xf1, 0x0e, 0xf1
  },
  {
    0xe0, 0x1f, 0xe0, 0x1f, 0xf1, 0x0e, 0xf1, 0x0e
  },
  {
    0x01, 0xe0, 0x01, 0xe0, 0x01, 0xf1, 0x01, 0xf1
  },
  {
    0xe0, 0x01, 0xe0, 0x01, 0xf1, 0x01, 0xf1, 0x01
  },
  {
    0x1f, 0xfe, 0x1f, 0xfe, 0x0e, 0xfe, 0x0e, 0xfe
  },
  {
    0xfe, 0x1f, 0xfe, 0x1f, 0xfe, 0x0e, 0xfe, 0x0e
  },
  {
    0x01, 0x1f, 0x01, 0x1f, 0x01, 0x0e, 0x01, 0x0e
  },
  {
    0x1f, 0x01, 0x1f, 0x01, 0x0e, 0x01, 0x0e, 0x01
  },
  {
    0xe0, 0xfe, 0xe0, 0xfe, 0xf1, 0xfe, 0xf1, 0xfe
  },
  {
    0xfe, 0xe0, 0xfe, 0xe0, 0xfe, 0xf1, 0xfe, 0xf1
  }
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int des_is_weak_key(FAR des_cblock *key)
{
  int i;

  for (i = 0; i < NUM_WEAK_KEY; i++)
    {
      /* Added == 0 to comparison, I obviously don't run
       * this section very often :-(, thanks to
       * engineering@MorningStar.Com for the fix
       * eay 93/06/29
       */

      if (bcmp(weak_keys[i], key, sizeof(des_cblock)) == 0)
        {
          return 1;
        }
    }

  return 0;
}

/* NOW DEFINED IN des_local.h
 * See ecb_encrypt.c for a pseudo description of these macros.
 * #define PERM_OP(a, b, t, n, m) ((t) = ((((a) >> (n))^(b)) & (m)),\
 *  (b)^=(t),\
 *  (a) = ((a)^((t) << (n))))
 */

#define HPERM_OP(a, t, n, m) ((t) = ((((a) << (16 - (n)))^(a)) & (m)),\
  (a) = (a)^(t)^(t >> (16 - (n))))

static int shifts2[16] =
{
  0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0
};

/* return 0 if key parity is odd (correct),
 * return -1 if key parity error,
 * return -2 if illegal weak key.
 */

int des_set_key(FAR des_cblock *key, des_key_schedule schedule)
{
  register uint32_t c;
  register uint32_t d;
  register uint32_t t;
  register uint32_t s;
  FAR register unsigned char *in;
  FAR register uint32_t *k;
  register int i;

  if (des_check_key)
    {
      if (!check_parity(key))
        {
          return(-1);
        }

      if (des_is_weak_key(key))
        {
          return(-2);
        }
    }

  k = (FAR uint32_t *)schedule;
  in = (FAR unsigned char *)key;

  c2l(in, c);
  c2l(in, d);

  /* do PC1 in 60 simple operations */

  /* PERM_OP(d, c, t, 4, 0x0f0f0f0fL);
   * HPERM_OP(c, t, -2, 0xcccc0000L);
   * HPERM_OP(c, t, -1, 0xaaaa0000L);
   * HPERM_OP(c, t, 8, 0x00ff0000L);
   * HPERM_OP(c, t, -1, 0xaaaa0000L);
   * HPERM_OP(d, t, -8, 0xff000000L);
   * HPERM_OP(d, t, 8, 0x00ff0000L);
   * HPERM_OP(d, t, 2, 0x33330000L);
   * d = ((d & 0x00aa00aaL) << 7L) |
   * ((d & 0x55005500L) >> 7L) | (d & 0xaa55aa55L);
   * d = (d >> 8) | ((c & 0xf0000000L) >> 4);
   * c &= 0x0fffffffL;
   */

  /* I now do it in 47 simple operations :-)
   * Thanks to John Fletcher (john_fletcher@lccmail.ocf.llnl.gov)
   * for the inspiration. :-)
   */

  PERM_OP (d, c, t, 4, 0x0f0f0f0fl);
  HPERM_OP(c, t, -2, 0xcccc0000l);
  HPERM_OP(d, t, -2, 0xcccc0000l);
  PERM_OP (d, c, t, 1, 0x55555555l);
  PERM_OP (c, d, t, 8, 0x00ff00ffl);
  PERM_OP (d, c, t, 1, 0x55555555l);
  d = (((d & 0x000000ffl) << 16l) | (d & 0x0000ff00l) |
      ((d & 0x00ff0000l) >> 16l) | ((c & 0xf0000000l) >> 4l));
  c &= 0x0fffffffl;

  for (i = 0; i < ITERATIONS; i++)
    {
      if (shifts2[i])
        {
          c = ((c >> 2l) | (c << 26l)); d = ((d >> 2l) | (d << 26l));
        }
      else
        {
          c = ((c >> 1l) | (c << 27l)); d = ((d >> 1l) | (d << 27l));
        }

      c &= 0x0fffffffl;
      d &= 0x0fffffffl;

      /* could be a few less shifts but I am to lazy at this
       * point in time to investigate
       */

      s = des_skb[0][(c) & 0x3f] |
          des_skb[1][((c >> 6) & 0x03) | ((c >> 7l) & 0x3c)] |
          des_skb[2][((c >> 13) & 0x0f) | ((c >> 14l) & 0x30)] |
          des_skb[3][((c >> 20) & 0x01) | ((c >> 21l) & 0x06) |
                ((c >> 22l) & 0x38)];
      t = des_skb[4][(d) & 0x3f] |
          des_skb[5][((d >> 7l) & 0x03) | ((d >> 8l) & 0x3c)] |
          des_skb[6][(d >> 15l) & 0x3f] |
          des_skb[7][((d >> 21l) & 0x0f) | ((d >> 22l) & 0x30)];

      /* table contained 0213 4657 */

      *(k++) = ((t << 16l) | (s & 0x0000ffffl)) & 0xffffffffl;
      s = ((s >> 16l) | (t & 0xffff0000l));

      s = (s << 4l) | (s >> 28l);
      *(k++) = s & 0xffffffffl;
    }

  return 0;
}
