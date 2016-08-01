#include <avr/pgmspace.h>
#include "pinMap3.h"
#include "breakoutMap3.h"

//TODO: Fix voltage reading of CC pins so is differential.
//      Add bandgap calibration fix/tool/menu option, store BANDGAPCALIBRATE in nonvolatile EEPROM
//      Consolidate Rd and Rp checking functions so they recycle code instead of being separate.
//      Make monitor mode monitor both banks
//      Allow Rp detect to automatically detect Vbus as Monitor automatically detects Gnd

/*
pinDelay      - Delay in ms between pin ADC reads (to allow sampling capacitor to refill)
shiftDelay    - Delay in ms between moving pin to pin (to allow debugging)
truncate      - Display or truncate table display of "NULL"/empty pins
vbusDelayOn   - Delay in ms to wait for vBus to switch on after triggering Rd (from Type-C spec)
vbusDelayOff  - Delay in ms to wait for vBus to switch off after removing Rd (from Type-C spec)

vcc_actual       - calibrated value of the actual Vcc of the Arduino (after regulator) 
BANDGAPCALIBRATE - **BOARD-SPECIFIC** calibration value of the ATMega2560 internal 1.1v bandgap reference
                   (actual bandgap voltage in mV*1023) [1125300L = 1.1v default]
ACC1_DRIVER_PIN  - the pin tied to CC1-BankA through a 5.1kR (Rd) resistor
ACC2_DRIVER_PIN  - the pin tied to CC2-BankA through a 5.1kR (Rd) resistor

ACC1_SLAVE_PIN  - this is "CC1-BankA" you tied to the resistor above.
ACC2_SLAVE_PIN  - this is "CC2-BankA" you ties to the resistor above.
*/

  static const int pinDelay=0;
  static const int shiftDelay=0;
  static const int truncate=1;
  static const int vbusDelayOn=275*2;
  static const int vbusDelayOff=650*2;
  static long vcc_actual=0;
  
//#define BANDGAPCALIBRATE   1113098L
#define BANDGAPCALIBRATE   1125300L
#define ACC1_DRIVER_PIN 10
#define ACC2_DRIVER_PIN 11

#define ACC1_SLAVE_PIN 4
#define ACC2_SLAVE_PIN 20

/*
4.11.2 Timing Parameter
tVBUSON 0 ms 275 ms
From entry to Attached.SRC until VBUS reaches the minimum vSafe5V threshold as measured at the source’s
receptacle.

tVBUSOFF 0 ms 650 ms
From the time the Sink is detached until the Source removes VBUS and
reaches vSafe0V (See USB PD).
*/

typedef struct {
  char conName[32];
  const uint8_t* conMap;
  uint8_t conUID;
} CONtyp_STRUCT;

/*
This governs the menu options. The last number  is the menu entry # and is numbered automatically.
The breakout definitions are stored in breakoutMap_.h, which ties the physical layout of the breakout to the pin (and type) used.
*/

CONtyp_STRUCT menuOptions[] ={
  {"NULL / NONE", UsbNULL,0},
  {"Type-C Female", UsbCF,0},
  {"Type-C Male", UsbCM,0},
  {"A 3.0 Female (DOUBLE)", UsbA3F,0},
  {"A 3.0 Female (SINGLE)", UsbA3F1,0},
  {"B 3.0 Female", UsbB3F,0},
  {"microB 2.0 Female", UsbmB2F,0}
    /*
  {"A 3.0 Male", UsbA3M,0},
  {"B 3.0 Female", UsbB3F,0},
  {"microB 3.0 Female", UsbuB3F,0},
  {"microB 2.0 Male", UsbuB2M,0},
  {"micro/miniB 2.0 Female", UsbumB2F,0},
  {"miniB 2.0 Male", UsbmB2M,0},
  {"[FACE OUT] A 3.0 Male",UsbA3M_REV,0}*/
};
int8_t menuOptions_size=(sizeof(menuOptions)/sizeof(CONtyp_STRUCT));



//================ CLASSES =========================

/*
This is the internal storage class used for storing pin(x,y) associations. 1 is bridged, 0 is open. (x is at 0v, y is weak-pull-up'd)
I used a class to disambiguate the data from how it is stored.

Originally I used a bit.set and bit.get to bitpack into a 64-bit uint64_t tableData[52] to be highly efficient with the Mega's limited RAM.
(bools/uint8_t are 1-byte minimum, vs 1-bit by packing into a larger structure)

However, it was bugging out for some reason and storing all "1"s beyond a certain point (32-bits or so).
I think it was because of the maximum capacity of the temp variable used in the stock Arduino bit.get function.

I may revisit this and write my own bit.set and bit.get subfunctions to reduce memory usage of this sketch by A LOT.
Currently it is literally wasting 7/8 bits. tableData is HUGE [52*52*8 bits].
*/

class bitTable
{
   private:
      uint8_t tableData[52][52];
   public:
      // required constructors
      bitTable(){
         this->clearTable();
      }
      void set(uint8_t x, uint8_t y)
      {
         tableData[x][y]=1;
      }
      void clear(uint8_t x, uint8_t y)
      {
         tableData[x][y]=0;
      }
      uint8_t get(uint8_t x, uint8_t y)
      {
        return tableData[x][y];        
      }
      void make(uint8_t x, uint8_t y, uint8_t data)
      {
        tableData[x][y]=data;
      }
      void clearTable()
      {
        memset(tableData, 0, sizeof tableData);
      }  
};





