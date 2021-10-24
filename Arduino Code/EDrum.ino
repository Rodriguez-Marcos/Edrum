 int a;
 
#define DEBUG(a) Serial.println(a);
#define NUM_PIEZOS 8
#define BOMBO_THRESHOLD 30
#define REDOBLANTE_THRESHOLD 30//anything < TRIGGER_THRESHOLD is treated as 0
#define HIHAT_THRESHOLD 30
#define TOMH_THRESHOLD 30
#define TOMM_THRESHOLD 30
#define TOML_THRESHOLD 30
#define RIDE_THRESHOLD 30
#define PLATILLO_THRESHOLD 30
#define START_SLOT 0     //first analog slot of piezos

//Piezo scaling defines
#define BOMBO_SCALE 100           //    100 is 100% of raw value - that is no scaling
#define REDOBLANTE_SCALE 100     //  < 100 scales the velocity down so that you have to hit harder to get maximum velocity
#define HIHAT_SCALE 100         //  > 100 scales the velocity up so you get maximum velocity with softer hits
#define TOMH_SCALE 100     
#define TOMM_SCALE 100
#define TOML_SCALE 100
#define RIDE_SCALE 100
#define PLATILLO_SCALE 100
unsigned short velScale[NUM_PIEZOS];

//MIDI note defines for each trigger
#define BOMBO_NOTE 48
#define REDOBLANTE_NOTE 50
int HIHAT_NOTE=a;
#define TOMH_NOTE 60
#define TOMM_NOTE 57
#define TOML_NOTE 53
#define RIDE_NOTE 63
#define PLATILLO_NOTE 69
//MIDI defines
#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80
#define MAX_MIDI_VELOCITY 127

//MIDI baud rate
#define SERIAL_RATE 9600

//Program defines
//ALL TIME MEASURED IN MILLISECONDS
#define SIGNAL_BUFFER_SIZE 50
#define PEAK_BUFFER_SIZE 20
#define MAX_TIME_BETWEEN_PEAKS 30
#define MIN_TIME_BETWEEN_NOTES 3

//map that holds the mux slots of the piezos
unsigned short slotMap[NUM_PIEZOS];

//map that holds the respective note to each piezo
unsigned short noteMap[NUM_PIEZOS];

//map that holds the respective threshold to each piezo
unsigned short thresholdMap[NUM_PIEZOS];

//Ring buffers to store analog signal and peaks
short currentSignalIndex[NUM_PIEZOS];
short currentPeakIndex[NUM_PIEZOS];
unsigned short signalBuffer[NUM_PIEZOS][SIGNAL_BUFFER_SIZE];
unsigned short peakBuffer[NUM_PIEZOS][PEAK_BUFFER_SIZE];

boolean noteReady[NUM_PIEZOS];
unsigned short noteReadyVelocity[NUM_PIEZOS];
boolean isLastPeakZeroed[NUM_PIEZOS];

unsigned long lastPeakTime[NUM_PIEZOS];
unsigned long lastNoteTime[NUM_PIEZOS];

void setup()
{
  Serial.begin(SERIAL_RATE);

  //initialize globals
  for(short i=0; i<NUM_PIEZOS; ++i)
  {
    currentSignalIndex[i] = 0;
    currentPeakIndex[i] = 0;
    memset(signalBuffer[i],0,sizeof(signalBuffer[i]));
    memset(peakBuffer[i],0,sizeof(peakBuffer[i]));
    noteReady[i] = false;
    noteReadyVelocity[i] = 0;
    isLastPeakZeroed[i] = true;
    lastPeakTime[i] = 0;
    lastNoteTime[i] = 0;    
    slotMap[i] = START_SLOT + i;
  }

  thresholdMap[0] = BOMBO_THRESHOLD;
  thresholdMap[1] = REDOBLANTE_THRESHOLD;
  thresholdMap[2] = HIHAT_THRESHOLD;
  thresholdMap[3] = TOMH_THRESHOLD;
  thresholdMap[4] = TOMM_THRESHOLD;
  thresholdMap[5] = TOML_THRESHOLD;  
  thresholdMap[6] = RIDE_THRESHOLD;
  thresholdMap[7] = PLATILLO_THRESHOLD; 

  velScale[0] = BOMBO_SCALE;
  velScale[1] = REDOBLANTE_SCALE;
  velScale[2] = HIHAT_SCALE;
  velScale[3] = TOMH_SCALE;
  velScale[4] = TOMM_SCALE;
  velScale[5] = TOML_SCALE;
  velScale[6] = RIDE_SCALE;
  velScale[7] = PLATILLO_SCALE;

  noteMap[0] = BOMBO_NOTE;
  noteMap[1] = REDOBLANTE_NOTE;

  noteMap[3] = TOMH_NOTE;
  noteMap[4] = TOMM_NOTE;
  noteMap[5] = TOML_NOTE;  
  noteMap[6] = RIDE_NOTE;
  noteMap[7] = PLATILLO_NOTE;
}

