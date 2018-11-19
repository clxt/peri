/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers
*********************************************************************/
// --------------------------------------------------------------------------------------------------------------------
// unsigned int waitFor(timer, period) 
// Timer pour taches périodiques 
// configuration :
//  - MAX_WAIT_FOR_TIMER : nombre maximum de timers utilisés
// arguments :
//  - timer  : numéro de timer entre 0 et MAX_WAIT_FOR_TIMER-1
//  - period : période souhaitée
// retour :
//  - nombre de période écoulée depuis le dernier appel
// --------------------------------------------------------------------------------------------------------------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdlib.h>
//---- RF
#include <SPI.h>
#include "RF24.h"
#include "printf.h"
//---- OLED
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define EMPTY 0
#define FULL 1
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#ifdef __AVR__
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
 #include <pgmspace.h>
#else
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) && !defined(ESP32) && !defined(__arc__)
 #include <util/delay.h>
#endif

// Multi-tâches cooperatives:
// --------------------------------------------------------------------------------------------------------------------
#define MAX_WAIT_FOR_TIMER 5
unsigned int waitFor(int timer, unsigned long period){
  static unsigned long waitForTimer[MAX_WAIT_FOR_TIMER];
  unsigned long newTime = micros() / period;              // numéro de la période modulo 2^32 
  int delta = newTime - waitForTimer[timer];              // delta entre la période courante et celle enregistrée
  if ( delta < 0 ) delta += 1 + (0xFFFFFFFF / period);    // en cas de dépassement du nombre de périodes possibles sur 2^32 
  if ( delta ) waitForTimer[timer] = newTime;             // enregistrement du nouveau numéro de période
  return delta;
}

//**** Définition des variables globales et fonctions ****

//:::: Variable Global du module NRF ::::
RF24 radio(9,10);
byte addresses[][6] = {"50000"};
char mess[16];
char buffer[32];
unsigned long timer;


//:::: NRF Setup ::::
void setup_nrf() {
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[0]);
  radio.printDetails();}

//:::: Definition de la tache Message :::://
struct Mess_st {
  int timer;                                              // numéro de timer utilisé par WaitFor
  unsigned long period;                                       // periode d'affichage
  char mess[20];
} Mess_t ; 

void setup_Mess(struct Mess_st * ctx, int timer, unsigned long period, const char * mess) {
  ctx->timer = timer;
  ctx->period = period;
  strcpy(ctx->mess, mess);
}
  
void loop_Mess(struct Mess_st *ctx) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée
  Serial.println(ctx->mess);                              // affichage du message
}

//----------------------------------------------------//

//----------------------------------------------------//


//--------- Definition structures et fonctions des Communications inter-tâches
struct mailbox {
  int state = EMPTY;
  int val;
} mailbox;

void loop_T1(struct Mess_st *ctx, struct mailbox *mb, char* bufPR) {
  if (!(waitFor(ctx->timer,ctx->period))) return;         // sort s'il y a moins d'une période écoulée     
  if (mb->state != EMPTY) return; // attend que la mailbox soit vide
  mb->val  = analogRead(15); // read the input pin  
  mb->state = FULL;
}

void loop_T2(struct mailbox *mb, char* bufPR) {
  if (mb->state != FULL) return; // attend que la mailbox soit pleine
  sprintf(bufPR, "%d", mb->val);  //Converti la valeur decimal en caractere
  display.println("Valeur Photo-Resistance:");      // chargement du message sur l'écran console
  display.println(bufPR);      // chargement du message sur l'écran console
  display.display();           // affichage du message sur l'écran console
  display.clearDisplay();
  display.setCursor(0,0);
  NRF_write(bufPR);			//On envoie la valeur du capteur NRF vers la photo_res
  mb->state = EMPTY;	//On re autorise l'ecriture dans la boite à lettre.
}


//****************************** Déclaration *******************************************//
//--------- variable global ----//
int a = 0;
int count = 0;
char buf[9];
char bufPR[9];

//--------- Déclaration des structures -----------//
struct Mess_st Mess1, Timer_Photo_Res;
struct mailbox mb0;
//---------------------------------------------------

//--------- Setup et Loop
void setup() {
  Serial.begin(9600);                                 // initialisation du débit de la liaison série
  mb0.state = EMPTY;                                  // Init de l'etat de la boite à lettre
  setup_Mess(&Mess1, 1, 1500000, "bonjour");          // Mess est exécutée toutes les secondes 
  setup_Mess(&Timer_Photo_Res, 4, 1000000, "PR");      // La mesure de photo résistance est exécutée toutes les 5 secondes 
  setup_nrf();

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
}

//Permet d'envoyer une donné au module NRF24
void NRF_write( char* buf_NRF) {
  sprintf(buffer,"%s", buf_NRF);
  radio.write( &buffer, sizeof(buffer) );
  timer++;  
  Serial.println(buffer);
}

void loop(){
  loop_T1(&Timer_Photo_Res, &mb0, bufPR);
  loop_T2 (&mb0, bufPR);
}