// ============= SETUP =================
void setup() {
  Serial.begin(115200);
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);

  inputFloatAll();
  vcc_actual=readVcc();

  for(int i=0;i<menuOptions_size;i++){
    menuOptions[i].conUID=i+1;
  }
}


// =============== FUNCTIONS ===================

void inputPullupAll(){
  //Set all pins in range to weak INPUT_PULLUP
  for (int i = 0; i < read_size; i++) { //or i <= 4
    pinMode(read_pins[i],INPUT_PULLUP);
  }

  //Make drivers 10/11 floating
    pinMode(ACC1_DRIVER_PIN,INPUT);
    pinMode(ACC2_DRIVER_PIN,INPUT);
}

void inputFloatAll(){
  //Set all pins in range to foating INPPUT
  for (int i = 0; i < read_size; i++) { //or i <= 4
    pinMode(read_pins[i],INPUT);
  }
  //Make drivers 10/11 floating
    pinMode(ACC1_DRIVER_PIN,INPUT);
    pinMode(ACC2_DRIVER_PIN,INPUT);
}

int getPinMode(uint8_t pin)
{  
  //This function is copy+pasted from somewhere
  //It is just here for convenience, and is unused.
  
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *reg, *out;

  if (port == NOT_A_PIN) return -1;

  // JWS: can I let the optimizer do this?
  reg = portModeRegister(port);
  out = portOutputRegister(port);

  if ((~*reg & bit) == bit) // INPUT OR PULLUP
  {
    if ((~*out & bit)  == bit) return INPUT;
    else return INPUT_PULLUP;
  }
  return OUTPUT;
}

long readVcc() {
/*
  This function is a bit complicated, but very important for high-accuracy ADC measurements.
  
  It first sets the "voltage to compare to" as Vcc, the Arduino's supply rail.
  (Coincidentally, this is the ADC's default reference max value.... "1023" from ReadAnalog means this Vcc.)
  It then sets the ADC MUX to point at the ATMega's (or Uno's) internal 1.1v bandgap voltage generator.

  This bandgap generator is normally usually used to determine when the Arduino is browning out.
  (The Arduino has no external voltage reference, remember? It's all relative.)

  The result you get is a number 0-1023 that ranges from [0 - 4.???] volts. Note we don't know what ??? is.
  This number represents where the 1.1??v bandgap lies. This bandgap value varies per-IC chip, but is generally constant.
  
  By manually measuring Vcc with a multimeter BEFOREHAND, we can forward-calculate the bandgap voltage, and hardcode it.
  Later, when we are running readings, we can back-calculate what Vcc is (on-the-fly) when we change power sources/etc.

  Illustrated:
  [0v ---------1.1??v-------------------- 4.???v]  ------- 5v (notice this is off the chart! Especially when powered off USB.)
  [0 -----------||------------------------- 1023]
                ^^ This is where our bandgap reading will be. Note we initially don't even know what 1.1???v is.
                              First we need to read Vcc manually with a multimeter, then calculate!
*/
  
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    ADCSRB = ADCSRB & ~_BV(MUX5);
    //Serial.print(ADCSRB,BIN);
    //Serial.print("MEGA");
    //Serial.print(ADMUX,BIN);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  for (int i=0;i<7;i++){
    ADCSRA |= _BV(ADSC); // Start conversion
    while (bit_is_set(ADCSRA,ADSC)); // measuring
  }
  
  static uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  static uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;
//Serial.print("----");
//Serial.print(result);
//Serial.print("----");
  //result = 1125300L / result; // Calculate Vcc_bandgap (in mV); 1125300 = 1.1*1023*1000
  //4957 -> 4880 = 1107820 for my Mega
  //result = 1107820L / result;
  result = BANDGAPCALIBRATE / result;

  Serial.print("Vcc calibration complete (mV): ");
  Serial.print(result);
  return result; // Vcc in millivolts
}

void printDec(int num, int precision) {
  //Basic function to space interger values.
  //This function is unused
      char tmp[16];
      char format[16];

      sprintf(format, "%%.%du", precision);
      sprintf(tmp, format, num);
      Serial.print(tmp);
}

void printStr(const char *str, int width) {
  //Basic function to rigt-align text to a width
      char tmp[16];
      char format[16];

      sprintf(format,"%%-%ds", width);
      //Serial.print(format);
      sprintf(tmp, format, str);
      Serial.print(tmp);
}


void printSymbol( bitTable* myTable, uint8_t pin_x, uint8_t pin_y, uint8_t width, uint8_t a, uint8_t b)
{
  //Select a symbol to display based on tableData (x,y) relationship
  //Symbol meanings are noted in the "LEGEND" entry. (Ctrl+F for it.)

    //Basic case (self-pin)
    if (pin_x == pin_y)
      printStr("\\\\",width);

    //Symmetric short (aka no diode)
    else if ( (myTable->get(pin_x,pin_y)) &&  (myTable->get(pin_y,pin_x))  && (a == b))
      printStr("OO",width);

    //TYPE MISMATCH
    else if ( (myTable->get(pin_x,pin_y)) &&  (myTable->get(pin_y,pin_x))  && (a != b))
      printStr("XX",width);

    //ONE-WAY connection
    else if ( myTable->get(pin_x,pin_y) )
       printStr("!!",width);  

    //No connection
    else printStr("--",width);
}

