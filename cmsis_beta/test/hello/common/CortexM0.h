#ifndef ARMCM0_HDEF
#define ARMCM0_HDEF

/** @brief SYSTICK for Cortex-M0
 * Není to moc domyšlené, před tt. hlavičkou je nutné mít definován NVIC a IRQn,
 * což je v STM generované hlavičce většinou uděláno. NVIC_EnableIRQ je zjednodušen
 * jen pro CM0, jinak se tam čaruje s PRIO_BITS, tady to není potřeba.
 */

// tohle je jediné, co je potřeba z core_cm0.h
static inline void NVIC_EnableIRQ (IRQn irq) {
  NVIC.ISER.R = ((1 << (static_cast<uint32_t>(irq) & 0x1F)));
}
static constexpr uint32_t SysTick_LOAD_RELOAD_Msk = (0xFFFFFFUL);        /*!< SysTick LOAD: RELOAD Mask */
// ////////////////////+++ SysTick +-+//////////////////// //
struct SysTick_DEF { /*!< 24Bit System Tick Timer for use in RTOS */
  union CSR_DEF   {  //!< [0000](04)[0x00000004] SysTick Control and Status Register
    enum ENABLE_ENUM /*: uint32_t */ {
      ENABLE_0 = 0,  //!< disabled
      ENABLE_1 = 1,  //!< enabled
    };
    enum TICKINT_ENUM /*: uint32_t */ {
      TICKINT_0 = 0,  //!< Enable SysTick Exception
      TICKINT_1 = 1,  //!< Disable SysTick Exception
    };
    enum CLKSOURCE_ENUM /*: uint32_t */ {
      CLKSOURCE_0 = 0,  //!< External Clock
      CLKSOURCE_1 = 1,  //!< CPU Clock
    };
    struct {
      __IO ENABLE_ENUM     ENABLE    :  1; //!<[00] Enable SysTick Timer
      __IO TICKINT_ENUM    TICKINT   :  1; //!<[01] Generate Tick Interrupt
      __IO CLKSOURCE_ENUM  CLKSOURCE :  1; //!<[02] Source to count from
           uint32_t        UNUSED0   : 13; //!<[03] 
      __IO ONE_BIT         COUNTFLAG :  1; //!<[16] SysTick counted to zero
    } B;
    __IO uint32_t  R;

    explicit CSR_DEF () noexcept { R = 0x00000004u; }
    template<typename F> void setbit (F f) volatile {
      CSR_DEF r;
      R = f (r);
    }

    template<typename F> void modify (F f) volatile {
      CSR_DEF r; r.R = R;
      R = f (r);
    }
  };
  __IO CSR_DEF CSR ;  //!< register definition

  union RVR_DEF   {  //!< [0004](04)[0x00000000] SysTick Reload Value Register
    struct {
      __IO uint32_t   RELOAD : 24; //!<[00] Value to auto reload SysTick after reaching zero
    } B;
    __IO uint32_t  R;

    explicit RVR_DEF () noexcept { R = 0x00000000u; }
    template<typename F> void setbit (F f) volatile {
      RVR_DEF r;
      R = f (r);
    }

    template<typename F> void modify (F f) volatile {
      RVR_DEF r; r.R = R;
      R = f (r);
    }
  };
  __IO RVR_DEF RVR ;  //!< register definition

  union CVR_DEF   {  //!< [0008](04)[0x00000000] SysTick Current Value Register
    struct {
      __IO uint32_t   CURRENT : 24; //!<[00] Current value
    } B;
    __IO uint32_t  R;

    explicit CVR_DEF () noexcept { R = 0x00000000u; }
    template<typename F> void setbit (F f) volatile {
      CVR_DEF r;
      R = f (r);
    }

    template<typename F> void modify (F f) volatile {
      CVR_DEF r; r.R = R;
      R = f (r);
    }
  };
  __IO CVR_DEF CVR ;  //!< register definition

  union CALIB_DEF {  //!< [000c](04)[0x00000000] SysTick Calibration Value Register
    enum SKEW_ENUM /*: uint32_t */ {
      SKEW_0 = 0,  //!< 10ms calibration value is exact
      SKEW_1 = 1,  //!< 10ms calibration value is inexact, because of the clock frequency
    };
    enum NOREF_ENUM /*: uint32_t */ {
      NOREF_0 = 0,  //!< Ref Clk available
      NOREF_1 = 1,  //!< Ref Clk not available
    };
    struct {
      __I  uint32_t    TENMS   : 24; //!<[00] Reload value to use for 10ms timing
           uint32_t    UNUSED0 :  6; //!<[24] 
      __I  SKEW_ENUM   SKEW    :  1; //!<[30] Clock Skew
      __I  NOREF_ENUM  NOREF   :  1; //!<[31] No Ref
    } B;
    __IO uint32_t  R;

    explicit CALIB_DEF () noexcept { R = 0x00000000u; }
    template<typename F> void setbit (F f) volatile {
      CALIB_DEF r;
      R = f (r);
    }

    template<typename F> void modify (F f) volatile {
      CALIB_DEF r; r.R = R;
      R = f (r);
    }
  };
  __IO CALIB_DEF CALIB ;  //!< register definition
  // methods :
  bool Config (const uint32_t ticks) {
    if (ticks > SysTick_LOAD_RELOAD_Msk)  return false;         // Reload value impossible
    RVR.B.RELOAD    = ticks  - 1u;                              // set reload register
    NVIC_EnableIRQ    (SysTick_IRQn);                           // Enable Interrupt
    CVR.B.CURRENT   = 0;                                        // Load the SysTick Counter Value
    CSR.modify([](CSR_DEF & r) -> auto {                        // Enable SysTick IRQ and SysTick Timer
      r.B.CLKSOURCE = CSR_DEF::CLKSOURCE_ENUM::CLKSOURCE_1;
      r.B.TICKINT   = CSR_DEF::TICKINT_ENUM  ::TICKINT_1;
      r.B.ENABLE    = CSR_DEF::ENABLE_ENUM   ::ENABLE_1;
      return r.R;
    });
    return true;                                                 // Function successful
  }

}; /* total size = 0x0010, struct size = 0x0010 */
static SysTick_DEF & SysTick = * reinterpret_cast<SysTick_DEF *> (0xe000e010);

static_assert (sizeof(struct SysTick_DEF) == 16, "size error SysTick");

#endif
