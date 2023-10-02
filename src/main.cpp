/*

Project:      LoRa32_TextieTalkie 
File:         main.cpp
Date Created: 9/8/2023
Author:       Paul Price 
Libraries:	  RadioLib from Jan Gromeš
			  Link: https://github.com/jgromes/RadioLib

Description:
LoRa transmission code based off the RadioLib PingPong example for SX1278.
The idea is an offline communication channel for back and forth text messages.

The LoRa32's initialize with a back and forth message to confirm the devices
are operating correctly and within range. The MCU serves a webpage running on 
the second core of the ESP32, so it is then always accessable and ready to 
recive user inputs and messages. The onboard OLED display prompts the user 
to the communication process, indicating when they have recived a message 
and should reload their webage, when they can send a new message, and if 
that message was sent successfully. 
 
*/

// include the library
#include <Arduino.h>
#include <WiFi.h>
#include <RadioLib.h>

// Libraries for OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED pins
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Replace with custom LoRa32 credentials
const char *ssid = "LoRa32-Access-Point-One";				//CHANGE FOR EACH DEVICE
const char *password = "123456789";

// Set web server port number to 80
WiFiServer server(80);
WiFiClient client;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

// uncomment the following only on one of the nodes to initiate the pings
#define INITIATING_NODE								    //CHANGE FOR EACH DEVICE

// SX1278 has the following connections on TTGO LoRa32 Board:
// NSS pin:   18
// DIO0 pin:  26
// NRST pin:  23
// DIO1 pin:  19
SX1278 radio = new Module(18, 26, 23, 19);

String LoRaData = "Initializing...";
String header;
String parsedTxData = "Initialized... You may start the conversation!";
String userData = "Initializing";
String RxStr;

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

// this function is called when a complete packet is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type and MUST NOT have any arguments!
void setFlag(void)
{
	// we sent or received  packet, set the flag
	operationDone = true;
}

TaskHandle_t task1; // Create task

void task1code(void *pvParameters)
{
	for (;;)
	{
		// Small delay needed to keep watch dog timer working
		vTaskDelay(1); // https://github.com/espressif/arduino-esp32/issues/5048

		client = server.available(); // Listen for incoming clients
		if (client)
		{								   // If a new client connects,
			Serial.println("New Client."); // print a message out in the serial port
			String currentLine = "";	   // make a String to hold incoming data from the client
			while (client.connected())
			{ // loop while the client's connected
				if (client.available())
				{							// if there's bytes to read from the client,
					char c = client.read(); // read a byte, then
					//Serial.write(c);		// print it out the serial monitor
					header += c;
					if (c == '\n')
					{ // if the byte is a newline character
						// if the current line is blank, you got two newline characters in a row.
						// that's the end of the client HTTP request, so send a response:
						if (currentLine.length() == 0)
						{
							// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
							// and a content-type so the client knows what's coming, then a blank line:
							client.println("HTTP/1.1 200 OK");
							client.println("Content-type:text/html");
							client.println("Connection: close");
							client.println();

							// Display the HTML web page
							client.println("<!DOCTYPE html><html>");
							client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
							// CSS
							// Feel free to change the background-color and font-size attributes to fit your preferences
							client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
							client.println(".button { background-color: #0390fc; border: none; color: white; padding: 16px 40px;");
							client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
							client.println("</style></head>");

							// Web Page Heading
							client.println("<body><h1>LoRa32 Textie Talkie</h1>");
							client.println("<form> Enter your message in the text box below.");
							client.println("<br><br> <textarea rows=\"5\" cols=\"40\" type=\"text\" id=\"TxString\"></textarea></form>");
							client.println("<p><button type=\"submit\" onclick=\"updateURL(document.getElementById(\'TxString\'))\" id=\"demo\" class=\"button\">Submit and Send</button></p><br><br><br>");
							client.println("<script type=\"text/javascript\"> function updateURL(chk) {window.location.replace(':' + chk.value + '~'); } </script>");
							client.println("<br><br>");
							client.println("<h3>Received Data</h3>");
							client.println("<p>Please use the button below to load response when prompted!</p>");
							client.println("<textarea rows=\"5\" cols=\"40\" type=\"text\">");
							client.println(RxStr);
							client.println("</textarea>");
							client.println("<p><button type=\"submit\" onclick=\"reloadURL()\" id=\"demo1\" class=\"button\">Receive and Refresh</button></p><br><br><br>");
							client.println("<script type=\"text/javascript\"> function reloadURL() {window.location.replace(':reload~');} </script>");
							client.println("</body></html>");

							// The HTTP response ends with another blank line
							client.println();
							// Break out of the while loop
							break;
						}
						else
						{ // if you got a newline, then clear currentLine
							currentLine = "";
						}
					}
					else if (c != '\r')
					{					  // if you got anything else but a carriage return character,
						currentLine += c; // add it to the end of the currentLine
					}
				}
			}
			Serial.println(header);
			int ind1 = header.indexOf(':');
			int ind2 = header.indexOf('~');
			String parsedTxData1 = header.substring(ind1 + 1, ind2);
			parsedTxData1.replace("%20", " ");
			parsedTxData1.replace("%E2%80%%99", "'");
			parsedTxData = parsedTxData1;
			parsedTxData1 = "";
			Serial.println(parsedTxData);
			Serial.println();

			// Close the connection
			client.stop();
			Serial.println("Client disconnected.");
			Serial.println("");
		}
	}
}

