#pragma once

#include <Update.h>
#include <Wire.h>

namespace SDUpdaterNS
{

  namespace I2CUpdater
  {

    typedef void (*onProgress_t)(size_t progress, size_t total);

    void onProgress( onProgress_t cb );

    namespace Control // A.K.A. FOTA I2C server, the ESP serving the flash data
    {
      void setup( TwoWire* srcBus, uint8_t dev_addr, Stream *srcStream, size_t updateSize );
      void flashI2C(); // syncron
    }


    namespace Target // A.K.A. FOTA I2C client, the ESP to be flashed
    {
      void setup( TwoWire* srcBus, uint8_t dev_addr );
      void onReceive( int len ); // asynchron
    }

  }

}
