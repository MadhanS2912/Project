#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// GPS module connected to pins 5 (TX) and 3 (RX)
SoftwareSerial gpsSerial(5, 6);
TinyGPSPlus gps;

// GSM module connected to pins 9 (TX) and 10 (RX)
SoftwareSerial gsm(9, 10);

// Ultrasonic Sensor
const int trigPin = 4;
const int echoPin = 3;

void setup() {
    Serial.begin(9600);
    gpsSerial.begin(9600);
    gsm.begin(9600);
    
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    Serial.println("Initializing GPS and GSM Modules...");
    delay(500);
    
    gsm.println("AT"); // Check module communication
    delay(500);
    Serial.println("Checking GSM response...");
    if (gsm.available()) Serial.println(gsm.readString());
    
    gsm.println("AT+CMGF=1"); // Set SMS mode to text
    delay(500);
}

void loop() {
    // Read GPS Data
    while (gpsSerial.available()) {
        char c = gpsSerial.read();
        gps.encode(c);
    }
    
    if (gps.location.isValid()) {
        Serial.print("Latitude: ");
        Serial.println(gps.location.lat(), 6);
        Serial.print("Longitude: ");
        Serial.println(gps.location.lng(), 6);
    } else {
        Serial.println("Waiting for GPS signal...");
    }
    
    // Read Ultrasonic Sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH);
    float distance = duration * 0.0343 / 2;
    
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    
    // If distance is below a threshold, send an emergency alert with location
    if (distance < 200 && gps.location.isValid()) { // Adjust threshold as needed
        sendEmergencyAlert();
    }
    
    delay(1000);
}

void sendEmergencyAlert() {
    for (int i = 1; i <= 5; i++) { // Send 5 SMS messages
        Serial.print("Sending SMS ");
        Serial.println(i);
        
        gsm.println("AT+CMGS=\"+918667882322\""); // Change to recipient number
        delay(500);
        
        gsm.print("Emergency! My location: ");
        gsm.print("https://www.google.com/maps/search/?api=1&query=");
        gsm.print(gps.location.lat(), 6);
        gsm.print(",");
        gsm.println(gps.location.lng(), 6);
        
        gsm.write(26); // ASCII code for CTRL+Z to send SMS
        delay(3000);
        
        Serial.println("SMS Sent Successfully!");
    }
    
    Serial.println("All SMS sent. Initiating 30-second call...");
    makeCall();
}

void makeCall() {
    Serial.println("Calling...");
    gsm.println("ATD+918667882322;"); // Dial the number
    delay(30000); // Call duration
    gsm.println("ATH"); // Hang up the call
    Serial.println("Call Ended.");
}