void setup()
{
	Serial.begin(115200);

	delay(1000);
	// Connect to Wi-Fi network with SSID and password
	Serial.print("Setting AP (Access Point)…");
	// Remove the password parameter, if you want the AP (Access Point) to be open
	WiFi.softAP(ssid, password);

	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);

	server.begin();

	xTaskCreatePinnedToCore(
		task1code, /* Task function. */
		"task1",   /* name of task. */
		10000,	   /* Stack size of task */
		NULL,	   /* parameter of the task */
		1,		   /* priority of the task */
		&task1,	   /* Task handle to keep track of created task */
		0);		   /* pin task to core 0 */

	// reset OLED display via software
	pinMode(OLED_RST, OUTPUT);
	digitalWrite(OLED_RST, LOW);
	delay(20);
	digitalWrite(OLED_RST, HIGH);

	// initialize OLED
	Wire.begin(OLED_SDA, OLED_SCL);
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
	{ // Address 0x3C for 128x32
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			; // Don't proceed, loop forever
	}

	display.clearDisplay();
	display.setTextColor(WHITE);
	display.setTextSize(1);
	display.setCursor(0, 0);

	// initialize SX1278 with default settings
	Serial.print(F("[SX1278] Initializing ... "));
	int state = radio.begin(434.0, 125.0, 12, 7, RADIOLIB_SX127X_SYNC_WORD, 17, 8, 0);
	if (state == RADIOLIB_ERR_NONE)
	{
		Serial.println(F("success!"));
		display.setCursor(0, 0);
		display.print("SX1278 Initialized!");
		display.display();
		delay(2000);
	}
	else
	{
		Serial.print(F("failed, code "));
		Serial.println(state);
		display.print("SX1278 Failed!");
		display.display();
		delay(2000);
		while (true)
			;
	}

	// set the function that will be called when new packet is received
	radio.setDio0Action(setFlag, RISING);

	#if defined(INITIATING_NODE)
		// send the first packet on this node
		Serial.print(F("[SX1278] Sending first packet ... "));
		transmissionState = radio.startTransmit(LoRaData);
		transmitFlag = true;

		display.clearDisplay();
		display.setCursor(0, 0);
		display.println("Sending first packet");
		display.setCursor(0, 20);
		display.print(LoRaData);

	#else
		// start listening for LoRa packets on this node
		Serial.print(F("[SX1278] Starting to listen ... "));
		display.clearDisplay();
		display.setCursor(0, 0);
		display.print("Starting to listen");
		state = radio.startReceive();
		if (state == RADIOLIB_ERR_NONE)
		{
			Serial.println(F("success!"));

			display.setCursor(0, 10);
			display.print("Success!");
			display.display();
		}
		else
		{
			Serial.print(F("failed, code "));
			Serial.println(state);

			display.setCursor(0, 10);
			display.print("Failed, code ");
			display.setCursor(70, 10);
			display.print(state);
			display.display();
			while (true);
		}
	#endif
}

void loop()
{
	// check if the previous operation finished
	if (operationDone)
	{
		// reset flag
		operationDone = false;

		if (transmitFlag)
		{
			// the previous operation was transmission, listen for response
			// print the result
			if (transmissionState == RADIOLIB_ERR_NONE)
			{
				// packet was successfully sent
				Serial.println(F("transmission finished!"));
				display.setCursor(0, 40);
				display.print("TX Finished!");
				display.display();
			}
			else
			{
				Serial.print(F("failed, code "));
				Serial.println(transmissionState);

				display.setCursor(0, 40);
				display.print("Failed, code ");
				display.setCursor(70, 40);
				display.print(transmissionState);
				display.display();
			}

			// listen for response
			radio.startReceive();
			delay(1000);
			display.clearDisplay();
			display.setCursor(0, 0);
			display.print("Waiting for Response!");
			display.display();
			delay(1000);
			transmitFlag = false;
		}
		else
		{
			// the previous operation was reception
			// print data and send another packet
			String str;
			int state = radio.readData(str);
			RxStr = str;

			if (state == RADIOLIB_ERR_NONE)
			{
				// packet was successfully received
				Serial.println(F("[SX1278] Received packet!"));

				// print data of the packet
				Serial.print(F("[SX1278] Data:\t\t"));
				Serial.println(str);

				// print RSSI (Received Signal Strength Indicator)
				Serial.print(F("[SX1278] RSSI:\t\t"));
				Serial.print(radio.getRSSI());
				Serial.println(F(" dBm"));

				// print SNR (Signal-to-Noise Ratio)
				Serial.print(F("[SX1278] SNR:\t\t"));
				Serial.print(radio.getSNR());
				Serial.println(F(" dB"));
			}

			// send another one
			Serial.print(F("[SX1278] Your turn to transmit ... "));

			while (userData == "") 
			{
				display.clearDisplay();
				display.setCursor(0, 0);
				display.print("NEW MESSAGE FOR YOU!!");
				display.setCursor(0, 20);
				display.print("RSSI: ");
				display.setCursor(40, 20);
				display.print(radio.getRSSI());
				display.print(" dBm");
				display.setCursor(0, 30);
				display.print("SNR: ");
				display.setCursor(40, 30);
				display.print(radio.getSNR());
				display.display();
				delay(1000);

				userData = parsedTxData;

				if (userData == "reload")
				{
					display.clearDisplay();
					display.setCursor(0, 0);
					display.print("Please Respond!");
					display.setCursor(0,20);
					display.print("Your Input from HTML -->");
					display.display();
					delay(1000);

					userData = "";
					parsedTxData = "";
					header = "";
					while (userData == "")
					{
						userData = parsedTxData;

						if (userData != "")
						{
							break;
						}
					}
				}
			}

			delay(500);

			transmissionState = radio.startTransmit(userData);

			display.clearDisplay();
			display.setCursor(0, 0);
			display.print("TX your message");
			display.setCursor(0, 20);
			display.print(userData);
			display.display();

			transmitFlag = true;

			header = "";
			userData = "";
			parsedTxData = "";
		}
	}
}