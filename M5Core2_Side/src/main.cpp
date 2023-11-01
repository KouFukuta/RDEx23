//ここから外付けマイクユニットを使ったプログラム

#include <M5Core2.h>
#include "arduinoFFT.h"

#define MIC 33

#define SAMPLING_FREQUENCY 40000
const uint16_t FFTsamples = 256;  // 2のべき乗を入れる

double vReal[FFTsamples];  // vReal[]にサンプリングしたデーターを入れる
double vImag[FFTsamples];
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFTsamples, SAMPLING_FREQUENCY);  // FFTオブジェクトを作る

unsigned int sampling_period_us;

//マイクから音声を取得
void sample(int nsamples) {
    for (int i = 0; i < nsamples; i++) {
        unsigned long t = micros();
        vReal[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132; // ESP32のADCの特性を補正
        vImag[i] = 0;
        while ((micros() - t) < sampling_period_us) ;
    }
}

//描画位置指定
int X0 = 30;
int Y0 = 20;

//描画領域の高さ、幅を指定
int _height = 240 - Y0;
int _width = 320;

//描画する高さを調整
float dmax = 5.0;

int maxBand = 0;
float maxAmplitude = 0;
float maxFrequency = 0;

//FFT描画
void drawChart(int nsamples) {
    int band_width = floor(_width / nsamples);
    int band_pad = band_width - 1;

    for (int band = 0; band < nsamples; band++) {
        int hpos = band * band_width + X0;
        float d = vReal[band];
        
        if (d > dmax)d = dmax; 
        
        int h = (int)((d / dmax) * (_height));
        M5.Lcd.fillRect(hpos, _height - h, band_pad, h, WHITE);
        
        if(d > maxAmplitude){
          maxAmplitude = d;
          maxBand = band;
        }


        if ((band % (nsamples / 4)) == 0) {
            M5.Lcd.setCursor(hpos, _height + Y0 - 10);
            M5.Lcd.printf("%.1fkHz", ((band * 1.0 * SAMPLING_FREQUENCY) / FFTsamples / 1000));

            //一番高い振幅を持つ周波数を計算
            maxFrequency = (maxBand * 1.0 * SAMPLING_FREQUENCY) / FFTsamples / 1000;
        }
    }
}


void setup() {
    M5.begin();
    Serial.begin(115200);
    while (!Serial) ;
    M5.lcd.setBrightness(20);

    pinMode(MIC, INPUT);

    sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}


//ノイズ軽減
void DCRemoval(double *vData, uint16_t samples) {
    double mean = 0;
    for (uint16_t i = 1; i < samples; i++) {
        mean += vData[i];
    }
    mean /= samples;
    for (uint16_t i = 1; i < samples; i++) {
        vData[i] -= mean;
    }
}

void loop() {
    sample(FFTsamples);

    DCRemoval(vReal, FFTsamples);
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // 窓関数
    FFT.Compute(FFT_FORWARD); // FFT処理(複素数で計算)
    FFT.ComplexToMagnitude(); // 複素数を実数に変換
    M5.Lcd.fillScreen(BLACK);
    drawChart(FFTsamples / 2);

    Serial.printf("%fkHz\n",maxFrequency);
}


/*

以下内臓マイクを使用していたプログラム
感度の問題で要調整

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
*/

