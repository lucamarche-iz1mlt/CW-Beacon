/***************************************************************************
 * 
 *                         CW Beacon from 35 to 4400Mhz
 * _________________________________________________________________________
 * 
 *CW beacon software for PLL ADF4351. 
 *Please check the ADF4351 datasheet or use ADF4351 software to get PLL register values.
 *Download ADF435X from https://ez.analog.com/rf/f/q-a/75978/adf4350-and-adf4351-evaluation-board-control-software
 *V1.0 IZ1MLT 14/02/2019
 */
 
//////////////////////////////////SETTING///////////////////////////////////

#define CW_TEXT  "VVV IZ1MLT/B JN44LN"  //Send CW text

unsigned int speed_wpm    = 12;             //WPM
unsigned int tone_on      = 10000;          //ms TX after beacon
unsigned int tone_off     = 2000;           //ms pause before send beacon
unsigned int time_pause   = 500;            //ms pause after beacon text

// Freq:  1296,410MHz 
// RFstep: 10.000kHz
// Clock in: 10 MHz Ref.
long int r0       = 0x00409408;
long int r1       = 0x08001f41;
long int r2       = 0x18004e42;
long int r3       = 0x000004B3;
long int r4       = 0x0015003c;
long int r5       = 0x00580005;


/////////////////////////////////DEBUG/////////////////////////////////////

// #define DEBUG_VIA_SERIAL


////////////////////////DEFINE AND VARIABLE////////////////////////////////
// Label            Arduino pins          Description                                        ADF4351 datasheet page
#define CE_Pin      4                     // 2  chip enable, low powers down the chip        (DS7, Pin 4)
#define LE_Pin      5                     // 3  load enable, high load register selected     (DS7, Pin 3)
#define CLK_Pin     6                     // 4  clock to shift in data                       (DS7, Pin 1)
#define DATA_Pin    7                     // 8  Serial Data goes here                        (DS7, Pin 2)

// useful abbreviations
#define pulseHigh(pin)  {digitalWrite(pin, 1); digitalWrite(pin, 0); }
#define pulseLow(pin)   {digitalWrite(pin, 0); digitalWrite(pin, 1); }
#define Led_On          {digitalWrite(Led_Pin, HIGH);}
#define Led_Off         {digitalWrite(Led_Pin, LOW);}

#define N_MORSE  (sizeof(morsetab)/sizeof(morsetab[0]))

int dotlen;
int dashlen;
char *str;


//////////////////////////////////STRUCT///////////////////////////////////
struct t_mtab {
  char c, pat;
} ;

struct t_mtab morsetab[] = {
  {'.', 106},
  {',', 115},
  {'?', 76},
  {'/', 41},
  {'A', 6},
  {'B', 17},
  {'C', 21},
  {'D', 9},
  {'E', 2},
  {'F', 20},
  {'G', 11},
  {'H', 16},
  {'I', 4},
  {'J', 30},
  {'K', 13},
  {'L', 18},
  {'M', 7},
  {'N', 5},
  {'O', 15},
  {'P', 22},
  {'Q', 27},
  {'R', 10},
  {'S', 8},
  {'T', 3},
  {'U', 12},
  {'V', 24},
  {'W', 14},
  {'X', 25},
  {'Y', 29},
  {'Z', 19},
  {'1', 62},
  {'2', 60},
  {'3', 56},
  {'4', 48},
  {'5', 32},
  {'6', 33},
  {'7', 35},
  {'8', 39},
  {'9', 47},
  {'0', 63}
} ;

// Morse code tokens. 0 = dit, 1 = dah any other character will generate 8 dit error.

// NOTE: These need to be in ASCII code sequence to work

char tokens [][7] = { "110011", "100001", // ,,-
                      "010101", "10010", // .,/
                      "11111", "01111", "00111", "00011", "00001", "00000", // 0,1,2,3,4,5
                      "10000", "11000", "11100", "11110", // 6,7,8,9
                      "111000", "10101", "<", // :,;,<"=",">","001100", // =,>,?
                      "@", // @,
                      "01", "1000", "1010", // A,B,C
                      "100", "0", "0010", // D,E,F
                      "110", "0000", "00", // G,H,I
                      "0111", "101", "0100", // J,K,L
                      "11", "10", "111", // M,N,O
                      "0110", "1101", "010", // P,Q,R
                      "000", "1", "001", // S,T,U
                      "0001", "011", "1001", // V,W,X
                      "1011", "1100"
                     }; // Y,Z };


//////////////////////////////MORSE FUNCTION//////////////////////////////////

