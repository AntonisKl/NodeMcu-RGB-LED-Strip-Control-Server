#include <ESP8266WebServer.h>
#include <TaskScheduler.h>

#include "ntp.h"

// #define TIME_HEADER  "T"   // Header tag for serial time sync message
// #define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

// // NTP Servers:
// static const char ntpServerName[] = "us.pool.ntp.org";
// //static const char ntpServerName[] = "time.nist.gov";
// //static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
// //static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
// //static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

// const int timeZone = 2;     // EET
// //const int timeZone = -5;  // Eastern Standard Time (USA)
// //const int timeZone = -4;  // Eastern Daylight Time (USA)
// //const int timeZone = -8;  // Pacific Standard Time (USA)
// //const int timeZone = -7;  // Pacific Daylight Time (USA)

// WiFiUDP Udp;
// unsigned int localPort = 8888;  // local port to listen for UDP packets

// RGB
const int redPin = D0;
const int greenPin = D1;
const int bluePin = D2;

// communication
const char* ssid = "Main";
const char* password = "";

const char* spaceDel = " ", *umbersangDel = "&";
char *rgbStrs[4];
char requestMessage[20];
char rgbValues[3];
Scheduler runner;
bool periodForTurningOnEnabled = false;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

// // NTP code

// const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
// byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// time_t getNtpTime()
// {
//   IPAddress ntpServerIP; // NTP server's ip address

//   while (Udp.parsePacket() > 0) ; // discard any previously received packets
//   Serial.println("Transmit NTP Request");
//   // get a random server from the pool
//   WiFi.hostByName(ntpServerName, ntpServerIP);
//   Serial.print(ntpServerName);
//   Serial.print(": ");
//   Serial.println(ntpServerIP);
//   sendNTPpacket(ntpServerIP);
//   uint32_t beginWait = millis();
//   while (millis() - beginWait < 1500) {
//     int size = Udp.parsePacket();
//     if (size >= NTP_PACKET_SIZE) {
//       Serial.println("Receive NTP Response");
//       Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
//       unsigned long secsSince1900;
//       // convert four bytes starting at location 40 to a long integer
//       secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
//       secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
//       secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
//       secsSince1900 |= (unsigned long)packetBuffer[43];
//       return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
//     }
//   }
//   Serial.println("No NTP Response :-(");
//   return 0; // return 0 if unable to get the time
// }

// // send an NTP request to the time server at the given address
// void sendNTPpacket(IPAddress &address)
// {
//   // set all bytes in the buffer to 0
//   memset(packetBuffer, 0, NTP_PACKET_SIZE);
//   // Initialize values needed to form NTP request
//   // (see URL above for details on the packets)
//   packetBuffer[0] = 0b11100011;   // LI, Version, Mode
//   packetBuffer[1] = 0;     // Stratum, or type of clock
//   packetBuffer[2] = 6;     // Polling Interval
//   packetBuffer[3] = 0xEC;  // Peer Clock Precision
//   // 8 bytes of zero for Root Delay & Root Dispersion
//   packetBuffer[12] = 49;
//   packetBuffer[13] = 0x4E;
//   packetBuffer[14] = 49;
//   packetBuffer[15] = 52;
//   // all NTP fields have been given values, now
//   // you can send a packet requesting a timestamp:
//   Udp.beginPacket(address, 123); //NTP requests are to port 123
//   Udp.write(packetBuffer, NTP_PACKET_SIZE);
//   Udp.endPacket();
// }

void setLEDColor(char r, char g, char b) {
  Serial.println("setting color");
  Serial.print("r=");
  Serial.print((int)r);
  Serial.print("g=");
  Serial.print((int)g);
  Serial.print("b=");
  Serial.println((int)b);
  analogWrite(redPin, map(r, 0, 255, 0, 1023));
  analogWrite(greenPin, map(g, 0, 255, 0, 1023));
  analogWrite(bluePin, map(b, 0, 255, 0, 1023));
}

