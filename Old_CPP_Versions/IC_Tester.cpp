#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0x20);

// Buttons
const int resetButton = A0;
const int testButton  = A1;
const int pinCheckButton = A2; 

// NAND, AND, OR, XOR
const int inputPins[][2] = {{2, 3}, {5, 6}, {9, 10}, {12, 13}};
const int outputPins[]   = {4, 7, 8, 11};

// HEX Inverter
const int hexInvInputs[]  = {2, 4, 6, 9, 11, 13}; 
const int hexInvOutputs[] = {3, 5, 7, 8, 10, 12};

// NOR (74LS02)
const int norInputs[][2] = {{3, 4}, {6, 7}, {8, 9}, {11, 12}};
const int norOutputs[]   = {2, 5, 10, 13};

String detectedIC = "";

void setup() {
    lcd.begin(16, 2);
    pinMode(testButton, INPUT_PULLUP);
    pinMode(resetButton, INPUT_PULLUP);
    pinMode(pinCheckButton, INPUT_PULLUP);
    lcd.print("IC TESTER READY");
}

void loop() {
    if (digitalRead(testButton) == LOW) {
        delay(200);
        lcd.clear();
        lcd.print("Testing...");
        delay(1000);

        detectedIC = testLogicIC();

        lcd.clear();
        lcd.print(detectedIC);
        waitForReset();
    }
    
    if (digitalRead(pinCheckButton) == LOW && detectedIC != "Unknown") {
        delay(200);
        lcd.clear();
        lcd.print("Checking Pins...");
        delay(1000);
        checkPins(detectedIC);
        waitForReset();
    }
    
    if (digitalRead(resetButton) == LOW) {
        delay(200);
        lcd.clear();
        lcd.print("IC TESTER READY");
    }
}

void waitForReset() {
    while (true) {
        if (digitalRead(resetButton) == LOW) { 
            delay(200);
            lcd.clear();
            lcd.print("IC TESTER READY");
            break;
        }
        delay(100);
    }
}

void setInputs(const int pins[], const int values[]) {
    for (int i = 0; i < 2; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], values[i]);
    }
}

int readPin(int pin) {
    pinMode(pin, INPUT);
    return digitalRead(pin);
}

String testLogicIC() {
    int testValues[][2] = {{LOW, LOW}, {LOW, HIGH}, {HIGH, LOW}, {HIGH, HIGH}};
    int expectedNAND[] = {HIGH, HIGH, HIGH, LOW};
    int expectedAND[]  = {LOW, LOW, LOW, HIGH};
    int expectedOR[]   = {LOW, HIGH, HIGH, HIGH};
    int expectedXOR[]  = {LOW, HIGH, HIGH, LOW};
    int expectedNOR[]  = {HIGH, LOW, LOW, LOW};
    
    bool isNAND = true, isAND = true, isOR = true, isXOR = true, isNOR = true;
    
    for (int i = 0; i < 4; i++) {
        setInputs(inputPins[i], testValues[i]);
        int result = readPin(outputPins[i]);

        if (result != expectedNAND[i]) isNAND = false;
        if (result != expectedAND[i])  isAND = false;
        if (result != expectedOR[i])   isOR = false;
        if (result != expectedXOR[i])  isXOR = false;
    }

    for (int i = 0; i < 4; i++) {
        setInputs(norInputs[i], testValues[i]);
        int result = readPin(norOutputs[i]);
        if (result != expectedNOR[i]) isNOR = false;
    }

    if (isNAND) return "74LS00 NAND";
    if (isAND)  return "74LS08 AND";
    if (isOR)   return "74LS32 OR";
    if (isXOR)  return "74LS86 XOR";
    if (isNOR)  return "74LS02 NOR";

    // HEX Inverter
    bool isHexInv = true;
    for (int i = 0; i < 6; i++) {
        digitalWrite(hexInvInputs[i], LOW);
        delay(5);
        if (readPin(hexInvOutputs[i]) != HIGH) isHexInv = false;

        digitalWrite(hexInvInputs[i], HIGH);
        delay(5);
        if (readPin(hexInvOutputs[i]) != LOW) isHexInv = false;
    }
    if (isHexInv) return "74LS04 INV";

    return "Unknown";
}

void checkPins(String icType) {
    lcd.clear();
    lcd.print("Pins OK?");
    delay(500);
    lcd.setCursor(0, 1);

    if (icType.startsWith("74LS00")) lcd.print(testAllNANDGates() ? "All Good" : "Faulty");
    else if (icType.startsWith("74LS08")) lcd.print(testAllANDGates() ? "All Good" : "Faulty");
    else if (icType.startsWith("74LS32")) lcd.print(testAllORGates() ? "All Good" : "Faulty");
    else if (icType.startsWith("74LS86")) lcd.print(testAllXORGates() ? "All Good" : "Faulty");
    else if (icType.startsWith("74LS04")) lcd.print(testAllHexInverters() ? "All Good" : "Faulty");
    else if (icType.startsWith("74LS02")) lcd.print(testAllNORGates() ? "All Good" : "Faulty");
    delay(2000);
}

bool testAllNANDGates() {
    for (int i = 0; i < 4; i++) {
        int values[] = {HIGH, HIGH};
        setInputs(inputPins[i], values);
        if (readPin(outputPins[i]) != LOW) return false;
    }
    return true;
}
bool testAllANDGates() {
    for (int i = 0; i < 4; i++) {
        int values[] = {HIGH, HIGH};
        setInputs(inputPins[i], values);
        if (readPin(outputPins[i]) != HIGH) return false;
    }
    return true;
}
bool testAllORGates() {
    for (int i = 0; i < 4; i++) {
        int values[] = {LOW, LOW};
        setInputs(inputPins[i], values);
        if (readPin(outputPins[i]) != LOW) return false;
    }
    return true;
}
bool testAllXORGates() {
    for (int i = 0; i < 4; i++) {
        int values[] = {LOW, LOW};
        setInputs(inputPins[i], values);
        if (readPin(outputPins[i]) != LOW) return false;
    }
    return true;
}
bool testAllNORGates() {
    for (int i = 0; i < 4; i++) {
        int values[] = {LOW, LOW};
        setInputs(norInputs[i], values);
        if (readPin(norOutputs[i]) != HIGH) return false;
    }
    return true;
}
bool testAllHexInverters() {
    for (int i = 0; i < 6; i++) {
        digitalWrite(hexInvInputs[i], HIGH);
        if (readPin(hexInvOutputs[i]) != LOW) return false;
    }
    return true;
}