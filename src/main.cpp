#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>

//led initializing ---
#define LED_PIN     2  // GPIO pin connected to the LED
#define relay_PIN     22  // GPIO pin connected to the LED

#define NUMPIXELS   1   // Number of NeoPixels
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Your custom pin configuration
#define SPI_INT_PIN  15  // Not typically used for basic reading
#define SPI_RST_PIN  17
#define SPI_SS_PIN   5
#define SPI_MOSI_PIN 13
#define SPI_MISO_PIN 12
#define SPI_SCK_PIN  14
#define BUZZER_PIN   25  // Buzzer pin

MFRC522 mfrc522(SPI_SS_PIN, SPI_RST_PIN);  // Create MFRC522 instance

// Define authorized UID (replace with your actual card's UID)
const byte authorizedUID[] = {0x94, 0x51, 0x94, 0xBF};

// ===== FUNCTION DECLARATIONS =====
// (Put these at the top so they're known to all functions)

// Helper function to print byte array
void dump_byte_array(byte *buffer, byte bufferSize);

// Action to perform when authorized card is detected
void authorizedAction();

// Action to perform when unauthorized card is detected
void unauthorizedAction();

// Compare two UIDs
bool compareUID(byte *uid1, const byte *uid2, byte size);

// ===== FUNCTION DEFINITIONS =====

void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(relay_PIN, OUTPUT);

    Serial.begin(9600);

    pixels.begin();            // Initialize NeoPixel
    // Turn off led ---
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));  // Red (R,G,B)
    pixels.show();

    // Initialize SPI with your custom pins
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SS_PIN);
    mfrc522.PCD_Init();
    delay(4);
    mfrc522.PCD_DumpVersionToSerial();
    Serial.println(F("Scan RFID/NFC Tag..."));

    // Startup sound
    for (int freq = 500; freq <= 1500; freq += 100) {
        tone(BUZZER_PIN, freq);
        delay(100);
    }
    noTone(BUZZER_PIN);
}

void loop() {
    // Reset the loop if no new card present
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Print UID
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();

    // Check if the detected UID matches our authorized UID
    if (compareUID(mfrc522.uid.uidByte, authorizedUID, mfrc522.uid.size)) {
        // Turn led to blue ---
        pixels.setPixelColor(0, pixels.Color(255, 0, 255));  // Red (R,G,B)
        pixels.show();

        Serial.println(F("Authorized card detected!"));
        authorizedAction();
    }
    else {
        // Turn led to blue ---
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));  // Red (R,G,B)
        pixels.show();

        Serial.println(F("Unknown card"));
        unauthorizedAction();
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
}

// Helper function to print byte array
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

// Action to perform when authorized card is detected
void authorizedAction() {
    for (int i = 0; i < 1; i++) {
        tone(BUZZER_PIN, 1500); // Play 1500 Hz tone
        delay(100);             // Beep duration
        noTone(BUZZER_PIN);     // Stop tone
        delay(100);             // Pause
    }

    digitalWrite(relay_PIN, low);
}

// Action to perform when unauthorized card is detected
void unauthorizedAction() {
    // Triple beep pattern for unauthorized cards
    for (int i = 0; i < 3; i++) {
        tone(BUZZER_PIN, 1000); // Lower pitch for unauthorized
        delay(100);
        noTone(BUZZER_PIN);
        delay(100);
    }
}

// Compare two UIDs
bool compareUID(byte *uid1, const byte *uid2, byte size) {
    for (byte i = 0; i < size; i++) {
        if (uid1[i] != uid2[i]) {
            return false;
        }
    }
    return true;
}