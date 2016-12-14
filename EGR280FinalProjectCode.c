#include <PS3BT.h>
#include <usbhub.h>
#include <Servo.h>
//#include <NewPing.h>


#define IN_DEBUG

Servo direcServo;
Servo escServo;

int frontConstLEDS = 7;
int backConstLEDS = 8;
int blinkerL = A4;
int blinkerR = A2;

int buzzerPin = 5;
int pitchLow = 200;
int pitchHigh = 1000;
int pitchStep = 10;
int currentPitch;
int delayTime;


const int trigPin = 4;
const int echoPin = 2;
long duration, distance;



// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside

BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so
/* You can create the instance of the class in two ways */
PS3BT PS3(&Btd); // This will just create the instance
//PS3BT PS3(&Btd, 0x00, 0x15, 0x83, 0x3D, 0x0A, 0x57); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch

bool printTemperature, printAngle;

void distSensor(int rightY)
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    
    duration = pulseIn(echoPin, HIGH);
    
    distance = ((duration / 2) / 29.1);
    
    if (distance < 25)
    {
        speed(150);
    }
    
    else
    {
        speed(rightY);
    }
    
    /*
     if (distance >= 200 || distance <= 0)
     {
     Serial.println("Out of Range");
     }
     
     else
     {
     Serial.print(distance);
     Serial.println(" cm");
     }
     */
    
}

void horn()
{
    tone(buzzerPin, currentPitch, 10);
    currentPitch += pitchStep;
    if(currentPitch >= pitchHigh)
    {
        pitchStep = -pitchStep;
    }
    else if(currentPitch <= pitchLow)
    {
        pitchStep = -pitchStep;
    }
    delay(delayTime);
}

int directionValue = 0;
void direction(int leftX)
{
    int direc;
    Serial.print(leftX);
    direc = leftX;
    direc = map(direc, 0, 255, 0, 179);
    
#ifdef IN_DEBUG
    Serial.print(F("\r\nDirection: "));
    Serial.print(direc);
#endif
    
    if (direc != directionValue)
    {
        direcServo.write(direc);
        directionValue = direc;
    }
    delay(15);
}

int speedValue = 0;
void speed(int rightY)
{
    int velocity;
    Serial.print(rightY);
    velocity = rightY;
    velocity = map(velocity, 255, 0, 0, 179);
#ifdef IN_DEBUG
    Serial.print(F("\r\nSpeed: "));
    Serial.print(velocity);
#endif
    if (velocity != speedValue)
    {
        Serial.print(F("\r\nSpeed: "));
        escServo.write(velocity);
        speedValue = velocity;
    }
    
    /*
     if(velocity >= 100 && velocity <= 150)
     {
     
     escServo.write(velocity = map(velocity, 0, 255, 0, 0));
     }
     else
     {
     escServo.write(velocity);
     }
     */
    delay(15);
}


class Flasher
{
    // Class Member Variables
    // These are initialized at startup
    int ledPin;      // the number of the LED pin
    long OnTime;     // milliseconds of on-time
    long OffTime;    // milliseconds of off-time
    
    // These maintain the current state
    int ledState;                 // ledState used to set the LED
    unsigned long previousMillis;   // will store last time LED was updated
    
    // Constructor - creates a Flasher
    // and initializes the member variables and state
public:
    Flasher(int pin, long on, long off)
    {
        ledPin = pin;
        pinMode(ledPin, OUTPUT);
        
        OnTime = on;
        OffTime = off;
        
        ledState = LOW;
        previousMillis = 0;
    }
    
    void Update()
    {
        // check to see if it's time to change the state of the LED
        unsigned long currentMillis = millis();
        
        if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
        {
            ledState = LOW;  // Turn it off
            previousMillis = currentMillis;  // Remember the time
            digitalWrite(ledPin, ledState);  // Update the actual LED
        }
        else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
        {
            ledState = HIGH;  // turn it on
            previousMillis = currentMillis;   // Remember the time
            digitalWrite(ledPin, ledState);   // Update the actual LED
        }
    }
};

