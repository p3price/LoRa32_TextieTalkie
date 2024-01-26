# LoRa32_TextieTalkie Project
This project spun off of the research work I was doing during my EE undergraduate.
I was experimenting with LoRa32 boards and had the idea to create an "off-line" texting communication application.

# Description
The LoRa32 boards have ESP32 and Semtech LoRa Transciever chips.
On the ESP32 the main core performs a loop either accepting an incoming message or sending a message. The communication between the devices can only happen in a ping-pong type activity. Only send, then receive, send, then receive, etc... 
The other core of the ESP32 is serving a webpage that the user connects to with their mobile device. Two text boxes allow the user to input a message to send and view received messages.
The OLED display on the LoRa32 promps the user to the current stage of communication they are in, whether they are able to send a new message or if there is a message waiting for them to be viewed.

This project is ready to be opened in VSCode and depends on the PlatformIO Extenstion to communicate with the mcu.

Edit: Looking back on the project now, the methodology that I used is fairly wacky especially in how the message data is grabbed from the webpage. Nevertheless this was a fun project that I was proud to have developed mostly on my own.

# Hardware
LilyGo LoRa32 915Mz
https://www.lilygo.cc/products/lora3?variant=42272562315445
![IMG_0914 Large](https://github.com/p3price/LoRa32_TextieTalkie/assets/126983543/a0ddb84a-a2cf-463f-93b5-dcb840cfe487)
(Show with 3d printed housing)

# Reference Webpage Image
![IMG_0913](https://github.com/p3price/LoRa32_TextieTalkie/assets/126983543/ed90bb2a-e8d0-457f-a926-d30b05a7f836)
