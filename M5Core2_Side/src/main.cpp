#include <M5Core2.h>
#include <driver/i2s.h>
#include <math.h>

#define MIC_PIN_CLK 0  // マイクのクロックピン
#define MIC_PIN_DAT 34 // マイクのデータピン
#define SAMPLE_RATE 44100

#include <Adafruit_NeoPixel.h>

#define PIN 32
#define NUMPIXELS 24
#define NUMPIXELS 24


Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

uint8_t microphonedata0[1024];


void setup() {
  M5.begin(true, true, true, true, kMBusModeOutput, true);
  M5.Axp.SetSpkEnable(true);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);

  // NeoPixelの初期化
  pixels.begin();
  pixels.setBrightness(30);
}

void loop() {
  M5.update();

  uint32_t audioSum = 0;
  size_t audioCount = 0;

  M5.Spk.InitI2SSpeakOrMic(MODE_MIC);

  while (true) {
    size_t byte_read;
    i2s_read(Speak_I2S_NUMBER, (char *)(microphonedata0), 1024, &byte_read, portMAX_DELAY);

    for (size_t i = 0; i < byte_read; i += 2) {
      int16_t sample = microphonedata0[i] | (microphonedata0[i + 1] << 8);

      // デジタル->アナログ変換
      audioSum += sample * sample;
      audioCount++;

      if (audioCount >= 1024) {
        // デシベルに変換
        float rms = sqrtf((float)audioSum / audioCount);
        float soundLevel = 20 * log10(rms);

        // デシベルの値を表示
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setCursor(10, 10);
        M5.Lcd.printf("Sound Level (dB): %.2f", soundLevel);

        // NeoPixelの色設定
        int r, g, b;


        
        int lightPower = map(soundLevel, 50, 80, 0, 150);

        pixels.setBrightness(lightPower);

        for (int j = 0; j < NUMPIXELS; j++) {
          pixels.setPixelColor(j, pixels.Color(255, 255, 255));
        }
        pixels.show();

        // リセット
        audioSum = 0;
        audioCount = 0;
      }
    }
  }
}
