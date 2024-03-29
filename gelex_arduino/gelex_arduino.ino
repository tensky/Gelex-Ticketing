#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <RTClib.h>

char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

#define RST_PIN 9
#define SS_PIN 10
#define CS_PIN 8
#define BUZZER_PIN 4

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
RTC_DS3231 rtc;
File file;
bool isOpen = false;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  SPI.begin();
  mfrc522.PCD_Init();
  SD.begin(CS_PIN);
  if (! rtc.begin()) {
    tone(BUZZER_PIN, 500);
    while (1);
  }
  key.keyByte[0] = 0xA0;
  key.keyByte[1] = 0xA1;
  key.keyByte[2] = 0xA2;
  key.keyByte[3] = 0xA3;
  key.keyByte[4] = 0xA4;
  key.keyByte[5] = 0xA5;
}

void loop() {
  if (isOpen)file.close();
  noTone(BUZZER_PIN);
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  DateTime now = rtc.now();

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 11, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    tone(BUZZER_PIN, 500);
    delay(2000);
    return;
  }

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(9, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    tone(BUZZER_PIN, 500);
    delay(2000);
    return;
  }

  file = SD.open("data.txt", FILE_WRITE);
  isOpen = true;

  for (int i = 0; i < 16; i++) {
    file.print((char)buffer[i]);
  }
  file.println();

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 27, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    tone(BUZZER_PIN, 500);
    delay(2000);
    return;
  }

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(24, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    tone(BUZZER_PIN, 500);
    delay(2000);
    return;
  }

  for (int i = 0; i < 8; i++) {
    file.print(String(buffer[i], HEX));
  }
  file.println();

  file.print(daysOfTheWeek[now.dayOfTheWeek()]);
  file.print(", ");
  file.print(now.day(), DEC);
  file.print('/');
  file.print(now.month(), DEC);
  file.print('/');
  file.print(now.year(), DEC);
  file.print(' ');
  file.print(now.hour(), DEC);
  file.print(':');
  file.print(now.minute(), DEC);
  file.print(':');
  file.print(now.second(), DEC);

  file.println();
  file.println("$");

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  file.close();
  isOpen = false;
  tone(BUZZER_PIN, 3000);
  delay(250);
}
