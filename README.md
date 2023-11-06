# RDEx23
 ## 情報表現領域_展示作品

### 進捗2023/11/09  
<dl>
   <dt>RSSI(通信強度)の測定、親機、子機ともにプログラムを作成</dt>
      <dd>BLE通信は少しやり方がわからなかったので、Wi-Fiで測定</dd>
      <dd>参考サイト:https://fabcross.jp/category/make/sorem5/20210512_social_distance_onigokko.html</dd>
      <dd>近くて30付近、3mほど離れるとRSSI:70、5mでもRSSI:80ほどだった</dd>
   
   <dt>MajorPeakを使ってFFTのピークの値を取得させてみた</dt>
      <dd>MajorPeakを使ってピーク値を取得してみたが、たまにピークの値とは全く違う値が取得されるので、見直す必要があるかもしれない。</dd>
      <dd>参考サイト:https://101010.fun/m5stickc-plus-accel.html</dd>

   <dt>FFTとRSSI取得のプログラムをひとつにまとめた</dt>
      <dd>一応どちらの値の取得はできるようになったので、これを改変しつつ、本番用に変えていく予定。</dd>
</dl>


 ### 進捗2023/11/02
 <dl>
   <dt>M5側に外付けマイク用のプログラムに変更</dt>
      <dd>FFTにも対応させた。参考サイト:https://ambidata.io/samples/m5stack/sound/</dd>
      <dd>いちばん振幅の多い周波数を抜き出して、それの値でNeoPixelの色を変更する予定</dd>
      <dd>拾った音の周波数の値が少しおかしい、1kHzを出すと4kHzぐらいの表記になる</dd>
      <dd>今はFFTのスペクトルを表示してあるが、ここをどうするかも考える必要あり</dd>

   <dt>ESP側とM5側でのBluetooth通信のテスト</dt>
      <dd>参考サイト:https://www.remma.net/?p=367</dd>
      <dd>M5側を送信、ESP側を受信に設定、ペアリングからデータ送受信までは一応できた</dd>
      <dd>このままのプログラムだとM5側の液晶の表示がおかしくなるので要改良</dd>
      <dd>delayをはさむと通信できなくなるかも？</dd>
      <dd>たまに接続した瞬間に接続が切れることあり、用改良</dd>
      <dd>どうやってここからおおまかな距離を測れるようにするかを考える必要あり</dd>

   <dt>追加のスピーカーの素材の注文完了</dt>
      <dd>すべて到着したので、また工房に切りに行く(来週の前半あたり？)</dd>
</dl>


