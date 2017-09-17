# YMF825voiceeditor

YMF825 voice editor arduino , VT100ターミナルで動作する YMF825用ボイスエディタ　スケッチです YMF825 boardとArduino用に配線が必要です 下記を参照下さい

https://yamaha-webmusic.github.io/ymf825board/intro/

ScreenFMEdit.ino ymf825cont.ino 2つのファイルをArduino IDEで読み込んでスケッチを書き込んでください。

・使い方

TeraTerm等VT100対応の端末ソフトで接続して下さい。


キー操作

     '+'　　パラメータ値インクリメント

     '-'　　パラメータ値デクリメント
 
     CR　　再描画
 
     'p'　テスト演奏 (何かキーを押すと中断)

     's'　音色をEEPROMへ保存

     'l'　音色をEEPROMから読込

     'd'　音色データをHEXダンプ
