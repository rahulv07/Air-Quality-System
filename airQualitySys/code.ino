#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "bsec.h"
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

IPAddress local_IP(192,168,4,22);
IPAddress gateway(192,168,4,9);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

const unsigned char happy [] PROGMEM = {
  0x03, 0xfc, 0x00, 0x07, 0x0e, 0x00, 0x0c, 0x03, 0x00, 0x18, 0x01, 0x80, 0x31, 0x98, 0xc0, 0x31, 
  0x98, 0x40, 0x21, 0x98, 0x40, 0x21, 0x98, 0x40, 0x20, 0x00, 0x40, 0x2c, 0x03, 0x40, 0x3c, 0x03, 
  0x40, 0x36, 0x06, 0xc0, 0x1b, 0xfd, 0x80, 0x0d, 0xfb, 0x00, 0x06, 0x06, 0x00, 0x03, 0xfc, 0x00
};

const unsigned char mask[] PROGMEM = {
  0x03, 0xfc, 0x00, 0x06, 0x06, 0x00, 0x0c, 0x03, 0x00, 0x18, 0x01, 0x80, 0x00, 0x00, 0x00, 0x13, 
  0x9c, 0x80, 0x38, 0x01, 0xc0, 0x28, 0x01, 0x40, 0x2f, 0xff, 0x40, 0x2f, 0xff, 0x40, 0x2f, 0xff, 
  0x40, 0x3f, 0xff, 0xc0, 0x1f, 0xff, 0x80, 0x0f, 0xff, 0x00, 0x07, 0xfe, 0x00, 0x03, 0xfc, 0x00
};

const unsigned char tree [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x01, 0xf8, 0x00, 0x01, 
  0xf8, 0x00, 0x03, 0xfc, 0x00, 0x07, 0xfe, 0x00, 0x03, 0xfc, 0x00, 0x07, 0xfe, 0x00, 0x0f, 0xff, 
  0x00, 0x07, 0xfe, 0x00, 0x0f, 0xff, 0x00, 0x1f, 0xff, 0x80, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00
};




 
//char auth[] = "sfedhTry_1oamaquCCLWqDXGJ-hVq9fD";    
//char ssid[] = "vivo1951";                          
//char pass[] = "naveen12164";

#define SEALEVELPRESSURE_HPA (1013.25)
 
Bsec iaqSensor;
String output;
void checkIaqSensorStatus(void);
void errLeds(void);
float temperature;
float humidity;
float pressure;
float IAQ;
float carbon;
float VOC;
const char* IAQsts;


void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Wire.begin();
  //Blynk.begin(auth, ssid, pass);

  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP("ESPsoftAP_01","sudharsan") ? "Ready" : "Failed!");

  delay(100);

  
  server.on("/", handle_OnConnect);
//  server.on("/servoon", handle_servoon);
//  server.on("/servooff", handle_servooff);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");

  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
   
  Serial.println(F("Starting..."));
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
 
  Serial.println("OLED begun");
 
  display.display();
  delay(100);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(0);
 

  
 
  iaqSensor.begin(BME680_I2C_ADDR_PRIMARY, Wire);
  output = "\nBSEC library version " + String(iaqSensor.version.major) + "." + String(iaqSensor.version.minor) + "." + String(iaqSensor.version.major_bugfix) + "." + String(iaqSensor.version.minor_bugfix);
  Serial.println(output);
  checkIaqSensorStatus();
 
  bsec_virtual_sensor_t sensorList[10] =
  {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
  };
 
  iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);
  checkIaqSensorStatus();
}
 

 