// ======================================
// =========== MAIN LOOP ================
// ======================================


//Various utility variables, such as sstring buffers, storage for previous menus, etc.
//Mainly used for navigation of the text-based interface. AKA "Remember last selection"
  static char lineOut[128]={};
  static int prevMenu[]={-1,-1};
  static int currentMenu[]={-1,-1};
  static int myMode=-1;
  static int lastMode=-1;



//a and b are pointers to the breakoutMap of whatever is in BankA or BankB.
//It is far more efficient to just store a pointer to the conMap than the array.
//It also permis rapid assignment, rather than necessitating a memcpy() call.
  static const uint8_t *a;
  static const uint8_t *b;

void loop() {
  
//The most generic code ever. I use it as a status indicator if the board's fozen up.
//LED13 lit = waiting for instructions, LED13 dim = "I'm doing something"
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)

// ==========================================
// ============ MENU SELECT CODE ============
// ==========================================

Serial.println();
Serial.println("Enter connector types for BankA (top) and BankB (bottom) and test type (run):\n");
for(int i=0; i<menuOptions_size; i++){
  sprintf(lineOut,"\t%2d - %-30s\t%2d - %-30s",i+1,menuOptions[i].conName,i+1,menuOptions[i].conName);
  Serial.println(lineOut);
}
 Serial.println();

RETRYHERE:

Serial.println ("Enter two numbers to start, or enter for repeat setup.");
  

for(int i=0;i<2;i++){
    while(Serial.available() == 0){}
    currentMenu[i]=Serial.parseInt();
    Serial.print("I got a ");
    Serial.println(currentMenu[i]);
// FLUSH RETURN
while(Serial.peek()==10) Serial.read();

  if (i == 0 && currentMenu[0] == 0) {
    currentMenu[0]=prevMenu[0];
    currentMenu[1] =prevMenu[1];
    break;
  }

    int temp=Serial.parseInt();
    
    //FLUSH RETURN CHAR
    while(Serial.peek()==10) Serial.read();
    if (temp == 0){
      //FLUSH ALL
       while(Serial.available()>0)Serial.read();
    }
    else if (i == 0){
      currentMenu[i+1]=temp;
      break;
    }  
}

   Serial.println(currentMenu[0]);
   Serial.println(currentMenu[1]);
         
  for(int i=0;i<2;i++){
    if (currentMenu[i]<=0 || currentMenu[i]>menuOptions_size){
      Serial.println("Error, invalid selection.");
      goto RETRYHERE;
    }  
  }

  prevMenu[0]=currentMenu[0];
  prevMenu[1]=currentMenu[1];
  
  a= menuOptions[currentMenu[0]-1].conMap;
  b = menuOptions[currentMenu[1]-1].conMap;


//GOTO's are ugly things. Avoid them if possible.
//But they serve a purpose sometimes, like error handling.
RETRYHERE2:

Serial.println();
Serial.println (F("Enter test type:  t=Connection test    p=Rp test    d =Rd test    m=Monitor Mode (interactive/live)   (enter=previous test)"));

// FLUSH SPACES
while(Serial.peek()==32) Serial.read();
// FLUSH RETURNS
while(Serial.peek()==10) Serial.read();

    //READ    
    while(Serial.available() == 0){}
    myMode=Serial.read();
    Serial.print("I got a ");
    Serial.print((int)myMode);
    Serial.println((char)myMode);
    //FLUSH    
    while(Serial.available()>0) Serial.read();

    if(myMode==10) myMode=lastMode;

    switch(myMode){
      case 't':
      case 'T':
        digitalWrite(13, LOW);
        testMode();
        digitalWrite(13,HIGH);
      break;
      case 'm':
      case 'M':
        digitalWrite(13, LOW);
        monitorMode(a);
        digitalWrite(13,HIGH);
      break;
      case 'p':
      case 'P':
        digitalWrite(13, LOW);
        rpDetectMode(a);
        digitalWrite(13,HIGH);
        break;
      case 'd':
      case 'D':
        digitalWrite(13, LOW);
        rdDetectMode(a);
        digitalWrite(13,HIGH);
        break;
      default:
        Serial.println("Error: invalid mode type.");
       delay(100);
       //FLUSH
       while(Serial.available()>0)Serial.read();
       goto RETRYHERE2;
    }

    lastMode=myMode;
}



//======================================
//============ TEST MODE ===============
//======================================

