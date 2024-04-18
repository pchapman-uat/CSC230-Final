#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

#include <M5StickCPlus.h>

HTTPClient client;

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

	
}

void loop()
{
	M5.update();
	if(M5.BtnA.wasReleased()){
		client.begin(HTTP_POST);
		int httpResponceCode = client.POST("");
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
	}	
}
