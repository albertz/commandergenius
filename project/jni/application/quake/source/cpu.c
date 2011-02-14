/*  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "cpu.h"

/* Define this to the CPU frequency */
#define CPU_FREQ 336000000    /* CPU clock: 336 MHz */
#define CFG_EXTAL 12000000    /* EXT clock: 12 Mhz */

// SDRAM Timings, unit: ns
#define SDRAM_TRAS    45  /* RAS# Active Time */
#define SDRAM_RCD    20  /* RAS# to CAS# Delay */
#define SDRAM_TPC    20  /* RAS# Precharge Time */
#define SDRAM_TRWL    7  /* Write Latency Time */
#define SDRAM_TREF          15625  /* Refresh period: 4096 refresh cycles/64ms */ 
//#define SDRAM_TREF      7812  /* Refresh period: 8192 refresh cycles/64ms */

#include "jz4740.h"

static unsigned long jz_dev=0;
static volatile unsigned long  *jz_cpmregl, *jz_emcregl;
volatile unsigned short *jz_emcregs; 

static inline int sdram_convert(unsigned int pllin,unsigned int *sdram_freq)
{
  register unsigned int ns, tmp;
 
  ns = 1000000000 / pllin;
  /* Set refresh registers */
  tmp = SDRAM_TREF/ns;
  tmp = tmp/64 + 1;
  if (tmp > 0xff) tmp = 0xff;
        *sdram_freq = tmp; 

  return 0;

}
 
static int 
pll_init(unsigned int clock)
{
  register unsigned int cfcr, plcr1;
  unsigned int sdramclock = 0;
  int n2FR[33] = {
    0, 0, 1, 2, 3, 0, 4, 0, 5, 0, 0, 0, 6, 0, 0, 0,
    7, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0,
    9
  };
  //int div[5] = {1, 4, 4, 4, 4}; /* divisors of I:S:P:L:M */
    int div[5] = {1, 3, 3, 3, 3}; /* divisors of I:S:P:L:M */
  int nf, pllout2;

  cfcr = CPM_CPCCR_CLKOEN |
    (n2FR[div[0]] << CPM_CPCCR_CDIV_BIT) | 
    (n2FR[div[1]] << CPM_CPCCR_HDIV_BIT) | 
    (n2FR[div[2]] << CPM_CPCCR_PDIV_BIT) |
    (n2FR[div[3]] << CPM_CPCCR_MDIV_BIT) |
    (n2FR[div[4]] << CPM_CPCCR_LDIV_BIT);

  pllout2 = (cfcr & CPM_CPCCR_PCS) ? clock : (clock / 2);

  /* Init UHC clock */
  //  REG_CPM_UHCCDR = pllout2 / 48000000 - 1;
  jz_cpmregl[0x6C>>2] = pllout2 / 48000000 - 1;

  nf = clock * 2 / CFG_EXTAL;
  plcr1 = ((nf - 2) << CPM_CPPCR_PLLM_BIT) | /* FD */
    (0 << CPM_CPPCR_PLLN_BIT) |  /* RD=0, NR=2 */
    (0 << CPM_CPPCR_PLLOD_BIT) |    /* OD=0, NO=1 */
    (0x20 << CPM_CPPCR_PLLST_BIT) | /* PLL stable time */
    CPM_CPPCR_PLLEN;                /* enable PLL */          

  /* init PLL */
  //  REG_CPM_CPCCR = cfcr;
  //  REG_CPM_CPPCR = plcr1;
  jz_cpmregl[0] = cfcr;
  jz_cpmregl[0x10>>2] = plcr1;
  
  sdram_convert(clock,&sdramclock);
  if (sdramclock > 0) {
//  REG_EMC_RTCOR = sdramclock;
//  REG_EMC_RTCNT = sdramclock;    
    jz_emcregs[0x8C>>1] = sdramclock;
    jz_emcregs[0x88>>1] = sdramclock;  

  }
}

void 
cpu_init()
{
  jz_dev = open("/dev/mem", O_RDWR);
  jz_cpmregl=(unsigned long  *)mmap(0, 0x80, PROT_READ|PROT_WRITE, MAP_SHARED, jz_dev, 0x10000000);
  jz_emcregl=(unsigned long  *)mmap(0, 0x90, PROT_READ|PROT_WRITE, MAP_SHARED, jz_dev, 0x13010000);
  jz_emcregs=(unsigned short *)jz_emcregl;

}

void 
cpu_deinit()
{
  cpu_set_clock( GP2X_MAX_CLOCK );
  munmap((void *)jz_cpmregl, 0x80); 
  munmap((void *)jz_emcregl, 0x90);   
  close(jz_dev);
//  fcloseall();
  sync();
}

static void
loc_set_clock(int clock_in_mhz )
{
  if (clock_in_mhz >= GP2X_MIN_CLOCK && clock_in_mhz <= GP2X_MAX_CLOCK) {
    pll_init( clock_in_mhz * 1000000 );
  }
}

static unsigned int loc_clock_in_mhz = GP2X_DEF_CLOCK;

void 
cpu_set_clock(unsigned int clock_in_mhz)
{
  if (clock_in_mhz == loc_clock_in_mhz) return;
  loc_clock_in_mhz = clock_in_mhz;
  
  loc_set_clock(clock_in_mhz);

  return;
}

unsigned int
cpu_get_clock()
{
  return loc_clock_in_mhz;
}