void testMode(){

static int var1, var2;
static bitTable tableOut;

REDO:
Serial.println("--------START---------");
  inputPullupAll();
  tableOut.clearTable();



//==========WARNING===============
    Serial.println();
     Serial.println(F("WARNING: DO NOT RUN WHEN PLUGGED INTO Vbus HOT CHARGER! It may result in damage. Do you acknowledge?"));
     Serial.println("               Press any key to begin test");
     Serial.println();
          
  //FLUSH    
  delay(100);
  while(Serial.available()>0) Serial.read();
  while(Serial.available()==0) {}
  while(Serial.available()>0) Serial.read();

// ============================================
// ========== TABLE GENERATION CODE ===========
// ============================================

  for (int i = 0; i < read_size; i++) {
    delay(shiftDelay);

    //=========DANGER, REMEMBER TO TURN OFF============
    /*
    Setting pins to output mode with unknown connections is inherently dangerous. You can short out and damage Arduino pins easily.
    http://www.rugged-circuits.com/10-ways-to-destroy-an-arduino/
    
    For this sequence, and everywhere else, we first:
      - disable the INPUT_PULLUP pullup resistor (to set the pin to floating) by changing it to INPUT
      - set it to OUTPUT so the pin is now driven  [which it is now LOW (0v) since we changed it to INPUT]
      - write it LOW just in case (if it insn't aleady)
    */
    pinMode(read_pins[i],INPUT);
    pinMode(read_pins[i],OUTPUT);
    digitalWrite(read_pins[i], LOW);
    var1=0;

    for (int j = 0; j < read_size; j++) {

      //DO NOT attempt to read self
      if (read_pins[i] == read_pins[j]) {
        tableOut.set(i,j);
        continue;
      }
      
      delay(pinDelay);
      var2=digitalRead(read_pins[j]);
        
      if ( var2 == var1 ){
        tableOut.set(i,j);
        sprintf(lineOut,"HIT on %d and %d!\n",i,j);
        Serial.print(lineOut);
      }
      else {
        tableOut.clear(i,j);     
      }
    }
    
    //============ DANGER, RESTORING TO SAFE =============
    pinMode(read_pins[i],INPUT_PULLUP);
   
  }

  inputFloatAll();

// =========================================
// ========== TABLE DISPLAY CODE ===========
// =========================================

Serial.println();
sprintf(lineOut,"\t[%s] vs [%s]",menuOptions[currentMenu[0]-1].conName,menuOptions[currentMenu[1]-1].conName);
Serial.print(lineOut);

Serial.println();
Serial.println(F("\tLEGEND:\t\\\\ = (Self)\tOO = Symmetric/Type match \tXX = Symmetric/Type MISMATCH\t!! = Asymmetric/diode\t-- = NA"));
Serial.println();

//-------------------------
//Header 6-space formatting
//-------------------------

  //Serial.print("      ");
  Serial.print  ("-v +> ");
  for(int i=0;i < bankA_size; i++){
    if (truncate) if (a[i]==NA) continue;
    printStr(PINtyp_STRING[a[i]],6);
  }
  Serial.print("      ");
  for(int j=0;j < bankB_size; j++){
    if (truncate) if (b[j]==NA) continue;
    printStr(PINtyp_STRING[b[j]],6);
  }
    Serial.println();

//----------------------------
//Vertical 6-space lead bank A
//----------------------------

for(int l=0;l < bankA_size; l++){
  if (truncate) if (a[l]==NA) continue;
  printStr(PINtyp_STRING[a[l]],6); 
  
  for(int i=0;i < bankA_size; i++){
    if (truncate) if (a[i]==NA) continue;
    printSymbol(&tableOut,bankA[l],bankA[i],6, a[l], a[i]);   
  } 
  Serial.print("      ");
  for(int j=0;j < bankB_size; j++){
    if (truncate) if (b[j]==NA) continue;
    printSymbol(&tableOut,bankA[l],bankB[j],6, a[l], b[j]);  
  }
  Serial.println();
}

//----------------------------
//Vertical 6-space lead bank B
//----------------------------
Serial.println();

for(int l=0;l < bankB_size; l++){
  if (truncate) if (b[l]==NA) continue;
  printStr(PINtyp_STRING[b[l]],6); 
  
  for(int i=0;i < bankA_size; i++){
    if (truncate) if (a[i]==NA) continue;
    printSymbol(&tableOut,bankB[l],bankA[i],6, b[l], a[i]);
  } 
  Serial.print("      ");
  for(int j=0;j < bankB_size; j++){
    if (truncate) if (b[j]==NA) continue;
    printSymbol(&tableOut,bankB[l],bankB[j],6, b[l], b[j]);   
  }
  Serial.println();
}
  
  Serial.println("--------STOP---------");
   Serial.println("Press enter to continue (or r to repeat)");

       //FLUSH    
    while(Serial.available()>0) Serial.read();
   while(Serial.available() == 0){}
    int temp = Serial.read();

    delay(100);
   //FLUSH    
    while(Serial.available()>0) Serial.read();
    Serial.flush();
    
     //if(temp=='r' || temp =='R') goto REDO;
     switch(temp){
        case 'r':
        case 'R':
          goto REDO;
          break;
        default:
          break;
     }


}


//============================================
//============== MONITOR MODE  ===============
//============================================