void handleSetColor() {
  unsigned int serverArgsNum = server.args();

  Serial.println(serverArgsNum);

  if (serverArgsNum != 3)
    return;

  setLEDColor(server.arg(0).toInt(), server.arg(1).toInt(), server.arg(2).toInt());

   server.send(200, "text/plain", "Color set");
}

void getTimeAndTurnOnCallback();

Task turnOnTask(10000, TASK_FOREVER, &getTimeAndTurnOnCallback, &runner, true);  //adding task to the chain on creation

void getTimeAndTurnOnCallback() {
//  time_t timeNow = now();
//  Serial.print("Current time is: ");
//  Serial.print(hour());
//  Serial.print(" hours, ");
//  Serial.print(minute());
//  Serial.println(" minutes");

  if (periodForTurningOnEnabled == true)
  {
    setLEDColor(255, 255, 255);

    return;
  }

  unsigned int hourNow = hour();
  unsigned int minuteNow = minute();

  if (hourNow >= 9 && hourNow <= 12 &&  minute() >= 0) {
    setLEDColor(255, 255, 255);
    turnOnTask.setInterval(86400000); // 24 hours
    periodForTurningOnEnabled = true;
  }

  return;
}


void rainbow(unsigned int transitionSpeed)
{
  // Start off with red.
  rgbValues[0] = 255;
  rgbValues[1] = 0;
  rgbValues[2] = 0;  

  // Choose the colours to increment and decrement.
  for (int decColour = 0; decColour < 3; decColour++) {
    int incColour = decColour == 2 ? 0 : decColour + 1;

    // cross-fade the two colours.
    for(int i = 0; i < 255; i += 1) {
      rgbValues[decColour] -= 1;
      rgbValues[incColour] += 1;
        
      analogWrite(redPin, map(rgbValues[0], 0, 255, 0, 1023));
      analogWrite(greenPin, map(rgbValues[1], 0, 255, 0, 1023));
      analogWrite(bluePin, map(rgbValues[2], 0, 255, 0, 1023));
      delay(transitionSpeed);
    }
  }
}

void setup() {
  rgbValues[0] = 0;
  rgbValues[1] = 0;
  rgbValues[2] = 0;
  
  Serial.begin(9600);
  delay(10);

  // prepare GPIOS
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

//  setSyncProvider(requestSync);  //set function to call when sync required
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  server.on("/setColor", handleSetColor);

  server.begin();
  runner.startNow();  // set point-in-time for scheduling start
}

void loop() {
//  if (Serial.available()) {
//    processSyncMessage();
//  }
    server.handleClient();

  runner.execute();
  
//   // Check if a client has connected
//   WiFiClient client = server.available();

// //  rainbow(10);
//   if (!client) {
//     return;
//   }

// //   runner.execute();
  
//   // Wait until the client sends some data
//   Serial.println("new client");
//   while(!client.available()){
//     delay(1);
//   }
  
//   // Read the first line of the request
//   String req = client.readStringUntil('\r');
//   req.toCharArray(requestMessage, 20);
//   Serial.print("Full request:");
//   Serial.println(req);
//   client.flush();
  
//   rgbStrs[0] = strtok(requestMessage, spaceDel);
//   if (rgbStrs[0] != NULL) 
//     rgbStrs[0] = strtok(NULL, spaceDel);

//   rgbStrs[0] = rgbStrs[0] + 1;

//   rgbStrs[1] = strtok(rgbStrs[0], umbersangDel);
//   rgbStrs[2] = strtok(NULL, umbersangDel);
//   rgbStrs[3] = strtok(NULL, umbersangDel);

//   rgbValues[0] = (char) atoi(rgbStrs[1]);
//   rgbValues[1] = (char) atoi(rgbStrs[2]);
//   rgbValues[2] = (char) atoi(rgbStrs[3]);
  
//   Serial.print("red:");
//   Serial.println((int) rgbValues[0]);

//   Serial.print("green:");
//   Serial.println((int) rgbValues[1]);

//   Serial.print("blue:");
//   Serial.println((int) rgbValues[2]);
  
//   writeToPins(rgbValues[0], rgbValues[1], rgbValues[2]);
 }
