#include <Adafruit_MAX31856.h>

#include <ESP8266WebServer.h>
#include <strings_en.h>
#include <WiFiManager.h>

#include <EEPROM.h>


#include <Wire.h>

#include <SPI.h>
#include "Adafruit_MAX31855.h"

#define numberOfPointsToSave 100
#define saveInterval 5
float tData[numberOfPointsToSave] = {};

#define MAXDO   D2
#define MAXCS   D4
#define MAXCLK  D3

#define DO2 D2
#define CLK2 D3
#define DI2 D1
#define CS2 D0

Adafruit_MAX31855 thermocouple(MAXCLK, MAXCS, MAXDO);
Adafruit_MAX31856 thermocouple2 = Adafruit_MAX31856(CS2, DI2, DO2, CLK2);

double temperature = 0, humidity = 0, pressure = 0, altitude = 0;
double temperature2 = 0;
String timeData = "";
//String tempData = "";
int count = 0;
int t = 0;
int count1 = 0;
int x = 0;
int x1 = 0;
int x2 = 0;
int x3 = 0;

int currentTime = 0;


bool dispIP = true;
int count2 = 0;
bool printTemp = false;

int connectCount = 0;
bool conn = true;

#include <ShiftRegister74HC595.h>

#define numberOfShiftRegisters 2
#define serialDataPin D7
#define clockPin D5
#define latchPin D6

#define serialDataPin2 D7
#define clockPin2 D5
#define latchPin2 D8

ShiftRegister74HC595<numberOfShiftRegisters> sr1(serialDataPin, clockPin, latchPin);

ShiftRegister74HC595<numberOfShiftRegisters> sr2(serialDataPin2, clockPin2, latchPin2);


ESP8266WebServer server(80);

uint8_t setValues[] = { B00000000, B00000000 };

class LEDDISP
{
    int num;
    int pin;
    uint8_t pinValues = { B11111111 };

  public:
    //void setNum(int n)
    //{
    // num = n;
    //}
    void setPin(int p)
    {
      pin = p;
    }

    uint8_t getPinValues()
    {
      return pinValues;
    }

    void setNum(int n)
    {
      num = n;
      switch (num)
      {
        case 0:
          pinValues = B11000000;
          break;

        case 1:
          pinValues = B11111001;
          break;

        case 2:
          pinValues = B10100100;
          break;

        case 3:
          pinValues = B10110000;
          break;

        case 4:
          pinValues = B10011001;
          break;

        case 5:
          pinValues = B10010010;
          break;

        case 6:
          pinValues = B10000010;
          break;

        case 7:
          pinValues = B11111000;
          break;

        case 8:
          pinValues = B10000000;
          break;

        case 9:
          pinValues = B10010000;
          break;

      }
    }

    void dispOff()
    {
      pinValues = B11111111;
    }

};

LEDDISP LED1;
LEDDISP LED2;
LEDDISP LED3;
LEDDISP LED4;

LEDDISP LED21;
LEDDISP LED22;
LEDDISP LED23;
LEDDISP LED24;


class LEDDISPLAY
{
    int digits;
    int num;
    int num1;
    int num2;
    int num3;
    int num4;
    int displaying = 1;
    int decimal = 0;
    LEDDISP *Digit1;
    LEDDISP *Digit2;
    LEDDISP *Digit3;
    LEDDISP *Digit4;
    uint8_t digitValues = B00000000;
    uint8_t pinVals = B00000000;
    bool display = true;

    ShiftRegister74HC595<numberOfShiftRegisters> *sr;

  public:
    LEDDISPLAY(ShiftRegister74HC595<numberOfShiftRegisters> *s)
    {
      sr =s;
    }

    void setDigits(LEDDISP *D1, LEDDISP *D2, LEDDISP *D3, LEDDISP *D4)
    {
      Digit1 = D1;
      Digit2 = D2;
      Digit3 = D3;
      Digit4 = D4;
    }

    void DisplayNum(double n)
    {
      if (n < 100)
      {
        decimal = 2;
        num = n * 100;
      }
      else if (n >= 100 && n < 1000)
      {
        decimal = 1;
        num = n * 10;
      }
      else
      {
        decimal = 0;
        num = n;
      }
      //Serial.println(num);
      num1 = (num / 1U) % 10;
      num2 = (num / 10U) % 10;
      num3 = (num / 100U) % 10;
      num4 = (num / 1000U) % 10;
      Digit1->setNum(num1);
      Digit2->setNum(num2);
      Digit3->setNum(num3);
      Digit4->setNum(num4);
      display = true;
    }

