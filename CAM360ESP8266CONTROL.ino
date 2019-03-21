#include "ESP8266WiFi.h"
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include "DRV8825.h"
#include "Bounce2.h"


// pin config
const int buttonPin = D3;
Bounce debouncer = Bounce();
#define servo_tilt D4
#define DIR D5
#define STEP D6

// motor config
#define MOTOR_STEPS 200
#define RPM 160
#define MICROSTEPS 1
Servo tilt;
DRV8825 stepper(MOTOR_STEPS, DIR, STEP);

// RGB config

const uint16_t PixelCount = 1;
const uint8_t PixelPin = D1; // pin RGB diode Din
#define colorSaturation 128
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PixelCount, PixelPin, NEO_GRB + NEO_KHZ800);

int32_t red      = strip.Color (127,0,0);
int32_t green    = strip.Color (0,127,0);
int32_t blue     = strip.Color (0,0,127);
int32_t yellow   = strip.Color (100,127,0);
int32_t magenta  = strip.Color (100,0,127);

int buttonState = 0;
int ledState = 0;

WiFiClient client;
String YI_SSID;

void setup() {
  // startup process
  Serial.begin(115200);
  Serial.println("STARTUP");
  delay(100);

  pinMode(BUILTIN_LED, OUTPUT);  // initialize onboard LED as output
  digitalWrite(BUILTIN_LED, HIGH);   // turn off LED with voltage LOW
  pinMode(buttonPin, INPUT_PULLUP);  // button
  debouncer.attach(buttonPin); debouncer.interval(50);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // LED red on - init started
  strip.setPixelColor(0,red);
  strip.show();

  // stepper settings
  stepper.begin(RPM, MICROSTEPS);
  stepper.enable();

  // init servo
  tilt.attach(servo_tilt);
  tilt.write(90);

  searchCamera();
  connectToCamera();

  // LED green on - init started
    strip.setPixelColor(0,green);
    strip.show();
}

void searchCamera() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(10);
  int cnt = WiFi.scanNetworks();
  Serial.print("Networks: ");
  if (cnt > 0) {
    for (int i = 0; i < cnt; ++i) {
      Serial.print(WiFi.SSID(i) + ",");
      if (WiFi.SSID(i).startsWith("YDXJ_")) {
        YI_SSID = WiFi.SSID(i);
        break;
      }
    }
  }
  Serial.println();
}

void connectToCamera() {
  bool result = true;
  short retry = 30;
  const int jsonPort = 7878;
  char password[11] = "1234567890";
  char ssid[30];
  Serial.print("Con: ");
  YI_SSID.toCharArray(ssid, YI_SSID.length() + 1);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    if (retry == 0) {
      result = false;
      break;
    }
    delay(500);
    retry--;
  }
  Serial.print(" -> wifi con:");
  if (result == true) Serial.print("OK "); else Serial.print("XX ");

  if (!client.connect("192.168.42.1", jsonPort)) result = false;
  Serial.print(" IP con:");
  if (result == true) Serial.print("OK."); else Serial.print("XX.");
  Serial.println();

}

// request
String requestToken() {

  client.print("{\"msg_id\":257,\"token\":0}\n\r");

  // Give the server time to respond, then read the entire reply
  yield(); delay(250); yield(); delay(250); yield(); delay(250); yield(); delay(250);
  String response;
  while (client.available()) {
    char character = client.read();
    response.concat(character);
  }

  // Search for token in the response

  String searchString = "\"param\":";
  int offset = response.indexOf(searchString);
  if (offset == -1)
    return ""; // failure

  String token;
  for (int i = offset + searchString.length(); i < response.length(); ++i) {
    char c = response.charAt(i);
    if ((c == ',') || (c == ' ') || (c == '}') )
      break;
    else
      token.concat(response.charAt(i));
  }

  return token;
}

void loop() {

		debouncer.update();
	    if (debouncer.fell()) {
	    String token = requestToken();
	    if (token.length() != 0) {
	      TakePhoto(token);
	    	}

	    }
		yield();
}

void TakePhoto(String token) {

  client.print("{\"msg_id\":769,\"token\":");
  client.print(token);
  client.print("}\n\r");
  Serial.print("Photo - Response: ");
  yield(); delay(250); yield(); delay(250); yield(); delay(250); yield(); delay(250);
  String response;
  while (client.available()) {
    char character = client.read();
    response.concat(character);
  }
  Serial.println(response);
}
