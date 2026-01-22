```cpp
#include <avr/pgmspace.h>
#include <Wire.h>

const int testButton = A3;
const int SLAVE_ADDR = 8;
char resultBuffer[32] = "Ready"; // Buffer to send to Master

// --- Pin Mappings ---
// 14-pin ICs shifted so pin 7 = GND aligns with 16-pin IC pin 8
const int PinOut14Shifted[14] = {3, 4, 5, 6, 7, 8, -1, 9, 10, 11, 12, 13, A0, A1};
const int PinOut16[16]        = {2, 3, 4, 5, 6, 7, 8, -1, 9, 10, 11, 12, 13, A0, A1, A2};

// ------------------
// PROGMEM IC Patterns
// ------------------

// --- 7400 ---
const char p7400_0[] PROGMEM = "00H00HGH00H00V";
const char p7400_1[] PROGMEM = "10H10HGH10H10V";
const char p7400_2[] PROGMEM = "01H01HGH01H01V";
const char p7400_3[] PROGMEM = "11L11LGL11L11V";
const char* const patterns7400[] PROGMEM = {p7400_0, p7400_1, p7400_2, p7400_3};

// --- 7402 ---
const char p7402_0[] PROGMEM = "H00H00G00H00HV";
const char p7402_1[] PROGMEM = "L10L10G10L10LV";
const char p7402_2[] PROGMEM = "L01L01G01L01LV";
const char p7402_3[] PROGMEM = "L11L11G11L11LV";
const char* const patterns7402[] PROGMEM = {p7402_0, p7402_1, p7402_2, p7402_3};

// --- 7404 ---
const char p7404_0[] PROGMEM = "0H0H0HGH0H0H0V";
const char p7404_1[] PROGMEM = "1L1L1LGL1L1L1V";
const char* const patterns7404[] PROGMEM = {p7404_0, p7404_1};

// --- 7408 ---
const char p7408_0[] PROGMEM = "00L00LGL00L00V";
const char p7408_1[] PROGMEM = "01L01LGL01L01V";
const char p7408_2[] PROGMEM = "10L10LGL10L10V";
const char p7408_3[] PROGMEM = "11H11HGH11H11V";
const char* const patterns7408[] PROGMEM = {p7408_0, p7408_1, p7408_2, p7408_3};

// --- 7432 ---
const char p7432_0[] PROGMEM = "00L00LGL00L00V";
const char p7432_1[] PROGMEM = "01H01HGH10H10V";
const char p7432_2[] PROGMEM = "10H10HGH01H01V";
const char p7432_3[] PROGMEM = "11H11HGH11H11V";
const char* const patterns7432[] PROGMEM = {p7432_0, p7432_1, p7432_2, p7432_3};

// --- 7447 ---
const char p7447_0[] PROGMEM = "001H100GLLLLLHLV";
const char p7447_1[] PROGMEM = "001H001GHHLLHHHV";
const char p7447_2[] PROGMEM = "101H100GLLHLLLHV";
const char p7447_3[] PROGMEM = "101H001GHLLLLLHV";
const char p7447_4[] PROGMEM = "011H100GHHLLHLLV";
const char p7447_5[] PROGMEM = "011H001GHLLHLLLV";
const char p7447_6[] PROGMEM = "111H100GLLLHHLLV";
const char p7447_7[] PROGMEM = "111H001GHHLLLHHV";
const char p7447_8[] PROGMEM = "001H110GLLLLLLLV";
const char p7447_9[] PROGMEM = "001H011GHHLLLLLV";
const char* const patterns7447[] PROGMEM = {p7447_0,p7447_1,p7447_2,p7447_3,p7447_4,p7447_5,p7447_6,p7447_7,p7447_8,p7447_9};

// --- 7486 ---
const char p7486_0[] PROGMEM = "00L00LGL00L00V";
const char p7486_1[] PROGMEM = "01H01HGH01H01V";
const char p7486_2[] PROGMEM = "10H10HGH10H10V";
const char p7486_3[] PROGMEM = "11L11LGL11L11V";
const char* const patterns7486[] PROGMEM = {p7486_0,p7486_1,p7486_2,p7486_3};

// --- 7490A ---
const char p7490A_0[] PROGMEM = "0000H0001G0000V";
const char p7490A_1[] PROGMEM = "0001H0010G0001V";
const char p7490A_2[] PROGMEM = "0010H0011G0010V";
const char p7490A_3[] PROGMEM = "0011H0100G0011V";
const char p7490A_4[] PROGMEM = "0100H0101G0100V";
const char p7490A_5[] PROGMEM = "0101H0110G0101V";
const char p7490A_6[] PROGMEM = "0110H0111G0110V";
const char p7490A_7[] PROGMEM = "0111H1000G0111V";
const char p7490A_8[] PROGMEM = "1000H1001G1000V";
const char p7490A_9[] PROGMEM = "1001L0000G1001V";
const char* const patterns7490A[] PROGMEM = {p7490A_0,p7490A_1,p7490A_2,p7490A_3,p7490A_4,p7490A_5,p7490A_6,p7490A_7,p7490A_8,p7490A_9};

// --- 74148 (partial example) ---
const char p74148_0[] PROGMEM = "01011HHGH0101HHV";
const char p74148_1[] PROGMEM = "11110HHGH1111HLV";
const char p74148_2[] PROGMEM = "10100LLGL0101LHV";
const char p74148_3[] PROGMEM = "01010LLGH1010LHV";
const char p74148_4[] PROGMEM = "10110LHGL0101LHV";
const char p74148_5[] PROGMEM = "01110LHGH1010LHV";
const char p74148_6[] PROGMEM = "11110HLGL1010LHV";
const char p74148_7[] PROGMEM = "11110HLGH0101LHV";
const char p74148_8[] PROGMEM = "11110HHGL1011LHV";
const char p74148_9[] PROGMEM = "11110HHGH0111LHV";
const char* const patterns74148[] PROGMEM = {p74148_0,p74148_1,p74148_2,p74148_3,p74148_4,p74148_5,p74148_6,p74148_7,p74148_8,p74148_9};

// --- 74194 (partial example) ---
const char p74194_0[] PROGMEM = "0000000G00CLLLLV";
const char p74194_1[] PROGMEM = "0111111G11CLLLLV";
const char p74194_2[] PROGMEM = "1000000G00CLLLLV";
const char p74194_3[] PROGMEM = "1111111G11CLLLLV";
const char p74194_4[] PROGMEM = "1011110G11CHHHHV";
const char p74194_5[] PROGMEM = "1100001G11CLLLLV";
const char p74194_6[] PROGMEM = "1111110G10CLLLHV";
const char p74194_7[] PROGMEM = "1100001G10CLLHHV";
const char p74194_8[] PROGMEM = "1100001G10CLHHHV";
const char p74194_9[] PROGMEM = "1100001G10CHHHHV";
const char p74194_10[] PROGMEM = "1000000G10CHHHLV";
const char p74194_11[] PROGMEM = "1011111G10CHHLLV";
const char p74194_12[] PROGMEM = "1000001G01CHHHLV";
const char p74194_13[] PROGMEM = "1111111G01CHHHHV";
const char p74194_14[] PROGMEM = "1000000G01CLHHHV";
const char p74194_15[] PROGMEM = "1111110G01CLLHHV";
const char p74194_16[] PROGMEM = "1000000G00CLLHHV";
const char p74194_17[] PROGMEM = "1111111G00CLLHHV";
const char* const patterns74194[] PROGMEM = {p74194_0,p74194_1,p74194_2,p74194_3,p74194_4,p74194_5,p74194_6,p74194_7,p74194_8,p74194_9,p74194_10,p74194_11,p74194_12,p74194_13,p74194_14,p74194_15,p74194_16,p74194_17};

// ------------------
// IC List
// ------------------
struct IC {
    const char* name;
    const char* type;
    int pinCount;
    int patternCount;
    const char* const* patterns;
};

IC icList[] = {
    {"7400","Quad 2-NAND",14,4,patterns7400},
    {"7402","Quad 2-NOR",14,4,patterns7402},
    {"7404","Hex Inverter",14,2,patterns7404},
    {"7408","Quad 2-AND",14,4,patterns7408},
    {"7432","Quad 2-OR",14,4,patterns7432},
    {"7447","BCD to 7-Seg",16,10,patterns7447},
    {"7486","Quad 2-XOR",14,4,patterns7486},
    {"7490A","Decade Counter",14,10,patterns7490A},
    {"74148","8-to-3 Encoder",16,10,patterns74148},
    {"74194","4-Bit Shift Reg",16,18,patterns74194}
};

const int numICs = sizeof(icList)/sizeof(IC);
bool lastButtonState = HIGH;

// I2C Event Handler
void requestEvent() {
    Wire.write(resultBuffer); 
}

// ---------------- //
// Helper Functions //
// ---------------- //

bool testPattern(const char* const patternPtr, int pinCount) {
    const int* pinMap = (pinCount == 14) ? PinOut14Shifted : PinOut16;
    
    // First pass: Set pin modes and write outputs
    for(int i=0;i<pinCount;i++){
        int pin = pinMap[i];
        if(pin<0) continue;
        
        char s = pgm_read_byte_near(patternPtr+i);
        
        if(s=='0') {
            pinMode(pin,OUTPUT); 
            digitalWrite(pin,LOW);
        }
        else if(s=='1'||s=='V'){
            pinMode(pin,OUTPUT); 
            digitalWrite(pin,HIGH);
        }
        else if(s=='L'||s=='H'||s=='X'){
            pinMode(pin,INPUT_PULLUP); // Use PULLUP to avoid floating inputs
        }
        else if(s=='C'){
            pinMode(pin,OUTPUT); 
            digitalWrite(pin,LOW); // Clock low initially
        }
    }

    delay(5);
    
    // Toggle Clock if present
    for(int i=0;i<pinCount;i++) {
        if(pgm_read_byte_near(patternPtr+i)=='C'){
            int pin = pinMap[i]; 
            if(pin>=0) {
                // Clock pulse: Low -> High -> Low (or just Input/High depending on logic)
                // Original code set it to INPUT, which is effectively High-Z/Pullup? 
                // Let's stick to original logic: set to INPUT (High-Z if no pullup, but we probably want a clean edge)
                // Actually original code was: if(s=='C') pinMode(pin,INPUT);
                // Creating a rising edge if external pullup exists, or just valid VIH.
                pinMode(pin,INPUT_PULLUP); 
            }
        }
    }
    delay(5);

    bool result=true;
    // Second pass: Read and Verify
    for(int i=0;i<pinCount;i++){
        int pin = pinMap[i];
        if(pin<0) continue;
        
        char s = pgm_read_byte_near(patternPtr+i);
        
        if(s=='H') {
            if(digitalRead(pin)==LOW) { result=false; break; }
        }
        else if(s=='L') {
            if(digitalRead(pin)==HIGH) { result=false; break; }
        }
    }
    return result;
}

void runICTest(){
    Serial.println("Testing...");
    strcpy(resultBuffer, "Testing...");
    
    bool found=false;
    int foundIndex=-1;

    for(int i=0;i<numICs;i++){
        bool ok=true;
        for(int p=0;p<icList[i].patternCount;p++){
            const char* patternPtr = (const char*)pgm_read_ptr(&(icList[i].patterns[p]));
            if(!testPattern(patternPtr,icList[i].pinCount)){ok=false; break;}
        }
        if(ok){found=true; foundIndex=i; break;}
    }

    if(found){
        Serial.print("IC_FOUND: ");
        Serial.print(icList[foundIndex].name);
        Serial.print(" ");
        Serial.println(icList[foundIndex].type);
        
        // Format: "IC: 74xxx"
        strcpy(resultBuffer, "IC: ");
        strcat(resultBuffer, icList[foundIndex].name);
    } else {
        Serial.println("IC_NOT_FOUND or FAILED");
        strcpy(resultBuffer, "Unknown IC");
    }

    // Reset pins to safe state
    for(int i=2;i<=13;i++) pinMode(i,INPUT);
    pinMode(A0,INPUT); pinMode(A1,INPUT); pinMode(A2,INPUT);
}

void setup(){
    Serial.begin(9600);
    Serial.println("IC Tester Slave Ready.");
    
    // I2C Slave Setup
    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(requestEvent);
    
    pinMode(testButton,INPUT_PULLUP);
    
    // FIX: Initialize lastButtonState to the current state.
    // This prevents the tester from "auto-firing" if the button is held down 
    // or if the switch logic keeps it LOW at startup.
    lastButtonState = digitalRead(testButton);
}

void loop(){
    bool buttonState = digitalRead(testButton);
    
    // Detect falling edge (HIGH -> LOW)
    if(buttonState==LOW && lastButtonState==HIGH){
        delay(50); // Debounce
        if(digitalRead(testButton)==LOW){
            runICTest();
            // Wait for button release to prevent loop re-triggering
            while(digitalRead(testButton)==LOW) delay(10);
        }
    }
    
    lastButtonState = buttonState;
}
```
