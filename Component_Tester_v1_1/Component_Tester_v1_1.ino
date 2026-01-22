#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD - I2C Address 0x27 standard
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= PINS =================
const int testButton = A3; // PC3

// Sense Pins (J2 Ports)
const int TP1 = A0; // PC0
const int TP2 = A1; // PC1
const int TP3 = A2; // PC2

// Resistor Ladder Drivers
// TP1
const int TP1_680  = 8;
const int TP1_470k = 10;
// TP2
const int TP2_680  = 9;
const int TP2_470k = 11;
// TP3
const int TP3_680  = 12; 
const int TP3_470k = 13;

// Shift Register (7-Segment)
const int SHIFT_DATA  = 4; 
const int SHIFT_LATCH = 5; 
const int SHIFT_CLOCK = 6;

// Variables
int lastTestBtnState = HIGH;
unsigned long lastPollTime = 0;
const int SLAVE_ADDR = 8;
bool isSlaveActive = false;
String lastSlaveMsg = "";

// Hex Map
const byte segmentMap[] = { 
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 
  0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 
};

void setup() {
    Wire.begin(); // Master Mode
    lcd.init();
    lcd.backlight();
    lcd.print("COMPONENT TESTER");
    lcd.setCursor(0,1);
    lcd.print("System Ready");
    
    pinMode(testButton, INPUT_PULLUP);
    
    pinMode(SHIFT_DATA, OUTPUT);
    pinMode(SHIFT_LATCH, OUTPUT);
    pinMode(SHIFT_CLOCK, OUTPUT);
    
    resetPins();
    lastTestBtnState = digitalRead(testButton);
}

void loop() {
    // ----------------------------------------
    // SCHEMATIC LOGIC HANDLER
    // Poll Slave (IC Tester) every 200ms
    // If Slave responds, it means SW_MODE is "IC".
    // We display Slave's buffer.
    // ----------------------------------------
    
    if (millis() - lastPollTime > 200) {
        lastPollTime = millis();
        
        // Polling Strategy: Answer to Q3
        // We act as the "Bus Master". We ask the Slave "Do you have data?"
        // The Slave (if active and button pressed) will have prepared a buffer.
        // We read 32 bytes to capture full names like "74148 8-to-3 Encoder".
        
        Wire.requestFrom(SLAVE_ADDR, 32); 
        String msg = "";
        while(Wire.available()) {
            char c = Wire.read();
            if(c != 0) msg += c;
        }
        
        // If we got a valid message that isn't emptry or just "Ready"
        if (msg.length() > 2 && msg != "Ready") { 
             if(msg != lastSlaveMsg) { // Only update LCD on change
                 lcd.clear();
                 lcd.print("IC TESTER MODE");
                 lcd.setCursor(0,1);
                 // Safety: Prevent overflow on LCD (16 chars)
                 // But we want to show as much as possible.
                 // The LCD library handles wrapping or truncation usually, 
                 // but let's just print it.
                 lcd.print(msg); 
                 lastSlaveMsg = msg;
                 isSlaveActive = true;
             }
             return; 
        } else {
             if(isSlaveActive) {
                 if(msg.length() == 0) isSlaveActive = false; 
             }
        }
    }
    
    // ----------------------------------------
    // COMPONENT TESTER LOGIC
    // Only runs if Slave isn't dominating the display
    // ----------------------------------------
    
    int btn = digitalRead(testButton);
    if(btn == LOW && lastTestBtnState == HIGH) {
        delay(50);
        if(digitalRead(testButton) == LOW) {
            // Force Mode: If user presses THIS button, we assume they want Component Test
            isSlaveActive = false; 
            runTests();
        }
    }
    lastTestBtnState = btn;
}

void resetPins() {
    pinMode(TP1, INPUT); pinMode(TP2, INPUT); pinMode(TP3, INPUT);
    pinMode(TP1_680, INPUT); pinMode(TP1_470k, INPUT);
    pinMode(TP2_680, INPUT); pinMode(TP2_470k, INPUT);
    pinMode(TP3_680, INPUT); pinMode(TP3_470k, INPUT);
}

// ================= MAIN TEST SEQUENCE =================
void runTests() {
    lcd.clear();
    lcd.print("Testing...");
    
    // 1. 7-Segment (Quick Check)
    testSevenSegment();
   
    // 2. Component Logic
    // We try to identify the component by priority:
    // Transistor -> Diode/LED -> Resistor -> Capacitor
    
    if(checkTransistor()) return;
    if(checkDiodeLED()) return;
    if(checkResistor()) return;
    if(checkCapacitor()) return;
    
    lcd.clear();
    lcd.print("No Part Found");
    lcd.setCursor(0,1);
    lcd.print("Check Probes");
}

// ---------------- 7-SEGMENT ----------------
void displayHex(int num) {
    if(num < 0 || num > 15) return;
    byte segments = segmentMap[num];
    digitalWrite(SHIFT_LATCH, LOW);
    shiftOut(SHIFT_DATA, SHIFT_CLOCK, MSBFIRST, segments);
    digitalWrite(SHIFT_LATCH, HIGH);
}
void testSevenSegment() {
    // Quick Cycle 0-F
    for(int i=0; i<8; i++){ // Short cycle to be fast
        displayHex(i);
        delay(100);
    }
    displayHex(0);
}

// ---------------- UTILS ----------------
int readADC(int pin) { return analogRead(pin); }

