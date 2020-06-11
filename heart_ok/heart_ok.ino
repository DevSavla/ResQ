
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

const int OUTPUT_TYPE = SERIAL_PLOTTER;

const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle
int hr,myBPM;


byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;

PulseSensorPlayground pulseSensor;

void setup() {
 
  Serial.begin(115200);

  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

if (!pulseSensor.begin()) {
  
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
}

void loop()
{

  if (pulseSensor.sawNewSample()) 
  {
    
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      //pulseSensor.outputSample();
      hr= pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
   if(hr>50 && hr<150)
   {
    myBPM=hr;
   Serial.println(myBPM);
   }

      
      if (pulseSensor.sawStartOfBeat()) {
        //pulseSensor.outputBeat();

        if (--samplesUntilReport == (byte) 0) 
        {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      //pulseSensor.outputSample();
      hr= pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
   if(hr>50 && hr<150)
   {
    myBPM=hr;
   Serial.println(myBPM);
   }
      }
 }
}
}
}
