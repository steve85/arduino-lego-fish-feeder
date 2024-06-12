#include <Servo.h>

// Set this to true if the feeder should run immediately once plugged in and then follow the value
// from FISH_FEED_INTERVAL_MILLIS afterwards
const bool IS_IMMEDIATE_START = true;

// SET the delay in miliiseconds before the immediate start execution starts place (useful when you don't
// want the feeder to run instantly when it is plugged in).
const int IMMEDIATE_START_DELAY_MILLIS = 10000;

// Set the number of milliseconds interval between fish feeds
const long FISH_FEED_INTERVAL_MILLIS = 30000;

// Set the Digital I/O pin that the servo control wire is connected to
const int SERVO_PIN = 9;

// Set the speed that the servo should rotate at once the feed starts and the duration in milliseconds
// that the servo should rotate at this speed for (these two values need to be calibrated so that the 
// combination of these values produces a result where the servo completes one exact rotation)
const int SERVO_ROTATION_SPEED = 110;
const int SERVO_ROTATION_DURATION = 2450;

// Set the off position that will need to be written in order for the servo to stop rotation
const int SERVO_STOP_POSITION = 92;

// Set the Digital I/O pin that the status LED is connected to
const int STATUS_LED_PIN = 8;

// Set the duration in milliseconds that the status led should remain on for when a status blink
// takes place
const int STATUS_LED_BLINK_DURATION = 50;

// Set the default interval between status led blinks
const int STATUS_LED_BLINK_DEFAULT_INTERVAL = 5000;

bool isFirstRun = true;
bool isFishFeedInProgress = false;
int fishFeedCount = 0;
unsigned long fishFeedStartMillis = 0;
unsigned long previousFishFeedMillis = 0;

bool isStatusLedOn = false;
unsigned long previousLedOnMillis = 0;
unsigned long statusLedBlinkInterval = 5000;

// List of intervals in milliseconds from the next feed at which the led status light blink frequency
// will be increased. Needs to be stored in ascending order for calculation to work correctly.
int increasingLedStatusIntervals[3] = { 2500, 5000, 10000 };

Servo servo;

void setup() {
  // Setup I/O
  Serial.begin(9600);
  pinMode(STATUS_LED_PIN, OUTPUT);
  servo.attach(SERVO_PIN);

  // Before entering the main loop illuminate the status led for 5 seconds
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(5000);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  // If immediate start is set to true and this if the first run then set the previousFishFeedMillis
  // so that it will trigger a feed on the first loop.
  if (IS_IMMEDIATE_START && isFirstRun) {
    previousFishFeedMillis = ((FISH_FEED_INTERVAL_MILLIS * -1) + IMMEDIATE_START_DELAY_MILLIS);
    isFirstRun = false;
  }

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
      calculateNextBlinkInterval(currentMillis);
    }
  }
}

void calculateNextBlinkInterval(unsigned long currentMillis) {
  // Caculate the duration until the next feed
  long durationUntilNextFeed = (FISH_FEED_INTERVAL_MILLIS - (currentMillis - previousFishFeedMillis));

  // Calculate the size of the array containing the dynamic interval data
  size_t intervalArrLen = sizeof(increasingLedStatusIntervals)/sizeof(increasingLedStatusIntervals[0]);

  // As the time until the duration increases, decrease the interval at which the status led blinks
  for (size_t i = 0; i < intervalArrLen; i++) {
    int interval = increasingLedStatusIntervals[i];
    if (durationUntilNextFeed <= interval) {
      statusLedBlinkInterval = (interval / 10);
      break;
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
  fishFeedCount++;
  Serial.print("Fish Feed Start - Count: ");
  Serial.println(fishFeedCount);
  servo.write(SERVO_ROTATION_SPEED);
  digitalWrite(STATUS_LED_PIN, HIGH);
  isFishFeedInProgress = true;
  isStatusLedOn = true;
}

void stopFishFeed() {
  Serial.println("Fish Feed Complete");
  servo.write(SERVO_STOP_POSITION);
  digitalWrite(STATUS_LED_PIN, LOW);
  isFishFeedInProgress = false;
  isStatusLedOn = false;

  // Reset the status led blink interval to the default value
  statusLedBlinkInterval = STATUS_LED_BLINK_DEFAULT_INTERVAL;
}