void monitorMode(const uint8_t* a)
{
 
  int temp;
  int togVbus=0;
  int togCC1=0;
  int togCC2=0;
  int togGnd=0;

  // ==================================
  // === PLEASE CONSULT YOUR PINMAP ===
  // ==================================

  bool ccSwitch=false;
  bool vSwitch=false;
  bool gndSwitch=false;
  int altGnd=read_pins[bankA[24]]; //Hardcode GND as backup, will re-detect below.
  
  if((a[ACC1_SLAVE_PIN]!=CC1 && a[ACC1_SLAVE_PIN]!=CC2) && (a[ACC2_SLAVE_PIN]!=CC1 && a[ACC2_SLAVE_PIN]!=CC2)){
    Serial.println(F("ERROR, CONtyp does not present a valid CC line on analog input/Rd (given hardware)! CC disabled."));
    ccSwitch=false;
  }
  else ccSwitch=true;

  //Drive the Vbus pin OPPOSITE the CC pin.... hardcode it for now.
   
  if(a[3]!=VBUS && a[21]!=VBUS){
    Serial.println(F("ERROR, CONtyp does not present two VBUS lines on Analog input (given hardware)! VBUS disabled."));
    vSwitch=false;
  }
  else vSwitch=true;

  if(a[0]!=GND && a[24]!=GND){
    Serial.println(F("ERROR, CONtyp does not present two GND lines on Analog input (given hardware)! ALTERNATE GND USED."));
    gndSwitch=false;

    int j;
    for(j=25;j>=0;j--){
      if(a[j]==GND){
         altGnd=read_pins[bankA[j]];
         Serial.print("Using alternate ground BankA-");
         Serial.println(j);
         break;
      }

      //Find LAST GND pin in bankA
    }
    if (j!=-1) gndSwitch=true;
  }
  else gndSwitch=true;
              

    inputFloatAll();

     Serial.println();
     Serial.println(F("Press any key to begin monitoring, press ENTER to stop.    [HINT: apply ground to stabilize voltage readings.]"));
     Serial.println(F("     m=FLOAT ALL/ABORT    v=toggle voltage  g=toggle ground     (5.1k:)   1/2=toggle Rp-CC1/2    7/8=toggle 5.1k Rd-CC1/2"));
     Serial.println();
          
  //FLUSH    
  delay(100);
  while(Serial.available()>0) Serial.read();
  while(Serial.available()==0) {}
  while(Serial.available()>0) Serial.read();

  temp=0;

  while(temp != 10){
    cycleBank(a);
    delay(300);
    //Serial.available()
    temp=Serial.read();
    while(Serial.available()>0) Serial.read();


    switch(temp){
      case 'm':
      case 'M':
        inputFloatAll();
        togVbus=0;
        togCC1=0;
        togCC2=0;
        togGnd=0;
        Serial.println("SHUT. DOWN. EVERYTHING!");
      break;
      case 'v':
      case 'V':
        if(vSwitch==false){
          Serial.println("Vbus switching is disabled!");
          break;
        }
        if(togVbus!=1){
          Serial.println("CAUTION. Supply power for this burst.");
          pinMode(read_pins[bankA[21]],INPUT);
          digitalWrite(read_pins[bankA[21]],HIGH);  //Power on pullup
          pinMode(read_pins[bankA[21]],OUTPUT);     //Power on heavy
          digitalWrite(read_pins[bankA[21]],HIGH);  //VBUS HOT
        }
        else {
          Serial.println("CAUTION. Removing power.");
          pinMode(read_pins[bankA[21]],INPUT);
        }
        togVbus=!togVbus;
      break;

      case 'g':
      case 'G':
        if(gndSwitch==false){
          Serial.println("Gnd switching is disabled!");
          break;
        }
        if(togGnd!=1){
          Serial.println("CAUTION. Supply ground for this burst.");
          pinMode(altGnd,INPUT);
          digitalWrite(altGnd,LOW);  //remove pullup
          pinMode(altGnd,OUTPUT);     //Power on heavy
          digitalWrite(altGnd,LOW);  //GND CON 
        }
        else {
          Serial.println("CAUTION. Removing ground.");
          pinMode(altGnd,INPUT);
        }
        togGnd=!togGnd;
      break;

      case '1':
        if(ccSwitch==false){
          Serial.println("CC switching is disabled!");
          break;
        }

        //4-10 20-11
        if(togCC1!=1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[ACC1_SLAVE_PIN]]);
          Serial.println("-Rp [5.1k] for this burst.");
          pinMode(ACC1_DRIVER_PIN,INPUT_PULLUP);
          digitalWrite(ACC1_DRIVER_PIN,HIGH);
          pinMode(ACC1_DRIVER_PIN,OUTPUT);
          digitalWrite(ACC1_DRIVER_PIN,HIGH);
          togCC1=1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[ACC1_SLAVE_PIN]]);
          Serial.println("-Rp.");
          pinMode(ACC1_DRIVER_PIN,INPUT);
          togCC1=0;
        }
      break;
      case '2':
        if(ccSwitch==false){
          Serial.println("CC switching is disabled!");
          break;
        }

        //4-10 20-11
        if(togCC2!=1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[ACC2_SLAVE_PIN]]);
          Serial.println("-Rp [5.1k] for this burst.");
          pinMode(ACC2_DRIVER_PIN,INPUT_PULLUP);
          digitalWrite(ACC2_DRIVER_PIN,HIGH);
          pinMode(ACC2_DRIVER_PIN,OUTPUT);
          digitalWrite(ACC2_DRIVER_PIN,HIGH);
          togCC2=1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[ACC2_SLAVE_PIN]]);
          Serial.println("-Rp.");
          pinMode(ACC2_DRIVER_PIN,INPUT);
          togCC2=0;
        }
      break;


      case '7':
        if(ccSwitch==false){
          Serial.println("CC switching is disabled!");
          break;
        }

        //4-10 20-11
        if(togCC1!=-1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[ACC1_SLAVE_PIN]]);
          Serial.println("-Rd [5.1k] for this burst.");
          pinMode(ACC1_DRIVER_PIN,INPUT);
          digitalWrite(ACC1_DRIVER_PIN,LOW);
          pinMode(ACC1_DRIVER_PIN,OUTPUT);
          digitalWrite(ACC1_DRIVER_PIN,LOW);
          togCC1=-1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[ACC1_SLAVE_PIN]]);
          Serial.println("-Rd.");
          pinMode(ACC1_DRIVER_PIN,INPUT);
          togCC1=0;
        }
      break;
      case '8':
        if(ccSwitch==false){
          Serial.println("CC switching is disabled!");
          break;
        }

        if(togCC2!=-1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[ACC2_SLAVE_PIN]]);
          Serial.println("-Rd [5.1k] for this burst.");
          pinMode(ACC2_DRIVER_PIN,INPUT);
          digitalWrite(ACC2_DRIVER_PIN,LOW);
          pinMode(ACC2_DRIVER_PIN,OUTPUT);
          digitalWrite(ACC2_DRIVER_PIN,LOW);
          togCC2=-1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[ACC2_SLAVE_PIN]]);
          Serial.println("-Rd.");
          pinMode(ACC2_DRIVER_PIN,INPUT);
          togCC2=0;
        }
      break;
      
      default:
      break;
    }
  }
    
   //FLUSH    
    while(Serial.available()>0) Serial.read();

    inputFloatAll();
}


