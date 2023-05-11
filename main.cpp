#include <LCDWIKI_GUI.h>  // core graphics library
#include <LCDWIKI_SPI.h>  // hardware-specific library

// define parameters
#define MODEL SSD1283A
#define CS 10
#define CD 9
#define SDA 11
#define MOSI SDA
#define SCK 13
#define RST 8
#define LED -1

#define BUTTON1 4  // on/off button
#define BUTTON2 5  // corner 1
#define BUTTON3 6  // corner 2

LCDWIKI_SPI mylcd(MODEL,CS,CD, MISO, MOSI, RST, SCK, LED);

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define NAVY 0x000080
volatile int temp, counter = 0;  // increases/decreases depending on the rotation of encoder
float conversionFactor = 0.0066239475;  // converts encoder value to inches
float inches = 0.0;
float feet = 0.0;
float meters = 0.0;
int greenCounter = 0;  // monitors the green button
boolean locked = false;  // lock mode
int i = 0;

int angleArr[8] = {15, 30, 45, 60, 75, 90, 105, 120};  // different angle modes
int selectAngle = 0;  // (0-7)

void setup() {
    mylcd.Init_LCD();
    mylcd.Fill_Screen(BLACK);

    Serial.begin(9600);
    pinMode(2, INPUT_PULLUP);  // internal pull-up input pin 2
    pinMode(3, INPUT_PULLUP);  // internal pull-up input pin 3

    // set buttons as inputs
    pinMode(BUTTON1, INPUT);
    pinMode(BUTTON2, INPUT);
    pinMode(BUTTON3, INPUT);

    // set up interrupts
    // A rising pulse from encoder activated ai0(); AttachInterrupt 0 is DigitalPin nr 2 on Arduino
    attachInterrupt(0, ai0, RISING);
    // B rising pulse from encoder activated ai1(); AttachInterrupt 1 is DigitalPin nr 3 on Arduino
    attachInterrupt(1, ai1, RISING);

    // set start screen
    mylcd.Set_Text_colour(WHITE);
    mylcd.Set_Text_Back_colour(BLACK);
    mylcd.Set_Text_Size(2);
    mylcd.Print_String("Measurements", 0, 0);

    // set starting angle
    mylcd.Set_Text_Size(1);
    String degree = String(angleArr[selectAngle]);
    degree += " degree  ";
    mylcd.Print_String(degree, LEFT, 120);
    mylcd.Set_Text_Size(2);
    printMeasurements(0);
    Serial.begin(9600);
}

void loop() {
    // send counter value
    // increment counter when encoder is turned
    if (counter != temp && !locked) {
        printMeasurements(counter);  // update screen
        temp = counter;
    }

    // angleArr[8] = {15, 30, 45, 60, 75, 90, 105, 120};
    if (digitalRead(BUTTON1) == 1) {  // green
        greenCounter++;
        delay(10);
    }
    else {
        // if button is hit short
        if (greenCounter < 50 && greenCounter >=1) {
            selectAngle += 1;
            if (selectAngle == 8) {
                selectAngle = 0;
            }
            mylcd.Set_Text_Size(1);
            String degree = String(angleArr[selectAngle]);
            degree += " degree  ";
            mylcd.Print_String(degree, LEFT, 120);
            mylcd.Set_Text_Size (2);
            delay(30);
        }
        // if button is held down
        else if (greenCounter >= 50) {
            conversionFactor *= -1;
            counter *= -1;
            temp *= -1;
        }
        // reset counter after button is lifted
        greenCounter = 0;
    }

    // blue button adds the selected angle distance
    if (digitalRead(BUTTON2) == 1) {  // blue
        counter += 4.655 / (conversionFactor * tan (angleArr[selectAngle] * 0.0174533 / 2));
        temp += 4.655 / (conversionFactor * tan (angleArr[selectAngle] * 0.0174533 / 2));
        printMeasurements(counter);
    }

    // red button locks if pressed and resets if held down
    if (digitalRead(BUTTON3) == 1) {  // red
        if (!locked) {
            locked = true;
            mylcd.Set_Text_Size(1);
            mylcd.Print_String("locked", RIGHT, 120);
            String degree = String(angleArr[selectAngle]);
            degree += " degree  ";
            mylcd.Print_String(degree, LEFT, 120);
            mylcd.Set_Text_Size(2);
            delay(2000);
            if (digitalRead(BUTTON3) == 1) {
                counter = 0;
                temp = 0;
                printMeasurements(counter);
            }
        }
        else if (locked) {
            locked = false;
            mylcd.Set_Text_Size(1);
            mylcd.Print_String("            ", RIGHT, 120);
            String degree = String(angleArr[selectAngle]);
            degree += " degree  ";
            mylcd.Print_String(degree, LEFT, 120);
            mylcd.Set_Text_Size(2);
            Serial.print(locked);
            delay(700);
            if (digitalRead(BUTTON3) == 1) {
                counter = 0;
                temp = 0;
                printMeasurements(counter);
            }
        }
    }
}

void ai0() {
    // ai0 is activated if DigitalPin nr 2 is going from LOW to HIGH
    // checks pin 3 to determine direction
    if (!locked) {
        if (digitalRead(3) == LOW) {
            counter--;
        }
        else {
            counter++;
        }
    }
}

void ai1() {
    // ai0 is activated if DigitalPin nr 3 is going from LOW to HIGH
    // checks pin 2 to determine direction
    if (!locked) {
        if (digitalRead(2) == LOW) {
            counter++;
        }
        else {
            counter--;
        }
    }
}

// inputs counter and updates screen
void printMeasurements(double counter) {
    inches = counter * conversionFactor;  // converts encoder cycles into inches
    feet = inches / 12;
    meters = feet / 3.280839895;

    String stringInches = "    " + String(inches) + " in    ";
    String stringFeet = "    " + String(feet) + " ft    ";
    String stringMeter = "    " + String(meters) + " m    ";

    mylcd.Print_String(stringInches, CENTER, 30);
    mylcd.Print_String(stringFeet, CENTER, 60);
    mylcd.Print_String(stringMeter, CENTER, 90);
}