// ---------------- TRANSISTOR CHECK (Simple NPN/PNP) ----------------
bool checkTransistor() {
    resetPins();
    
    // Test NPN (BJT) logic:
    // Collector (TP3) High, Emitter (TP1) Low, Apply Base (TP2) High
    // Check if C-E conducts
    
    // NPN Setup: C=TP3, B=TP2, E=TP1
    pinMode(TP3_680, OUTPUT); digitalWrite(TP3_680, HIGH); // VCC to C
    pinMode(TP1, OUTPUT); digitalWrite(TP1, LOW);          // GND to E
    
    int valOff = analogRead(TP3); // Should be High (not conducting yet)
    
    // Apply Base Current
    pinMode(TP2_680, OUTPUT); digitalWrite(TP2_680, HIGH);
    delay(5);
    int valOn = analogRead(TP3); // Should pull Low if conducting
    
    if(valOn < 200 && valOff > 600) {
        lcd.clear();
        lcd.print("NPN Transistor");
        lcd.setCursor(0,1);
        lcd.print("E:1 B:2 C:3");
        return true;
    }
    
    // PNP Setup: E=TP3 (High), B=TP2 (Low), C=TP1 (Low Load)
    resetPins();
    pinMode(TP3, OUTPUT); digitalWrite(TP3, HIGH); // VCC to E
    pinMode(TP1_680, OUTPUT); digitalWrite(TP1_680, LOW); // Pull-down C
    
    valOff = analogRead(TP1); // Should be Low
    
    // Apply Base Low
    pinMode(TP2_680, OUTPUT); digitalWrite(TP2_680, LOW);
    delay(5);
    int valOn2 = analogRead(TP1); // Should go High (conducting)
    
    if(valOn2 > 600 && valOff < 200) {
        lcd.clear();
        lcd.print("PNP Transistor");
        lcd.setCursor(0,1);
        lcd.print("C:1 B:2 E:3");
        return true;
    }
    
    return false;
}

// ---------------- DIODE / LED CHECK ----------------
bool checkDiodeLED() {
    resetPins();
    
    // Anode TP1, Cathode TP2
    pinMode(TP1_680, OUTPUT); digitalWrite(TP1_680, HIGH);
    pinMode(TP2, OUTPUT); digitalWrite(TP2, LOW);
    delay(5);
    int vFwd = analogRead(TP1); // Voltage drop
    
    // Reverse
    resetPins();
    pinMode(TP2_680, OUTPUT); digitalWrite(TP2_680, HIGH);
    pinMode(TP1, OUTPUT); digitalWrite(TP1, LOW);
    delay(5);
    int vRev = analogRead(TP2);
    
    // Analysis
    // Diodes drop ~0.6V (ADC ~120) to ~2.0V (ADC ~400 for LED)
    // 5V = 1023. Open > 1000. Short < 10.
    
    if(vFwd < 900 && vFwd > 100 && vRev > 1000) {
        lcd.clear();
        if(vFwd > 300) lcd.print("LED Found");
        else lcd.print("Diode Found");
        lcd.setCursor(0,1);
        lcd.print("A:1 C:2"); // Anode 1, Cathode 2
        return true;
    }
    
    if(vRev < 900 && vRev > 100 && vFwd > 1000) {
         lcd.clear();
         if(vRev > 300) lcd.print("LED Found");
         else lcd.print("Diode Found");
         lcd.setCursor(0,1);
         lcd.print("A:2 C:1"); // Anode 2, Cathode 1
         return true;
    }
    
    return false;
}

// ---------------- RESISTOR CHECK ----------------
bool checkResistor() {
    resetPins();
    // Test 1-2
    pinMode(TP1_680, OUTPUT); digitalWrite(TP1_680, HIGH);
    pinMode(TP2, OUTPUT); digitalWrite(TP2, LOW);
    delay(5);
    int val = analogRead(TP1);
    
    if(val < 1015 && val > 10) {
         // Calculate R
         float R = (float)val * 680.0 / (1023.0 - (float)val);
         lcd.clear();
         lcd.print("Resistor 1-2");
         lcd.setCursor(0,1);
         if(R >= 1000) { lcd.print(R/1000.0); lcd.print("k"); }
         else { lcd.print((int)R); lcd.print("R"); }
         return true;
    }
    return false;
}

// ---------------- CAPACITOR CHECK ----------------
bool checkCapacitor() {
    resetPins();
    // Test 1-3 (Standard slot often used)
    
    // Discharge
    pinMode(TP1, OUTPUT); digitalWrite(TP1, LOW);
    pinMode(TP3, OUTPUT); digitalWrite(TP3, LOW);
    delay(200);
    resetPins();
    
    // Charge TP1 via 470k, GND TP3
    pinMode(TP3, OUTPUT); digitalWrite(TP3, LOW);
    pinMode(TP1_470k, OUTPUT); digitalWrite(TP1_470k, HIGH);
    
    unsigned long start = micros();
    // Charge to ~63% (1 Tau) ~ADC 644
    while(analogRead(TP1) < 644) {
        if(micros() - start > 500000) break; // Timeout .5s
    }
    unsigned long elapsed = micros() - start;
    
    resetPins();
    
    if(elapsed > 10 && elapsed < 500000) {
        float uF = (float)elapsed / 470000.0; // T = RC -> C = T/R
        uF = uF * 1000000.0; // Convert to uF units logic
        // Actually: C (Farads) = Time (sec) / R (Ohms)
        // C(uF) = Time(uS) / R(Ohms)
        
        float val = (float)elapsed / 470.0; // nF?
        // Let's print raw uF
        
        lcd.clear();
        lcd.print("Capacitor 1-3");
        lcd.setCursor(0,1);
        lcd.print(val / 1000.0, 3);
        lcd.print(" uF");
        return true;
    }
    
    return false;
}


