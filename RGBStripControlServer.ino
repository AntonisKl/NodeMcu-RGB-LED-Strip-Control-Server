#include <ESP8266WebServer.h>
#include <TaskScheduler.h>

#include "ntp.h"

// RGB
const int redPin = D0;
const int greenPin = D1;
const int bluePin = D2;

// communication
const char *ssid = "Main";
const char *password = "";

const char *spaceDel = " ", *umbersangDel = "&";
char *rgbStrs[4];
char requestMessage[20];
char rgbValues[3];
Scheduler runner;
bool periodForTurningOnEnabled = false;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

void setLEDColor(char r, char g, char b)
{
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

void handleSetColor()
{
  unsigned int serverArgsNum = server.args();

  Serial.println(serverArgsNum);

  if (serverArgsNum != 3)
    return;

  setLEDColor(server.arg(0).toInt(), server.arg(1).toInt(), server.arg(2).toInt());

  server.send(200, "text/plain", "Color set");
}

void getTimeAndTurnOnCallback();

Task turnOnTask(10000, TASK_FOREVER, &getTimeAndTurnOnCallback, &runner, true); //adding task to the chain on creation

void getTimeAndTurnOnCallback()
{
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

  if (hourNow >= 9 && hourNow <= 12 && minute() >= 0)
  {
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
  for (int decColour = 0; decColour < 3; decColour++)
  {
    int incColour = decColour == 2 ? 0 : decColour + 1;

    // cross-fade the two colours.
    for (int i = 0; i < 255; i += 1)
    {
      rgbValues[decColour] -= 1;
      rgbValues[incColour] += 1;

      analogWrite(redPin, map(rgbValues[0], 0, 255, 0, 1023));
      analogWrite(greenPin, map(rgbValues[1], 0, 255, 0, 1023));
      analogWrite(bluePin, map(rgbValues[2], 0, 255, 0, 1023));
      delay(transitionSpeed);
    }
  }
}

void setup()
{
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

  while (WiFi.status() != WL_CONNECTED)
  {
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
  // runner.startNow();
}

void loop()
{
  server.handleClient();
  // runner.execute();
}
