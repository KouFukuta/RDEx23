#include <M5Core2.h>
#include "arduinoFFT.h"
#include <Ethernet.h>
#include <ArduinoOSCWiFi.h>


// Wi-Fiの設定
// RSSIを読み取るため
const char *ssid = "M5_Send";  // SSID
const char *pass = "M5rssiii"; // passwordは自由に設定してください
const int incomingPort = 7070; // 受信ポート番号
const int outgoingPort = 8080; // 送信ポート番号

const IPAddress ipServer(192, 168, 1, 1);   // server IPアドレス
const IPAddress ipGateway(192, 168, 1, 1);  // gateway IPアドレス
const IPAddress subnet(255, 255, 255, 0);   // サブネットマスク
const IPAddress ipClient(192, 168, 1, 255); // client IPアドレス

const char *host = "192.168.1.255";

// RSSIを受信した時に起動する関数です。
void rcv_rssi(const OscMessage &msg)
{
  int rssi = msg.arg<int>(0);

  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("rssi: %d", rssi); // 受信したRSSI値を画面に表示します
}


// マイクのピンを定義
// FFTの設定
#define MIC 33
#define SAMPLING_FREQUENCY 10000
const uint16_t FFTsamples = 256; // 2のべき乗を入れる

double vReal[FFTsamples]; // vReal[]にサンプリングしたデーターを入れる
double vImag[FFTsamples];
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFTsamples, SAMPLING_FREQUENCY); // FFTオブジェクトを作る

unsigned int sampling_period_us;

// マイクから音声を取得
void sample(int nsamples)
{
  for (int i = 0; i < nsamples; i++)
  {
    unsigned long t = micros();
    vReal[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132; // ESP32のADCの特性を補正
    vImag[i] = 0;
    while ((micros() - t) < sampling_period_us)
      ;
  }
}

// 描画位置指定
int X0 = 30;
int Y0 = 20;

// 描画領域の高さ、幅を指定
int _height = 240 - Y0;
int _width = 320;

// 描画する高さを調整
float dmax = 5.0;

// FFT & 描画
void drawChart(int nsamples)
{
  int band_width = floor(_width / nsamples);
  int band_pad = band_width - 1;

  for (int band = 0; band < nsamples; band++)
  {
    int hpos = band * band_width + X0;
    float d = vReal[band];

    if (d > dmax)
      d = dmax;

    int h = (int)((d / dmax) * (_height));
    M5.Lcd.fillRect(hpos, _height - h, band_pad, h, WHITE);

    if ((band % (nsamples / 4)) == 0)
    {
      M5.Lcd.setCursor(hpos, _height + Y0 - 10);
      M5.Lcd.printf("%.1fkHz", ((band * 1.0 * SAMPLING_FREQUENCY) / FFTsamples / 1000));
    }
  }
}

void setup()
{
  M5.begin();
  Serial.begin(115200);

  WiFi.softAP(ssid, pass);
  delay(100);
  WiFi.softAPConfig(ipServer, ipGateway, subnet);

  delay(1000);
  OscWiFi.subscribe(incomingPort, "/rssi", rcv_rssi);

  while (!Serial)
    ;
  M5.lcd.setBrightness(20);

  pinMode(MIC, INPUT);

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

// ノイズ軽減
void DCRemoval(double *vData, uint16_t samples)
{
  double mean = 0;
  for (uint16_t i = 1; i < samples; i++)
  {
    mean += vData[i];
  }
  mean /= samples;
  for (uint16_t i = 1; i < samples; i++)
  {
    vData[i] -= mean;
  }
}

void loop()
{

  OscWiFi.parse();
  sample(FFTsamples);

  DCRemoval(vReal, FFTsamples);
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD); // 窓関数
  FFT.Compute(FFT_FORWARD);                        // FFT処理(複素数で計算)
  FFT.ComplexToMagnitude();                        // 複素数を実数に変換
  M5.Lcd.fillScreen(BLACK);
  drawChart(FFTsamples / 2);

  // ピークの値を取り出す
  double x = FFT.MajorPeak(vReal, FFTsamples, SAMPLING_FREQUENCY);

  int peak = x;
  Serial.printf("%dHz\n", peak);

  M5.Lcd.setCursor(0, 40);
  M5.Lcd.printf("peak: %d", peak);

  delay(10);
}