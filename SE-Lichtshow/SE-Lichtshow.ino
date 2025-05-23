// FASTLED + ObjectFLED setup
  #include<ObjectFLED.h> // imports the ObjectFLED library - this must always be done before including the modified FastLED library
  #include<FastLED.h> // imports the modified FASTLED library that always uses ObjectFLED

  #define PIX_PER_STR 30  // define 30 LED pixels per strip
  #define NUM_STR 2 // define the number of strips to be four

  CRGB leds[NUM_STR][PIX_PER_STR];  // initializes the draw buffer

  uint8_t pinList[NUM_STR] = {2, 3}; // pin list for teensy output

  // initializes the LED controller object based on number of LEDs and link the Teensy pins as well as the drawing buffer to it
  ObjectFLED dispLEDs(PIX_PER_STR * NUM_STR, leds, CORDER_GRB, 2, pinList); 

  enum ColorChannel {rd, gr, bl}; // enumeration for color channels rd = red, gr = green and bl = blue

//
// MIDI config

  #include <MIDI.h> // imports the Arduino MIDI library
Dis
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial4, MIDI); // reads the MIDI input from serial RX4

  uint8_t channel_config; // initializes the MIDI channel variable that is read into MIDIstate

  float bpm; // initializes bpm variable of the animation sine waves

//

// LED animation functions for writing into drawing buffer columns (= single strips)

  // fills the inputted drawing buffer column (= one strip) with the specified color and a specified value
  void fillSolidOneColor(ColorChannel color, CRGB* ledsIN, uint8_t value){
    switch(color){
      case bl:
        for(uint8_t i=0; i<PIX_PER_STR; i++){
          ledsIN[i].b = value;
        } 
        break;
      case gr:
        for(uint8_t i=0; i<PIX_PER_STR; i++){
          ledsIN[i].g = value;
        } 
        break;
      case rd:
        for(uint8_t i=0; i<PIX_PER_STR; i++){
          ledsIN[i].r = value;
        } 
        break;
      default:
        break;
    }

  } // fillSolidOneColor()

  // TEXT
  void OneColPosWave(ColorChannel color, CRGB* ledsIN, uint8_t value, uint8_t position) {
      switch(color){
      case bl:
          ledsIN[position].b = value;
        break;
      case gr:
          ledsIN[position].g = value;
        break;
      case rd:
          ledsIN[position].r = value;
        break;
      default:
        break;
      }
      fadeToBlackBy(ledsIN, PIX_PER_STR, 1); // make internal state variable and call this every 1, 2 or 3 times when using double waves or triple waves
  }; // OneColPosWave()

  // TEXT
  void OneColWave(ColorChannel color, CRGB* ledsIN, uint8_t value, uint8_t position) {
      switch(color){
      case bl:
        for(uint8_t i=0;i<PIX_PER_STR;i++) {
          ledsIN[i].b = value;
        }
        break;
      case gr:
        for(uint8_t i=0;i<PIX_PER_STR;i++) {
          ledsIN[i].g = value;
        }
        break;
      case rd:
        for(uint8_t i=0;i<PIX_PER_STR;i++) {
          ledsIN[i].r = value;
        }
        break;
      default:
        break;
      }
  }; // OneColWave()

//

