#include <Arduino.h>
#include <WiFi.h>
#define PIEZO_A 32
#define LED_BUILTIN 2
#define ADC_RESOLUTION 4095

hw_timer_t * timer = NULL;
const int bufferSize = 50;
volatile int analogBuffer[bufferSize];
volatile int bufferIndex = 0;

void IRAM_ATTR myTimerEvent() {
  // Read analog value and store it in the circular buffer and use the quick read methods
  int analogValue = analogRead(PIEZO_A);
  analogBuffer[bufferIndex] = analogValue;

  // Move to the next buffer index and wrap if necessary
  bufferIndex = (bufferIndex + 1) % bufferSize;

  // digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); 
}

const int sample_interval = 500; // .5ms

void setup() {
  // Disable wifi and bluetooth
  WiFi.mode(WIFI_OFF);
  btStop();

  // Initialize serial
  Serial.begin(115200);
  pinMode(PIEZO_A, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  timer = timerBegin(0, 80, true); // Timer 0, prescaler 80, counting up
  timerAttachInterrupt(timer, &myTimerEvent, true);
  timerAlarmEnable(timer);
}

static int lastPrintTime = 0;

void loop() {
  static int lastProcessedIndex = 0;

  // Check if new data is available in the buffer
  if (lastProcessedIndex != bufferIndex) {
    while (lastProcessedIndex != bufferIndex && (millis() - lastPrintTime) > 1000){
      // Print entire buffer
      for (int i = 0; i < bufferSize; i++) {
        // Convert adc value to voltage
        float voltage = analogBuffer[i] * (3.3 / ADC_RESOLUTION);

        Serial.print(voltage);
        Serial.print(",");
      }
      Serial.println();
      lastPrintTime = millis();
      
      // Move to the next buffer index and wrap if necessary
      lastProcessedIndex = (lastProcessedIndex + 1) % bufferSize;
    }
  }

}