    void updateDisplay()
    {
      if (displaying == 1)
      {
        //Digit1->dispOff();
        //Digit3->dispOff();
        //Digit4->dispOff();
        //Digit2->updateDisp();
        pinVals = Digit2->getPinValues();
        digitValues = B11110010;
        if (decimal == 1)
        {
          pinVals = pinVals + B10000000;
        }
        displaying = 2;
      }
      else if (displaying == 2)
      {
        //Digit2->dispOff();
        //Digit3->dispOff();
        //Digit4->dispOff();
        //Digit1->updateDisp();
        pinVals = Digit3->getPinValues();
        digitValues = B11110100;
        if (decimal == 2)
        {
          pinVals = pinVals + B10000000;
        }
        displaying = 3;
      }
      else if (displaying == 3)
      {
        //Digit2->dispOff();
        //Digit1->dispOff();
        //Digit4->dispOff();
        //Digit3->updateDisp();
        pinVals = Digit4->getPinValues();
        digitValues = B11111000;

        displaying = 4;
      }
      else if (displaying == 4)
      {
        //Digit2->dispOff();
        //Digit3->dispOff();
        //Digit1->dispOff();
        //Digit4->updateDisp();
        pinVals = Digit1->getPinValues();
        digitValues = B11110001;
        displaying = 1;
      }
      if (display == false)
      {
        digitValues = B11111111;
      }
      setValues[0] = digitValues;
      setValues[1] = pinVals;
      sr->setAll(setValues);
    }

    void displayOff()
    {
      Digit1->dispOff();
      Digit2->dispOff();
      Digit3->dispOff();
      Digit4->dispOff();
      display = false;
    }
};

LEDDISPLAY LEDDisplay1(&sr1);
LEDDISPLAY LEDDisplay2(&sr2);

class dataStore
{
    int l;
    float *data;
    int endIndex;
  public:
    dataStore(int ll, float *d)
    {
      l = ll;
      data = d;
      endIndex = ll - 1;
    }

    int getLength()
    {
      return l;
    }

    float getElement(int i)
    {
      int ii = i + 1 + endIndex;
      //Serial.print("Index:");

      if (ii >= l)
      {
        ii = ii - l;
      }
      //Serial.println(ii);
      return data[ii];
    }

    void storeElement(float i)
    {
      endIndex++;
      if (endIndex >= l)
      {
        endIndex = 0;
      }

      data[endIndex] = i;
    }


};

dataStore tempData(numberOfPointsToSave, tData);



void setup() {
  Serial.begin(115200);
  delay(100);
  
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("Thermometer"); // password protected ap

  if (!res) 
  {
    Serial.println("Failed to connect");
    // ESP.restart();
  }
  else 
  {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }

  //displayIP();

  LEDDisplay1.setDigits(&LED1, &LED2, &LED3, &LED4);
  LEDDisplay2.setDigits(&LED21, &LED22, &LED23, &LED24);

  server.on("/", handle_OnConnect);
  server.on("/getTemp", handle_getTemp);
  server.on("/getTemp2", handle_getTemp2);
  server.on("/getInitData", handle_initialData);
  server.on("/getTime", handle_getTime);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  
  if (!thermocouple2.begin()) {
    Serial.println("Could not initialize thermocouple.");
  }

  thermocouple2.setThermocoupleType(MAX31856_TCTYPE_K);

  Serial.print("Initializing sensor...");
  if (!thermocouple.begin()) {
    Serial.println("ERROR.");
    while (1) delay(10);
  }
  Serial.println("DONE.");

  thermocouple2.setConversionMode(MAX31856_CONTINUOUS);

  displayIP();


}

void loop() {
  server.handleClient();
  LEDDisplay1.updateDisplay();
  LEDDisplay2.updateDisplay();
  delay(2);
  if (count == 166)
  {
    readTemp1();
  }
  else if (count == 333)
  {
    readTemp2();
  }
  else if (count == 500)
  {
    pushDisplay();
    count = 0;
  }
  count++;
}

void displayIP()
{
  Serial.print("IP: ");  
  Serial.println(WiFi.localIP());
  for (int i = 0; i < 4; i++)
  {
    LEDDisplay1.displayOff();
    LEDDisplay1.updateDisplay();
    delay(200);
    LEDDisplay1.DisplayNum(WiFi.localIP()[i]);
    for (int j = 0; j < 1000; j++)
    {
      LEDDisplay1.updateDisplay();
      delay(1);
    }
  }

}

