#ifndef PINMAP_H
#define PINAMP_H

// =============== PREPROCESSOR DIRECTIVES ================

#define FOREACH_PINtyp(PREP) \
        PREP(NA)   \
        PREP(OPEN) \
        PREP(GND)  \
        PREP(VBUS)   \
        PREP(DP)  \
        PREP(DN)  \
        PREP(TX1P)  \
        PREP(TX1N)  \
        PREP(CC1)  \
        PREP(SBU1)  \
        PREP(RX2N)  \
        PREP(RX2P)  \
        PREP(TX2P)  \
        PREP(TX2N)  \
        PREP(CC2)  \
        PREP(SBU2)  \
        PREP(RX1N)  \
        PREP(RX1P)  \
        PREP(SSRXN)  \
        PREP(SSRXP)  \
        PREP(GND_D)  \
        PREP(SSTXN)  \
        PREP(SSTXP)  \
        PREP(SHELL)  \
        PREP(ID)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum PINtyp_ENUM {
    FOREACH_PINtyp(GENERATE_ENUM)
};

static const char *PINtyp_STRING[] = {
    FOREACH_PINtyp(GENERATE_STRING)
};



//===== PHYSICAL LAYOUT to GUID mapping ====
// This converts the "Bank ID" {A|B} {0-25} of a pin to its "Globally Unique ID"

static const uint8_t bankA[] = 
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
static const uint8_t bankA_size = 26;

static const uint8_t bankB[] = 
  { 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };
static const uint8_t bankB_size = 26;

//Analog mask is >=54 <=69.

//====== GUID to ARDUINO PIN # mapping =======
//This converts the "Globally Unique ID" {0-51} of a pin to the actual Arduino pin #.
// Logical pin number <-> physical pin number. (AKA "number on top to slot on bottom".)
// Bank A on top, Bank B on bottom. Single array.

/*
//Cable tester v1.0
static const uint8_t read_pins[52] = {
    63, 64, 65, 66, 67, 25, 23, 24, 22, 17, 16, 15, 14,
    61, 60, 59, 58, 55, 54, 12,  9,  7,  6,  5,  3,  2,
    
    69, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26,
    68, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27 };
 */

//Cable tester v2.0 
static const uint8_t read_pins[52] = {
    A0, 48, 46, A12, A1, A4, A5, 44, 42, 40, 38, 36, 34,
    49, 47, 45, 43, 41, A6, A7, A11, A13,  39,  37,  A10, 35,
    
    A8, 32, 30, A14, A9, 28, 26, 24, 22, 17, 16, 15, 14,
    33, 31, 29, 27, 25, 23, 2, A2, A15, 3, 5, 6, 7 };
static const uint8_t read_size = (sizeof(read_pins)/sizeof(uint8_t));



#endif
