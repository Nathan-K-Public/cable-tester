#ifndef BREAKOUTMAP_H
#define BREAKOUTMAP_H


// ================= DEFINING PIN NAMES/MASKS PER BANK =================

static const uint8_t UsbNULL[] =
{   0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0,
    0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};

static const uint8_t UsbCF[] =
{ GND, TX1P, TX1N, VBUS,  CC1, DP, DN, SBU1, VBUS, RX2N, RX2P, GND, 0,
  GND, RX1P, RX1N, VBUS, SBU2, DN, DP,  CC2, VBUS, TX2N, TX2P, GND, 0 };

static const uint8_t UsbCM[] =
{ GND, TX1P, TX1N, VBUS,  CC1, DP, DN, SBU1, VBUS, RX2N, RX2P, GND, 0,
  GND, RX1P, RX1N, VBUS, SBU2, DN, DP,  CC2, VBUS, TX2N, TX2P, GND, 0 };


static const uint8_t UsbA3F[] =
{  SSTXN, SSTXP, GND_D, SSRXN, SSRXP, OPEN, SSTXN, SSTXP, GND_D, SSRXN, SSRXP, 0,0,
   SHELL, GND,   DP,    DN,    VBUS , OPEN, SHELL, GND,   DP,    DN,    VBUS,  0,0
};

static const uint8_t UsbA3F1[] =
{  SSTXN, SSTXP, GND_D, SSRXN, SSRXP, 0, 0, 0, 0, 0, 0, 0,0,
   SHELL, GND,   DP,    DN,    VBUS , 0, 0, 0,   0,    0,    0,  0,0
};

static const uint8_t UsbB3F[] =
{  SSTXN, SSTXP, GND_D, SSRXN, SSRXP, 0, 0, 0, 0, 0, 0, 0,0,
   SHELL, GND,   DP,    DN,    VBUS , 0, 0, 0,   0,    0,    0,  0,0
};

static const uint8_t UsbmB2F[] =
{  VBUS, DN, DP, ID, GND, 0, 0, 0, 0, 0, 0, 0,0,
   0, 0,   0,    0,    0 , 0, 0, 0,   0,    0,    0,  0,0
};

/*
static const uint8_t UsbA3M[] =
{ SHELL, SSRXN, SSRXP, GND_D, SSTXN, SSTXP, GND, DP, DN, VBUS, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}; 

static const uint8_t UsbA3M_REV[] =
{ VBUS, DN, DP, GND, SSTXP, SSTXN, GND_D, SSRXP, SSRXN, SHELL, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}; 




static const uint8_t UsbuB3F[] =
{ VBUS, DN, DP, ID, GND, SSTXN, SSTXP, GND_D, SSRXN, SSRXP, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};

static const uint8_t UsbuB2M[] =
{ GND, ID, DP, DN, VBUS, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};

static const uint8_t UsbumB2F[] =
{ VBUS, DN, DP, ID, GND, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};

static const uint8_t UsbmB2M[] =
{ GND, ID, DP, DN, VBUS, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};  
  */

#endif
