#include <avr/pgmspace.h>
/* Sanity check for Input_Pullup mode*/


//TODO: Fix voltage comparison so is differential.
//      Add bandgap calibration fix

// add "read delay" to let input cap recharge??
  static const int pinDelay=0;
  static const int shiftDelay=0;
  static const int truncate=1;
  static const int vbusDelayOn=275*2;
  static const int vbusDelayOff=650*2;
  static long vcc_actual=0;
#define BANDGAPCALIBRATE   1113098L

/*
4.11.2 Timing Parameter
tVBUSON 0 ms 275 ms
From entry to Attached.SRC until VBUS reaches the minimum vSafe5V threshold as measured at the source’s
receptacle.

tVBUSOFF 0 ms 650 ms
From the time the Sink is detached until the Source removes VBUS and
reaches vSafe0V (See USB PD).
*/

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

/*
      OPEN,
      GND,
      VBUS,
      DP,
      DN,
      TX1P, TX1N, CC1, SBU1, RX2N, RX2P,
      TX2P, TX2N, CC2, SBU2, RX1N, RX1P,
      SSRXN, SSRXP, GND_D, SSTXN, SSTXP, SHELL,
      ID
*/

        
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum PINtyp_ENUM {
    FOREACH_PINtyp(GENERATE_ENUM)
};

static const char *PINtyp_STRING[] = {
    FOREACH_PINtyp(GENERATE_STRING)
};




//Set a dynamic array of pins.....

  //static const uint8_t read_pins[] = {A10,A11,A12,A13,A14,A15};
  //static const uint8_t read_pins[] = {7,6,5,4,3,2,1,0};
  //static const uint8_t read_pins[52] = {12,9,7,6,5,3,2,14,15,16,17,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,A15,A14,A13,A12,A11,A10,A9,A7,A6,A5,A4,A1,A0};
  //static const uint8_t read_size = (sizeof(read_pins)/sizeof(uint8_t));


//===== PHYSICAL LAYOUT to GUID mapping ====

static const uint8_t bankA[] = 
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
static const uint8_t bankA_size = 26;
    
//  static const uint8_t bankA[] =   
//  { 63, 64, 65, 66, 67, 25, 23, 24, 22, 17, 16, 15, 14,
//    61, 60, 59, 58, 55, 54, 12,  9,  7,  6,  5,  3,  2 };

static const uint8_t bankB[] = 
  { 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };
static const uint8_t bankB_size = 26;

//  static const uint8_t bankB[] =  
//  { 69, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26,
//    68, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27 };


//Analog mask is >=54 <=69.

//====== GUID to ARDUINO PIN # mapping =======

static const uint8_t read_pins[52] = {
    63, 64, 65, 66, 67, 25, 23, 24, 22, 17, 16, 15, 14,
    61, 60, 59, 58, 55, 54, 12,  9,  7,  6,  5,  3,  2,
    
    69, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26,
    68, 49, 47, 45, 43, 41, 39, 37, 35, 33, 31, 29, 27 };
static const uint8_t read_size = (sizeof(read_pins)/sizeof(uint8_t));

// ================= DEFINING PIN NAMES/MASKS PER BANK =================

static const uint8_t UsbNULL[] =
{   0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0,
    0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};

static const uint8_t UsbCF[] =
{ GND, TX1P, TX1N, VBUS, CC1, DP, DN, SBU1, VBUS, RX2N, RX2P, GND, 0,
  GND, TX2P, TX2N, VBUS, CC2, DP, DN, SBU2, VBUS, RX1N, RX1P, GND, 0 };

static const uint8_t UsbCM[] =
{ GND, RX2P, RX2N, VBUS, SBU1, DN, DP, CC1, VBUS, TX1N, TX1P, GND, 0,
  GND, RX1P, RX1N, VBUS, SBU2, DN, DP, CC2, VBUS, TX2N, TX2P, GND, 0 };

static const uint8_t UsbCM_REV[] =
{ GND, TX2P, TX2N, VBUS, CC2, DP, DN, SBU2, VBUS, RX1N, RX1P, GND, 0,
  GND, TX1P, TX1N, VBUS, CC1, DP, DN, SBU1, VBUS, RX2N, RX2P, GND, 0 };
    

static const uint8_t UsbA3F[] =
{ VBUS, DN, DP, GND, SSRXN, SSRXP, GND_D, SSTXN, SSTXP, SHELL, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0};

static const uint8_t UsbA3M[] =
{ SHELL, SSRXN, SSRXP, GND_D, SSTXN, SSTXP, GND, DP, DN, VBUS, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}; 

