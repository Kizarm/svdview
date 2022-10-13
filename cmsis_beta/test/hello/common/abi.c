//////////// pomocne funkce (clang - gcc abi) ////////////
typedef __SIZE_TYPE__ size_t;
size_t strlen (const char * s) {
  unsigned n = 0;
  while (*s++) n++;
  return n;
}
#ifdef __clang__
/** Linkování llvm (ld.lld) postrádá vnitřní funkce gcc, je nutné je doplnit.
 * Je možné použít arm-none-eabi-gcc jen pro linkování, ale je to takové divné.
 * Nebo nastavit cestu ke správné knihovně gcc, ale která je ta správná je otázka.
 * */
static void * memset (void * dest, const int val, const size_t len) {
  unsigned char * ptr = (unsigned char *) dest;
  for (unsigned n=0; n<len; n++) ptr [n] = (unsigned char) val;
  return dest;
}
void __aeabi_memclr (void * dst, const size_t len) {
  memset (dst, 0, len);
}
void __aeabi_memclr4 (void * dst, const size_t len) {
  __aeabi_memclr (dst, len);
}
struct qr {
  unsigned q;     /* computed quotient */
  unsigned r;     /* computed remainder */
  unsigned q_n;   /* specficies if quotient shall be negative */
  unsigned r_n;   /* specficies if remainder shall be negative */
};

static void division_qr (unsigned n, unsigned p, struct qr * qr) {
  unsigned i = 1, q = 0;
  if (p == 0) {
    qr->r = 0xFFFFFFFF; /* division by 0 */
    return;
  }
  while ( (p >> 31) == 0) {
    i = i << 1;     /* count the max division steps */
    p = p << 1;     /* increase p until it has maximum size*/
  }
  while (i > 0) {
    q = q << 1;     /* write bit in q at index (size-1) */
    if (n >= p) {
      n -= p;
      q++;
    }
    p = p >> 1;     /* decrease p */
    i = i >> 1;     /* decrease remaining size in q */
  }
  qr->r = n;
  qr->q = q;
}
static void uint_div_qr (unsigned numerator, unsigned denominator, struct qr * qr) {
  division_qr (numerator, denominator, qr);
  /* negate quotient and/or remainder according to requester */
  if (qr->q_n) qr->q = -qr->q;
  if (qr->r_n) qr->r = -qr->r;
}

unsigned __aeabi_uidiv (unsigned numerator, unsigned denominator) {
  struct qr qr = { .q_n = 0, .r_n = 0 };
  uint_div_qr (numerator, denominator, &qr);
  return qr.q;
}
#endif //  __clang__

