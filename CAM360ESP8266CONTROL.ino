#include "ESP8266WiFi.h"

WiFiClient client;
String YI_SSID;

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

void setup() {
  //Only For debug
  Serial.begin(115200);
  Serial.println("STARTUP");
  searchCamera();
  connectToCamera();
}

// request for new firmware
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

void loop() {

    String token = requestToken();
    if (token.length() != 0) {
      TakePhoto(token);
    }

	yield();

}

