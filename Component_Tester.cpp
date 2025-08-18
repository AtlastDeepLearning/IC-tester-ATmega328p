#include <Adafruit_LiquidCrystal.h>

// LCD
Adafruit_LiquidCrystal lcd(0);

// Buttons
const int testButton   = 8;
const int checkPinsBtn = 9;
const int resetButton  = 10;
const int ledButton    = 11;
const int resButton    = 12;
const int capButton    = 13;
const int diodeButton  = 6;   // NEW: Diode test button

// Measurement Pins
const int resistorPin  = A2;
const int capacitorPin = A3;
const int ledPin       = A1;

// 7-Segment Decoder Pins
const int DP2 = 7;
const int DP3 = 2;
const int DP4 = 1;
const int DP5 = 6;

// Capacitance Measurement
const int chargePin    = 13;
const int dischargePin = 11;
const float resistorValue = 10000.0;

// Diode Testing Pins
const int diodeAnodePin   = A4;
const int diodeCathodePin = 4;
const int diodeVccPin     = 5;

void setup() {
    lcd.begin(16, 2);
    lcd.print("COMPONENT TESTER");
    
    pinMode(testButton, INPUT_PULLUP);
    pinMode(checkPinsBtn, INPUT_PULLUP);
    pinMode(resetButton, INPUT_PULLUP);
    pinMode(ledButton, INPUT_PULLUP);
    pinMode(resButton, INPUT_PULLUP);
    pinMode(capButton, INPUT_PULLUP);
    pinMode(diodeButton, INPUT_PULLUP);

    pinMode(DP2, OUTPUT);
    pinMode(DP3, OUTPUT);
    pinMode(DP4, OUTPUT);
    pinMode(DP5, OUTPUT);
    pinMode(ledPin, OUTPUT);
}

void loop() {
    if (digitalRead(resetButton) == LOW) resetDisplay();
    if (digitalRead(testButton) == LOW) testSevenSegment();
    if (digitalRead(checkPinsBtn) == LOW) checkSevenSegment();
    if (digitalRead(ledButton) == LOW) testLED();
    if (digitalRead(resButton) == LOW) checkResistance();
    if (digitalRead(capButton) == LOW) checkCapacitance();
    if (digitalRead(diodeButton) == LOW) checkDiode();
}

// Reset
void resetDisplay() {
    lcd.clear();
    lcd.print("COMPONENT TESTER");
}

// ================= 7-SEGMENT =================
void testSevenSegment() {
    lcd.clear();
    lcd.print("Testing 7-Seg...");
    delay(500);

    bool isCommonAnode = detectCommonAnode();
    lcd.clear();
    lcd.print(isCommonAnode ? "Common Anode" : "Common Cathode");
    delay(2000);
    displayHex(15);
    delay(1000);
}

bool detectCommonAnode() {
    digitalWrite(DP2, HIGH); digitalWrite(DP3, HIGH);
    digitalWrite(DP4, HIGH); digitalWrite(DP5, HIGH);
    delay(100);
    return !(digitalRead(DP2) || digitalRead(DP3) || digitalRead(DP4) || digitalRead(DP5));
}

void checkSevenSegment() {
    lcd.clear();
    lcd.print("7-Seg Check");
    for (int i = 0; i < 16; i++) {
        displayHex(i);
        delay(500);
    }
}

void displayHex(int num) {
    digitalWrite(DP2, num & 0x01);
    digitalWrite(DP3, num & 0x02);
    digitalWrite(DP4, num & 0x04);
    digitalWrite(DP5, num & 0x08);
}

// ================= LED =================
void testLED() {
    lcd.clear();
    lcd.print("LED Test");
    digitalWrite(ledPin, HIGH);
    delay(2000);
    digitalWrite(ledPin, LOW);
}

// ================= RESISTOR =================
void checkResistance() {
    lcd.clear();
    lcd.print("Measuring R...");
    delay(500);
    
    int raw = analogRead(resistorPin);
    if (raw) {
        float Vin = 5.0;
        float Vout = (raw * Vin) / 1023.0;
        float R1 = 1000.0;
        float buffer = (Vin / Vout) - 1;
        float R2 = R1 * buffer;
        
        lcd.clear();
        lcd.print("R=");
        lcd.print(R2);
        lcd.print(" Ohm");
    } else {
        lcd.clear();
        lcd.print("No Resistor");
    }
    delay(2000);
}

// ================= CAPACITOR =================
void checkCapacitance() {
    lcd.clear();
    lcd.print("Measuring C...");
    delay(500);
    
    pinMode(chargePin, OUTPUT);
    digitalWrite(chargePin, HIGH);
    unsigned long startTime = millis();
    while (analogRead(capacitorPin) < 648) {}
    unsigned long elapsed = millis() - startTime;
    
    float microFarads = ((float)elapsed / resistorValue) * 1000;
    lcd.clear();
    lcd.print("C=");
    lcd.print(microFarads);
    lcd.print(" uF");
    delay(2000);
    
    digitalWrite(chargePin, LOW);
    pinMode(dischargePin, OUTPUT);
    digitalWrite(dischargePin, LOW);
    while (analogRead(capacitorPin) > 0) {}
    pinMode(dischargePin, INPUT);
}

// ================= DIODE =================
void checkDiode() {
    lcd.clear();
    lcd.print("Diode Test...");
    delay(500);

    // Forward bias
    pinMode(diodeCathodePin, OUTPUT);
    pinMode(diodeVccPin, OUTPUT);
    digitalWrite(diodeCathodePin, LOW);
    digitalWrite(diodeVccPin, HIGH);
    delay(10);
    float fwdV = analogRead(diodeAnodePin) * (5.0 / 1023.0);

    // Reverse bias
    digitalWrite(diodeCathodePin, HIGH);
    digitalWrite(diodeVccPin, LOW);
    delay(10);
    float revV = analogRead(diodeAnodePin) * (5.0 / 1023.0);

    lcd.clear();
    lcd.print("Fwd=");
    lcd.print(fwdV, 2);
    lcd.print("V");
    lcd.setCursor(0, 1);
    lcd.print("Rev=");
    lcd.print(revV, 2);
    lcd.print("V");
    delay(3000);

    lcd.clear();
    if (fwdV > 0.2 && fwdV < 1.0 && revV > 4.0) lcd.print("Diode OK");
    else if (fwdV < 0.1 && revV < 0.5) lcd.print("Shorted");
    else if (fwdV > 4.0) lcd.print("Open Diode");
    else lcd.print("Faulty");
    delay(2000);
}