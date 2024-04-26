// Include the Wifi library for internet connections
#include <WiFi.h>
// Inclue the HTTP Client Libary for connecting to HTTP websites
#include <HTTPClient.h>
// Include the M5Stick plus library for the display and RTC
#include <M5StickCPlus.h>
// Include the JSON library to parse the HTTP responce 
#include <ArduinoJson.h>

// Inclue the secrets header file
// The contains the following information
// HTTP_GET = URL for getting the current active session
	// Returns JSON of the current session
// HTTP_POST = URL for clocking out
	// Returns nothing
// HTTP_IN = URL for clocking in
	// Returns the ID of the new session
// HTTP_TIME = URL for getting the current time
// SSID = SSID For the wifi network
// PSWRD = Password for the wifi network
#include "secrets.h"

// Create the HTTP Client object
HTTPClient client;
// Create a Dynamic JSON Buffer
// Dynamic is used as the size changes with each responce
DynamicJsonBuffer jsonBuffer;
// Define the RTC Date time structure
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

// Initilaze the ID, Clock in time, and mode variables
int id;
String clockIn = "";
int mode = 0;
String httpGETString(String HTTP_URL, String Header1 = "", String Header2 ="",String body = "")
{	

	client.begin(HTTP_URL);
	if(Header1 != "") client.addHeader(Header1, Header2);
	// Post to the HTTP client and return the responce code
	int httpResponceCode = client.POST(body);
	// If code was a sucsess
	if (httpResponceCode > 0) {
		// Display the responce code and payload
		Serial.print("HTTP Response code: ");
		Serial.println(httpResponceCode);
		// Display thre response payload body
		String payload = client.getString();
		Serial.println("Response payload:");
		Serial.println(payload);

		// Return the body as a string
		return payload;
	} else {
		// Display an error to the serial
		Serial.print("Error in HTTP GET request. HTTP Error code: ");
		Serial.println(httpResponceCode);
	}
	// End the client
	client.end();
}

void setup()
{
	// Begin the M5StickC plus display and sensors
	M5.begin();
	// Begin the serial
	Serial.begin(115200);
	// Begin a wifi connection to the SSID and password
	WiFi.begin(SSID, PSWRD);
	// Fill the screen red
	M5.Lcd.fillScreen(RED);
	// While wifi is not connected
	while (WiFi.status()!= WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
	// Fill screen once conected 
	M5.Lcd.fillScreen(GREEN);
	// Print connected to the serial
	Serial.println("Connected");
	// Set the rotation of the display
	M5.Lcd.setRotation(3);
	// Set the text size
	M5.Lcd.setTextSize(2);
	// beginn the RTC
	M5.Rtc.begin();
}

void loop()
{
	// Update the M5Stick button data
	M5.update();
	// Set the cursor to the top left
	M5.Lcd.setCursor(0,0);
	// Display the mode
	M5.Lcd.println("Mode: ");
	if(mode == 0) M5.Lcd.println("GET");
	else if(mode == 1) M5.Lcd.println("CLOCK OUT");
	else if(mode == 2) M5.Lcd.println("CLOCK IN");
	// Display the clock in time 
	if(clockIn != "") M5.Lcd.printf("Clock In: %s", formatTime(clockIn).c_str());
	M5.Lcd.println();
	// Displat the ID
	M5.Lcd.println("ID: ");
	M5.Lcd.println(id);
	// Get the current time
	M5.Rtc.GetTime(&RTC_TimeStruct);
	M5.Lcd.println("Now:");
	// Display the time
	 M5.Lcd.printf("Time : %02d : %02d : %02d\n", RTC_TimeStruct.Hours,
                  RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
	// If Button A was release
	if(M5.BtnA.wasReleased()){
		// End the current client
		client.end();
		// If the mode is GET
		if(mode == 0){
			// Fill the screen yellow
			M5.Lcd.fillScreen(YELLOW);
			// Print the mode
			M5.Lcd.println("Mode: GET");
			// Get the JSON responce from the HTTP URL active session
			JsonObject& response = httpGETJSON(HTTP_GET);
			response.printTo(Serial);
			// Set the ID from the responce
			id = response["ID"].as<int>();
			// Set the clock in time from the response
			clockIn = response["Start"].as<String>();
			// Get the String response from the HTTP url for the time
			String response2 = httpGETString(HTTP_TIME);
			setRTC(response2);
		} else if (mode == 1){
			M5.Lcd.fillScreen(YELLOW);
			M5.Lcd.println("Mode: CLOCK OUT");
			// Send JSON to the clock out HTTP url
			httpSendJson(HTTP_POST);
		} else if (mode == 2){
			M5.Lcd.fillScreen(YELLOW);
			M5.Lcd.println("Mode: CLOCK IN");
			// Get the JSON from clocking in HTTP URL
			JsonObject& response = httpGETJSON(HTTP_IN);
			// Update the ID from the response
			id = response["id"].as<int>();
		}
		M5.Lcd.fillScreen(GREEN);
	}	
	// If button B was release
	else if(M5.BtnB.wasReleased()) {
		// Increase the mode from 0-2, looping back to 0 if over 2
		mode = (mode + 1) % 3;
		// Clear the screen
		M5.Lcd.fillScreen(GREEN);
	}
}


String httpSendJson(String HTTP_URL){
	String body = "{\"id\": " + String(id) + "}";
	String response = httpGETString(HTTP_URL, "Content-Type", "application/json", body);
	return response;
}

JsonObject& httpGETJSON(String HTTP_URL) {
	return jsonBuffer.parseObject(httpGETString(HTTP_URL));
}

String formatTime(String time){
	// Example: 2024-04-25T19:08:27Z
	// Should be: 04/25/2024 12:08 PM
	int hour = time.substring(11, 13).toInt(); // Extract hour
    int minute = time.substring(14, 16).toInt(); // Extract minute
	// Adjust time zone
	hour -= 7;
	if(hour < 0) hour += 24;

	// Convert to 12 hour format
	String ampm = "AM";
	if(hour >= 12) {
		ampm = "PM";
	} 
	if (hour > 12) {
		hour-=12;
	}
	String hourStr = String(hour);
	String minStr = String(minute);
	// Add leading zero
	if(hour < 10) hourStr = "0" + hourStr;
	if(minute < 10) minStr = "0" + minStr;
	return hourStr + ":" + minStr + " " + ampm;
}
void setRTC(String payload){
	// Example: 2024-04-25T20:41:37.1771909Z
	int year = payload.substring(0, 4).toInt(); // Extract year
	int month = payload.substring(5, 7).toInt(); // Extract month
	int day = payload.substring(8, 10).toInt(); // Extract day
	int hour = payload.substring(11, 13).toInt(); // Extract hour
	int minute = payload.substring(14, 16).toInt(); // Extract minute
	hour -= 7;
	if(hour < 0) hour += 24;
	RTC_TimeTypeDef TimeStruct;
	TimeStruct.Hours = hour;
	TimeStruct.Minutes = minute;
	TimeStruct.Seconds = 0;
	M5.Rtc.SetTime(&TimeStruct);
	RTC_DateTypeDef DateStruct;
	DateStruct.Date = day;
	DateStruct.Month = month;
	DateStruct.Year = year;
	M5.Rtc.SetDate(&DateStruct);
	
}