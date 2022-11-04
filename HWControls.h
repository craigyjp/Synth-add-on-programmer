#include <Bounce2.h>

#define RECALL_SW 17
#define ENCODER_PINA 15
#define ENCODER_PINB 16

#define DEBOUNCE 30

//These are pushbuttons and require debouncing

Bounce recallButton = Bounce(RECALL_SW, DEBOUNCE); //On encoder
boolean recall = true; //Hack for recall button
Bounce EncAButton = Bounce(ENCODER_PINA, DEBOUNCE);
boolean encA = true; //Hack for settings button
Bounce EncBButton = Bounce(ENCODER_PINB, DEBOUNCE);
boolean encB = true; //Hack for settings button

void setupHardware()
{
  //Switches

  pinMode(RECALL_SW, INPUT_PULLUP); //On encoder
  pinMode(ENCODER_PINA, INPUT_PULLUP);
  pinMode(ENCODER_PINB, INPUT_PULLUP);

}