void loop()
{
//################################################################
     if (Serial.available())
   {
      int data = Serial.parseInt();
      DEBUG((int)data);
   }
//#############################################################
  unsigned long currentTime = millis();

  for(short i=0; i<NUM_PIEZOS; ++i)
  {
        if (digitalRead(2)==LOW && digitalRead(3)==LOW){HIHAT_NOTE=54;}else if(digitalRead(3)==HIGH && digitalRead(2)==LOW) HIHAT_NOTE=56; else HIHAT_NOTE=58; 
          noteMap[2] = HIHAT_NOTE;
    //get a new signal from analog read
    unsigned short newSignal = analogRead(slotMap[i]);
    signalBuffer[i][currentSignalIndex[i]] = newSignal;

    //if new signal is 0
    if(newSignal < thresholdMap[i])
    {
      if(!isLastPeakZeroed[i] && (currentTime - lastPeakTime[i]) > MAX_TIME_BETWEEN_PEAKS)
      {
        recordNewPeak(i,0);
      }
      else
      {
        //get previous signal
        short prevSignalIndex = currentSignalIndex[i]-1;
        if(prevSignalIndex < 0) prevSignalIndex = SIGNAL_BUFFER_SIZE-1;        
        unsigned short prevSignal = signalBuffer[i][prevSignalIndex];

        unsigned short newPeak = 0;

        //find the wave peak if previous signal was not 0 by going
        //through previous signal values until another 0 is reached
        while(prevSignal >= thresholdMap[i])
        {
          if(signalBuffer[i][prevSignalIndex] > newPeak)
          {
            newPeak = signalBuffer[i][prevSignalIndex];        
          }

          //decrement previous signal index, and get previous signal
          prevSignalIndex--;
          if(prevSignalIndex < 0) prevSignalIndex = SIGNAL_BUFFER_SIZE-1;
          prevSignal = signalBuffer[i][prevSignalIndex];
        }

        if(newPeak > 0)
        {
          recordNewPeak(i, newPeak);
        }
      }

    }

    currentSignalIndex[i]++;
    if(currentSignalIndex[i] == SIGNAL_BUFFER_SIZE) currentSignalIndex[i] = 0;
  }
}

void recordNewPeak(short slot, short newPeak)
{
  isLastPeakZeroed[slot] = (newPeak == 0);

  unsigned long currentTime = millis();
  lastPeakTime[slot] = currentTime;

  //new peak recorded (newPeak)
  peakBuffer[slot][currentPeakIndex[slot]] = newPeak;

  //1 of 3 cases can happen:
  // 1) note ready - if new peak >= previous peak
  // 2) note fire - if new peak < previous peak and previous peak was a note ready
  // 3) no note - if new peak < previous peak and previous peak was NOT note ready

  //get previous peak
  short prevPeakIndex = currentPeakIndex[slot]-1;
  if(prevPeakIndex < 0) prevPeakIndex = PEAK_BUFFER_SIZE-1;        
  unsigned short prevPeak = peakBuffer[slot][prevPeakIndex];

  if(newPeak > prevPeak && (currentTime - lastNoteTime[slot])>MIN_TIME_BETWEEN_NOTES)
  {
    noteReady[slot] = true;
    if(newPeak > noteReadyVelocity[slot])
      noteReadyVelocity[slot] = newPeak * velScale[slot] / 100;
  }
  else if(newPeak < prevPeak && noteReady[slot])
  {
    noteFire(noteMap[slot], noteReadyVelocity[slot]);
    noteReady[slot] = false;
    noteReadyVelocity[slot] = 0;
    lastNoteTime[slot] = currentTime;
  }

  currentPeakIndex[slot]++;
  if(currentPeakIndex[slot] == PEAK_BUFFER_SIZE) currentPeakIndex[slot] = 0;  
}

void noteFire(unsigned short note, unsigned short velocity)
{
  if(velocity > MAX_MIDI_VELOCITY)
    velocity = MAX_MIDI_VELOCITY;

  midiNoteOn(note, velocity);
  midiNoteOff(note, velocity);
}

void midiNoteOn(byte note, byte midiVelocity)
{
  Serial.write(NOTE_ON_CMD);
  Serial.write(note);
  Serial.write(midiVelocity);
}

void midiNoteOff(byte note, byte midiVelocity)
{
  Serial.write(NOTE_OFF_CMD);
  Serial.write(note);
  Serial.write(midiVelocity);
}
