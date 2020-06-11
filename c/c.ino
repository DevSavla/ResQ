#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define USE_ARDUINO_INTERRUPTS false

float t = 1;
float cur_lat;
float cur_lon;

const uint8_t MPU6050SlaveAddress = 0x68;
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = A0;
const int PULSE_BLINK = 13;    // Pin 13 is the on-board LED
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;

const uint8_t scl = D6;
const uint8_t sda = D7;

int myBPM, hr;
int fall = 0;

const char *ssid = "Wizdem";
const char *password = "wizdemroxx";
const char *host = "http://192.168.43.209:8000/api/post_data/1";

const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;

PulseSensorPlayground pulseSensor;
Adafruit_SSD1306 display(-1);

void setup() {
  WiFi.begin(ssid, password);
  MPU6050_Init();
  Serial.begin(115200);
  
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Wire.begin(sda, scl);
  
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.blinkOnPulse(PULSE_BLINK);
  pulseSensor.fadeOnPulse(PULSE_FADE);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

  if(!pulseSensor.begin()) {
    for(;;) {
      // Flash the led to show things didn't work.
      digitalWrite(PULSE_BLINK, LOW);
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);
      delay(50);
    }
  }
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(t);
  display.setTextColor(WHITE);
  display.clearDisplay();
}

void loop() {
  cur_lat = 19.0968;
  cur_lon = 72.8517;
  double Ax, Ay, Az, T, Gx, Gy, Gz;

  gethr();
  
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  
  //divide each with their sensitivity scale factor
  Ax = (double)AccelX/AccelScaleFactor;
  Ay = (double)AccelY/AccelScaleFactor;
  Az = (double)AccelZ/AccelScaleFactor;
  T = (double)Temperature/340+36.53; //temperature formula
  Gx = (double)GyroX/GyroScaleFactor;
  Gy = (double)GyroY/GyroScaleFactor;
  Gz = (double)GyroZ/GyroScaleFactor;

  float at = sqrt(AccelX*AccelX+AccelY*AccelY+AccelZ*AccelZ);
  float pre_gt = sqrt(GyroX*GyroX+GyroY*GyroY+GyroZ*GyroZ);  
  float post_gt;

  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  
    //divide each with their sensitivity scale factor
  Ax = (double)AccelX/AccelScaleFactor;
  Ay = (double)AccelY/AccelScaleFactor;
  Az = (double)AccelZ/AccelScaleFactor;
  T = (double)Temperature/340+36.53; //temperature formula
  Gx = (double)GyroX/GyroScaleFactor;
  Gy = (double)GyroY/GyroScaleFactor;
  Gz = (double)GyroZ/GyroScaleFactor;
    
  if(at > 25000) {
    delay(100);      
    post_gt = sqrt(GyroX*GyroX+GyroY*GyroY+GyroZ*GyroZ);
    if (pre_gt>post_gt+600) {
//          digitalWrite(LED_BUILTIN,HIGH);
      Serial.println("FALL");
//          Serial.println();
// 
//          Serial.print(pre_gt);
//          Serial.print('\t');
//          Serial.print(post_gt);
//          Serial.print('\t');
//          Serial.print(at);
//          Serial.println();
      fall=1;
      delay(1000);
    }  
  }else {
    digitalWrite(LED_BUILTIN,LOW);
  }
    //delay(100);

    HTTPClient http;
    http.begin("http://192.168.43.209:8000/api/post_data/1");
    http.addHeader("Content-Type", "text/plain");
    
    String str_post = String(fall)+String("&")+String(cur_lat)+String("&")+String(cur_lon)+String("&")+String(myBPM);
    int httpCode = http.POST(str_post);

    if (httpCode == HTTP_CODE_OK || httpCode > 0) {
      String payload = http.getString();
      Serial.println("posted successfully");
    }else {
      Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    
    fall=0;
    printscreen();
}

void printscreen() {
  display.setCursor(0*t,0*t);
  display.println("Time : 10:00");
  display.display();
  
  display.setCursor(0*t,9*t);
  display.println("Date : 05/02/2020");
  display.display();
  
  display.setCursor(0*t,19*t);
  display.println("Location : Mumbai");
  display.display();
  
  display.setCursor(0*t,28*t);
  display.println("BPM : 90");
  
  display.setCursor(55*t,28*t);
  display.write(3);  
  display.display();
}

void gethr() {
  if(pulseSensor.sawNewSample()) {
    if(--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      //pulseSensor.outputSample();
      hr= pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
      if(hr>50 && hr<150) {
        myBPM=hr;
        Serial.println(myBPM);
      }
      
      if(pulseSensor.sawStartOfBeat()) {
        //pulseSensor.outputBeat();
        if (--samplesUntilReport == (byte) 0) {
          samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

      //pulseSensor.outputSample();
          hr= pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
          if(hr>50 && hr<150) {
            myBPM=hr;
            Serial.println(myBPM);
          }
        }
      }
    }
  }
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050
void MPU6050_Init() {
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
