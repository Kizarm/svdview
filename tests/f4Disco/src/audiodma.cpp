#include "audiodma.h"
#include "gpio.h"

#define RCC_APB1Periph_I2C1              ((uint32_t)0x00200000)
#define RCC_AHB1Periph_DMA1              ((uint32_t)0x00200000)
#define RCC_APB1Periph_SPI3              ((uint32_t)0x00008000)

static AudioDma * Instance = 0;

// SPI3 mapped DMA1 channel 0 stream 7 
static inline void Dma1Init (Sample * ptr) {
  // configure increment, size, interrupts and circular mode
  DMA1.S7CR.setbits([](DMA_S7CR_s & r) -> auto {
    r.B.MINC  = 1u;
    r.B.MSIZE = 1u;   // 16.bit
    r.B.PSIZE = 1u;
    r.B.CIRC  = 1u;
    r.B.HTIE  = 1u;
    r.B.TCIE  = 1u;
    r.B.PL    = 1u;
    r.B.DIR   = 1u;
    return r.R;
  });
  // Configure the peripheral data register address
  DMA1.S7PAR  = (uint32_t) (& (SPI3.DR));
  // Configure the memory address
  DMA1.S7M0AR = (uint32_t) (ptr);
  // Configure the number of DMA tranfer to be performs on DMA channel
  DMA1.S7NDTR = 4 * DmaBufLen;
  // Enable DMA Stream 7
  DMA1.S7CR.B.EN = 1u;
}


AudioDma::AudioDma () : OneWay() {
  Instance = this;
  pbuf0 = buf;
  pbuf1 = buf + DmaBufLen;
  // Intitialize state.
  RCC.AHB1ENR.B.DMA1EN = 1u;
  RCC.APB1ENR.modify([](RCC_APB1ENR_s & r) -> auto {
    r.B.I2C1EN = 1u;
    r.B.SPI3EN = 1u;
    return r.R;
  });

  GpioClass resetPin (GpioPortD, 4);
  
  GpioClass sclPin (GpioPortB, 6, GPIO_Mode_AF);
  GpioClass sdaPin (GpioPortB, 9, GPIO_Mode_AF);
  sclPin.setOType(GPIO_OType_OD);
  sdaPin.setOType(GPIO_OType_OD);
  sclPin.setAF(4);
  sdaPin.setAF(4);
  
  GpioClass mckPin (GpioPortC, 7, GPIO_Mode_AF);
  GpioClass sckPin (GpioPortC,10, GPIO_Mode_AF);
  GpioClass sddPin (GpioPortC,12, GPIO_Mode_AF);
  mckPin.setAF (6);
  sckPin.setAF (6);
  sddPin.setAF (6);
    
  GpioClass iisPin (GpioPortA, 4, GPIO_Mode_AF);
  iisPin.setAF (6);
  
  // Reset the codec.
  -resetPin;
  volatile int i;
  for (i = 0; i < 0x4fff; i++) {
    __asm__ volatile ("nop");
  }
  +resetPin;

  // Reset I2C.
  RCC.APB1RSTR.B.I2C1RST = 1u;
  RCC.APB1RSTR.B.I2C1RST = 0u;

  // Configure I2C.
  uint32_t pclk1 = 42000000;

  I2C1.CR2.R = pclk1 / 1000000; // Configure frequency and disable interrupts and DMA.
  I2C1.OAR1.setbits([](I2C_OAR1_s & r) -> auto {
    r.B.ADDMODE = 1u;
    r.B.ADD0    = 1u;
    r.B.ADD7    = 0x19u;
    return r.R;
  });

  // Configure I2C speed in standard mode.
  const uint32_t i2c_speed = 100000;
  int ccrspeed = pclk1 / (i2c_speed * 2);
  if (ccrspeed < 4) {
    ccrspeed = 4;
  }
  I2C1.CCR.R = ccrspeed;
  I2C1.TRISE = pclk1 / 1000000 + 1;
  I2C1.CR1.setbits([](I2C_CR1_s & r) -> auto {
    r.B.ACK = 1u;
    r.B.PE  = 1u;
    return r.R;        // Enable and configure the I2C peripheral.
  });

  // Configure codec.
  WriteRegister (0x02, 0x01); // Keep codec powered off.
  WriteRegister (0x04, 0xaf); // SPK always off and HP always on.

  WriteRegister (0x05, 0x81); // Clock configuration: Auto detection.
  WriteRegister (0x06, 0x04); // Set slave mode and Philips audio standard.

  SetVolume (206);      // -12 dB

  // Power on the codec.
  WriteRegister (0x02, 0x9e);

  // Configure codec for fast shutdown.
  WriteRegister (0x0a, 0x00); // Disable the analog soft ramp.
  WriteRegister (0x0e, 0x04); // Disable the digital soft ramp.

  WriteRegister (0x27, 0x00); // Disable the limiter attack level.
  WriteRegister (0x1f, 0x0f); // Adjust bass and treble levels.

  WriteRegister (0x1a, 0x0a); // Adjust PCM volume level.
  WriteRegister (0x1b, 0x0a);

  // Disable I2S.
  SPI3.I2SCFGR.R = 0u;

  RCC.CR.B.PLLI2SON = 0u;

  const unsigned plln=271,pllr=2,i2sdiv=6, i2sodd=0;       // 44.1 kHz
//const unsigned plln=271,pllr=2,i2sdiv=12,i2sodd=0;       // 22.05kHz
  // I2S clock configuration
  RCC.CFGR.B.I2SSRC = 0u;
  RCC.PLLI2SCFGR.setbits([](RCC_PLLI2SCFGR_s & r) -> auto {
    r.B.PLLI2SNx = plln;
    r.B.PLLI2SRx = pllr;
    return r.R;
  });

  // Enable PLLI2S and wait until it is ready.
  RCC.CR.B.PLLI2SON = 1u;
  while (!RCC.CR.B.PLLI2SRDY);

  // Configure I2S.     // 6 , 0
  SPI3.I2SPR.setbits([](SPI_I2SPR_s & r) -> auto {
    r.B.I2SDIV = i2sdiv;
    r.B.ODD    = i2sodd;
    r.B.MCKOE  = 1u;
    return r.R;
  });
  /* nechame to az na Turn (on=true), tak se to cele spusti
  SPI3.I2SCFGR.setbits([](SPI_I2SCFGR_s & r) -> auto {
    r.B.I2SMOD = 1u;  // I2S mode is selected
    r.B.I2SCFG = 2u;  // Master - transmit
    r.B.I2SE   = 1u;  // enable
    return r.R;       // Master transmitter, Phillips mode, 16 bit values, clock polarity low, enable.
  });
  */
  Dma1Init       (buf);
  NVIC_EnableIRQ (DMA1_Stream7_IRQn);
  SPI3.CR2.B.TXDMAEN = 1u;
}

