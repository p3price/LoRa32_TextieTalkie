# LoRa32_TextieTalkie Project
This project spun off of the research work I was doing during my EE undergraduate.
I was experimenting with LoRa32 boards and had the idea to create an "off-line" texting communication application.

# Description
The LoRa32 boards have ESP32 and Semtech LoRa Transciever chips.
On the ESP32 the main core performs a loop either accepting an incoming message or sending a message. The communication between the devices can only happen in a ping-pong type activity. Only send, then receive, send, then receive, etc... 
The other core of the ESP32 is serving a webpage that the user connects to with their mobile device. Two text boxes allow the user to input a message to send and view received messages.
The OLED display on the LoRa32 promps the user to the current stage of communication they are in, whether they are able to send a new message or if there is a message waiting for them to be viewed.

# Hardware


# Reference Images
