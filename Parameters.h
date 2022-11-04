//Values below are just for initialising and will be changed when synth is initialised to current panel controls & EEPROM settings
byte midiChannel = MIDI_CHANNEL_OMNI;//(EEPROM)
//boolean encCW = true;//This is to set the encoder to increment when turned CW - Settings Option


int PitchBendLevel = 0;
int PitchBendLevelstr = 0; // for display
int modWheelDepth = 0;
float modWheelLevel = 0;
float modWheelLevelstr = 0;
int filterLogLin = 0;
int ampLogLin = 0;
int linLog = 0;
float afterTouch = 0;
int AfterTouchDest = 0;
int NotePriority = 0;

int returnvalue = 0;