void AudioDma::WriteRegister (uint8_t address, uint8_t value) {
  while (I2C1.SR2.B.BUSY);
  I2C1.CR1.B.START = 1u;     // Start the transfer sequence.
  while (! I2C1.SR1.B.SB);   // Wait for start bit.
  I2C1.DR = 0x94;
  while (! I2C1.SR1.B.ADDR); // Wait for master transmitter mode.
  volatile uint32_t x = I2C1.SR2.R;
  (void) x; // ???
  I2C1.DR = address;       // Transmit the address to write to.
  while (! I2C1.SR1.B.TxE);  // Wait for byte to move to shift register.
  I2C1.DR = value;         // Transmit the value.
  while (!I2C1.SR1.B.BTF);   // Wait for all bytes to finish.
  I2C1.CR1.B.STOP = 1u;      // End the transfer sequence.
}

void AudioDma::SetVolume (int volume) {
  WriteRegister (0x20, (volume + 0x19) & 0xff);
  WriteRegister (0x21, (volume + 0x19) & 0xff);
}
void AudioDma::Turn (bool on) {
  if (on) {
    WriteRegister (0x02, 0x9e);
    SPI3.I2SCFGR.setbits([](SPI_I2SCFGR_s & r) -> auto {
      r.B.I2SMOD = 1u;
      r.B.I2SCFG = 2u;
      r.B.I2SE   = 1u;
      return r.R;       // Master transmitter, Phillips mode, 16 bit values, clock polarity low, enable.
    });
  } else {
    WriteRegister (0x02, 0x01);
    SPI3.I2SCFGR.R = 0u;
  }
}
void AudioDma::irq (void) {
  Sample * cur = 0;
  DMA_HISR_s status(DMA1.HISR);
  if (status.B.HTIF7) cur = pbuf0;
  if (status.B.TCIF7) cur = pbuf1;
  // znuluj příznaky
  DMA1.HIFCR.R |= status.R;
  if (!cur)    return;
  // zpracuj data, pokud je potřeba
  Up (cur, DmaBufLen);
}
extern "C" void DMA1_Stream7_IRQHandler (void) {
  if (Instance) Instance->irq();
}
