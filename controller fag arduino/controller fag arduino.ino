#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>  //Ticker Library
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <string>

#ifdef VM_DEBUG_GDB 
#include "GDBStub.h"
#endif
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Ticker blinker1;
Ticker blinker2;
const char* ssid = "NodeMCU_Station_Mode";
const char* password = "h5pd091121_Styrer";
ESP8266WebServer server(80);

volatile bool timer1Running = false;
volatile bool timer2Running = false;

volatile int blinkIterator1 = 0;
volatile int blinkIterator2 = 0;

volatile bool BlinkFlag1 = false;
volatile bool BlinkFlag2 = false;

volatile int interruptCounter = 0;
String LEDStrength1;
String LEDStrength2;

enum LedState
{
    OFF = 1,
    ON = 2,
    BLINKING = 3,
    DIMMED = 4
};

volatile LedState LED1status = OFF;
volatile LedState LED2status = OFF;

void externalInterruptDisplay()
{
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.print("Pin 1 Blinked: ");
    display.println(blinkIterator1);

    display.setCursor(0, 10);             // Start at top-left corner
    display.print("Pin 2 Blinked: ");
    display.println(blinkIterator2);
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 20);             // Start at top-left corner
    display.print("External interrupts: ");
    display.println(interruptCounter);
    display.display();
}

void ICACHE_RAM_ATTR ButtonPressedInterrupt()
{
    interruptCounter++;
    externalInterruptDisplay();

}

void setup() {
#ifdef VM_DEBUG_GDB gdbstub_init();
    // Add/extend the below delay if you want to debug the setup code
    delay(3000);
#endif
    Serial.begin(115200);
    delay(100);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    pinMode(D0, INPUT_PULLUP);

    analogWriteRange(1023);

    Serial.println("Connecting to ");
    Serial.println(ssid);

    //connect to your local wi-fi network
    WiFi.begin(ssid, password);

    //check wi-fi is connected to wi-fi network
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

    server.on("/", handle_OnConnect);
    server.on("/led1on", handle_led1on);
    server.on("/led1off", handle_led1off);
    server.on("/led1blinking", handle_led1blinking);

    server.on("/led2on", handle_led2on);
    server.on("/led2off", handle_led2off);
    server.on("/led2blinking", handle_led2blinking);
    server.onNotFound(handle_NotFound);

    server.begin();
    Serial.println("HTTP server started");
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();
    attachInterrupt(D0, ButtonPressedInterrupt, FALLING);
    
}
void loop() {
    server.handleClient();
    if (LED1status == OFF)
    {
        digitalWrite(D6, LOW);
        DetachTimer(1);
    }
    else if(LED1status == ON)
    {
        digitalWrite(D6, HIGH);
        DetachTimer(1);
    }
    else if (LED1status == BLINKING)
    {
        AttachTimer(D6);
        CheckIteratorFlag(D6);
    }
    else if (LED1status == DIMMED)
    {
        analogWrite(D6, LEDStrength1.toInt());
        DetachTimer(1);
    }

    if (LED2status == OFF)
    {
        digitalWrite(D7, LOW);
        DetachTimer(2);
    }
    else if (LED2status == ON)
    {
        digitalWrite(D7, HIGH);
        DetachTimer(2);
    }
    else if (LED2status == BLINKING)
    {
        AttachTimer(D7);
        CheckIteratorFlag(D7);
    }
    else if (LED2status == DIMMED)
    {
        analogWrite(D7, LEDStrength2.toInt());
        DetachTimer(2);
    }
   
}


void CheckIteratorFlag(uint16_t pin)
{
    if(pin == D6)
    {
        if (BlinkFlag1)
        {
            blinkIterator1++;
            BlinkFlag1 = false;
            Serial.println(blinkIterator1);
        }
    }
    else if(pin == D7)
    {
        if (BlinkFlag2)
        {
            blinkIterator2++;
            BlinkFlag2 = false;
            Serial.println(blinkIterator2);
        }
    }
    display.clearDisplay();
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.print("Pin 1 Blinked: ");
    display.println(blinkIterator1);

    display.setCursor(0, 10);             // Start at top-left corner
    display.print("Pin 2 Blinked: ");
    display.println(blinkIterator2);
    display.setCursor(0, 20);             // Start at top-left corner
    display.print("External interrupts: ");
    display.println(interruptCounter);
    display.display();

}


void AttachTimer(uint8_t pin)
{
    if (pin == D6)
    {
        if (!timer1Running)
        {
            blinker1.attach_ms(1000, TimerPin1, pin);
            timer1Running = true;
        }
    }
    else if (pin == D7)
    {
        if (!timer2Running)
        {
            blinker2.attach_ms(1000, TimerPin1, pin);
            timer2Running = true;
        }
    }
}
void DetachTimer(int timer)
{
    if (timer == 1)
    {
        if (timer1Running)
        {
            blinker1.detach();
            timer1Running = false;
        }
    }
    else if(timer == 2)
    {
        if (timer2Running)
        {
            blinker2.detach();
            timer2Running = false;
        }
    }
}

