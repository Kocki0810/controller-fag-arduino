#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>  //Ticker Library
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

#ifdef DEBUG 
#include "GDBStub.h"
#endif

Ticker blinker1;
Ticker blinker2;
const char* ssid = "FTTH_JU3332";
const char* password = "lottePink72!";
ESP8266WebServer server(80);
static const uint8_t D0 = 16;
static const uint8_t D1 = 5;
static const uint8_t D2 = 4;
static const uint8_t D3 = 0;
static const uint8_t D4 = 2;
static const uint8_t D5 = 14;
static const uint8_t D6 = 12;
static const uint8_t D7 = 13;
static const uint8_t D8 = 15;
static const uint8_t D9 = 3;
static const uint8_t D10 = 1;

enum LedState
{
    OFF = 1,
    ON = 2,
    BLINKING = 3
};

LedState LED1status = OFF;
LedState LED2status = OFF;
bool timer1Running = false;
bool timer2Running = false;

void setup() {
#ifdef DEBUG    gdbstub_init();
    // Add/extend the below delay if you want to debug the setup code
    delay(3000);
#endif
    Serial.begin(115200);
    delay(100);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);

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
    }
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
}

void handle_OnConnect() {
    LED1status = OFF;
    LED2status = OFF;
    Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
    server.send(200, "text/html", SendHTML(LED1status, LED2status));
}

void handle_led1on() {
    LED1status = ON;
    Serial.println("GPIO7 Status: ON");
    server.send(200, "text/html", SendHTML(true, LED2status));
}

void handle_led1off() {
    LED1status = OFF;
    Serial.println("GPIO7 Status: OFF");
    server.send(200, "text/html", SendHTML(false, LED2status));
}

void handle_led1blinking() {
    LED1status = BLINKING;
    Serial.println("GPIO7 Status: BLINKING");
    server.send(200, "text/html", SendHTML(true, LED1status));
}

void handle_led2on() {
    LED2status = ON;
    Serial.println("GPIO6 Status: ON");
    server.send(200, "text/html", SendHTML(LED1status, true));
}

void handle_led2off() {
    LED2status = OFF;
    Serial.println("GPIO6 Status: OFF");
    server.send(200, "text/html", SendHTML(LED1status, false));
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
    <p>LED2 State: Click to switch</p><span onclick="switchLed(2)" id="pin2Text" class="button button-on">ON</span><span style="display:none;" id="pin2State">0</span>
</body>
<script>
    function switchLed(pin) {
        var text = $("#pin" + pin + "Text")[0].innerHTML;
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
</script>
</html>
    )V0G0N";
    
#pragma endregion
    return html;
}