#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <PulseSensorPlayground.h>
#include <SPI.h>
#include <Wire.h>

const String ssid = "Vimal";
const String password = "9821088672";
const String url = "http://192.168.1.135:8000/api/post_data/1";
const float text_size = 0.95;

Adafruit_SSD1306 display(-1);

void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(text_size);
  display.setTextColor(WHITE);
  display.clearDisplay();

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  Serial.println("");
  String wifiStatus = "Connected to: " + ssid;
  Serial.println(wifiStatus);
  Serial.print("User IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  httpClient();
  delay(1500);
}

void set_display(String cur_time, String cur_date, String locate, String bpm) {
  display.setCursor(0*text_size, 0*text_size);
  display.println("Time : " + cur_time);
  display.display();

  display.setCursor(0*text_size, 9*text_size);
  display.println("Date : " + cur_date);
  display.display();

  display.setCursor(0*text_size, 19*text_size);
  display.println("Location : " + locate);
  display.display();

  display.setCursor(0*text_size, 28*text_size);
  display.println("BPM : " + bpm);
  display.display();
 
  display.setCursor(55*text_size,28*text_size);
  display.write(3);
  display.display();
  display.clearDisplay();
}

void httpClient() {
  int httpCode;
  String payload = "";
 
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type", "text/plain");

    String str_post = "";
    int httpCode = http.POST(str_post);

    if (httpCode == HTTP_CODE_OK || httpCode > 0) {
      payload = http.getString();
     
      Serial.println(payload);
     
      char spike = payload[0];
      String cur_date = payload.substring(1, 9);
      String cur_time = payload.substring(9, 17);
      String bpm = payload.substring(17, 19);
      String locate = payload.substring(19);
      set_display(cur_time, cur_date, locate, bpm);
    }else {
      Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
    }
   
    http.end();
  }else {
    Serial.println("Error in WiFi connection");
  }
}