//================= CYCLEBANK SUBFUNCTION ===================

void cycleBank(const uint8_t* a)
{
 int temp; 
  Serial.print("      ");
  for(int i=0;i<13&& a[i]!=NA; i++){
    printStr(PINtyp_STRING[a[i]],7);
  }
  Serial.println();

  Serial.print("     ");
  for(int i=0;i<13 && a[i]!=NA; i++){
    Serial.print("[");
    if(read_pins[bankA[i]]>=54 && read_pins[bankA[i]]<=69){
      temp=analogRead(read_pins[bankA[i]]);
      Serial.print(((float)temp)/1023.0*vcc_actual/1000.0);
    }
    else{
      temp=digitalRead(read_pins[bankA[i]]);
      Serial.print((temp)?"HIGH":" LO ");
    }      
    Serial.print("] ");
  }
  Serial.println();
  //--------second row-----------
  
  Serial.print("      ");
  for(int i=13;i<26&& a[i]!=NA; i++){
    printStr(PINtyp_STRING[a[i]],7);
  }
  Serial.println();
  Serial.print("     ");
  for(int i=13;i<26&& a[i]!=NA; i++){
    Serial.print("[");
    if(read_pins[bankA[i]]>=54 && read_pins[bankA[i]]<=69){
      temp=analogRead(read_pins[bankA[i]]);
      Serial.print(((float)temp)/1023.0*vcc_actual/1000.0);
    }
    else{
      temp=digitalRead(read_pins[bankA[i]]);
      Serial.print((temp)?"HIGH":" LO ");
    }      
    Serial.print("] ");
  }
  Serial.println();


    Serial.println();    
}




//====================================
//========== RP DETECT MODE ==========
//====================================

