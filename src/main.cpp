#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <Ticker.h>

// --- Pins ---
#define LED_PIN       2
#define relay_PIN     22
#define NUMPIXELS     1
#define SPI_RST_PIN   17
#define SPI_SS_PIN    5
#define SPI_MOSI_PIN  13
#define SPI_MISO_PIN  12
#define SPI_SCK_PIN   14
#define BUZZER_PIN    25

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
MFRC522 mfrc522(SPI_SS_PIN, SPI_RST_PIN);

// Authorized UID
const byte authorizedUID[] = {0xB8, 0x24, 0xA4, 0x51};

// Ticker objects
Ticker buzzerTicker;
Ticker relayTicker;

// Variables for beeping pattern
int beepCount = 0;
int totalBeeps = 0;
int beepFrequency = 0;
bool buzzerState = false;

// ===== Function Declarations =====
void dump_byte_array(byte *buffer, byte bufferSize);
bool compareUID(byte *uid1, const byte *uid2, byte size);

void startBuzzerPattern(int beeps, int frequency, int intervalMs);
void buzzerHandler();

void authorizedAction();
void unauthorizedAction();

// ===== SETUP =====
void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(relay_PIN, OUTPUT);

    Serial.begin(9600);
    pixels.begin();
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();

    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SS_PIN);
    mfrc522.PCD_Init();
    delay(4);
    mfrc522.PCD_DumpVersionToSerial();
    Serial.println(F("Scan RFID/NFC Tag..."));

    // Startup sound (non-blocking version)
    startBuzzerPattern(5, 800, 150);  // Just for bootup
}

// ===== LOOP =====
void loop() {
    if (!mfrc522.PICC_IsNewCardPresent()) return;
    if (!mfrc522.PICC_ReadCardSerial()) return;

    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();

    if (compareUID(mfrc522.uid.uidByte, authorizedUID, mfrc522.uid.size)) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 255)); // Pink
        pixels.show();

        Serial.println(F("Authorized card detected!"));
        authorizedAction();
    } else {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // Red
        pixels.show();

        Serial.println(F("Unknown card"));
        unauthorizedAction();
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

// ===== FUNCTIONS =====

// Start buzzer pattern
void startBuzzerPattern(int beeps, int frequency, int intervalMs) {
    totalBeeps = beeps * 2; // On + off counts as 2
    beepCount = 0;
    beepFrequency = frequency;
    buzzerState = false;
    buzzerTicker.attach_ms(intervalMs, buzzerHandler);
}

// Buzzer tick
void buzzerHandler() {
    if (beepCount >= totalBeeps) {
        buzzerTicker.detach();
        noTone(BUZZER_PIN);
        return;
    }

    if (buzzerState) {
        noTone(BUZZER_PIN);
    } else {
        tone(BUZZER_PIN, beepFrequency);
    }

    buzzerState = !buzzerState;
    beepCount++;
}

// Authorized action
void authorizedAction() {
    startBuzzerPattern(1, 1500, 100); // One short high beep
    digitalWrite(relay_PIN, LOW);
    relayTicker.once(2, []() { digitalWrite(relay_PIN, HIGH); }); // Turn off after 2 sec
}

// Unauthorized action
void unauthorizedAction() {
    startBuzzerPattern(3, 1000, 100); // Triple low beep
}

// Compare UIDs
bool compareUID(byte *uid1, const byte *uid2, byte size) {
    for (byte i = 0; i < size; i++) {
        if (uid1[i] != uid2[i]) return false;
    }
    return true;
}

// Print UID
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
