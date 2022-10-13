#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "deflate.h"

#define memzero(s, n)     memset ((voidp)(s), 0, (n))
#define WSIZE 0x8000      // window size--must be a power of two

static unsigned       insize;                           // delka vstupnich dat
static const char*    inbuf;                            // vstupni data - je definovano v cFile.data

typedef char*          voidp;
typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

extern int xwrite (int fd, char * buf, int len);

typedef struct {
  int      outfd;
  unsigned inptr;     // index, ukazujici na vstupni data
  unsigned wp;        // write index
  uch window [WSIZE+1];   // data na vystupu
  ulg bb;                 // bit buffer
  unsigned bk;            // bits in bit buffer
  unsigned hufts;         // track memory usage
} zip_t;

struct huft {
  uch e;                // number of extra bits or operation
  uch b;                // number of bits in this code or subcode
  union {
    ush n;              // literal, length base, or distance base
    struct huft *t;     // pointer to next level of table
  } v;
}__attribute__((__packed__));

#define LBITS 9         // bits in base literal/length lookup table
#define DBITS 6         // bits in base distance lookup table
/*
int sysopen (const char * name, int flag, mode_t mode) {
  return open(name, flag, mode);
}
*/
// tady to je vlastni vystup
void flush_output (zip_t* pz, unsigned w) {
  int i;
  pz->wp = w;
  i = xwrite (pz->outfd, (char*) pz->window, w);
  (void) i;
}
// nacteni bytu ze vstupu
int get_byte (zip_t* pz) {
  if (pz->inptr > insize) return -1;
  return (int) inbuf [pz->inptr++];
}

static const unsigned border[] = {    // Order of the bit length code lengths
  16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};
static const ush cplens[] = {         // Copy lengths for literal codes 257..285
  3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
  35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0
};
// note: see note #13 above about the 258 in this list.
static const ush cplext[] = {         // Extra bits for literal codes 257..285
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
  3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 99, 99
}; // 99==invalid
static const ush cpdist[] = {         // Copy offsets for distance codes 0..29
  1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
  257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
  8193, 12289, 16385, 24577
};
static const ush cpdext[] = {         // Extra bits for distance codes
  0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
  7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
  12, 12, 13, 13
};

