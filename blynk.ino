#include <SPI.h>
#include <MFRC522.h>

int Count = 0;
String warning = " ";
String message = " ";
int authorized = 0;
int secure = 0;
int lock = 0;
int Distance = 30;
int trigPin_1 = 13;
int echoPin_1 = 12;
int trigPin_2 = 33;
int echoPin_2 = 32;
 
#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "<SENSITIVE_DATA>" // Not uploading this on github :)
#define BLYNK_DEVICE_NAME "Bidirectional"

#define BLYNK_FIRMWARE_VERSION "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

#include "BlynkEdgent.h"

BLYNK_WRITE(V0){
  int pinValue = param.asInt();
  if(pinValue == 1){
    message = "Event set to private!";
    secure = pinValue;
  }else{
    message = "Event set to public!";
  }
}

BLYNK_WRITE(V3){
  int lockPin = param.asInt();
  lock = lockPin;
}

BlynkTimer timer; 

// Publish counter value to blynk
void counter() 
{
  // This function describes what will happen with each timer tick
  // i.e. writing sensor value to datastream V1
  Blynk.virtualWrite(V1, Count);  
  
}

// Publish warning messages to blynk
void wrn() 
{
  // This function describes what will happen with each timer tick
  // i.e. writing sensor value to datastream V2
  Blynk.virtualWrite(V2, warning);  
  
}

// Publish promept messages to blynk
void msg() 
{
  // This function describes what will happen with each timer tick
  // i.e. writing sensor value to datastream V4
  Blynk.virtualWrite(V4, message);  
  
}

// Clear message/warning for prompt effect
void clr() 
{
  // This function describes what will happen with each timer tick
  // i.e. writing sensor value to datastream V2
  if(warning != " "){
    Blynk.virtualWrite(V2, " ");
    warning = " "; 
  }

  if(message != " "){
    Blynk.virtualWrite(V4, " ");
    message = " "; 
  }
}

// Function for reading ultrasonic sensor distance value
long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

void setup()
{
  Serial.begin(9600);   // Initiate a serial communication
  SPI.begin();      // Initiate  SPI bus
  pinMode(LED_BUILTIN, OUTPUT); // Builtin LED light(Blue)
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  // Using timer to avoid publishing info frequently, that may result in blynk servers blocking us 
  timer.setInterval(1000L, counter);
  timer.setInterval(1000L, wrn);
  timer.setInterval(1000L, msg);
  timer.setInterval(5000L, clr);

  BlynkEdgent.begin();
}

void loop() {
  // Check if lock button is pushed or not, send appropriate message accordingly
  if(lock == 1){
    if(Count > 0){
      lock = 0;
      Blynk.virtualWrite(V3, lock);
      warning = "Can't lock, someone is still inside!";
    }else{
      message = "Door is currently locked!";
    }
  }
  BlynkEdgent.run();
  timer.run();

  // When Person enters
  while(0.01723 * readUltrasonicDistance(trigPin_1, echoPin_1) < Distance){
    if(secure == 1){
      if(authorized == 1){
        Count = (Count + 1);
        authorized = 0;
        Serial.print("Count: ");
        Serial.println(Count);
        message = "Authorized!";
      }else{
        Serial.println("Non authorized entry"); 
        warning = "Non-authorized entry";
      }
    }else{
      Count = (Count + 1);
      Serial.print("Count: ");
      Serial.println(Count);
    }

    while(0.01723 * readUltrasonicDistance(trigPin_1, echoPin_1) < Distance){
      delay(500);
    }
  }

  // When Person exits
  while(0.01723 * readUltrasonicDistance(trigPin_2, echoPin_2) < Distance){
    if(Count > 0){
      Count = (Count - 1); 
    }
    Serial.print("Count: ");
    Serial.println(Count);

    while(0.01723 * readUltrasonicDistance(trigPin_2, echoPin_2) < Distance){
      delay(500);
    }
  }
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
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
  
  if (content.substring(1) == "83 58 9B 94") //change here the UID of the card/cards that you want to give access
  {
    // Special effect for access approved
    Serial.println("Authorized access");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1500);
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println();
    authorized = 1;
  }
 
 else   {
    // Special effect for access denial
    Serial.println(" Access denied");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
 }
}
