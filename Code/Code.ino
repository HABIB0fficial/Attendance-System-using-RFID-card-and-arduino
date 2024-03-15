#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define SS_PIN 10
#define RST_PIN 9
#define LED_G 5 // Green LED pin
#define LED_R 4 // Red LED pin
#define BUZZER 2 // Buzzer pin

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

// Valid UIDs of students
byte validUIDs[][4] = {
    {0xB0, 0xBD, 0x79, 0xA4}, // UID of student 1
   
    {0x28, 0xAA, 0x18, 0x89}, // UID of student 3
};

// Student names
const char *studentNames[] = {
    "NABEERA", // Name of student 1
    "AIMAN",   // Name of student 2
    "KHADIJA", // Name of student 3
};

void setup()
{
    Serial.begin(9600);  // Initiate a serial communication
    SPI.begin();          // Initiate SPI bus
    mfrc522.PCD_Init();   // Initiate MFRC522
    lcd.begin(16, 2);
    lcd.backlight(); // Turn on the blacklight and print a message.
    pinMode(LED_G, OUTPUT);
    pinMode(LED_R, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    noTone(BUZZER);
}

void loop()
{
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        lcd.setCursor(3, 0);
        lcd.print("SHOW YOUR");
        lcd.setCursor(4, 1);
        lcd.print("ID CARD");
        return;
    }
    else
    {
        lcd.clear();
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    // Show UID on the serial monitor
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    content.toUpperCase();

    // Check if the scanned UID is valid
    bool validUID = false;
    int studentIndex = -1;
    for (int i = 0; i < sizeof(validUIDs) / sizeof(validUIDs[0]); i++)
    {
        if (compareUID(mfrc522.uid.uidByte, validUIDs[i]))
        {
            validUID = true;
            studentIndex = i;
            break;
        }
    }

    // Display the result
    if (validUID)
    {
        lcd.print("STUDENT: ");
        lcd.print(studentNames[studentIndex]);
        lcd.setCursor(0, 1);
        lcd.print("PRESENT");
        digitalWrite(LED_G, HIGH);
        tone(BUZZER, 500);
        delay(300);
        noTone(BUZZER);
        delay(1000);
        digitalWrite(LED_G, LOW);
        lcd.clear();
        // Send data to PLX-DAQ
        Serial.print("DATA,");
        Serial.print(studentNames[studentIndex]);
        Serial.print(",");
        Serial.println("1"); // 1 for present, 0 for absent
    }
    else
    {
        lcd.print("UNAUTHORIZED");
        lcd.setCursor(0, 1);
        lcd.print("ACCESS");
        digitalWrite(LED_R, HIGH);
        tone(BUZZER, 300);
        delay(2000);
        digitalWrite(LED_R, LOW);
        noTone(BUZZER);
        lcd.clear();
        // Send data to PLX-DAQ
        Serial.print("DATA,Unknown,");
        Serial.println("0"); // 1 for present, 0 for absent
    }
}

// Function to compare two UIDs
bool compareUID(byte *uid1, byte *uid2)
{
    for (int i = 0; i < 4; i++)
    {
        if (uid1[i] != uid2[i])
        {
            return false;
        }
    }
    return true;
}
