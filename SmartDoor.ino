
// Library Section
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
 
// Constant Section
#define SS_PIN 10
#define RST_PIN 9
const byte ROWS = 4;
const byte COLS = 4; 

// Tools
 //1. KeyPad
  /// KeyPad Map
char keys[ROWS][COLS] = {
  {'D','#','0','*'},
  {'C','9','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}
};
  /// KeyPad Pins
byte rowPins[ROWS] = {4, 3, 2, 0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 7, 6, 5}; //connect to the column pinouts of the keypad
 /// KeyPad Variables
int counter=0;

// Users Details
char User[][6]={ // Users Name
  {"Ahmed"}, // User 1 ( Card 1, Name Ahmed, Enable)
  {"Ali"},   // User 2 ( Card 2, Name Ali, Enable)
//  {"Max"}    // User 3 ( Card 3, Name Max, Disable)
},
 cardsAccept[][13]={ // Users Card
  {"45 5E 80 20"},// User 1 ( Card 1, Name Ahmed, Enable)
  {"40 B0 02 74"},// User 2 ( Card 2, Name Ali, Enable)
//  {"D0 33 81 7A"} // User 3 ( Card 3, Name Max, Disable)
},
UserPassword[][6]={ // Users Password
  {"C157B"}, // User 1 ( Card 1, Name Ahmed, Enable)
  {"1AC45"}, // User 2 ( Card 2, Name Ali, Enable)
//  {"A12DC"}  // User 3 ( Card 3, Name Max, Disable)
};
 //2. RFID
int cardsNumber = sizeof(cardsAccept) / sizeof (cardsAccept[0]);
 //3. LED
int ledR = A3, ledG = A2;
 //4. Sound
int openSound = A0;

// Class Section
LiquidCrystal_I2C lcd(0x27,16,2);
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
Servo servoOpen;
// Servo servoLock;

void setup() 
{
  // Public 
  Serial.begin(9600);
  
  // RFID
  SPI.begin();      
  mfrc522.PCD_Init(); 
  Serial.println("Approximate your card to the reader...");
  Serial.println();

  // LCD i2c
  lcd.init(); 
  lcd.backlight();

  // Servo Motor
  //servoLock.attach(A0);
  servoOpen.attach(A1);

  // Leds
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);

  // Sound
  pinMode(openSound, OUTPUT);
  
}
void loop() 
{

  // Is Cards Access Authorized Or Not.
  bool access; 
  int myCard;
  
  closeBoth();  // Close Both Door & Lock
  welcomCard(); //Say Hello On LCD
  digitalWrite(ledR,HIGH);
  digitalWrite(ledG,LOW);
  
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
    return;
    
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
    return;

  // Wait Checking Access On LCD
  waitChecking();

  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  for(int i=0; i<cardsNumber; i++){
    if ( content.substring(1) == cardsAccept[i] ){ //change here the UID of the card/cards that you want to give access
      access = true;
      myCard = i;
      break;
    }
    else 
      access =false;
  }
  
  if(access)
    Serial.println(" Authorized access");
  else 
    Serial.println(" Access denied");

  checkAccept(access, myCard);
  Serial.println();
  delay(300);

} 

// Screen Section
void welcomCard(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello There!"); // Print a message to the LCD.
  lcd.setCursor(0,1);
  lcd.print("Slide your card.");
  delay(500);
}

void waitChecking(){
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("When read a card");
  lcd.setCursor(0,0);
  lcd.print("Please Wait."); // Print a message to the LCD.
  delay(500);
  lcd.print('.');
  delay(500);
  lcd.print('.');
  delay(500);
}
void checkAccept(bool access, int userNumber){
    
  // نشغل الشاشة مع الترحيب
  lcd.clear();
  lcd.setCursor(0,0);
  if( access == true ){
    lcd.print(" Ok, Now Write");
    lcd.setCursor(0,1);
    lcd.print(" The Password ");
    delay(1000);
    setPassword(userNumber);
  } else {
    lcd.print(" OPS! ");
    lcd.setCursor(0,1);
    lcd.print(" Card Undefine!");
    delay(1500);

  }
}
void setPassword(int user){
  bool TFP;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(User[user]);
  lcd.print(" Password: ");
  lcd.setCursor(0,1);
  
  TFP = keyProcess(user);
  if(TFP){
    Serial.println("Password True!");    
    openDoorLCD();
  } else {
    Serial.println("Password False!");
    tryAgain(user);
  }
      
}
void tryAgain(int user){
  bool TFP;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(User[user]);
  lcd.print(" Try Again: ");
  lcd.setCursor(0,1);
  
  TFP = keyProcess(user);
  if(TFP){
    Serial.println("Password True!");    
    openDoorLCD();
  } else {
    Serial.println("Password False!");
    tryAgain(user);
  }
}

void openDoorLCD(){

     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Successful!");
     lcd.setCursor(0,1);
     lcd.print("The door opens!");
     digitalWrite(ledG,HIGH);
     digitalWrite(ledR,LOW);
     digitalWrite(openSound,HIGH);
     delay(350);
     digitalWrite(openSound,LOW);
     openDoor();
     
}
// KeyPad Section
bool keyProcess(int userID){
  bool prse=true, check = false;
  char inputPass[15];

  Serial.print("Password: ");
  for(counter = 0; prse; counter++){
    inputPass[counter] = keypad.waitForKey();
    Serial.print(inputPass[counter]);
    if( inputPass[counter] == '#' ){
      prse = false;
      check = checkPass(inputPass, userID);
      break;
    } 
    else if ( inputPass[counter] == '*' ){
      tryAgain(userID);
    }
    else
      lcd.print('*');
  }
  Serial.println();
  
  if( check ){
    Serial.println("Door Unlock!");
    return true;  
  } else {
    Serial.println("Door Still Lock!");
    return false;  
  }
  delay(100);
}

bool checkPass( char pass[], int user ){
   int passCounter=0;
        for(int i=0; i<counter; i++){
          if( pass[i] == UserPassword[user][i] )
            passCounter++;
          else 
            return false;
        }
   if( passCounter == 5 )
      return true;
   else
    return false;
}


// Door Lock
void closeBoth(){

  // Close Both
  //servoLock.write(7); // زاوية الاغلاق لماطور القفل 
  servoOpen.write(180); // زاوية الاغلق للباب

}
void openDoor(){
  
  // Open Lock
  //servoLock.write(180); // زاوية الفتح للقفل
  //delay(1000);

  //Open Door
  servoOpen.write(10); // زاوية الفتح للباب
  delay(5000);
  
  //Close Door
  servoOpen.write(180); // اغلاق الباب
  //delay(1000);

  // Close Lock
  //servoLock.write(7); // اغلاق القفل
  //delay(100);

}