void TimerPin1(uint8_t pin) {
    digitalWrite(pin, !(digitalRead(pin)));  //Toggle LED Pin
    if (pin == D6)
    {
        BlinkFlag1 = true;   
    }
    else if (pin == D7)
    {
        BlinkFlag2 = true;
    }
}

void handle_OnConnect() {
    LED1status = OFF;
    LED2status = OFF;
    Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
    server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led1on() {
    if (server.arg("Strength") != "")
    {
        LED1status = DIMMED;
        LEDStrength1 = server.arg("Strength");
    }
    else
    {
        LED1status = ON;
        Serial.println("GPIO7 Status: ON");
    }
    server.send(200, "text/html", SendHTML(true, LED1status));
}

void handle_led1off() {
    LED1status = OFF;
    Serial.println("GPIO7 Status: OFF");
    server.send(200, "text/html", SendHTML(false, LED1status));
}

void handle_led1blinking() {
    LED1status = BLINKING;
    Serial.println("GPIO7 Status: BLINKING");
    server.send(200, "text/html", SendHTML(true, LED1status));
}

void handle_led2on() {
    if (server.arg("Strength") != "")
    {
        LED2status = DIMMED;
        LEDStrength2 = server.arg("Strength");
    }
    else
    {
        LED2status = ON;
        Serial.println("GPIO6 Status: ON");

    }
    server.send(200, "text/html", SendHTML(LED2status, true));
}

void handle_led2off() {
    LED2status = OFF;
    Serial.println("GPIO6 Status: OFF");
    server.send(200, "text/html", SendHTML(LED2status, false));
}

void handle_led2blinking() {
    LED2status = BLINKING;
    Serial.println("GPIO7 Status: BLINKING");
    server.send(200, "text/html", SendHTML(true, LED2status));
}

void handle_NotFound() {
    server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat, uint8_t led2stat) {
    #pragma region html


    String html = R"V0G0N(
<html>
<head>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script>

    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>LED Control</title>
    <style>
        html {
            font-family: Helvetica;
            display: inline-block;
            margin: 0px auto;
            text-align: center;
        }

        body {
            margin-top: 50px;
        }

        h1 {
            color: #444444;
            margin: 50px auto 30px;
        }

        h3 {
            color: #444444;
            margin-bottom: 50px;
        }

        .button {
            display: block;
            width: 110px;
            background-color: #1abc9c;
            border: none;
            color: white;
            padding: 13px 30px;
            text-decoration: none;
            font-size: 25px;
            margin: 0px auto 35px;
            cursor: pointer;
            border-radius: 4px;
        }

        .button-on {
            background-color: #1abc9c;
        }

            .button-on:active {
                background-color: #16a085;
            }

        .button-off {
            background-color: #34495e;
        }

            .button-off:active {
                background-color: #2c3e50;
            }

        p {
            font-size: 14px;
            color: #888;
            margin-bottom: 10px;
        }
    </style>
</head>
<body>
    <h1>ESP8266 Web Server</h1>
    <h3>Using Station(STA) Mode</h3>
    <p>LED1 State: Click to switch</p><span onclick="switchLed(1)" id="pin1Text" class="button button-on">ON</span><span style="display:none;" id="pin1State">0</span>
    <div id="led1StrengthPicker">
        <input onclick="LedStrength(1)" type="range" id="pin1Strength" value="0" max="1023">
        <p id="rangeValue1">0</p>
    </div>
    
    <p>LED2 State: Click to switch</p><span onclick="switchLed(2)" id="pin2Text" class="button button-on">ON</span><span style="display:none;" id="pin2State">0</span>
    <div id="led2StrengthPicker">
        <input onclick="LedStrength(2)" type="range" id="pin2Strength" value="0" max="1023">
        <p id="rangeValue2">0</p>
    </div>
</body>
<script>
    function switchLed(pin) {
        var state = $("#pin" + pin + "State")[0].innerHTML;
        var stateText = ["off", "on", "blinking"];

        if (state == 2) {
            $("#pin" + pin + "State")[0].innerText = 0;
            state = 0;
        }
        else {
            $("#pin" + pin + "State")[0].innerText++;
            state++;
        }
        if (state == 1) {
            $("#led" + pin + "StrengthPicker").show();
        }
        else {
            $("#led" + pin + "StrengthPicker").hide();
        }

        $.ajax({

            url: '/led' + pin + stateText[state],
            type: 'GET',
            success: function (data) {
                $("#pin" + pin + "Text")[0].innerHTML = stateText[state].toUpperCase();

            },
            error: function (request, error) {
                //alert("Request: " + JSON.stringify(request));
            },
        });

    }
    function LedStrength(pin) {
        var ledStrength = $("#pin"+pin+"Strength")[0].value;
        $("#rangeValue" + pin)[0].innerHTML = ledStrength;

        $.ajax({

            url: '/led' + pin + "on?Strength="+ledStrength,
            type: 'GET',
            success: function (data) {
            },
            error: function (request, error) {
                //alert("Request: " + JSON.stringify(request));
            },
        });
    }
</script>
</html>
    )V0G0N";
    
#pragma endregion
    return html;
}