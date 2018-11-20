/*
 *  This sketch demonstrates how to set up a simple HTTP-like server.
 *  The server will set a GPIO pin depending on the request
 *    http://server_ip/gpio/0 will set the GPIO2 low,
 *    http://server_ip/gpio/1 will set the GPIO2 high
 *  server_ip is the IP address of the ESP8266 module, will be 
 *  printed to Serial when the module is connected.
 */

#include <ESP8266WiFi.h>

// RGB
const int redPin = D0;
const int greenPin = D1;
const int bluePin = D2;

// communication
const char* ssid = "Home_wifi";
const char* password = "2106203917";

const char* spaceDel = " ", *umbersangDel = "&";
char *rgbStrs[4];
char requestMessage[20];
char rgbValues[3];

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

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

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
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();

//  rainbow(10);
  if (!client) {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  req.toCharArray(requestMessage, 20);
  Serial.print("Full request:");
  Serial.println(req);
  client.flush();
  
   rgbStrs[0] = strtok(requestMessage, spaceDel);
   if (rgbStrs[0] != NULL) 
      rgbStrs[0] = strtok(NULL, spaceDel);

  rgbStrs[0] = rgbStrs[0] + 1;

  rgbStrs[1] = strtok(rgbStrs[0], umbersangDel);
  rgbStrs[2] = strtok(NULL, umbersangDel);
  rgbStrs[3] = strtok(NULL, umbersangDel);

  rgbValues[0] = (char) atoi(rgbStrs[1]);
  rgbValues[1] = (char) atoi(rgbStrs[2]);
  rgbValues[2] = (char) atoi(rgbStrs[3]);
  
  Serial.print("red:");
  Serial.println((int) rgbValues[0]);

  Serial.print("green:");
  Serial.println((int) rgbValues[1]);

  Serial.print("blue:");
  Serial.println((int) rgbValues[2]);
  

  analogWrite(redPin, map(rgbValues[0], 0, 255, 0, 1023));
  analogWrite(greenPin, map(rgbValues[1], 0, 255, 0, 1023));
  analogWrite(bluePin, map(rgbValues[2], 0, 255, 0, 1023));
  
  


  // Match the request
//  int val;
//  if (req.indexOf("/gpio/0") != -1)
//    val = 0;
//  else if (req.indexOf("/gpio/1") != -1)
//    val = 1;
//  else {
//    Serial.println("invalid request");
//    client.stop();
//    return;
//  }

  // Set GPIO2 according to the request
//  digitalWrite(2, val);
  
//  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
//  s += (val)?"high":"low";
//  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}