// MIDI data and functions

  // Structure for saving the input MIDI state
    struct {
      uint8_t type;
      uint8_t note;
      uint32_t trigTime;
      bool noteOn;
      uint8_t velocity;
      uint8_t channel;
      uint8_t sensitivity = 100; // standard value for startup
    } MIDIstate;
  //

  // reads MIDI input from the serial and saves data into the MIDIstate structure
  void readMIDI(uint8_t channel) {
    if(MIDI.read()){ //read in MIDI from serial - false if no message is read
      MIDIstate.type = MIDI.getType();
      switch (MIDIstate.type) {
        case midi::NoteOn:
          MIDIstate.channel = MIDI.getChannel();
          if (MIDIstate.channel==channel){
            MIDIstate.note = MIDI.getData1();
            MIDIstate.noteOn = true;
            MIDIstate.trigTime = micros();
            Serial.print("MIDI note ");
            Serial.print(MIDIstate.note);
            Serial.print(" on channel ");
            Serial.print(channel);
            Serial.print(" is active");
            MIDIstate.velocity = MIDI.getData2();
            Serial.println(" with velocity");
          }
          break;
        case midi::NoteOff:
          MIDIstate.channel = MIDI.getChannel();
          if (MIDIstate.channel==channel){
            MIDIstate.noteOn = false;
          }
          // velocity is always 0 on note off with OXI ONE
          break;
        //case midi::ControlChange:   // this case is not used
          //break;
        case 208: // At/Gl value (knob 3) on OXI ONE in mono keyboard mode
          MIDIstate.sensitivity = MIDI.getData1();
          Serial.println(String("Message, type=") + MIDIstate.type + ", data = " + MIDIstate.sensitivity);
          break;
        default:
          break;
      } // MIDI.read()
    } 
  } //readMIDI()

  // checks if a note is active and returns the active note number if true
  uint8_t activeNote() { 
    if (MIDIstate.noteOn) {
      return MIDIstate.note;
    }
    else {
      return 0;
    }
  } // activeNote()

  // converts the read in velocity value to a tempo multiple
  uint8_t velToMult() {
    uint8_t vel = MIDIstate.velocity;
    accum88 factor;
        if(vel<=5){factor = 1;};
        if(vel>5&&vel<=20){factor = 2;};
        if(vel>20&&vel<=35){factor = 4;};
        if(vel>35&&vel<=55){factor = 8;};
        if(vel>55&&vel<=70){factor = 16;};
        if(vel>70&&vel<=90){factor = 32;};
        if(vel>90&&vel<=110){factor = 64;};
        if(vel>110){factor = 128;}; 
        return factor;
  } // velToMult()
  
  // TEXT
  void stateChange(ColorChannel color, CRGB* ledsIN, uint8_t value, uint8_t position) {
      uint8_t note = activeNote();
      switch (note) {
        case 33: // A0  
          OneColPosWave(bl,ledsIN,value,position);
          OneColPosWave(gr,ledsIN,value,position-3);
          break;
        case 35: // B0
          OneColWave(bl,ledsIN,position*MIDIstate.sensitivity*2/PIX_PER_STR,PIX_PER_STR);
          break;
        case 0: // no note active
          fillSolidOneColor(gr, ledsIN,0);
          fillSolidOneColor(bl, ledsIN,0);
          fillSolidOneColor(rd, ledsIN,0);
          break; 
      }
  } // stateChange()

//



// global functions

  // Custom bpm-synced sin function with high resolution timing and phase offset
    float beatsin(float bpm, uint8_t low = 0, uint8_t high = 255, uint32_t timebase = 0, float phase_offset_degrees = 0) {
      float t = (micros() - timebase) / 1e6;
      float freq = bpm / 60.0;
      float phase = radians(phase_offset_degrees);
      float sine = sin(TWO_PI * freq * t + phase);
      float range = high - low;
      float value = (sine + 1.0) * 0.5 * range + low;
      return value;
    } // beatsin()
  
//

void setup() {

// Serial monitor setup
  Serial.begin(2000000); // starts the serial monitor at a baud rate of 2 Mhz
//

// MIDI setup
  MIDI.begin(MIDI_CHANNEL_OMNI);
  channel_config = 2; // sets the MIDI channel that is read / 1 = first, 2 = second, etc.
  bpm = 128; // sets the beats per minute
//

// LEDs setup
  dispLEDs.begin(); // starts the LED controller object
  //FastLED.setCorrection(TypicalLEDStrip); // sets a white color correction
//

} // setup

void loop() {

// global loop variables

  uint8_t sinBeat; 
  uint8_t mult;
  float fourBars = bpm / 16;
  
//

// read MIDI input into data structure

  readMIDI(channel_config); // reads the MIDI input per cylce and save it in the MIDIstate structure
 
//

// get the sine wave value based on MIDI state (set tempo & magnitude)

   mult = velToMult(); // converts the read in velocity value to a tempo multiple

  // sets the tempo based on the velocity and the start time based on the trigger time of the note / only changes when triggering a new note
  sinBeat = beatsin(fourBars*mult,0,PIX_PER_STR-1,MIDIstate.trigTime,-80);

//

// set the drawing buffers and correct the color

  // writes the next frame into the drawing buffer for the 1st strip / via 2nd pin 
  stateChange(bl,leds[0],MIDIstate.sensitivity*2,sinBeat);
  // writes the next frame into the drawing buffer for the 2nd strip / via 3rd pin
  stateChange(bl,leds[1],MIDIstate.sensitivity*2,sinBeat); 

//

// write from the drawing buffer to the LED strips in the background
  dispLEDs.show(); // sends the values in the drawing buffer to the LEDs in the background
//

} // loop