void displayIP(IPAddress &ipaddr)
{
  Serial.print("IP: ");  Serial.println(ipaddr);
  for (int i = 0; i < 4; i++)
  {
    LEDDisplay1.displayOff();
    LEDDisplay1.updateDisplay();
    delay(200);
    LEDDisplay1.DisplayNum(ipaddr[i]);
    for (int j = 0; j < 1000; j++)
    {
      LEDDisplay1.updateDisplay();
      delay(1);
    }
  }

}

void readTemp1()
{
  temperature = thermocouple.readFahrenheit();
}

void readTemp2()
{
  temperature2 = thermocouple2.readThermocoupleTemperature()*1.8+32;
  thermocouple2.triggerOneShot();
}

void pushDisplay()
{
  
  LEDDisplay1.DisplayNum(temperature);
  LEDDisplay2.DisplayNum(temperature2);
  if (count1 == saveInterval)
  {
    tempData.storeElement(temperature);
    count1 = 0;
  }
  if (printTemp)
  {
    Serial.println(temperature);
  }
  count1++;
  currentTime++;
  
}



void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(temperature, humidity, pressure, altitude));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

void handle_getTemp()
{
  server.send(200, "text/plain", String(temperature));
}

void handle_getTemp2()
{
  server.send(200, "text/plain", String(temperature2));
}

void handle_initialData()
{
  String sendDat = "";
  for (int i = 0; i < tempData.getLength(); i++)
  {
    sendDat = sendDat + String(tempData.getElement(i)) + " ";
  }
  server.send(200, "text/plain", sendDat);
}

void handle_getTime()
{
  server.send(200, "text/plane", String(currentTime));
}


