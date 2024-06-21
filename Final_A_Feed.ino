#include <Wire.h>
#include "RTClib.h"
#include <Servo.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};
int jam, menit, detik;
int tanggal, bulan, tahun;
String hari;
float suhu;

Servo mekanik;

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* server = "http://your_server/get_schedule.php";

int feedHour = 8; // Default value
int feedMinute = 30; // Default value

SoftwareSerial esp8266(2, 3); // RX, TX

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  mekanik.attach(4);
  mekanik.write(0);

  updateFeedSchedule(); // Take schedule
}

void loop() {
  DateTime now = rtc.now();
  
  jam = now.hour();
  menit = now.minute();
  detik = now.second();
  tanggal = now.day();
  bulan = now.month();
  tahun = now.year();
  hari = daysOfTheWeek[now.dayOfTheWeek()];
  suhu = rtc.getTemperature();
  
  Serial.println(String() + hari + ", " + tanggal + "-" + bulan + "-" + tahun);
  Serial.println(String() + jam + ":" + menit + ":" + detik);
  Serial.println(String() + "Suhu: " + suhu + " C");
  Serial.println();

  // check Feed time
  if (jam == feedHour && menit == feedMinute && detik == 0) {
    kasih_Pakan();
  }

  delay(1000); // Wait 1 second for delay
}

void kasih_Pakan() {
  Serial.println("Feeding time!");
  
  mekanik.write(150); // servo activate
  delay(5000); // Time feed in Milisecond
  mekanik.write(0); // return position servo in default
}

void updateFeedSchedule() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);

      // Parse JSON
      int index = payload.indexOf(':');
      feedHour = payload.substring(index + 1, payload.indexOf(',', index)).toInt();
      feedMinute = payload.substring(payload.lastIndexOf(':') + 1, payload.indexOf('}', index)).toInt();

      Serial.println("Updated feed schedule:");
      Serial.print("Hour: ");
      Serial.println(feedHour);
      Serial.print("Minute: ");
      Serial.println(feedMinute);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
