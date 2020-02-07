/*
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2018 Sensnology AB
   Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   REVISION HISTORY
   Version 1.0 - Henrik EKblad
   Contribution by a-lurker and Anticimex,
   Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
   Contribution by Tomas Hozza <thozza@gmail.com>


   DESCRIPTION
   The EthernetGateway sends data received from sensors to the ethernet link.
   The gateway also accepts input on ethernet interface, which is then sent out to the radio network.

   The GW code is designed for Arduino 328p / 16MHz.  ATmega168 does not have enough memory to run this program.

   LED purposes:
   - To use the feature, uncomment MY_DEFAULT_xxx_LED_PIN in the sketch below
   - RX (green) - blink fast on radio message received. In inclusion mode will blink fast only on presentation received
   - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
   - ERR (red) - fast blink on error during transmission error or receive crc error

   See http://www.mysensors.org/build/ethernet_gateway for wiring instructions.

*/

// Enable debug prints to serial monitor
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

// Enable gateway ethernet module type
#define MY_GATEWAY_W5100

// W5100 Ethernet module SPI enable (optional if using a shield/module that manages SPI_EN signal)
#define MY_W5100_SPI_EN 4

// Enable Soft SPI for NRF radio (note different radio wiring is required)
// The W5100 ethernet module seems to have a hard time co-operate with
// radio on the same spi bus.
#if !defined(MY_W5100_SPI_EN) && !defined(ARDUINO_ARCH_SAMD)
#define MY_SOFTSPI
#define MY_SOFT_SPI_SCK_PIN 14
#define MY_SOFT_SPI_MISO_PIN 16
#define MY_SOFT_SPI_MOSI_PIN 15
#endif

// When W5100 is connected we have to move CE/CSN pins for NRF radio
#ifndef MY_RF24_CE_PIN
#define MY_RF24_CE_PIN 5
#endif
#ifndef MY_RF24_CS_PIN
#define MY_RF24_CS_PIN 6
#endif

// Enable UDP communication
//#define MY_USE_UDP  // If using UDP you need to set MY_CONTROLLER_IP_ADDRESS below

// Enable MY_IP_ADDRESS here if you want a static ip address (no DHCP)
//#define MY_IP_ADDRESS 192,168,178,66

// If using static ip you can define Gateway and Subnet address as well
//#define MY_IP_GATEWAY_ADDRESS 192,168,178,1
//#define MY_IP_SUBNET_ADDRESS 255,255,255,0

// Renewal period if using DHCP
//#define MY_IP_RENEWAL_INTERVAL 60000

// The port to keep open on node server mode / or port to contact in client mode
#define MY_PORT 5003

// Controller ip address. Enables client mode (default is "server" mode).
// Also enable this if MY_USE_UDP is used and you want sensor data sent somewhere.
//#define MY_CONTROLLER_IP_ADDRESS 192, 168, 178, 254

// The MAC address can be anything you want but should be unique on your network.
// Newer boards have a MAC address printed on the underside of the PCB, which you can (optionally) use.
// Note that most of the Arduino examples use  "DEAD BEEF FEED" for the MAC address.
#define MY_MAC_ADDRESS 0x0A, 0xA3, 0xF8, 0x07, 0xE5, 0x54

//0A-A3-F8-07-E5-54

// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
//#define MY_INCLUSION_BUTTON_FEATURE
// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
//#define MY_INCLUSION_MODE_BUTTON_PIN  3

// Set blinking period
#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Flash leds on rx/tx/err
// Uncomment to override default HW configurations
#define MY_DEFAULT_ERR_LED_PIN 8  // Error led pin
#define MY_DEFAULT_RX_LED_PIN  2  // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  9  // Transmit led pin

#if defined(MY_USE_UDP)
#include <EthernetUdp.h>
#endif
#include <Ethernet.h>
#include <MySensors.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define CHILD_ID_LCD_BACKLIGHT 0

#define NUMBER_OF_CHARS 20
#define NUMBER_OF_LINES 4

LiquidCrystal_I2C lcd(0x27, NUMBER_OF_CHARS, NUMBER_OF_LINES); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init();                      // initialize the lcd
  lcd.backlight();
  LcdIpVersionInfo();
  Serial.println(F("[DEBUG]- SKETCHFILE: "__FILE__));
  Serial.println(F("[DEBUG]- SKETCHDATE: "__DATE__));
  Serial.println(F("[DEBUG]- SKETCHTIME: "__TIME__));
}

void presentation()  {
  String version = (String)"" + MYSENSORS_LIBRARY_VERSION;

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("BW Gateway", string2char(version));

  // Register the lcd BackLight to gw (they will be created as child devices)
  present(CHILD_ID_LCD_BACKLIGHT, S_BINARY);

  // Register all LCD Lines to gw (they will be created as child devices)
  for (int lcdLine = 1; lcdLine <= NUMBER_OF_LINES; lcdLine++) {
    present(lcdLine, S_INFO);
  }
}

// tbv sendSketchInfo - Mysensors version nummer converteren naar char array
char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void loop()
{
  // volgens mij is het beter geen loops op te nemen in de gateway
}

void LcdIpVersionInfo()
{
  lcd.setCursor(0, 0);
  lcd.print((String)"Version: " + MYSENSORS_LIBRARY_VERSION);
  lcd.setCursor(0, 1);
  lcd.print((String)"IP:");
  lcd.setCursor(3, 1);
  lcd.print(Ethernet.localIP());
}



void receive(const MyMessage &message) {
  Serial.println((String)"[DEBUG]- Incoming Message from nodeID: " + message.getSender());
  if (getNodeId() == message.getSender())
  {
    if (message.type == V_TEXT) {
      Serial.print("[DEBUG]- Incoming Message for lcd line ");
      Serial.println(message.getSensor() - 1);
      Serial.print("[DEBUG]- Message:  ");
      Serial.println(message.getString());
      SetLcdLine(message.sensor - 1, message.getString());
    }
    if (message.type == V_STATUS && message.sensor == CHILD_ID_LCD_BACKLIGHT) {
      Serial.print("[DEBUG]- Incoming Message for lcd backlight ");
      Serial.println(message.getSensor());
      Serial.print("[DEBUG]- Message:  ");
      Serial.println(message.getBool());
      SetLcdBackLight(message.getBool());
    }
  }
}

void SetLcdLine(int line, String text)
{
  ClearLCDLine(line);
  lcd.print(text);
}

void ClearLCDLine(int line) {
  for (int n = 0; n < NUMBER_OF_CHARS; n++) {
    lcd.setCursor(n, line);
    lcd.print(" ");
  }
  // set cursor in the beginning of deleted line
  lcd.setCursor(0, line);
}

void SetLcdBackLight(bool state)
{
  if (state)
  {
    lcd.backlight();
  }
  else
  {
    lcd.noBacklight();
  }
}
