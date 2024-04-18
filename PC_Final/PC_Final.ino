#include <esp_crt_bundle.h>
#include <ssl_client.h>
#include <WiFiClientSecure.h>

#include "secrets.h"

#include <M5StickCPlus.h>

WiFiClientSecure client;

void setup()
{
	Serial.begin(115200);
	WiFi.begin(SSID, PSWRD);
	while (WiFi.status()!= WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
	Serial.println("Connected");

	
}

void loop()
{
	M5.update();
	if(M5.BtnA.wasReleased()){
		client.begin(HTTP_POST);
		int httpResponceCode = client.get();
	}	
}
