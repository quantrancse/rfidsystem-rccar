#include <LedControl.h>

//LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address
#include <SPI.h>

//RFID 
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define RST_PIN 5
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
/////////////////////////////////////////////////////////

/* Joystick PINs */
#define VRX     A0
#define VRY     A1
#define SW      3

/* Display PINs */
#define CLK     22
#define CS      23
#define DIN     24

#define BZ      2
#define Relay   6
#define SIZE    8
#define ADVANCE_DELAY 15

int snake[SIZE*SIZE][2];
int length;
int food[2], v[2];
bool is_game_over = false;
long current_time;
long prev_advance;
int blink_count;
int n = 0;
int point = 0;

LedControl lc = LedControl(DIN, CLK, CS, 1);

//Snake Game
void init_game() {
  prev_advance = current_time = 0;
  blink_count = 3;
  int half = SIZE / 2;
  length = SIZE / 3;
  for (int i = 0; i < length; i++) {
      snake[i][0] = half - 1;
      snake[i][1] = half + i;
  }
  food[0] = half + 1;
  food[1] = half - 1;
  v[0] = 0;
  v[1] = -1;
}

void render() {
  for (int i = 0; i < length; i++) {
    lc.setLed(0, snake[i][0], snake[i][1], 1);
  }
  lc.setLed(0, food[0], food[1], 1);
}

void restart() {  
  init_game(); 
  is_game_over = false;
  n = 0;
  lc.shutdown(0, true);
  lcd.clear();
  lcd.print ("Total Score: ");lcd.print(point);
  delay(3000);
  point = 0;
  lcd.clear();
  lcd.print ("Scan again to do");
  lcd.setCursor(0, 1);
  lcd.print ("other things");
}

void readControls() {
  int dx = map(analogRead(VRX), 0, 906, 2, -2);
  int dy = map(analogRead(VRY), 0, 906, -2, 2);
  if (dx != 0) {dx = dx / abs(dx);}
  if (dy != 0) {dy = dy / abs(dy);}   
  if (dy != 0 && v[0] != 0) {
    v[0] = 0;
    v[1] = dy;    
  }
  if (dx != 0 && v[1] != 0) {
    v[0] = dx;
    v[1] = 0;
  } 
}

void clearScreen() {
  for (int x = 0; x < SIZE; x++) {
    for (int y = 0; y < SIZE; y++) {
      lc.setLed(0, x, y, 0);
    }
  }
}

/**
 * moves the snake forward
 * returns true if the game is over
 */
bool advance() {
  int head[2] = {snake[0][0] + v[0], snake[0][1] + v[1]};
  for (int i = 0; i < length; i++) {
      if (snake[i][0] == head[0] && snake[i][1] == head[1]) {
          return true;
      }
  }
  bool grow = (head[0] == food[0] && head[1] == food[1]);
  
  if (grow) {
      digitalWrite(BZ, LOW);
      delay(500);
      digitalWrite(BZ, HIGH);
      length++;  
      randomSeed(current_time);    
      food[0] = random(SIZE);
      food[1] = random(SIZE);     
      point = point + 10;
      lcd.clear();
      lcd.print("Score: ");lcd.print(point);
  } 
  for (int i = length -1; i >= 0; i--) {
      snake[i + 1][0] = snake[i][0];
      snake[i + 1][1] = snake[i][1];      
  }
  snake[0][0] += v[0];
  snake[0][1] += v[1];

  if (snake[0][0] == 8){snake[0][0] = 0;}
  if (snake[0][1] == 8){snake[0][1] = 0;}

  if (snake[0][0] == -1){snake[0][0] = 7;}
  if (snake[0][1] == -1){snake[0][1] = 7;}
  return false;
}

void setup() {
  pinMode(SW, INPUT_PULLUP);
  pinMode(VRX, INPUT);
  pinMode(VRY, INPUT);
  pinMode(Relay, OUTPUT);
  pinMode(BZ, OUTPUT);
  digitalWrite(BZ, HIGH);

//LCD 
lcd.begin();
    lcd.setCursor(1, 0);
    lcd.print ("Product of NASA");
    lcd.setCursor(2, 1);
    lcd.print ("RFID System"); 
    delay (3000);
    lcd.clear ();
    lcd.setCursor(1, 0);
    lcd.print ("Scan Card");
///////////////////////////////////////

  Serial.begin(9600);
/*  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  init_game();
  render(); */
  
//RFID
SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  ////////////////////////////////////////
}

void loop() {
//RFID
// Look for new cards

if (n == 0) {
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
       
// Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    //Serial.println(rfid.uid.uidByte[0]);
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

//Case 152 - Door Lock
  if (rfid.uid.uidByte[0] == 152) {
    lcd.clear ();
    lcd.print("Access Granted");    
    digitalWrite(BZ, LOW);
    delay(500);
    digitalWrite(BZ, HIGH); 
    delay(1000);
    lcd.clear ();
    lcd.setCursor(2, 0);
    lcd.print("Door Access");
    lcd.setCursor(0, 1);
    lcd.print("Control System");
    delay(2500);
    digitalWrite(Relay, HIGH);
    lcd.clear();
    lcd.print ("Door is Open");
    delay(3000);
    lcd.clear();
    for (int i=5; i > 0; i--) {
        lcd.clear();
        lcd.print("Auto lock in ");lcd.print(i);lcd.print(" seconds");
        delay(1000);
    }    
    digitalWrite(Relay,LOW);
    lcd.clear();
    lcd.print("Door Locked");
    delay(800);
    lcd.clear();
    lcd.print("Scan Card");
  } else

//Case 208 - Snake Game
  if (rfid.uid.uidByte[0] == 208) {
    n = 1;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Access Granted");
    delay(1500);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Snake Game");
    lcd.setCursor(2, 1);
    lcd.print ("Let's play!");
    delay(3000);
    lcd.clear();
    lcd.print("Score: ");lcd.print(0);
    lc.shutdown(0, false);
    lc.setIntensity(0, 8);
    init_game();
    render();
  } 
  else {  
    lcd.clear();
    lcd.print("Wrong ID");
    digitalWrite(BZ, LOW);
    delay(3000);
    digitalWrite(BZ, HIGH); 
    lcd.setCursor(0, 1);
    lcd.print("Access Denied");
    delay(2000);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Scan Card Again");
  }
}
/////////////////////////////////////////////////////////

if (n == 1) { 
  if (digitalRead(SW) == LOW) {
    restart(); 
  }
  if (!is_game_over) {
    clearScreen();
    render();
    if (current_time - prev_advance > ADVANCE_DELAY) {
      is_game_over = advance();
      prev_advance = current_time;    
    }
  } else {
    while (blink_count > 0) {
      clearScreen();
      delay(300);
      render();
      delay(300);
      blink_count--;           
    }
    restart();
  }
  readControls();
  current_time++;
 }
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
  return;
}
