
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

#include <M5StickCPlus.h>
#include <ArduinoJson.h>

HTTPClient client;
DynamicJsonBuffer jsonBuffer;

int id;
String clockIn = "";
int mode = 0;
void setup()
{
	M5.begin();
	Serial.begin(115200);
	WiFi.begin(SSID, PSWRD);
	M5.Lcd.fillScreen(RED);
	while (WiFi.status()!= WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
	M5.Lcd.fillScreen(GREEN);
	Serial.println("Connected");
	M5.Lcd.setRotation(3);
	M5.Lcd.setTextSize(2);
}

void loop()
{
	M5.update();
	M5.Lcd.setCursor(0,0);
	M5.Lcd.println("Mode: ");
	if(mode == 0) M5.Lcd.println("GET");
	else if(mode == 1) M5.Lcd.println("CLOCK OUT");
	else if(mode == 2) M5.Lcd.println("CLOCK IN");
	if(clockIn != "") M5.Lcd.printf("Clock In: %s", clockIn.c_str());
	M5.Lcd.println("ID: ");
	M5.Lcd.println(id);
	if(M5.BtnA.wasReleased()){
		client.end();
		if(mode == 0){
			M5.Lcd.fillScreen(YELLOW);
			M5.Lcd.println("Mode: GET");
			client.begin(HTTP_GET);
			int httpResponceCode = client.POST("");
			if (httpResponceCode > 0) {
				Serial.print("HTTP Response code: ");
				Serial.println(httpResponceCode);
				
				String payload = client.getString();
				Serial.println("Response payload:");
				Serial.println(payload);

				JsonObject& response = jsonBuffer.parseObject(payload);
				response.printTo(Serial);
				id = response["ID"].as<int>();
				clockIn = response["Start"].as<String>();
			} else {
				Serial.print("Error in HTTP GET request. HTTP Error code: ");
				Serial.println(httpResponceCode);
			}
		} else if (mode == 1){
			M5.Lcd.fillScreen(YELLOW);
			M5.Lcd.println("Mode: CLOCK OUT");
			client.begin(HTTP_POST);
			client.addHeader("Content-Type", "application/json");
			int httpResponceCode = client.POST("{\"id\": " + String(id) + "}");
			if (httpResponceCode > 0) {
				Serial.print("HTTP Response code: ");
				Serial.println(httpResponceCode);
				String payload = client.getString();
				Serial.println("Response payload:");
				Serial.println(payload);
			} else {
				Serial.print("Error in HTTP GET request. HTTP Error code: ");
				Serial.println(httpResponceCode);
			}
		} else if (mode == 2){
			M5.Lcd.fillScreen(YELLOW);
			M5.Lcd.println("Mode: CLOCK IN");
			client.begin(HTTP_IN);
			int httpResponceCode = client.POST("");
			if(httpResponceCode > 0){
				Serial.print("HTTP Response code: ");
				Serial.println(httpResponceCode);
				String payload = client.getString();
				Serial.println("Response payload:");
				Serial.println(payload);
				JsonObject& response = jsonBuffer.parseObject(payload);
				id = response["id"].as<int>();
			} else {
				Serial.print("Error in HTTP GET request. HTTP Error code: ");
				Serial.println(httpResponceCode);
			}
		}
		M5.Lcd.fillScreen(GREEN);
	}	
	else if(M5.BtnB.wasReleased()) {
		mode = (mode + 1) % 3;
		M5.Lcd.fillScreen(GREEN);
	}
}