void loop(void)
{
  server.handleClient();
  display.setCursor (0,0);
  display.clearDisplay();
  
  unsigned long time_trigger = millis();
  if (iaqSensor.run())
  {
    output = String(time_trigger);
    output += ", " + String(iaqSensor.rawTemperature);
    output += ", " + String(iaqSensor.pressure);
    output += ", " + String(iaqSensor.rawHumidity);
    output += ", " + String(iaqSensor.gasResistance);
    output += ", " + String(iaqSensor.iaq);
    output += ", " + String(iaqSensor.iaqAccuracy);
    output += ", " + String(iaqSensor.temperature);
    output += ", " + String(iaqSensor.humidity);
    output += ", " + String(iaqSensor.staticIaq);
    output += ", " + String(iaqSensor.co2Equivalent);
    output += ", " + String(iaqSensor.breathVocEquivalent);
    Serial.println(output);
 
    Serial.print("Temperature = "); 
    Serial.print(iaqSensor.temperature); 
    Serial.println(" *C");
    display.print("Temperature: "); 
    display.print(iaqSensor.temperature); 
    display.println(" *C");
 
    Serial.print("Pressure = "); 
    Serial.print(iaqSensor.pressure / 100.0); 
    Serial.println(" hPa");
    display.print("Pressure: "); 
    display.print(iaqSensor.pressure / 100); 
    display.println(" hPa");
 
    Serial.print("Humidity = "); 
    Serial.print(iaqSensor.humidity); 
    Serial.println(" %");
    display.print("Humidity: "); 
    display.print(iaqSensor.humidity); 
    display.println(" %");
 
    Serial.print("IAQ = "); 
    Serial.print(iaqSensor.iaq); 
    Serial.println(" PPM");
    display.print("IAQ: "); 
    display.print(iaqSensor.iaq); 
    display.println(" PPM");
 
    Serial.print("CO2 equiv = "); 
    Serial.print(iaqSensor.co2Equivalent); 
    Serial.println(" PPM");
    display.print("CO2eq: "); 
    display.print(iaqSensor.co2Equivalent); 
    display.println(" PPM");
 
    Serial.print("Breath VOC = "); 
    Serial.print(iaqSensor.breathVocEquivalent); 
    Serial.println(" PPM");
    display.print("VOC: "); 
    display.print(iaqSensor.breathVocEquivalent); 
    display.println(" PPM");
    

    if ((iaqSensor.iaq > 0)  && (iaqSensor.iaq  <= 50)) {
    IAQsts = "Excellent";
    Serial.print("IAQ:Excellent");
    display.print("IAQ:Excellent");
    display.drawBitmap(109,26, happy , 20, 16, WHITE);
    }
   

    if ((iaqSensor.iaq > 50)  && (iaqSensor.iaq  <= 100)) {
    IAQsts = "Good";
    Serial.print("IAQ: Good");
    display.print("IAQ: Good");
    display.drawBitmap(109,26, happy , 20, 16, WHITE);
  }
    if ((iaqSensor.iaq > 100)  && (iaqSensor.iaq  <= 150)) {
    IAQsts = "Lightly polluted";
    Serial.print("IAQ: Lightly polluted");
    display.print("IAQ: Lightly polluted");
    display.drawBitmap(110,26, mask , 20, 16, WHITE);
  }
    if ((iaqSensor.iaq > 150)  && (iaqSensor.iaq  <= 200)) {
    IAQsts = "Moderately polluted";
    Serial.print("IAQ: Moderately polluted");
    display.print("IAQ: Moderately polluted");
    display.drawBitmap(111,26, tree , 20, 16, WHITE);
  }
    if ((iaqSensor.iaq > 200)  && (iaqSensor.iaq  <= 250)) {
    IAQsts = "Heavily polluted";
    Serial.print("IAQ: Heavily polluted");
    display.print("IAQ: Heavily polluted");
    display.drawBitmap(111,26, tree , 20, 16, WHITE);
  }
    if ((iaqSensor.iaq > 250)  && (iaqSensor.iaq  <= 350)) {
    IAQsts = "Severely polluted";
    Serial.print("IAQ: Severely polluted");
    display.print("IAQ: Severely polluted");
    display.drawBitmap(111,26, tree , 20, 16, WHITE);
  }
    if ((iaqSensor.iaq > 350)){
    IAQsts = "Extremely polluted";
    Serial.print("IAQ: Extremely polluted");
    display.print("IAQ: Extremely polluted");
    display.drawBitmap(111,26, tree , 20, 16, WHITE);
  }
    Serial.println();
    display.display();
    delay(2000);
 
//    Blynk.run(); // Initiates Blynk
//    Blynk.virtualWrite(V1, iaqSensor.temperature);        
//    Blynk.virtualWrite(V2, (iaqSensor.pressure)/100.0);            
//    Blynk.virtualWrite(V3, iaqSensor.humidity);               
//    Blynk.virtualWrite(V4, iaqSensor.iaq);                    
//    Blynk.virtualWrite(V5, iaqSensor.co2Equivalent);         
//    Blynk.virtualWrite(V6, iaqSensor.breathVocEquivalent);   
  }
  else{
      checkIaqSensorStatus();
    }
}

