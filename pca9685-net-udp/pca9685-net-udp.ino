/*
Copyright (c) 2017 Patrik Mayer - patrik.mayer@codm.de

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
 
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Adafruit_PWMServoDriver.h>

EthernetClient ethClient;
EthernetUDP udp;
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


//-------- config --------
#define udpPort 7000
#define pwmInvert true //should the pwm outputs be inverted?

byte mac[] = {
  0x00, 0x00, 0xDE, 0xAD, 0xBE, 0xEF
};

//------------------------

#define msgMaxSize 16
#define channelCount 16

char msgBuffer[msgMaxSize];
unsigned int channelValues[channelCount];

void setup() {

  Serial.begin(9600);
  Serial.println("starting up...");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }

  // print your local IP address:
  printIPAddress();

  pwm.begin();
  pwm.setPWMFreq(1600);  // This is the maximum PWM frequency

  //init pca9685 all outputs to 0%
  for (uint8_t i = 0; i < channelCount; i++) {
    pwm.setPin(i, 0, pwmInvert);
    channelValues[i] = 0;
  }

  //bring up the udp listener
  udp.begin(udpPort);

  //ready, blink LED_BUILTIN
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(140);
  }

  Serial.println("ready...");


}

void loop() {

  //maintain the ethernet connection
  Ethernet.maintain();

  //handle incoming UDP messages and set channels accordingly
  handleUdp();

}

void handleUdp() {

  if (udp.parsePacket())  {
    udp.read(msgBuffer, msgMaxSize);

    //Serial.println(msgBuffer);
    handleMsg();

    //clear packet buffer
    memset(msgBuffer, 0, sizeof msgBuffer);

  }
}

void handleMsg() {
  //<channel>;<value>
  uint8_t ch = 0;
  uint16_t val = 0;

  sscanf(msgBuffer, "%d;%d", &ch, &val);

  ch = (ch < 16) ? ch : 0;
  val = (val < 4096) ? val : 4095;

  //set desired value for desired channel
  pwm.setPin(ch, val, pwmInvert);
}

void printIPAddress() {
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

