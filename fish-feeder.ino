#include <Servo.h>

// Set the number of milliseconds interval between fish feeds
const long FISH_FEED_INTERVAL_MILLIS = 30000;

// Set the Digital I/O pin that the servo control wire is connected to
const int SERVO_PIN = 0;

// Set the speed that the servo should rotate at once the feed starts and the duration in milliseconds
// that the servo should rotate at this speed for (these two values need to be calibrated so that the 
// combination of these values produces a result where the servo completes one exact rotation)
const int SERVO_ROTATION_SPEED = 110;
const int SERVO_ROTATION_DURATION = 2130;

// Set the off position that will need to be written in order for the servo to stop rotation
const int SERVO_STOP_POSITION = 92;

// Set the Digital I/O pin that the status LED is connected to
const int STATUS_LED_PIN = 8;

// Set the duration in milliseconds that the status led should remain on for when a status blink
// takes place
const int STATUS_LED_BLINK_DURATION = 50;

// Set the default interval between status led blinks
const int STATUS_LED_BLINK_DEFAULT_INTERVAL = 5000;

bool isFishFeedInProgress = false;
unsigned long fishFeedStartMillis = 0;
unsigned long previousFishFeedMillis = 0;

bool isStatusLedOn = false;
unsigned long previousLedOnMillis = 0;
unsigned long statusLedBlinkInterval = 5000;

Servo servo;

void setup() {
  Serial.begin(9600);
  pinMode(STATUS_LED_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
}

void loop() {
  unsigned long currentMillis = millis();

  blinkStatusLight(currentMillis);
  evaluateFishFeed(currentMillis);
}

void blinkStatusLight(unsigned long currentMillis) {
  // Status light needs to be solid on when feeding is in progress so don't touch the light when a feed is underway
  if (isFishFeedInProgress) {
    return;
  }

  if (!isStatusLedOn) {
    if ((currentMillis - previousLedOnMillis) >= statusLedBlinkInterval) {
      previousLedOnMillis = currentMillis;
      digitalWrite(STATUS_LED_PIN, HIGH);
      isStatusLedOn = true;
    }
  } else {
    if ((currentMillis - previousLedOnMillis) >= STATUS_LED_BLINK_DURATION) {
      digitalWrite(STATUS_LED_PIN, LOW);
      isStatusLedOn = false;
    }
  }
}

void evaluateFishFeed(unsigned long currentMillis) {
  if (!isFishFeedInProgress) {
    if ((currentMillis - previousFishFeedMillis) >= FISH_FEED_INTERVAL_MILLIS) {
      previousFishFeedMillis = currentMillis;
      fishFeedStartMillis = currentMillis;
      startFishFeed();
    }
  } else {
    if ((currentMillis - fishFeedStartMillis) >= SERVO_ROTATION_DURATION) {
      stopFishFeed();
    }
  }
}

void startFishFeed() {
  Serial.println("Start fish feed");
  servo.write(SERVO_ROTATION_SPEED);
  digitalWrite(STATUS_LED_PIN, HIGH);
  isFishFeedInProgress = true;
  isStatusLedOn = true;
}

void stopFishFeed() {
  Serial.println("Stop fish feed");
  servo.write(SERVO_STOP_POSITION);
  digitalWrite(STATUS_LED_PIN, LOW);
  isFishFeedInProgress = false;
  isStatusLedOn = false;
  statusLedBlinkInterval = STATUS_LED_BLINK_DEFAULT_INTERVAL;
}