String SendHTML(float temperature, float humidity, float pressure, float altitude) {
  //  String ptr = "<!DOCTYPE html> <html>\n";
  //  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  //  //ptr += "<meta http-equiv=\"refresh\" content=\"5\" >\n";
  //  ptr += "<title>Oven Thermometer</title>\n";
  //  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  //  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  //  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  //  ptr += "</style>\n";
  //  ptr += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>";
  //
  //  ptr += "<script>\n";
  //  ptr += "var tempData1 = 0;\n";
  //  ptr += "setInterval(loadDoc,1000);\n";
  //  ptr += "function loadDoc() {\n";
  //  ptr += "var xhttp = new XMLHttpRequest();\n";
  //  ptr += "xhttp.onreadystatechange = function() {\n";
  //  ptr += "if (this.readyState == 4 && this.status == 200) {\n";
  //  ptr += "document.getElementById(\"Temp\").innerHTML =this.responseText;\n";
  //  ptr += "tempData1 = parseFloat(this.responseText);}\n";
  //  ptr += "};\n";
  //  ptr += "xhttp.open(\"GET\", \"/getTemp\", true);\n";
  //  ptr += "xhttp.send();\n";
  //  ptr += "}\n";
  //  ptr += "</script>\n";
  //
  //  ptr += "</head>\n";
  //
  //  ptr += "<body>\n";
  //  ptr += "<div id=\"webpage\">\n";
  //  ptr += "<h1>Arduino Oven Thermometer</h1>\n";
  //  ptr += "<p>Temperature: <span id=\"Temp\">0</span> &deg;F</p>";
  //  //ptr += temperature;
  //  //ptr += "&deg;F</p>";
  //  ptr += "</div>\n";
  //  ptr += "<div id='chart'><!-- Plotly chart will be drawn inside this DIV --></div>";
  //
  //  ptr += "<script>\n";
  //
  //
  ////  ptr += "var trace1 = {\n";
  ////  ptr += "x: [";
  ////  ptr += timeData;
  ////  ptr += "],\n";
  ////  ptr += "y: [";
  ////  ptr += tempData;
  ////  ptr += "],\n";
  ////  ptr += "type: 'scatter'};\n";
  ////  ptr += "Plotly.newPlot('myDiv', [trace1]);\n";
  //
  //  ptr += "function getData(){return parseInt(Temp);}\n";
  //
  //  ptr +="Plotly.plot('chart',[{\n";
  //  ptr +="y:[tempData1],\n";
  //  ptr +="type:'line'\n";
  //  ptr +="}]);\n";
  //
  //  ptr +="var cnt = 0;\n";
  //
  //  ptr += "setInterval(function(){\n";
  //  ptr += "Plotly.extendTraces('chart',{ y:[[tempData1]]}, [0]);\n";
  //  ptr += "cnt++;\n";
  //  ptr += "if(cnt>30) { \n";
  //  ptr += "Plotly.relayout('chart',{\n";
  //  ptr += "xaxis: {\n";
  //  ptr += "range: [cnt-30, cnt]\n";
  //  ptr += "}});}},1000);\n";
  //
  //
  //  ptr += "</script>\n";
  //
  //
  //
  //  ptr += "</body>\n";
  //  ptr += "</html>\n";

  //String html = "<!DOCTYPE html> <html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"> <title>Oven Thermometer</title> <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} p {font-size: 24px;color: #444444;margin-bottom: 10px;} </style> <script src='https://cdn.plot.ly/plotly-latest.min.js'></script> </head> <body> <div id=\"webpage\"> <h1>Arduino Oven Thermometer</h1> <p>Temperature: <span id=\"Temp\">0</span> &deg;F</p> </div> <div id='chart'><!-- Plotly chart will be drawn inside this DIV --></div> <p>Display Length: <input type=\"number\" id=\"dispLength\" value=\"60\"> sec <button onclick=\"doButton()\">Set</button></p> <form action=\"/action_page\"> First name:<br> <input type=\"text\" name=\"firstname\" value=\"Mickey\"> <br> Last name:<br> <input type=\"text\" name=\"lastname\" value=\"Mouse\"> <br><br> <input type=\"submit\" value=\"Submit\"> </form> <script> var tempData1 = 0; var cnt = 0; var i; var start = 0; var initialData =\"\"; var num = 0; var plotLength = 60; var currentTime = 0; function getData() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { initialData = this.responseText; startPlot(); } }; xhttp.open(\"GET\", \"/getInitData\", true); xhttp.send(); } /*function getTime() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { currentTime = parseInt(this.responseText); } }; xhttp.open(\"GET\", \"/getTime\", true); xhttp.send(); }*/ function startPlot() { for(i = 0; i < initialData.length; i++) { if(initialData.charAt(i) == ' ') { num = parseFloat(initialData.slice(start, i-1)); start = i+1; Plotly.extendTraces('chart',{x:[[cnt]], y:[[num]]}, [0]); cnt = cnt + 5; if(cnt>plotLength) { Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } } } tempData1 = num; } getData(); function doButton() { plotLength = document.getElementById(\"dispLength\").value; /*getData();*/ Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } setInterval(loadDoc,1000); function loadDoc() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"Temp\").innerHTML =this.responseText; tempData1 = parseFloat(this.responseText); } }; xhttp.open(\"GET\", \"/getTemp\", true); xhttp.send(); } Plotly.plot('chart',[{y:[tempData1], type:'line'}]); setInterval(function() { Plotly.extendTraces('chart',{ y:[[tempData1]]}, [0]); cnt++; if(cnt>plotLength) { Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } },1000); </script> </body> </html>";

  //String html ="<!DOCTYPE html> <html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"> <title>Oven Thermometer</title> <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} p {font-size: 24px;color: #444444;margin-bottom: 10px;} </style> <script src='https://cdn.plot.ly/plotly-latest.min.js'></script> </head> <body> <div id=\"webpage\"> <h1>Arduino Oven Thermometer</h1> <p>Probe 1: <span id=\"Temp1\">0</span> &deg;F</p> <p>Probe 2: <span id=\"Temp2\">0</span> &deg;F</p> </div> <div id='chart'><!-- Plotly chart will be drawn inside this DIV --></div> <p>Display Length: <input type=\"number\" id=\"dispLength\" value=\"60\"> sec <button onclick=\"doButton()\">Set</button></p> <form action=\"/action_page\"> First name:<br> <input type=\"text\" name=\"firstname\" value=\"Mickey\"> <br> Last name:<br> <input type=\"text\" name=\"lastname\" value=\"Mouse\"> <br><br> <input type=\"submit\" value=\"Submit\"> </form> <script> var tempData1 = 0; var cnt = 0; var i; var start = 0; var initialData =\"\"; var num = 0; var plotLength = 60; var currentTime = 0; function getData() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { initialData = this.responseText; startPlot(); } }; xhttp.open(\"GET\", \"/getInitData\", true); xhttp.send(); } /*function getTime() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { currentTime = parseInt(this.responseText); } }; xhttp.open(\"GET\", \"/getTime\", true); xhttp.send(); }*/ function startPlot() { for(i = 0; i < initialData.length; i++) { if(initialData.charAt(i) == ' ') { num = parseFloat(initialData.slice(start, i-1)); start = i+1; Plotly.extendTraces('chart',{x:[[cnt]], y:[[num]]}, [0]); cnt = cnt + 5; if(cnt>plotLength) { Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } } } tempData1 = num; } getData(); function doButton() { plotLength = document.getElementById(\"dispLength\").value; /*getData();*/ Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } setInterval(loadDoc,1000); function loadDoc() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"Temp1\").innerHTML =this.responseText; document.getElementById(\"Temp2\").innerHTML =this.responseText; tempData1 = parseFloat(this.responseText); } }; xhttp.open(\"GET\", \"/getTemp\", true); xhttp.send(); } Plotly.plot('chart',[{y:[tempData1], type:'line'}]); setInterval(function() { Plotly.extendTraces('chart',{ y:[[tempData1]]}, [0]); cnt++; if(cnt>plotLength) { Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } },1000); </script> </body> </html>";

String html ="<!DOCTYPE html> <html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\"> <title>Oven Thermometer</title> <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} p {font-size: 24px;color: #444444;margin-bottom: 10px;} </style> <script src='https://cdn.plot.ly/plotly-latest.min.js'></script> </head> <body> <div id=\"webpage\"> <h1>Arduino Oven Thermometer</h1> <p>Probe 1: <span id=\"Temp1\">0</span> &deg;F</p> <p>Probe 2: <span id=\"Temp2\">0</span> &deg;F</p> </div> <div id='chart'><!-- Plotly chart will be drawn inside this DIV --></div> <p>Display Length: <input type=\"number\" id=\"dispLength\" value=\"60\"> sec <button onclick=\"doButton()\">Set</button></p> <script> var tempData1 = 0; var tempData2 = 0; var cnt = 0; var i; var start = 0; var initialData =\"\"; var num = 0; var plotLength = 60; var currentTime = 0; /*var trace1 = {x:[[cnt]], y:[[num]], name: 'Probe 1'}; var trace2 = {x:[[cnt]], y:[[num]], name: 'Probe 2'}; data = [ trace1, trace2 ];*/ function getData() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { initialData = this.responseText; startPlot(); } }; xhttp.open(\"GET\", \"/getInitData\", true); xhttp.send(); } /*function getTime() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { currentTime = parseInt(this.responseText); } }; xhttp.open(\"GET\", \"/getTime\", true); xhttp.send(); }*/ function startPlot() { for(i = 0; i < initialData.length; i++) { if(initialData.charAt(i) == ' ') { num = parseFloat(initialData.slice(start, i-1)); start = i+1; Plotly.extendTraces('chart',{x:[[cnt]], y:[[num]]}, [0]); cnt = cnt + 5; if(cnt>plotLength) { Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } } } tempData1 = num; } getData(); function doButton() { plotLength = document.getElementById(\"dispLength\").value; /*getData();*/ Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } setInterval(loadDoc,1000); function loadDoc() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"Temp1\").innerHTML =this.responseText; tempData1 = parseFloat(this.responseText); } }; xhttp.open(\"GET\", \"/getTemp\", true); xhttp.send(); } setInterval(loadDoc2,1000); function loadDoc2() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { document.getElementById(\"Temp2\").innerHTML =this.responseText; tempData2 = parseFloat(this.responseText); } }; xhttp.open(\"GET\", \"/getTemp2\", true); xhttp.send(); } /*Plotly.plot('chart',[{y:[tempData1], type:'line', name: 'Probe1'}]); setInterval(function() { Plotly.extendTraces('chart', {y: [[tempData1]]}, [0]); cnt++; if(cnt>plotLength) { Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } },1000);*/ Plotly.plot('chart', [{ y: [0], mode: 'lines+markers', marker: {color: 'blue', size: 8}, line: {width: 4}, name: 'Probe 1' }, { y: [0], mode: 'lines+markers', marker: {color: 'red', size:8}, line: {width: 4}, name: 'Probe 2' }]); var cnt = 0; setInterval(function() { Plotly.extendTraces('chart', { y: [[tempData1], [tempData2]] }, [0, 1]); cnt = cnt+1; if(cnt>plotLength) { Plotly.relayout('chart',{xaxis: {range: [cnt-plotLength, cnt]}}); } }, 1000); </script> </body> </html>";










































  return html;
}
