//送信側、Wi-Fiを発信する側
//本体にはマイクをつける
//FFTのPeakを送信する,RSSIを受信する
//白いほうに書き込む

#include <Arduino.h>
#include <M5Core2.h>
#include <Ethernet.h>
#include <ArduinoOSCWiFi.h>
#include "arduinoFFT.h"


//ここからFFTのやつ
//外付けマイクのピン指定
#define MIC 33
#define DIG_MIC 32


#define SAMPLING_FREQUENCY 10000  //ここがおかしいと取れる周波数もおかしくなる
const uint16_t FFTsamples = 256;  // 2のべき乗を入れる


double vReal[FFTsamples];  // vReal[]にサンプリングしたデーターを入れる
double vImag[FFTsamples];
arduinoFFT FFT = arduinoFFT(vReal, vImag, FFTsamples, SAMPLING_FREQUENCY);  // FFTオブジェクトを作る

unsigned int sampling_period_us;

//マイクから音声を取得
void sample(int nsamples) {
    for (int i = 0; i < nsamples; i++) {
        unsigned long t = micros();
        //vReal[i] = (double)analogRead(MIC) / 4095.0 * 3.6 + 0.1132; // ESP32のADCの特性を補正
        vReal[i] = (double)analogRead(MIC);
        vImag[i] = 0;
        while ((micros() - t) < sampling_period_us) ;
    }
}

/* 
//FFT描画
void drawChart(int nsamples) {
    int band_width = floor(_width / nsamples);
    int band_pad = band_width - 1;

    for (int band = 0; band < nsamples; band++) {
        int hpos = band * band_width + X0;
        float d = vReal[band];
        
        if (d > dmax)d = dmax; 
        
        int h = (int)((d / dmax) * (_height));

        //描画系のひとたちを黙らせてる
        //M5.Lcd.fillRect(hpos, _height - h, band_pad, h, WHITE);


         if ((band % (nsamples / 4)) == 0) {
            M5.Lcd.setCursor(hpos, _height + Y0 - 10);
            M5.Lcd.printf("%.1fkHz", ((band * 1.0 * SAMPLING_FREQUENCY) / FFTsamples / 1000));

        } 
    }
}

*/

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

//ここまでFFTのやつ


//ここからWi-Fi用の設定
const char *ssid = "M5_Send01";         // SSID
const char *pass = "sendSide01"; // passwordは自由に設定してください
const int incomingPort = 7070;       // 受信ポート番号
const int outgoingPort = 8080;       // 送信ポート番号

const IPAddress ipServer(192, 168, 1, 1);   // server IPアドレス
const IPAddress ipGateway(192, 168, 1, 1);  // gateway IPアドレス
const IPAddress subnet(255, 255, 255, 0);   // サブネットマスク
const IPAddress ipClient(192, 168, 1, 255); // client IPアドレス

const char *host = "192.168.1.255";

//ここまでWi-Fiのやつ

void rcv_rssi(const OscMessage &msg) //RSSIを受信した時に起動する関数です。
{
  int rssi = msg.arg<int>(0);

  M5.Lcd.setCursor(0, 195);
  M5.Lcd.printf("rssi: %d", rssi); //受信したRSSI値を画面に表示します

}

void setup()
{
  M5.begin();
  M5.Lcd.clearDisplay();
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);

  pinMode(DIG_MIC, INPUT);

  WiFi.softAP(ssid, pass);
  delay(100);
  WiFi.softAPConfig(ipServer, ipGateway, subnet);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("ssid: %s", ssid);
  M5.Lcd.setCursor(0, 65);
  M5.Lcd.printf("incomingPort: %d", incomingPort);
  M5.Lcd.setCursor(0, 105);
  M5.Lcd.printf("outgoingPort: %d", outgoingPort);
  delay(1000);

  OscWiFi.subscribe(incomingPort, "/rssi", rcv_rssi); //この記述で、Recieve側からRSSI値が送られてきた時にrcv_rssi関数が起動するようになります。

}

void loop()
{
  //ここからFFT
  sample(FFTsamples);

  DCRemoval(vReal, FFTsamples);
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // 窓関数
  FFT.Compute(FFT_FORWARD); // FFT処理(複素数で計算)
  FFT.ComplexToMagnitude(); // 複素数を実数に変換
  //M5.Lcd.fillScreen(BLACK);
  //drawChart(FFTsamples / 2);

  //ピークの値を取り出す
  double peak = FFT.MajorPeak(vReal, FFTsamples, SAMPLING_FREQUENCY);

  int dig_res = digitalRead(DIG_MIC);

  M5.lcd.setCursor(0, 155);
  M5.Lcd.printf("Peak: %2lf %1d", peak, dig_res);
  OscWiFi.send(host, outgoingPort, "/peak", peak);
  OscWiFi.post();
  OscWiFi.send(host, outgoingPort, "/dig_res", dig_res);//ピーク値を常に送信
  OscWiFi.post();

  
  OscWiFi.parse(); //何もない場合はRecieveからの通信を待ち続けます。

  //delay(50);
}