void setup()
{
    Serial.begin(115200);
    
#if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
    if (Usb.Init() == -1)
    {
        Serial.print(F("\r\nOSC did not start"));
        while (1); //halt
    }
    Serial.print(F("\r\nPS3 Bluetooth Library Started"));
    
    direcServo.attach(9);
    escServo.attach(3);
    
    pinMode(frontConstLEDS, OUTPUT);
    pinMode(backConstLEDS, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    //pinMode(blinkerL, OUTPUT);
    //pinMode(blinkerR, OUTPUT);
}

void loop()
{
    Usb.Task();
    
    int leftX = PS3.getAnalogHat(LeftHatX);
    int leftY = PS3.getAnalogHat(LeftHatY);
    int rightX = PS3.getAnalogHat(RightHatX);
    int rightY = PS3.getAnalogHat(RightHatY);
    
    if (PS3.PS3Connected || PS3.PS3NavigationConnected)
    {
        
        if (leftX > 137 || leftX < 117) // || leftY > 137 || leftY < 117 || rightX > 137 || rightX < 117 || rightY > 137 || rightY < 117)
        {
            direction(leftX);
        }
        else
        {
            direction(128);
        }
        if (rightY > 137 || rightY < 117)
        {
#ifdef IN_DEBUG
            Serial.print(F("\tRightHatX: "));
            Serial.print(rightX);
            Serial.print(F("\tRightHatY: "));
            Serial.print(rightY);
#endif
            //speed(rightY);
            distSensor(rightY);
        }
        else
        {
            speed(128);
            
        }
    }
    
    
    // Analog button values can be read from almost all buttons
    if (PS3.getAnalogButton(L2) || PS3.getAnalogButton(R2))
    {
        Serial.print(F("\r\nL2: "));
        Serial.print(PS3.getAnalogButton(L2));
        if (PS3.PS3Connected)
        {
            Serial.print(F("\tR2: "));
            Serial.print(PS3.getAnalogButton(R2));
        }
    }
    
    if (PS3.getButtonClick(PS))
    {
        Serial.print(F("\r\nPS"));
        PS3.disconnect();
    }
    else
    {
        if (PS3.getButtonClick(TRIANGLE))
        {
            Serial.print(F("\r\nOFF"));
            digitalWrite(backConstLEDS, LOW);
            digitalWrite(frontConstLEDS, LOW);
            digitalWrite(blinkerL, LOW);
            digitalWrite(blinkerR, LOW);
            noTone(5);
            //speed(128);
            // PS3.setRumbleOn(RumbleLow); A RUMBLE FEATURE
        }
        if (PS3.getButtonClick(CIRCLE))
        {
            Serial.print(F("\r\nHead Lights"));
            digitalWrite(frontConstLEDS, HIGH);
            
            
            //PS3.setRumbleOn(RumbleHigh); A RUMBLE FEATURE
        }
        if (PS3.getButtonClick(CROSS))
        {
            Serial.print(F("\r\nHorn"));
            horn();
        }
        if (PS3.getButtonClick(SQUARE))
        {
            Serial.print(F("\r\nTail Lights"));
            digitalWrite(backConstLEDS, HIGH);
        }
        if (PS3.getButtonClick(UP))
        {
            Serial.print(F("\r\nUp"));
            if (PS3.PS3Connected) {
                PS3.setLedOff();
                PS3.setLedOn(LED4);
            }
        }
        if (PS3.getButtonClick(RIGHT))
        {
            Serial.print(F("\r\nRight"));
            if (PS3.PS3Connected) {
                PS3.setLedOff();
                PS3.setLedOn(LED1);
            }
        }
        if (PS3.getButtonClick(DOWN))
        {
            Serial.print(F("\r\nDown"));
            if (PS3.PS3Connected)
            {
                PS3.setLedOff();
                PS3.setLedOn(LED2);
            }
        }
        if (PS3.getButtonClick(LEFT))
        {
            Serial.print(F("\r\nLeft"));
            if (PS3.PS3Connected)
            {
                PS3.setLedOff();
                PS3.setLedOn(LED3);
            }
        }
        
        if (PS3.getButtonClick(L1))
        {
            Serial.print(F("\r\nL-Blinker"));
            Flasher ledL(blinkerL, 100, 400);
            ledL.Update();
        }
        if (PS3.getButtonClick(L3))
            Serial.print(F("\r\nL3"));
        if (PS3.getButtonClick(R1))
        {
            Serial.print(F("\r\nR-Blinker"));
            Flasher ledR(blinkerR, 350, 350);
            ledR.Update();
        }
        if (PS3.getButtonClick(R3))
        {
            Serial.print(F("\r\nR3"));
        }
        if (PS3.getButtonClick(SELECT)) 
        {
            Serial.print(F("\r\nSensor"));
            //PS3.printStatusString();
        }
        if (PS3.getButtonClick(START)) 
        {
            Serial.print(F("\r\nStart"));
            printAngle = !printAngle;
        }
    }
}