static const uint8_t UsbA3M_REV[] =
{ VBUS, DN, DP, GND, SSTXP, SSTXN, GND_D, SSRXP, SSRXN, SHELL, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}; 

static const uint8_t UsbB3F[] =
{ VBUS, DN, DP, GND, SSTXN, SSTXP, GND_D, SSRXN, SSRXP, SHELL, 0, 0, 0,
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

typedef struct {
  char conName[32];
  const uint8_t* conMap;
  uint8_t conUID;
} CONtyp_STRUCT;

CONtyp_STRUCT menuOptions[12] ={
  {"NULL / NONE", UsbNULL,1},
  {"Type-C Female", UsbCF,2},
  {"Type-C Male", UsbCM,3},
  {"A 3.0 Female", UsbA3F,4},
  {"A 3.0 Male", UsbA3M,5},
  {"B 3.0 Female", UsbB3F,6},
  {"microB 3.0 Female", UsbuB3F,7},
  {"microB 2.0 Male", UsbuB2M,8},
  {"micro/miniB 2.0 Female", UsbumB2F,9},
  {"miniB 2.0 Male", UsbmB2M,10},
  {"[FACE OUT] Type-C Male",UsbCM_REV,11},
  {"[FACE OUT] A 3.0 Male",UsbA3M_REV,12}
};
int8_t menuOptions_size=12;


//b1[3] = {
//           {"Let us C",700,"YPK",300.00},
//           {"Wings of Fire",500,"APJ Abdul Kalam",350.00},
//           {"Complete C",1200,"Herbt Schildt",450.00}
//         };





//================ CLASSES =========================


class bitTable
{
   private:
      uint8_t tableData[52][52];
   public:
      // required constructors
      bitTable(){
         this->clearTable();
      }
//      void operator=(const uint8_t data )
//      { 
//         feet = D.feet;
//         inches = D.inches;
//      }
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

}


// =============== FUNCTIONS ===================

void inputPullupAll(){
  //Set all pins in range to weak INPUT_PULLUP
  for (int i = 0; i < read_size; i++) { //or i <= 4
    pinMode(read_pins[i],INPUT_PULLUP);
  }

  //Make drivers 10/11 floating
    pinMode(10,INPUT);
    pinMode(11,INPUT);
}

void inputFloatAll(){
  //Set all pins in range to weak INPUT_PULLUP
  for (int i = 0; i < read_size; i++) { //or i <= 4
    pinMode(read_pins[i],INPUT);
  }
  //Make drivers 10/11 floating
    pinMode(10,INPUT);
    pinMode(11,INPUT);
}

int getPinMode(uint8_t pin)
{
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
  //result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  //4957 -> 4880 = 1107820 for my Mega
  //result = 1107820L / result;
  result = BANDGAPCALIBRATE / result;

  Serial.print("Vcc calibration complete (mV): ");
  Serial.print(result);
  return result; // Vcc in millivolts
}

void printDec(int num, int precision) {
      char tmp[16];
      char format[16];

      sprintf(format, "%%.%du", precision);
      sprintf(tmp, format, num);
      Serial.print(tmp);
}

void printStr(const char *str, int width) {
      char tmp[16];
      char format[16];

      sprintf(format,"%%-%ds", width);
      //Serial.print(format);
      sprintf(tmp, format, str);
      Serial.print(tmp);
}


void printSymbol( bitTable* myTable, uint8_t pin_x, uint8_t pin_y, uint8_t width, uint8_t a, uint8_t b)
{

    //Basic case
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


static char lineOut[128]={};
static int prevMenu[]={-1,-1};
static int currentMenu[]={-1,-1};
static int myMode=-1;
static int lastMode=-1;
  //********DEBUG STUFF GOES HERE******
  static const uint8_t *a;
  static const uint8_t *b;

void loop() {

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

////FLUSH
//    while(Serial.available()>0) Serial.read();

//READ    

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
    // FLUSH RETURN
    while(Serial.peek()==10) Serial.read();
    if (temp == 0){
      //FLUSH
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
//
//    //FLUSH    
//    while(Serial.available()>0)
//     {
//     Serial.read();
//     }
//  
  
  a= menuOptions[currentMenu[0]-1].conMap;
  b = menuOptions[currentMenu[1]-1].conMap;


RETRYHERE2:
Serial.println();
Serial.println (F("Enter test type:  t=Connection test    p=Rp test    d =Rd test    m=Monitor Mode (interactive/live)   (enter=previous test)"));

//    //FLUSH
//    while(Serial.available()>0) Serial.read(); 

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
        digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
        testMode();
        digitalWrite(13,HIGH);
      break;
      case 'm':
      case 'M':
        digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
        monitorMode(a);
        digitalWrite(13,HIGH);
      break;
      case 'p':
      case 'P':
        digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
        rpDetectMode(a);
        digitalWrite(13,HIGH);
      case 'd':
      case 'D':
        digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
        rdDetectMode(a);
        digitalWrite(13,HIGH);
        
      break;
      default:
        Serial.println("Errorm invalid mode type.");
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
REDO:
Serial.println("--------START---------");
  inputPullupAll();

  static int var1, var2;
  static bitTable tableOut;
  tableOut.clearTable();



//==========WARNING===============
    Serial.println();
     Serial.println(F("WARNING: DO NOT RUN WHEN PLUGGED INTO Vbus HOT CHARGER!!!! It may result in damage. Do you acknowledge?"));
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
    
//    delay(pinDelay);
//    var1=digitalRead(read_pins[i]);
//    if(var != 1) Serial.print("ERROR!");

    //=========DANGER, REMEMBER TO TURN OFF============
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

  Serial.print("      ");
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
//  static const uint8_t UsbCF[] =
//{ GND, TX1P, TX1N, VBUS, CC1, DP, DN, SBU1, VBUS, RX2N, RX2P, GND, 0,
//  GND, TX2P, TX2N, VBUS, CC2, DP, DN, SBU2, VBUS, RX1N, RX1P, GND, 0 };

  bool ccSwitch=false;
  bool vSwitch=false;
  bool gndSwitch=false;
  int altGnd=read_pins[bankA[13]]; //HArdcode GND, fix later.
  
  if((a[4]!=CC1 && a[4]!=CC2) && (a[17]!=CC1 && a[17]!=CC2)){
    Serial.println(F("ERROR, CONtyp does not present a valid CC line on analog input/Rd (given hardware)! CC disabled."));
    ccSwitch=false;
  }
  else ccSwitch=true;

  //Drive the Vbus pin OPPOSITE the CC pin.... hardcode it for now.
   
  if(a[3]!=VBUS && a[16]!=VBUS){
    Serial.println(F("ERROR, CONtyp does not present two VBUS lines on Analog input (given hardware)! VBUS disabled."));
    vSwitch=false;
  }
  else vSwitch=true;

  if(a[0]!=GND && a[13]!=GND){
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
        if(vSwitch=false){
          Serial.println("Vbus switching is disabled!");
          break;
        }
        if(togVbus!=1){
          Serial.println("CAUTION. Supply power for this burst.");
          pinMode(read_pins[bankA[16]],INPUT);
          digitalWrite(read_pins[bankA[16]],HIGH);  //Power on pullup
          pinMode(read_pins[bankA[16]],OUTPUT);     //Power on heavy
          digitalWrite(read_pins[bankA[16]],HIGH);  //VBUS HOT
        }
        else {
          Serial.println("CAUTION. Removing power.");
          pinMode(read_pins[bankA[16]],INPUT);
        }
        togVbus=!togVbus;
      break;

      case 'g':
      case 'G':
        if(gndSwitch=false){
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
        if(ccSwitch=false){
          Serial.println("CC switching is disabled!");
          break;
        }

        //4-10 17-11
        if(togCC1!=1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[4]]);
          Serial.println("-Rp [5.1k] for this burst.");
          pinMode(10,INPUT_PULLUP);
          digitalWrite(10,HIGH);
          pinMode(10,OUTPUT);
          digitalWrite(10,HIGH);
          togCC1=1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[4]]);
          Serial.println("-Rp.");
          pinMode(10,INPUT);
          togCC1=0;
        }
      break;
      case '2':
        if(ccSwitch=false){
          Serial.println("CC switching is disabled!");
          break;
        }

        //4-10 17-11
        if(togCC2!=1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[17]]);
          Serial.println("-Rp [5.1k] for this burst.");
          pinMode(11,INPUT_PULLUP);
          digitalWrite(11,HIGH);
          pinMode(11,OUTPUT);
          digitalWrite(11,HIGH);
          togCC2=1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[17]]);
          Serial.println("-Rp.");
          pinMode(11,INPUT);
          togCC2=0;
        }
      break;


      case '7':
        if(ccSwitch=false){
          Serial.println("CC switching is disabled!");
          break;
        }

        //4-10 17-11
        if(togCC1!=-1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[4]]);
          Serial.println("-Rd [5.1k] for this burst.");
          pinMode(10,INPUT);
          digitalWrite(10,LOW);
          pinMode(10,OUTPUT);
          digitalWrite(10,LOW);
          togCC1=-1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[4]]);
          Serial.println("-Rd.");
          pinMode(10,INPUT);
          togCC1=0;
        }
      break;
      case '8':
        if(ccSwitch=false){
          Serial.println("CC switching is disabled!");
          break;
        }

        if(togCC2!=-1){
          Serial.print("CAUTION. Supply ");
          Serial.print(PINtyp_STRING[a[17]]);
          Serial.println("-Rd [5.1k] for this burst.");
          pinMode(11,INPUT);
          digitalWrite(11,LOW);
          pinMode(11,OUTPUT);
          digitalWrite(11,LOW);
          togCC2=-1;
        }
        else {
          Serial.print("CAUTION. Removing ");
          Serial.print(PINtyp_STRING[a[17]]);
          Serial.println("-Rd.");
          pinMode(11,INPUT);
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
  if((a[4]!=CC1 && a[4]!=CC2) && (a[17]!=CC1 && a[17]!=CC2)){
    Serial.println("ERROR, CONtyp does not present a valid CC line on analog input/Rd (given hardware)! CC disabled.");
    ccSwitch=false;
  }
  else ccSwitch=true;

  //Drive the Vbus pin OPPOSITE the CC pin.... hardcode it for now.
   
  if(a[3]!=VBUS && a[16]!=VBUS){
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
                  //pinMode(10,OUTPUT); //A-4
                  //pinMode(11,OUTPUT); //A-17
              //          digitalWrite(10,LOW);
              //          digitalWrite(11,LOW);   
                }
              
                //----------ALLOW VOLTAGE SOURCE TO SWITCH ON--------------
                delay(vbusDelayOn);

                if(ccpins[k]==-1) Serial.println("            Baseline (No CC driven):");          

                if(vSwitch && ccpins[k]!=-1){
                  if(analogRead(read_pins[bankA[16]])<=3.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[16]])<=3.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[16]])<=3.5*1024.0/(vcc_actual/1000.0)){
                       Serial.println("CAUTION. (Floating) Vbus COLD. Supply power for this burst.");
              //      digitalWrite(A4,HIGH);  //Power on pullup
              //      pinMode(A4,OUTPUT);     //Power on heavy
              //      digitalWrite(A4,HIGH);  //VBUS HOT
                    digitalWrite(read_pins[bankA[16]],HIGH);  //Power on pullup
                    pinMode(read_pins[bankA[16]],OUTPUT);     //Power on heavy
                    digitalWrite(read_pins[bankA[16]],HIGH);  //VBUS HOT 
                  }
                  else {
                    Serial.println("WARNINING! (Floating) Vbus HOT already? Charger connected/CC ON? Aborting driver, monitoring only.");
                  }
                
                }
              
              
                cycleBank(a);
              
                if (ccSwitch && vSwitch){
                  temp=analogRead(read_pins[bankA[4]]);
                  (a[4]==CC1)? cc1=temp:cc2=temp;
              
                  temp=analogRead(read_pins[bankA[17]]);
                  (a[17]==CC2)? cc2=temp:cc1=temp;
              
                  temp=analogRead(read_pins[bankA[3]]);
                  vbus_bridge=temp;
              
                  temp=analogRead(read_pins[bankA[16]]);
                  vbus=temp;
              
              //    Serial.print(cc1/1023.0*5.0);
              //    Serial.print(cc2/1023.0*5.0);
              //    Serial.print(vbus/1023.0*5.0);
              //    Serial.print(vbus_bridge/1023.0*5.0);
              //    
              //      for(int i=0;i<26; i++){
              //        if(read_pins[bankA[i]]>=54 && read_pins[bankA[i]]<=69){
              //          if(a[i]==CC1 || a[i]==CC2){
              //            temp=analogRead(read_pins[bankA[i]]);
              //          }
              //          if(a[i]==CC1) cc1=temp;
              //          if(a[i]==CC2) cc2=temp;
              //        }
              //      }
                
              //    cc1=analogRead(read_pins[bankA[4]]);
              //    cc2=analogRead(read_pins[bankA[17]]);
              
                  if((float)abs(vbus_bridge-vbus)/1023.0*vcc_actual/1000.0 > .2) Serial.println(F("ERROR! VBUSro/VBUS not bridged (dV > .2)! Cable error?"));
                }
              
                //======= SHUTOFF HERE MOVED BELOW============
                
                Serial.print("      ");

                if((ccpins[k]==10 && a[4] == CC1) || (ccpins[k]==11 && a[17] == CC1))
                    printStr(">CC1<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC1-v",7);
                else
                   printStr("Vconn",7); 
                   
                if((ccpins[k]==11 && a[17] == CC2) || (ccpins[k]==10 && a[4] == CC2))
                    printStr(">CC2<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC2-v",7);                    
                else 
                   printStr("Vconn",7); 


                