// Routine to output a dit
void dit()
{
  set_freq_on();
  delay(dotlen);
  set_freq_off();
  delay(dotlen);
}

// Routine to output a dah
void dash()
{
  set_freq_on();
  delay(dashlen);
  set_freq_off();
  delay(dotlen);
}

// Look up a character in the tokens array and send it
void send(char c)
{
  int i ;
  if (c == ' ') {

    delay(7 * dotlen) ;
    return ;
  }
  for (i = 0; i < N_MORSE; i++) {
    if (morsetab[i].c == c) {
      unsigned char p = morsetab[i].pat ;

      while (p != 1) {
        if (p & 1)
          dash() ;
        else
          dit() ;
        p = p / 2 ;
      }
      delay(2 * dotlen) ;
      
#ifdef DEBUG_VIA_SERIAL
    Serial.print(c);
#endif
      return ;
    }
  }
}

void sendmsg(char *str)
{
  while (*str)
    send(*str++);
}


////////////////////////////ADF4351 FUNCTION////////////////////////////////

void ADF4351_Init() {
#ifdef DEBUG_VIA_SERIAL
  Serial.println("ADF4351 initializing ...");
#endif
  // set initial values on the pins
  digitalWrite(CE_Pin,   HIGH);
  digitalWrite(LE_Pin,   LOW);
  digitalWrite(CLK_Pin,  LOW);
  digitalWrite(DATA_Pin, LOW);
}

void ADF4351_Reset() {
#ifdef DEBUG_VIA_SERIAL
  Serial.println("soft reset: ADF4351");
#endif
  pulseLow(CE_Pin);   // reset signal
  delay(1);
  pulseHigh(CLK_Pin); // clock signal
  delay(1);
  pulseHigh(LE_Pin);  // frequency update signal
#ifdef DEBUG_VIA_SERIAL
  Serial.println("soft reset: ADF4351 ready");
#endif
}

void ADF4351_SendData(int c) {
  for (int i = 7; i >= 0; i--) {      // LSB-MSB (i = 0; i >8 ; i++)
    digitalWrite(DATA_Pin, (c >> i) & 0x01);
    pulseHigh(CLK_Pin);
  }
}

void ADF4351_Frequenz(long frequenz) {
  long  y;
  y = frequenz;
  ADF4351_SendData((y >> 24) & 0xFF); //  send MSB first
  ADF4351_SendData((y >> 16) & 0xFF);
  ADF4351_SendData((y >> 8)  & 0xFF);
  ADF4351_SendData((y ) & 0xFF);      // send LSB last
  pulseHigh(LE_Pin);                  // set registers are valid
}

void set_freq_on() {                 
  ADF4351_Frequenz( r5 );  // Reg 5
  ADF4351_Frequenz( r4 );  // Reg 4
  ADF4351_Frequenz( r3 );  // Reg 3
  ADF4351_Frequenz( r2 );  // Reg 2
  ADF4351_Frequenz( r1 );  // Reg 1
  ADF4351_Frequenz( r0 );  // Reg 0
}

void set_freq_off() {                 
  ADF4351_Frequenz( (r4 & 0xffffffdf) | 0x00000800 );
}


//////////////////////////////////INIT/////////////////////////////////////

void setup () {
#ifdef DEBUG_VIA_SERIAL
  Serial.begin(57600);
#endif

  dotlen = (1200 / speed_wpm);
  dashlen = (3 * (1200 / speed_wpm));
  str = CW_TEXT;

  pinMode(CE_Pin,   OUTPUT);      // chip enable
  pinMode(LE_Pin,   OUTPUT);      // load enable
  pinMode(CLK_Pin,  OUTPUT);      // clock
  pinMode(DATA_Pin, OUTPUT);      // data
 
  ADF4351_Reset();
  delay(10);                      // spend time to relax from that shock#ifdef DEBUG_VIA_SERIAL
  ADF4351_Init();
  delay(500);                     // spend time for his initial work

#ifdef DEBUG_VIA_SERIAL
  Serial.println("Arduino-for-ADF4351 setup ready, start to loop");
  Serial.println("Freq: 1296.410Mhz");
#endif
   set_freq_off();  //TX off
}


///////////////////////////////MAIN LOOP///////////////////////////////////

void loop() {

  delay(tone_off);
  sendmsg(str);
  delay(time_pause);
  set_freq_on();
  delay(tone_on);
  set_freq_off();

#ifdef DEBUG_VIA_SERIAL
Serial.println();     //new line after end beacon
#endif
}