static const ush mask_bits[] = {
  0x0000,
  0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
  0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

#define NEXTBYTE()  (uch)get_byte(pz)
#define NEEDBITS(n) {while(k<(n)){b|=((ulg)NEXTBYTE())<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

// If BMAX needs to be larger than 16, then h and x[] should be ulg.
#define BMAX 16         // maximum bit length of any code (16 for explode)
#define N_MAX 288       // maximum number of codes in any set

int huft_free (zip_t* pz, struct huft *t) {
  register struct huft *p, *q;

  p = t;
  while (p != (struct huft *) NULL) {
    q = (--p)->v.t;
    free ( (char*) p);
    p = q;
  }
  return 0;
}
int huft_build (zip_t* pz, unsigned *b, unsigned n, unsigned s,
                const ush *d, const ush *e, struct huft **t, int *m) {
  unsigned a;                   // counter for codes of length k
  unsigned c[BMAX+1];           // bit length count table
  unsigned f;                   // i repeats in table every f entries
  int g;                        // maximum code length
  int h;                        // table level
  register unsigned i;          // counter, current code
  register unsigned j;          // counter
  register int k;               // number of bits in current code
  int l;                        // bits per table (returned in m)
  register unsigned *p;         // pointer into c[], b[], or v[]
  register struct huft *q;      // points to current table
  struct huft r;                // table entry for structure assignment
  struct huft *u[BMAX];         // table stack
  unsigned v[N_MAX];            // values in order of bit length
  register int w;               // bits before this table == (l * h)
  unsigned x[BMAX+1];           // bit offsets, then code stack
  unsigned *xp;                 // pointer into x
  int y;                        // number of dummy codes added
  unsigned z;                   // number of entries in current table

  memzero (c, sizeof (c));
  p = b;
  i = n;
  do {
    c[*p]++;                    // assume all entries <= BMAX
    p++;                      // Can't combine with above line (Solaris bug)
  } while (--i);
  if (c[0] == n) {              // null input--all zero length codes
    *t = (struct huft *) NULL;
    *m = 0;
    return 0;
  }

  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                        // minimum code length
  if ( (unsigned) l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                        // maximum code length
  if ( (unsigned) l > i)
    l = i;
  *m = l;

  for (y = 1 << j; j < i; j++, y <<= 1)
    if ( (y -= c[j]) < 0)
      return 2;                 // bad input: more codes than bits
  if ( (y -= c[i]) < 0)
    return 2;
  c[i] += y;

  x[1] = j = 0;
  p = c + 1;
  xp = x + 2;
  while (--i) {                 // note that i == g from above
    *xp++ = (j += *p++);
  }

  p = b;
  i = 0;
  do {
    if ( (j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);

  x[0] = i = 0;                 // first Huffman code is zero
  p = v;                        // grab values in bit order
  h = -1;                       // no tables yet--level -1
  w = -l;                       // bits decoded == (l * h)
  u[0] = (struct huft *) NULL;  // just to keep compilers happy
  q = (struct huft *) NULL;     // ditto
  z = 0;                        // ditto

  for (; k <= g; k++) {
    a = c[k];
    while (a--) {
      while (k > w + l) {
        h++;
        w += l;                 // previous table always l bits
        z = (z = g - w) > (unsigned) l ? l : z; // upper limit on table size
        if ( (f = 1 << (j = k - w)) > a + 1) {  // try a k-w bit table
          // too few codes for k-w bit table
          f -= a + 1;           // deduct codes from patterns left
          xp = c + k;
          while (++j < z) {     // try smaller tables up to z bits
            if ( (f <<= 1) <= *++xp)
              break;            // enough codes to use up j bits
            f -= *xp;           // else deduct codes from patterns
          }
        }
        z = 1 << j;             // table entries for j-bit table

        q = (struct huft*) malloc ((z + 1) * sizeof (struct huft));
        if (q == NULL) {
          if (h)
            huft_free (pz, u[0]);
          return 3;             // not enough memory
        }
        pz->hufts += z + 1;         // track memory usage
        *t = q + 1;             // link to list for huft_free()
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif // __clang__
         t = &(q->v.t);
#pragma GCC diagnostic pop
        *t = (struct huft *) NULL;
        u[h] = ++q;             // table starts after link

        if (h) {
          x[h] = i;             // save pattern for backing up
          r.b = (uch) l;        // bits to dump before this table
          r.e = (uch) (16 + j); // bits in this table
          r.v.t = q;            // pointer to this table
          j = i >> (w - l);     // (get around Turbo C bug)
          memcpy (u[h-1] + j, &r, sizeof (struct huft));
        }
      }

      r.b = (uch) (k - w);
      if (p >= v + n)
        r.e = 99;               // out of values--invalid code
      else if (*p < s) {
        r.e = (uch) (*p < 256 ? 16 : 15);   // 256 is end-of-block code
        r.v.n = (ush) (*p);            // simple code is just the value
        p++;                           // one compiler does not like *p++
      } else {
        r.e = (uch) e[*p - s];  // non-simple--look up in lists
        r.v.n = d[*p++ - s];
      }

      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        memcpy (q + j, &r, sizeof (struct huft));

      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

      while ( (i & ( (1 << w) - 1)) != x[h]) {
        h--;                    // don't need to update q
        w -= l;
      }
    }
  }

  return y != 0 && g != 1;
}


int inflate_codes (zip_t* pz, struct huft *tl, struct huft *td, int bl, int bd) {
  register unsigned e;  // table entry flag/number of extra bits
  unsigned n, d;        // length and index for copy
  unsigned w;           // current window position
  struct huft *t;       // pointer to table entry
  unsigned ml, md;      // masks for bl and bd bits
  register ulg b;       // bit buffer
  register unsigned k;  // number of bits in bit buffer

  b = pz->bb;                       // initialize bit buffer
  k = pz->bk;
  w = pz->wp;                       // initialize window position

  ml = mask_bits[bl];           // precompute masks for speed
  md = mask_bits[bd];
  for (;;) {                    // do until end of block
    NEEDBITS ( (unsigned) bl)
    if ( (e = (t = tl + ( (unsigned) b & ml))->e) > 16)
      do {
        if (e == 99)
          return 1;
        DUMPBITS (t->b)
        e -= 16;
        NEEDBITS (e)
      } while ( (e = (t = t->v.t + ( (unsigned) b & mask_bits[e]))->e) > 16);
    DUMPBITS (t->b)
    if (e == 16) {              // then it's a literal
      assert (w <= WSIZE);
      pz->window[w++] = (uch) t->v.n;
      if (w == WSIZE) {
        flush_output (pz, w);
        w = 0;
      }
    } else {                     // it's an EOB or a length
      if (e == 15)
        break;

      NEEDBITS (e)
      n = t->v.n + ( (unsigned) b & mask_bits[e]);
      DUMPBITS (e);

      NEEDBITS ( (unsigned) bd)
      if ( (e = (t = td + ( (unsigned) b & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS (t->b)
          e -= 16;
          NEEDBITS (e)
        } while ( (e = (t = t->v.t + ( (unsigned) b & mask_bits[e]))->e) > 16);
      DUMPBITS (t->b)
      NEEDBITS (e)
      d = w - t->v.n - ( (unsigned) b & mask_bits[e]);
      DUMPBITS (e)
      do {
        n -= (e = (e = WSIZE - ( (d &= WSIZE - 1) > w ? d : w)) > n ? n : e);
        do {
          assert (w < WSIZE);
          pz->window[w++] = pz->window[d++];
        } while (--e);
        if (w == WSIZE) {
          flush_output (pz, w);
          w = 0;
        }
      } while (n);
    }
  }

  pz->wp = w;                       // restore global window pointer
  pz->bb = b;                       // restore global bit buffer
  pz->bk = k;

  // done
  return 0;
}
int inflate_stored (zip_t* pz) {
  unsigned n;           // number of bytes in block
  unsigned w;           // current window position
  register ulg b;       // bit buffer
  register unsigned k;  // number of bits in bit buffer

  b = pz->bb;                       // initialize bit buffer
  k = pz->bk;
  w = pz->wp;                       // initialize window position

  n = k & 7;
  DUMPBITS (n);

  NEEDBITS (16)
  n = ( (unsigned) b & 0xffff);
  DUMPBITS (16)
  NEEDBITS (16)
  if (n != (unsigned) ( (~b) & 0xffff))
    return 1;                   // error in compressed data
  DUMPBITS (16)
  while (n--) {
    NEEDBITS (8)
    assert (w < WSIZE);
    pz->window[w++] = (uch) b;
    if (w == WSIZE) {
      flush_output (pz, w);
      w = 0;
    }
    DUMPBITS (8)
  }
  pz->wp = w;                       // restore global window pointer
  pz->bb = b;                       // restore global bit buffer
  pz->bk = k;
  return 0;
}
int inflate_fixed (zip_t* pz) {
  int i;                // temporary variable
  struct huft *tl;      // literal/length code table
  struct huft *td;      // distance code table
  int bl;               // lookup bits for tl
  int bd;               // lookup bits for td
  unsigned l[288];      // length list for huft_build

  for (i = 0; i < 144; i++)
    l[i] = 8;
  for (; i < 256; i++)
    l[i] = 9;
  for (; i < 280; i++)
    l[i] = 7;
  for (; i < 288; i++)          // make a complete, but wrong code set
    l[i] = 8;
  bl = 7;
  if ( (i = huft_build (pz, l, 288, 257, cplens, cplext, &tl, &bl)) != 0)
    return i;
  for (i = 0; i < 30; i++)      // make an incomplete code set
    l[i] = 5;
  bd = 5;
  if ( (i = huft_build (pz, l, 30, 0, cpdist, cpdext, &td, &bd)) > 1) {
    huft_free (pz, tl);
    return i;
  }
  if (inflate_codes (pz, tl, td, bl, bd))
    return 1;
  huft_free (pz, tl);
  huft_free (pz, td);
  return 0;
}
int inflate_dynamic (zip_t* pz) {
  int i;                // temporary variables
  unsigned j;
  unsigned l;           // last length
  unsigned m;           // mask for bit lengths table
  unsigned n;           // number of lengths to get
  struct huft *tl;      // literal/length code table
  struct huft *td;      // distance code table
  int bl;               // lookup bits for tl
  int bd;               // lookup bits for td
  unsigned nb;          // number of bit length codes
  unsigned nl;          // number of literal/length codes
  unsigned nd;          // number of distance codes
  unsigned ll[286+30];  // literal/length and distance code lengths
  register ulg b;       // bit buffer
  register unsigned k;  // number of bits in bit buffer

  b = pz->bb;
  k = pz->bk;
  NEEDBITS (5)
  nl = 257 + ( (unsigned) b & 0x1f);    // number of literal/length codes
  DUMPBITS (5)
  NEEDBITS (5)
  nd = 1 + ( (unsigned) b & 0x1f);      // number of distance codes
  DUMPBITS (5)
  NEEDBITS (4)
  nb = 4 + ( (unsigned) b & 0xf);       // number of bit length codes
  DUMPBITS (4)
  if (nl > 286 || nd > 30)
    return 1;                   // bad lengths

  for (j = 0; j < nb; j++) {
    NEEDBITS (3)
    ll[border[j]] = (unsigned) b & 7;
    DUMPBITS (3)
  }
  for (; j < 19; j++)
    ll[border[j]] = 0;


  bl = 7;
  if ( (i = huft_build (pz, ll, 19, 19, NULL, NULL, &tl, &bl)) != 0) {
    if (i == 1)
      huft_free (pz, tl);
    return i;                   // incomplete code set
  }
  n = nl + nd;
  m = mask_bits[bl];
  i = l = 0;
  while ( (unsigned) i < n) {
    NEEDBITS ( (unsigned) bl)
    j = (td = tl + ( (unsigned) b & m))->b;
    DUMPBITS (j)
    j = td->v.n;
    if (j < 16)                 // length of code in bits (0..15)
      ll[i++] = l = j;          // save last length in l
    else if (j == 16) {         // repeat last length 3 to 6 times
      NEEDBITS (2)
      j = 3 + ( (unsigned) b & 3);
      DUMPBITS (2)
      if ( (unsigned) i + j > n)
        return 1;
      while (j--)
        ll[i++] = l;
    } else if (j == 17) {        // 3 to 10 zero length codes
      NEEDBITS (3)
      j = 3 + ( (unsigned) b & 7);
      DUMPBITS (3)
      if ( (unsigned) i + j > n)
        return 1;
      while (j--)
        ll[i++] = 0;
      l = 0;
    } else {                     // j == 18: 11 to 138 zero length codes
      NEEDBITS (7)
      j = 11 + ( (unsigned) b & 0x7f);
      DUMPBITS (7)
      if ( (unsigned) i + j > n)
        return 1;
      while (j--)
        ll[i++] = 0;
      l = 0;
    }
  }
  huft_free (pz, tl);
  pz->bb = b;
  pz->bk = k;
  bl = LBITS;
  if ( (i = huft_build (pz, ll, nl, 257, cplens, cplext, &tl, &bl)) != 0) {
    if (i == 1) {
      huft_free (pz, tl);
    }
    return i;                   // incomplete code set
  }
  bd = DBITS;
  if ( (i = huft_build (pz, ll + nl, nd, 0, cpdist, cpdext, &td, &bd)) != 0) {
    if (i == 1) {
      huft_free (pz, td);
    }
    huft_free (pz, tl);
    return i;                   // incomplete code set
  }
  if (inflate_codes (pz, tl, td, bl, bd))
    return 1;
  huft_free (pz, tl);
  huft_free (pz, td);
  return 0;
}
int inflate_block (zip_t* pz, int *e) {
  unsigned t;           // block type
  register ulg b;       // bit buffer
  register unsigned k;  // number of bits in bit buffer
  b = pz->bb;
  k = pz->bk;
  NEEDBITS (1)
  *e = (int) b & 1;
  DUMPBITS (1)
  NEEDBITS (2)
  t = (unsigned) b & 3;
  DUMPBITS (2)
  pz->bb = b;
  pz->bk = k;
  if (t == 2)
    return inflate_dynamic (pz);
  if (t == 0)
    return inflate_stored (pz);
  if (t == 1)
    return inflate_fixed (pz);
  return 2;
}

int inflate () {
  int e;                // last block flag
  int r;                // result code
  unsigned h;           // maximum struct huft's malloc'ed
  zip_t    dzip;

  dzip.inptr = 0;
  // initialize window, bit buffer
  dzip.wp = 0;
  dzip.bk = 0;
  dzip.bb = 0;
  // decompress until the last block
  h = 0;
  do {
    dzip.hufts = 0;
    if ( (r = inflate_block (&dzip, &e)) != 0)
      return r;
    if (dzip.hufts > h)
      h = dzip.hufts;
  } while (!e);
  while (dzip.bk >= 8) {
    dzip.bk -= 8;
    dzip.inptr--;
  }
  flush_output (&dzip, dzip.wp);

  return 0;
}

void restore_file (const cFile* ptr) {
  insize = ptr->clen;
  inbuf  = ptr->data;
  inflate ();
}