//                printStr("CC1",7);
//                printStr("CC2",6);
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
  if((a[4]!=CC1 && a[4]!=CC2) && (a[17]!=CC1 && a[17]!=CC2)){
    Serial.println(F("ERROR, CONtyp does not present a valid CC line on analog input/Rd (given hardware)! CC disabled."));
    ccSwitch=false;
  }
  else ccSwitch=true;

  //Drive the Gnd pin OPPOSITE the CC pin.... hardcode it for now.
   
  if(a[0]!=GND && a[13]!=GND){
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
                  //pinMode(10,OUTPUT); //A-4
                  //pinMode(11,OUTPUT); //A-17
              //          digitalWrite(10,LOW);
              //          digitalWrite(11,LOW);   
                }
              
                //----------ALLOW VOLTAGE SOURCE TO SWITCH ON--------------
                delay(vbusDelayOn);
                
                if(ccpins[k]==-1) Serial.println("            Baseline (No CC driven):");          
                
                if(gndSwitch && ccpins[k]!=-1){
                  pinMode(read_pins[bankA[13]],INPUT_PULLUP);   //--- SPECIAL CASE FOR GND---
                  if(analogRead(read_pins[bankA[13]])>=1.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[13]])>=1.5*1024.0/(vcc_actual/1000.0) && analogRead(read_pins[bankA[13]])>=1.5*1024.0/(vcc_actual/1000.0)){
                       Serial.println("CAUTION. (Floating) Gnd N/C. Supply ground for this burst.");
              //      digitalWrite(A4,HIGH);  //Power on pullup
              //      pinMode(A4,OUTPUT);     //Power on heavy
              //      digitalWrite(A4,HIGH);  //VBUS HOT
                    digitalWrite(read_pins[bankA[13]],LOW);  //remove pullup
                    pinMode(read_pins[bankA[13]],OUTPUT);     //Power on heavy
                    digitalWrite(read_pins[bankA[13]],LOW);  //GND CON 
                  }
                  else {
                    Serial.println("WARNINING! (Floating) Gnd CON already? Charger connected/CC ON? Aborting driver, monitoring only.");
                    pinMode(read_pins[bankA[13]],INPUT); //--- SPECIAL CASE FOR GND---
                  }
                                  
                }
              
              
                cycleBank(a);
              
                if (ccSwitch && gndSwitch){
                  temp=analogRead(read_pins[bankA[4]]);
                  (a[4]==CC1)? cc1=temp:cc2=temp;
              
                  temp=analogRead(read_pins[bankA[17]]);
                  (a[17]==CC2)? cc2=temp:cc1=temp;
              
                  temp=analogRead(read_pins[bankA[0]]);
                  gnd_bridge=temp;
              
                  temp=analogRead(read_pins[bankA[13]]);
                  gnd=temp;
              
              //    Serial.print(cc1/1023.0*5.0);
              //    Serial.print(cc2/1023.0*5.0);
              //    Serial.print(vbus/1023.0*5.0);
              //    Serial.print(vbus_bridge/1023.0*5.0);
              //    
              //      for(int i=0;i<26; i++){
              //        if(read_pins[bankA[i]]>=54 && read_pins[bankA[i]]<=69){
              //          if(a[i]==CC1 || a[i]==CC2){
              //            temp=analogRead(read_pins[bankA[i]]);
              //          }
              //          if(a[i]==CC1) cc1=temp;
              //          if(a[i]==CC2) cc2=temp;
              //        }
              //      }
                
              //    cc1=analogRead(read_pins[bankA[4]]);
              //    cc2=analogRead(read_pins[bankA[17]]);
              
                  if((float)abs(gnd_bridge-gnd)/1023.0*vcc_actual/1000.0 > .2) Serial.println("ERROR! GNDro/GND not bridged (dV > .2)! Cable error?");
                }
              
                //======= SHUTOFF HERE MOVED BELOW============
                
                Serial.print("      ");

                if((ccpins[k]==10 && a[4] == CC1) || (ccpins[k]==11 && a[17] == CC1))
                    printStr(">CC1<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC1-v",7);
                else
                   printStr("Vconn",7); 
                   
                if((ccpins[k]==11 && a[17] == CC2) || (ccpins[k]==10 && a[4] == CC2))
                    printStr(">CC2<",7);  
                else if (ccpins[k]==-1)
                    printStr("CC2-v",7);                    
                else 
                   printStr("Vconn",7); 


                
//                printStr("CC1",7);
//                printStr("CC2",6);
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

