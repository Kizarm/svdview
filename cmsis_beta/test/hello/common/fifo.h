#ifndef FIFO_H
#define FIFO_H
/** Typ dbus_w_t je podobně definován jako sig_atomic_t v hlavičce signal.h.
 * Je to prostě největší typ, ke kterému je "atomický" přístup. V GCC je definováno
 * __SIG_ATOMIC_TYPE__, šlo by použít, ale je znaménkový.
 * */
#ifdef __SIG_ATOMIC_TYPE__
typedef unsigned __SIG_ATOMIC_TYPE__ dbus_w_t;
#else
typedef unsigned int dbus_w_t;  // pro AVR by to měl být uint8_t (šířka datové sběrnice)
#endif //__SIG_ATOMIC_TYPE__
/// Tahle podivná rekurzívní formule je použita pro validaci délky bufferu.
static constexpr bool isValidM (const int N, const dbus_w_t M) {
  // constexpr má raději rekurzi než cyklus (c++11)
  return (N > 12) ? false : (((1u << N) == M) ? true : isValidM (N+1, M));
}
/** @class FIFO
 *  @brief Jednoduchá fronta (kruhový buffer).
 * 
 * V tomto přikladu je vidět, že synchronizace mezi přerušením a hlavní smyčkou programu
 * může být tak jednoduchá, že je v podstatě neviditelná. Využívá se toho, že pokud
 * do kruhového buferu zapisujeme jen z jednoho bodu a čteme také jen z jednoho bodu
 * (vlákna), zápis probíhá nezávisle pomocí indexu m_head a čtení pomocí m_tail.
 * Délka dat je dána rozdílem tt. indexů, pokud v průběhu výpočtu délky dojde k přerušení,
 * v zásadě se nic špatného neděje, maximálně je délka určena špatně a to tak,
 * že zápis nebo čtení je nutné opakovat. Důležité je, že po výpočtu se nová délka zapíše
 * do paměti "atomicky". Takže např. pro 8-bit procesor musí být indexy jen 8-bitové.
 * To není moc velké omezení, protože tyto procesory obvykle mají dost malou RAM, takže
 * velikost bufferu stejně nebývá být větší než nějakých 64 položek.
 * Opět nijak nevadí že přijde přerušení při zápisu nebo čtení položky - to se provádí
 * dříve než změna indexu, zápis a čtení je vždy na jiném místě RAM. Celé je to uděláno
 * jako šablona, takže je možné řadit do fronty i složitější věci než je pouhý byte.
 * Druhým parametrem šablony je délka bufferu (aby to šlo konstruovat jako statický objekt),
 * musí to být mocnina dvou v rozsahu 8 až 4096, default je 64. Mocnina 2 je zvolena proto,
 * aby se místo zbytku po dělení mohl použít jen bitový and, což je rychlejší.
 * */
template<typename T, const dbus_w_t M = 64> class FIFO {
  T m_data [M];
  volatile dbus_w_t m_head;   //!< index pro zápis (hlava)
  volatile dbus_w_t m_tail;   //!< index pro čtení (ocas)
  /// vrací skutečnou délku dostupných dat
  constexpr dbus_w_t lenght () const { return (M + m_head - m_tail) & (M - 1); };
  /// zvětší a saturuje index, takže se tento motá v kruhu @param n index
  void sat_inc (volatile dbus_w_t & n) const { n = (n + 1) & (M - 1); };
  public:
    /// Konstruktor
    explicit constexpr FIFO<T,M> () noexcept {
      // pro 8-bit architekturu může být byte jako index poměrně malý
      static_assert (1ul << (8 * sizeof(dbus_w_t) - 1) >= M, "atomic type too small");
      // a omezíme pro jistotu i delku buferu na nějakou rozumnou delku
      static_assert (isValidM (3, M), "M must be power of two in range <8,4096> or <8,128> for 8-bit data bus (AVR)");
      m_head = 0;
      m_tail = 0;
    }
    /// Čtení položky
    /// @return true, pokud se úspěšně provede
    const bool Read  (T & c) {
      if (lenght() == 0)       return false;
      c = m_data [m_tail];
      sat_inc    (m_tail);
      return true;
    }
    /// Zápis položky
    /// @return true, pokud se úspěšně provede
    const bool Write (const T & c) {
      if (lenght() >= (M - 1)) return false;
      m_data  [m_head] = c;
      sat_inc (m_head);
      return true;
    }
};

#endif // FIFO_H
