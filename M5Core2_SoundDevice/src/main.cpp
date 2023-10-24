#include <M5Core2.h>

#define LGFX_AUTODETECT         // 自動認識
#define LGFX_USE_V1             // LovyanGFX v1.0.0を有効に
#include <LovyanGFX.hpp>        // LovyanGFXのヘッダを準備
#include <LGFX_AUTODETECT.hpp>  // クラス"LGFX"を用意する

#include <Adafruit_NeoPixel.h>  //NeoPixelライブラリ

//NeoPixelピン指定
#define PIN 32          //INピン指定
#define NUMPIXELS 24    //LEDの個数を指定
#define DELAYVAL 500

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //800kHzでNeoPixelを駆動

static LGFX lcd;                  // LGFXのインスタンスを作成
static LGFX_Sprite sprite(&lcd);  // スプライトを使う場合はLGFX_Spriteのインスタンスを作成

uint8_t microphonedata0[1024];
int data_offset = 0;


int r = 0;
int g = 0;
int b = 0;

//ディスプレイ初期設定
void DisplayInit() {
    lcd.init();
    sprite.setColorDepth(8);  // カラーモード設定
    sprite.createSprite(lcd.width(), lcd.height());  // メモリ描画領域を設定（画面サイズに設定）

    sprite.fillScreen(BLACK);
}


void setup() {
    M5.begin(true, true, true, true, kMBusModeOutput, true);
    M5.Axp.SetSpkEnable(true);
    DisplayInit();
    M5.Lcd.setTextColor(WHITE);
    delay(100);

    pixels.begin();
    pixels.setBrightness(30);
}

void loop() {
    M5.update();
    uint32_t audioValue = 0;
    uint32_t audioSum = 0;
    size_t audioCount = 0;

    //この値超えたらディスプレイ色変更
    int changeRed = 575;
    int changeYellow = 525;
    int changeGreen = 475;
    int changeCyan = 425;


    M5.Spk.InitI2SSpeakOrMic(MODE_MIC);

    pixels.clear(); //NeoPixel出力リセット


    while (true) {
        size_t byte_read;
        i2s_read(Speak_I2S_NUMBER, (char *)(microphonedata0), 1024, &byte_read, portMAX_DELAY);
        data_offset += byte_read;

        for (size_t i = 0; i < byte_read; i += 2) {
            int16_t sample = microphonedata0[i] | (microphonedata0[i + 1] << 8);
            
            //デジタル->アナログ変換
            audioValue += abs(sample);
            audioCount++;

            if (audioCount >= 1024) {
                // Calculate the average 
                uint32_t audioLevel = audioValue / audioCount;

                // Print the audio level to the Serial Monitor
                Serial.println("Sound Level: " + String(audioLevel));

                // LED色可変
                if (audioLevel <= 341)
                {                         // アナログ値が341以下なら赤、緑指定
                    g = (audioLevel / 1.337); // 緑を指定（0～255へ換算）
                    r = 255 - g;          // 緑が明るくなったら赤を暗く

                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);

                }
                else if (audioLevel > changeCyan && audioLevel <= changeGreen)
                {
                    // アナログ値が341より大きく682以下なら青、緑指定
                    b = ((audioLevel - 341) / 1.337); // 青を指定（0～255へ換算）
                    g = 255 - b;                  // 青が明るくなったら緑を暗く

                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);

                }
                else if (audioLevel > changeRed)
                {                                 // アナログ値が682より大きければ青、赤指定
                    r = ((audioLevel - changeRed) / 1.337); // 赤を指定（0～255へ換算）
                    b = 255 - r;                  // 赤が明るくなったら青を暗く

                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);
                }


                /* // Change display color to red if audio level exceeds the threshold
                if(audioLevel > changeRed) {
                    sprite.fillScreen(RED);
                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);

                    for(int j=0; j<NUMPIXELS; j++) {
                        pixels.setPixelColor(j, pixels.Color(255, 0, 0, 5));
                        pixels.show();
                        //delay(DELAYVAL);
                        }
                } 
                
                else if(audioLevel > changeYellow){
                    sprite.fillScreen(YELLOW);
                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);

                    //NeoPixel 色設定&反映
                    for(int j=0; j<NUMPIXELS; j++) {
                        pixels.setPixelColor(j, pixels.Color(255, 100, 0, 5));
                        pixels.show();
                        //delay(DELAYVAL);
                    }
                    
                }

                else if(audioLevel > changeGreen){
                    sprite.fillScreen(GREEN);
                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);
                    

                    //NeoPixel 色設定&反映
                    for(int j=0; j<NUMPIXELS; j++) {
                        pixels.setPixelColor(j, pixels.Color(0, 255, 0, 5));
                        pixels.show();
                        //delay(DELAYVAL);
                    }
                }

                else if(audioLevel > changeCyan){
                    sprite.fillScreen(CYAN);
                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);

                    //NeoPixel 色設定&反映
                    for(int j=0; j<NUMPIXELS; j++) {
                        pixels.setPixelColor(j, pixels.Color(0, 200, 200, 5));
                        pixels.show();
                        //delay(DELAYVAL);
                    }
                    
                }
                
                else {
                    sprite.fillScreen(BLUE);
                    sprite.setCursor(10, 10);
                    sprite.setTextSize(5);
                    sprite.printf("%u\n",audioLevel);
                    sprite.pushSprite(0, 0);

                    //NeoPixel 色設定&反映
                    for(int j=0; j<NUMPIXELS; j++) {
                        pixels.setPixelColor(i, pixels.Color(0, 0, 255));
                        pixels.show();
                        //delay(DELAYVAL);
                    }
                    
                } */
                        
                for(int j=0; j<NUMPIXELS; j++) {
                    pixels.setPixelColor(j, pixels.Color(r, g, b));
                    pixels.show();
                    //delay(DELAYVAL);
                }
                
                audioValue = 0;
                audioCount = 0;
            }
        }
    }
}
