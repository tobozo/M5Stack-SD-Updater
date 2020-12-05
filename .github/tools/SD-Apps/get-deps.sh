#!/bin/bash

# TODO: foreach this from JSON

wget --quiet https://github.com/adafruit/Adafruit_NeoPixel/archive/master.zip --output-document=Adafruit_NeoPixel.zip
unzip -d ~/Arduino/libraries Adafruit_NeoPixel.zip

wget --quiet https://github.com/adafruit/Adafruit_AMG88xx/archive/1.0.2.zip --output-document=Adafruit_AMG88xx.zip
unzip -d ~/Arduino/libraries Adafruit_AMG88xx.zip

wget --quiet https://github.com/bblanchon/ArduinoJson/archive/6.x.zip --output-document=Arduinojson-master.zip
unzip -d ~/Arduino/libraries Arduinojson-master.zip

# wget https://github.com/tomsuch/M5StackSAM/archive/master.zip --output-document=M5StackSAM-master.zip
wget --quiet https://github.com/tobozo/M5StackSAM/archive/patch-1.zip --output-document=M5StackSAM-master.zip
unzip -d ~/Arduino/libraries M5StackSAM-master.zip

#wget https://github.com/m5stack/M5Stack/archive/master.zip --output-document=M5Stack.zip
#curl -v --retry 5 "https://api.github.com/repos/M5Stack/M5Stack/releases/latest?access_token=$GH_TOKEN" | jq -r ".zipball_url" | wget --output-document=M5Stack.zip -i -
#unzip -d ~/Arduino/libraries M5Stack.zip

wget --quiet https://github.com/tobozo/ESP32-Chimera-Core/archive/master.zip --output-document=ESP32-Chimera-Core.zip
unzip -d ~/Arduino/libraries ESP32-Chimera-Core.zip

wget --quiet https://github.com/earlephilhower/ESP8266Audio/archive/master.zip --output-document=ESP8266Audio.zip
unzip -d ~/Arduino/libraries ESP8266Audio.zip

wget --quiet https://github.com/Seeed-Studio/Grove_BMP280/archive/1.0.1.zip --output-document=Grove_BMP280.zip
unzip -d ~/Arduino/libraries Grove_BMP280.zip

wget --quiet https://github.com/Gianbacchio/ESP8266_Spiram/archive/master.zip --output-document=ESP8266_Spiram.zip
unzip -d ~/Arduino/libraries ESP8266_Spiram.zip

wget --quiet http://www.buildlog.net/blog/wp-content/uploads/2018/02/Game_Audio.zip --output-document=Game_Audio.zip
mkdir -p ~/Arduino/libraries/Game_Audio
unzip -d ~/Arduino/libraries/Game_Audio Game_Audio.zip

wget --quiet https://github.com/lovyan03/M5Stack_TreeView/archive/master.zip --output-document=M5Stack_TreeView.zip
unzip -d ~/Arduino/libraries M5Stack_TreeView.zip

wget --quiet https://github.com/lovyan03/M5Stack_OnScreenKeyboard/archive/master.zip --output-document=M5Stack_OnScreenKeyboard.zip
unzip -d ~/Arduino/libraries M5Stack_OnScreenKeyboard.zip

wget --quiet https://github.com/kosme/arduinoFFT/archive/master.zip --output-document=arduinoFFT.zip
unzip -d ~/Arduino/libraries arduinoFFT.zip

rm -f *.zip