void rpDetectMode(const uint8_t* a)
{
  static int cc1;
  static int cc2;
  static int vbus_bridge;
  static int vbus;
  static int temp;
  REDORP:
  Serial.println("--------START---------");

  cc1=0;
  cc2=0;
  vbus_bridge=0;
  vbus=0;
  temp=0;
  
  Serial.println();
  
  bool ccSwitch=false;
  bool vSwitch=false;
  if((a[ACC1_SLAVE_PIN]!=CC1 && a[ACC1_SLAVE_PIN]!=CC2) && (a[ACC2_SLAVE_PIN]!=CC1 && a[ACC2_SLAVE_PIN]!=CC2)){
    Serial.println("ERROR, CONtyp does not present a valid CC line on analog input/Rd (given hardware)! CC disabled.");
    ccSwitch=false;
  }
  else ccSwitch=true;

  //Drive the Vbus pin OPPOSITE the CC pin.... hardcode it for now.
   
  if(a[3]!=VBUS && a[21]!=VBUS){
    Serial.println(F("ERROR, CONtyp does not present two VBUS lines on Analog input (given hardware)! VBUS disabled."));
    vSwitch=false;
  }
  else vSwitch=true;
              
  int ccpins[3]={-1,10,11};
  for(int k=0;k<3;k++){              
                
                //Safety the pins
                inputFloatAll();
                        
                if(ccSwitch){
                  if(ccpins[k]!=-1){
                    pinMode(ccpins[k],OUTPUT);
                    digitalWrite(ccpins[k],LOW);
                  }

                }
              
                //----------ALLOW VOLTAGE SOURCE TO SWITCH ON--------------
                delay(vbusDelayOn);

                if(ccpins[k]==-1) Serial.println("Baseline (No CC driven):");          

                if(vSwitch && ccpins[k]!=-1){
                  analogRead(read_pins[bankA[21]]); //--- SPECIAL CASE FOR VBUS--- (to cleaer out junk)
                  
                  if(analogRead(read_pins[bankA[21]])<=3.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[21]])<=3.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[21]])<=3.5*1024.0/(vcc_actual/1000.0)){
                       Serial.println("CAUTION. (Floating) Vbus COLD. Supply power for this burst.");

                    digitalWrite(read_pins[bankA[21]],HIGH);  //Power on pullup
                    pinMode(read_pins[bankA[21]],OUTPUT);     //Power on heavy
                    digitalWrite(read_pins[bankA[21]],HIGH);  //VBUS HOT 
                  }
                  else {
                    Serial.println("WARNINING! (Floating) Vbus HOT already? Charger connected/CC ON? Aborting driver, monitoring only.");
                  }
                
                }
              
              
                cycleBank(a);
              
                if (ccSwitch && vSwitch){
                  temp=analogRead(read_pins[bankA[ACC1_SLAVE_PIN]]);
                  (a[ACC1_SLAVE_PIN]==CC1)? cc1=temp:cc2=temp;
              
                  temp=analogRead(read_pins[bankA[ACC2_SLAVE_PIN]]);
                  (a[ACC2_SLAVE_PIN]==CC2)? cc2=temp:cc1=temp;
              
                  temp=analogRead(read_pins[bankA[3]]);
                  vbus_bridge=temp;
              
                  temp=analogRead(read_pins[bankA[21]]);
                  vbus=temp;
              
              
                  if((float)abs(vbus_bridge-vbus)/1023.0*vcc_actual/1000.0 > .2) Serial.println(F("ERROR! VBUSro/VBUS not bridged (dV > .2)! Cable error?"));
                }
              
                //======= SHUTOFF HERE MOVED BELOW============
                
                Serial.print("      ");

                if((ccpins[k]==ACC1_DRIVER_PIN && a[ACC1_SLAVE_PIN] == CC1) || (ccpins[k]==ACC2_DRIVER_PIN && a[ACC2_SLAVE_PIN] == CC1))
                    printStr(">CC1<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC1-v",7);
                else
                   printStr("Vconn",7); 
                   
                if((ccpins[k]==ACC2_DRIVER_PIN && a[ACC2_SLAVE_PIN] == CC2) || (ccpins[k]==ACC1_DRIVER_PIN && a[ACC1_SLAVE_PIN] == CC2))
                    printStr(">CC2<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC2-v",7);                    
                else 
                   printStr("Vconn",7); 


                printStr("VBUSro",8);
                printStr("VBUS",7);
                printStr("Rp-1",7);
                printStr("Rp-2",7);
                
                Serial.println();
                Serial.print("     [");
                Serial.print((float)cc1/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                Serial.print((float)cc2/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                Serial.print((float)vbus_bridge/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                Serial.print((float)vbus/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                //Serial.print((   (float)(vbus_bridge-cc1)/((float)(cc1)/(float)(5100))     )); //Assumes non-ideal Vcc source
                Serial.print((   (float)(vbus-cc1)/((float)(cc1)/(float)(5100))     )); //Assumes ideal voltage source
                Serial.print("]  [");
                //Serial.print((   (float)(vbus_bridge-cc2)/((float)(cc2)/(float)(5100))     )); 
                Serial.print((   (float)(vbus-cc2)/((float)(cc2)/(float)(5100))     )); 
                Serial.print("]");
                Serial.println();

                Serial.println();

                inputFloatAll();
                //----------ALLOW VOLTAGE SOURCE TO SWITCH OFF--------------
                delay(vbusDelayOff);
  }
  
   Serial.println("--------STOP---------");
   Serial.println("Press enter to continue (or r to repeat)");

       //FLUSH    
    while(Serial.available()>0) Serial.read();
   while(Serial.available() == 0){}
   temp = Serial.read();

    delay(100);
   //FLUSH    
    while(Serial.available()>0) Serial.read();
    Serial.flush();
    
     switch(temp){
        case 'r':
        case 'R':
          goto REDORP;
          break;
        default:
          break;
     }
}




//====================================
//========== RD DETECT MODE ==========
//====================================

void rdDetectMode(const uint8_t* a)
{
  static int cc1;
  static int cc2;
  static int gnd_bridge;
  static int gnd;
  static int temp;
  REDORD:
  Serial.println("--------START---------");

  cc1=0;
  cc2=0;
  gnd_bridge=0;
  gnd=0;
  temp=0;
  
  Serial.println();
  
  bool ccSwitch=false;
  bool gndSwitch=false;
  if((a[ACC1_SLAVE_PIN]!=CC1 && a[ACC1_SLAVE_PIN]!=CC2) && (a[ACC2_SLAVE_PIN]!=CC1 && a[ACC2_SLAVE_PIN]!=CC2)){
    Serial.println(F("ERROR, CONtyp does not present a valid CC line on analog input/Rd (given hardware)! CC disabled."));
    ccSwitch=false;
  }
  else ccSwitch=true;

  //Drive the Gnd pin OPPOSITE the CC pin.... hardcode it for now.
   
  if(a[0]!=GND && a[24]!=GND){
    Serial.println(F("ERROR, CONtyp does not present two GND lines on Analog input (given hardware)! GND disabled."));
    gndSwitch=false;
  }
  else gndSwitch=true;
              
  int ccpins[3]={-1,10,11};
  for(int k=0;k<3;k++){              
                
                //Safety the pins
                inputFloatAll();
                        
                if(ccSwitch){
                  if(ccpins[k]!=-1){
                    pinMode(ccpins[k],OUTPUT);
                    digitalWrite(ccpins[k],HIGH);
                  }

                }
              
                //----------ALLOW VOLTAGE SOURCE TO SWITCH ON--------------
                delay(vbusDelayOn);
                
                if(ccpins[k]==-1) Serial.println("Baseline (No CC driven):");          
                
                if(gndSwitch && ccpins[k]!=-1){
                  pinMode(read_pins[bankA[0]],INPUT_PULLUP);   //--- SPECIAL CASE FOR GND---

                  if(analogRead(read_pins[bankA[24]])>=1.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[24]])>=1.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[24]])>=1.5*1024.0/(vcc_actual/1000.0)){
                       Serial.println("CAUTION. (Floating) Gnd N/C. Supply ground for this burst.");

                    digitalWrite(read_pins[bankA[24]],LOW);  //remove pullup
                    pinMode(read_pins[bankA[24]],OUTPUT);     //Power on heavy
                    digitalWrite(read_pins[bankA[24]],LOW);  //GND CON 
                  }
                  else {
                    Serial.println("WARNINING! (Floating) Gnd CON already? Charger connected/CC ON? Aborting driver, monitoring only.");
                    pinMode(read_pins[bankA[24]],INPUT); //--- SPECIAL CASE FOR GND---
                  }
                                  
                }
              
              
                cycleBank(a);
              
                if (ccSwitch && gndSwitch){
                  temp=analogRead(read_pins[bankA[ACC1_SLAVE_PIN]]);
                  (a[ACC1_SLAVE_PIN]==CC1)? cc1=temp:cc2=temp;
              
                  temp=analogRead(read_pins[bankA[ACC2_SLAVE_PIN]]);
                  (a[ACC2_SLAVE_PIN]==CC2)? cc2=temp:cc1=temp;
              
                  temp=analogRead(read_pins[bankA[0]]);
                  gnd_bridge=temp;
              
                  temp=analogRead(read_pins[bankA[24]]);
                  gnd=temp;

              
                  if((float)abs(gnd_bridge-gnd)/1023.0*vcc_actual/1000.0 > .2) Serial.println("ERROR! GNDro/GND not bridged (dV > .2)! Cable error?");
                }
              
                //======= SHUTOFF HERE MOVED BELOW============
                
                Serial.print("      ");

                if((ccpins[k]==ACC1_DRIVER_PIN && a[ACC1_SLAVE_PIN] == CC1) || (ccpins[k]==ACC2_DRIVER_PIN && a[ACC2_SLAVE_PIN] == CC1))
                    printStr(">CC1<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC1-v",7);
                else
                   printStr("Vconn",7); 
                   
                if((ccpins[k]==ACC2_DRIVER_PIN && a[ACC2_SLAVE_PIN] == CC2) || (ccpins[k]==ACC1_DRIVER_PIN && a[ACC1_SLAVE_PIN] == CC2))
                    printStr(">CC2<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC2-v",7);                    
                else 
                   printStr("Vconn",7); 


                printStr("GNDro",8);
                printStr("GND",7);
                printStr("Rd-1",7);
                printStr("Rd-2",7);
                
                Serial.println();
                Serial.print("     [");
                Serial.print((float)cc1/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                Serial.print((float)cc2/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                Serial.print((float)gnd_bridge/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                Serial.print((float)gnd/1023.0*vcc_actual/1000.0);
                Serial.print("] [");
                //Serial.print((   (float)(vbus_bridge-cc1)/(((float)(1023-cc1))/(float)(5100))     )); //Assumes non-ideal Vcc source
                Serial.print((   (float)(cc1)/((float)( 1023 - cc1)/(float)(5100))     )); //Assumes ideal voltage source
                Serial.print("]  [");
                //Serial.print((   (float)(vbus_bridge-cc2)/((float)(cc2)/(float)(5100))     )); 
                Serial.print((   (float)(cc2)/((float)( 1023 - cc2)/(float)(5100))     )); 
                Serial.print("]");
                Serial.println();

                Serial.println();

                inputFloatAll();
                //----------ALLOW VOLTAGE SOURCE TO SWITCH OFF--------------
                delay(vbusDelayOff);
  }
  
   Serial.println("--------STOP---------");
   Serial.println("Press enter to continue (or r to repeat)");

       //FLUSH    
    while(Serial.available()>0) Serial.read();
   while(Serial.available() == 0){}
   temp = Serial.read();

    delay(100);
   //FLUSH    
    while(Serial.available()>0) Serial.read();
    Serial.flush();
    
     switch(temp){
        case 'r':
        case 'R':
          goto REDORD;
          break;
        default:
          break;
     }
}

