#include <ESP8266WebServer.h>
#include <TaskScheduler.h>

#include "ntp.h"

#define SET_COLOR_URI "/setColor"
#define RAINBOW_URI "/rainbowEffect"

// RGB
const int redPin = D0;
const int greenPin = D1;
const int bluePin = D2;

// communication
const char *ssid = "Main";
const char *password = "";

Scheduler runner;
bool periodForTurningOnEnabled = false;

String lastRequest;
unsigned int colorTransitionSpeed = 5; // milliseconds

char currentRGB[3] = {0, 0, 0};

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);

void writeRGB(char r, char g, char b)
{
  analogWrite(redPin, map(r, 0, 255, 0, 1023));
  analogWrite(greenPin, map(g, 0, 255, 0, 1023));
  analogWrite(bluePin, map(b, 0, 255, 0, 1023));
}

char getOperandValue(char currentValue, char goalValue)
{
  if (currentValue > goalValue)
    return -1;

  if (currentValue < goalValue)
    return 1;

  return 0;
}

void setLEDColorSmooth(char r, char g, char b, unsigned int transitionSpeed)
{
  // Serial.println("setting color");
  // Serial.print("r=");
  // Serial.print((int)r);
  // Serial.print("g=");
  // Serial.print((int)g);
  // Serial.print("b=");
  // Serial.println((int)b);
  if (transitionSpeed <= 0)
  {
    writeRGB(r, g, b);
    return;
  }

  char operandR = getOperandValue(currentRGB[0], r);
  char operandG = getOperandValue(currentRGB[1], g);
  char operandB = getOperandValue(currentRGB[2], b);

  while (currentRGB[0] != r || currentRGB[1] != g || currentRGB[2] != b)
  {
    if (currentRGB[0] != r)
      currentRGB[0] += operandR;
    if (currentRGB[1] != g)
      currentRGB[1] += operandG;
    if (currentRGB[2] != b)
      currentRGB[2] += operandB;
    
    writeRGB(currentRGB[0], currentRGB[1], currentRGB[2]);
    delay(transitionSpeed);
  }
}

void handleSetColor()
{
  lastRequest = SET_COLOR_URI;
  unsigned int serverArgsNum = server.args();

  Serial.println(serverArgsNum);

  if (serverArgsNum != 3)
    return;

  setLEDColorSmooth(server.arg(0).toInt(), server.arg(1).toInt(), server.arg(2).toInt(), colorTransitionSpeed);

  server.send(200, "text/plain", "Color set");
}

void handleRainbowEffect()
{
  lastRequest = RAINBOW_URI;
  unsigned int serverArgsNum = server.args();

  Serial.println(serverArgsNum);

  if (serverArgsNum != 1)
    return;

  int transitionInterval = server.arg(0).toInt();
  rainbow(transitionInterval);

  server.send(200, "text/plain", "Rainbow effect ended");
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
    setLEDColorSmooth(255, 255, 255, colorTransitionSpeed);

    return;
  }

  unsigned int hourNow = hour();
  unsigned int minuteNow = minute();

  if (hourNow >= 9 && hourNow <= 12 && minute() >= 0)
  {
    setLEDColorSmooth(255, 255, 255, colorTransitionSpeed);
    turnOnTask.setInterval(86400000); // 24 hours
    periodForTurningOnEnabled = true;
  }

  return;
}

void rainbow(unsigned int transitionInterval)
{
  // Start off with red.
  char rgbValues[3] = {255, 0, 0};

  // Choose the colours to increment and decrement.
  for (int decColour = 0; decColour < 3; decColour++)
  {
    int incColour = decColour == 2 ? 0 : decColour + 1;

    // cross-fade the two colours.
    for (int i = 0; i < 255; i++)
    {
      server.handleClient();
      if (lastRequest != RAINBOW_URI)
        return;

      rgbValues[decColour]--;
      rgbValues[incColour]++;

      writeRGB(rgbValues[0], rgbValues[1], rgbValues[2]);
      delay(transitionInterval);
    }
  }
}

void setup()
{
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

  server.on(SET_COLOR_URI, handleSetColor);
  server.on(RAINBOW_URI, handleRainbowEffect);

  server.begin();
  // runner.startNow();
}

void loop()
{
  server.handleClient();
  // runner.execute();
}
