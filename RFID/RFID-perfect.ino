#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address
#include <SPI.h>
#include <AddicoreRFID.h>
#define  uchar unsigned char
#define uint  unsigned int
uchar serNumA[5];
uchar fifobytes;
uchar fifoValue;

AddicoreRFID myRFID; // create AddicoreRFID object to control the RFID module
const int chipSelectPin = 10;
const int NRSTPD = 9; // RST Pin
const int Buzzer = 8;
const int Redled = 5;
const int Greenled = 6;
const int Relay = 3;
const byte End = 3;
const byte Score = 4;
const byte Open = 7;
int point = 0;
int n = 0;
#define MAX_LEN 30
void setup()
{
  pinMode(Buzzer, OUTPUT);
  pinMode(Redled, OUTPUT);
  pinMode(Greenled, OUTPUT);
  pinMode(Relay, OUTPUT);
  pinMode(Score, INPUT);
  pinMode(End, INPUT);
  pinMode(Open, OUTPUT);


  Serial.begin(9600);
  SPI.begin();
  pinMode(chipSelectPin, OUTPUT);             // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin
  digitalWrite(chipSelectPin, LOW);         // Activate the RFID reader
  pinMode(NRSTPD, OUTPUT);                    // Set digital pin 10 , Not Reset and Power-down
  digitalWrite(NRSTPD, HIGH);
  myRFID.AddicoreRFID_Init();
  lcd.begin();

  for (int i = 0; i < 3; i++)
  {
    Serial.print("Start");
    lcd.backlight();
    delay(500);
    digitalWrite(Redled, HIGH);

    digitalWrite(Greenled, HIGH);
    delay(500);
    digitalWrite(Redled, LOW);

    digitalWrite(Greenled, LOW);

    lcd.setCursor(1, 0);
    lcd.print ("Product of ASUS");
    lcd.setCursor(2, 1);
    lcd.print (" Smart Lock");

  }
  delay (500);
  lcd.clear ();
  lcd.setCursor(1, 0);
  lcd.print ("Scan Card");

}

/*void gameplay(x) { 
  if (digitalRead(Score) == HIGH) {             
            lcd.clear();
            point = point + 10;
            lcd.print("Score: ");lcd.print(point);
            delay(300);        
            }
  
} */
void loop()
{  
  if (n == 208) {
      if (digitalRead(Score) == HIGH) {             
            lcd.clear();
            point = point + 10;
            lcd.print("Score: ");lcd.print(point);
            delay(300);  
      }    
    if (digitalRead(End) == HIGH) {
    point = 0;
    lcd.clear();
    lcd.print("Score: ");lcd.print(point);  
 }          

 }
  uchar i, tmp, checksum1;
  uchar status;
  uchar str[MAX_LEN];
  uchar RC_size;
  uchar blockAddr;  //Selection operation block address 0 to 63
  String mynum = "";
  str[1] = 0x4400;
  //Find tags, return tag type
  status = myRFID.AddicoreRFID_Request(PICC_REQIDL, str);
  if (status == MI_OK)
  {
    Serial.println("RFID tag detected");
    Serial.print(str[0], BIN);
    Serial.print(" , ");
    Serial.print(str[1], BIN);
    Serial.println(" ");
  }

  //Anti-collision, return tag serial number 4 bytes
  status = myRFID.AddicoreRFID_Anticoll(str);
  if (status == MI_OK)
  {
    checksum1 = str[0] ^ str[1] ;
    Serial.println("The tag's number is  : ");
    //Serial.print(2);
    Serial.print(str[0]);
    Serial.print(" , ");
    Serial.print(str[1], BIN);
    Serial.print(" , ");


    Serial.println(checksum1, BIN);

    // Should really check all pairs, but for now we'll just use the first
    if (str[0] == 152 or str[0] == 208)
    {
      switch (str[0])
      {
        case 152: {
            n = 152;
            lcd.clear ();
            lcd.setCursor(0, 0);
            lcd.print("Access Granted");
            tone(Buzzer, 1500);
            digitalWrite(Greenled, HIGH);
            delay(600);
            noTone(Buzzer);
            digitalWrite(Greenled, LOW);
            lcd.setCursor(0, 1);
            lcd.print ("Welcome Master!");
            digitalWrite(Relay, HIGH);
            delay(1000);
            lcd.clear();
            delay(1000);
            lcd.setCursor(0, 0);
            
            digitalWrite(Open, HIGH);
            lcd.print("Auto lock in 5 seconds");

            delay(5000);
            digitalWrite(Open, LOW);
            lcd.clear();
            delay(800);
            lcd.print("Door Locked");
            digitalWrite(Relay,LOW);
            delay(800);
            lcd.clear();
            delay(800);
            lcd.print("Scan Card");
          }

          break;

        case 208: {
            n = 208;
            lcd.clear ();
            lcd.setCursor(1, 0);
            lcd.print("Access Granted");
            tone(Buzzer, 1500);
            digitalWrite(Greenled, HIGH);
            delay(600);
            noTone(Buzzer);
            digitalWrite(Greenled, LOW);
            lcd.setCursor(2, 1);
            lcd.print ("Let's play!");
            delay(1000);
            lcd.clear();
            delay(800);
            lcd.print("Score: ");lcd.print(0);           
          }
      }
    }
    else {
      n = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Wrong ID");
      tone(Buzzer, 2000);
      digitalWrite(Redled, HIGH);
      delay(500);
      lcd.setCursor(0, 1);
      lcd.print("Access Denied");
      delay(1000);
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Scan Card Again");
      delay(800);
      digitalWrite(Redled, LOW);
      noTone(Buzzer);
    }
       
  }
}