void handle_OnConnect() {
  Serial.println("Webpage opened");
  server.send(200, "text/html", SendHTML()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String SendHTML(){
  String ptr = "<html>\n";
  ptr+="<head>\n";
  ptr+="<title>Webserver</title>\n";
  ptr+="<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'>\n";
  ptr+="<style>*{padding: 0;margin: 0;box-sizing: border-box;}\n";
  ptr+="body {    background-color: lavender;    font-family: \"Poppins\", sans-serif;    font-size: 20px;    display: flex;    align-items: center;    justify-content:center;    min-height: 100vh;}\n";
  ptr+=".page{    margin-bottom: 10px;    padding-bottom: 20px;}\n";
  ptr+="p{    color: #101010;    margin-bottom: 5px;    display: flex;    justify-content: space-between;}\n";
  ptr+="span {    background-color: white;    border: 3px solid white;    border-radius:5px;    margin-left:30px;    border-right-width: 100px;}\n";
  ptr+="</style>\n";
  ptr+="</head>\n";
  ptr+="<body>\n";
  ptr+="<div class=\"container\">\n";
  ptr+="<div class=\"page\">        <!--For Temperature-->        <p>Temperature            <!-- Add integer to display -->        <span>"+String(iaqSensor.temperature)+" &deg;C</span>        </p>    </div>\n";
  ptr+="<div class=\"page\">        <!--For Pressure-->        <p>Pressure            <!-- Add integer to display -->        <span>"+String(iaqSensor.pressure / 100.0)+" hPa</span>        </p>    </div>\n";
  ptr+="<div class=\"page\">        <!--For Humidity-->        <p>Humidity            <!-- Add integer to display -->        <span>"+String(iaqSensor.humidity)+" %</span>        </p>    </div>\n";
  ptr+="<div class=\"page\">        <!--For IAQ-->        <p>IAQ            <!-- Add integer to display -->        <span>"+String(iaqSensor.iaq)+" PPM</span>        </p>    </div>\n";
  ptr+="<div class=\"page\">        <!--For Co2 Equivalent-->        <p>Co2 Equivalent            <!-- Add integer to display -->        <span>"+String(iaqSensor.co2Equivalent)+" PPM</span>        </p>    </div>\n";
  ptr+="<div class=\"page\">        <!--For Breath VOC-->        <p>Breath VOC            <!-- Add integer to display -->        <span>"+String(iaqSensor.breathVocEquivalent)+" PPM</span></p>    </div>\n";
  ptr+="</div>\n";
  ptr+="<script>\n";
  ptr+="setInterval(loadDoc,2000)\n";
  ptr+="function loadDoc() {\n";
  ptr+="var xhttp = new XMLHttpRequest();\n";
  ptr+="xhttp.onreadystatechange = function() {\n";
  ptr+="if (this.readyState == 4 && this.status == 200) {\n";
  ptr+="document.body.innerHTML =this.responseText\n";
  ptr+="};\n";
  ptr+="}\n";
  ptr+="xhttp.open(\"GET\", \"/\", true)\n";
  ptr+="xhttp.send();\n";
  ptr+="}\n";
  ptr+="</script>\n";
  ptr+="</body>\n";
  ptr+="</html>\n";
  
  return ptr;
 }

void checkIaqSensorStatus(void)
{
  if (iaqSensor.status != BSEC_OK) {
    if (iaqSensor.status < BSEC_OK) {
      output = "BSEC error code : " + String(iaqSensor.status);
      Serial.println(output);
      display.setCursor(0,0);
      display.println(output);
      display.display();
      for (;;) delay(1000); 
        errLeds();
    } else {
      output = "BSEC warning code : " + String(iaqSensor.status);
      Serial.println(output);
    }
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    if (iaqSensor.bme680Status < BME680_OK) {
      output = "BME680 error code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
      display.setCursor(0,0);
      display.println(output);
      display.display();
      for (;;) delay(1000); 
        errLeds(); 
    } else {
      output = "BME680 warning code : " + String(iaqSensor.bme680Status);
      Serial.println(output);
    }
  }
}
 
void errLeds(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
