/* 
スピーカー側デバイスを子機にする試み
Wi-Fiを繋ぎなおすタイムラグが減るのではないか
ESP32に書き込む
*/

#include <Arduino.h>
#include <Esp.h>
#include "arduinoFFT.h"
#include <WiFi.h>
//#include <WiFiUdp.h>
#include <ArduinoOSCWiFi.h>

//個体番号
int Number = 0;


//Wi-Fiの設定
const char *ssid = "M5_Send_Temochi";
const char *pass = "sendSide_Temochi";
const IPAddress ip(192, 168, 1, 201);
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);

//ArduinoOSC
const char *host = "192.168.1.255"; // ネットワーク内に送信

const int incomingPort = 8080; // 受信ポート番号
const int outgoingPort = 7070; // 送信ポート番号

//ここからFFT関連
#define MIC 36
#define DIG_MIC 15

#define SAMPLING_FREQUENCY 10000 //ここの値が大きいとおかしくなる
const uint16_t FFTsamples = 256; //2のべき乗を入れるらしい

double vReal[FFTsamples];  // vReal[]にサンプリングしたデーターを入れる
double vImag[FFTsamples];
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFTsamples, SAMPLING_FREQUENCY);  // FFTオブジェクトを作る

unsigned int sampling_period_us;

//マイクから音声を取得
void sample(int nsamples) {
    for (int i = 0; i < nsamples; i++) {
        unsigned long t = micros();
        vReal[i] = (double)analogRead(MIC);
        vImag[i] = 0;
        while ((micros() - t) < sampling_period_us) ;
    }
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

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  delay(1000);
}

void loop()
{
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Wi-Fi Disconnect");
    WiFi.disconnect();
    WiFi.begin(ssid, pass);
    delay(1000);
  }

  //ここからFFT 
  sample(FFTsamples);
  DCRemoval(vReal, FFTsamples);
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // 窓関数
  FFT.Compute(FFT_FORWARD); // FFT処理(複素数で計算)
  FFT.ComplexToMagnitude(); // 複素数を実数に変換

  //ピークの値を取り出す
  double peak = FFT.MajorPeak(vReal, FFTsamples, SAMPLING_FREQUENCY);
  Serial.printf("peak: %lf\n", peak);

  //デジタルマイクの値を取得
  int dig_res = digitalRead(DIG_MIC);
  Serial.printf("digitalMic: %d\n", dig_res);

  //ここからRSSI取得
  int rssi = 0;
  for(int i = 0; i < 100; i++)
  {
    rssi += WiFi.RSSI();
  }
  rssi /= -100;
  Serial.printf("rssi: %d\n", rssi);

  //ここからデータ送信
  OscWiFi.send(host, outgoingPort, "/data", Number, rssi, dig_res, peak);
  OscWiFi.post();

}