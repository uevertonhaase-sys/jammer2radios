#include <SPI.h>
#include "RF24.h"
#include "esp_bt.h"
#include "esp_wifi.h"

static SPIClass hspi(HSPI);

// CE, CSN, SPI speed
RF24 radio(16, 15, 10000000);

static uint8_t channel = 45;
static bool ascending = true;

constexpr uint8_t CHANNEL_MIN = 2;
constexpr uint8_t CHANNEL_MAX = 79;
constexpr uint8_t CHANNEL_STEP = 2;

void disableEspRadios() {
  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
}

bool initRadio() {
  hspi.begin();

  if (!radio.begin(&hspi)) {
    Serial.println("RF24 init failed");
    return false;
  }

  delay(200);
  Serial.println("RF24 started");

  radio.stopListening();
  radio.setPayloadSize(5);
  radio.setAddressWidth(3);
  radio.setPALevel(RF24_PA_MAX, true);
  radio.setDataRate(RF24_2MBPS);

  radio.startConstCarrier(RF24_PA_MAX, channel);
  radio.printPrettyDetails();

  return true;
}

void hopChannel() {
  if (ascending) {
    if (channel + CHANNEL_STEP > CHANNEL_MAX) {
      ascending = false;
      channel -= CHANNEL_STEP;
    } else {
      channel += CHANNEL_STEP;
    }
  } else {
    if (channel < CHANNEL_MIN + CHANNEL_STEP) {
      ascending = true;
      channel += CHANNEL_STEP;
    } else {
      channel -= CHANNEL_STEP;
    }
  }

  radio.setChannel(channel);
}

void setup() {
  Serial.begin(115200);
  disableEspRadios();

  if (!initRadio()) {
    while (true) {
      delay(1000);
    }
  }
}

void loop() {
  hopChannel();
  delayMicroseconds(200);
}