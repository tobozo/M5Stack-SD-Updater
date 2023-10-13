#pragma once

#if defined SDU_HAS_SD

  #include "../misc/config.h"
  #include "../misc/types.h"
  #include <SD.h>

  namespace SDUpdaterNS
  {

    namespace ConfigManager
    {
      struct SD_FS_Config_t
      {
        uint8_t csPin{TFCARD_CS_PIN};
        SPIClass *bus{&SPI};
        uint32_t freq{4000000};
      };
      static SD_FS_Config_t *SD_ConfigPtr = nullptr;
      static fs::FS *SDU_SD_Ptr = &SD;
    };


    inline ConfigManager::SD_FS_Config_t* SDU_SD_CONFIG_GET()
    {
      if( ConfigManager::SD_ConfigPtr ) return ConfigManager::SD_ConfigPtr;
      static ConfigManager::SD_FS_Config_t SD_Config = ConfigManager::SD_FS_Config_t();
      log_d("CREATED DEFAULT SD config csPin:%d, bus:%s, frq:%d", SD_Config.csPin, SD_Config.bus?"true":"false", SD_Config.bus?SD_Config.freq:0);
      ConfigManager::SD_ConfigPtr = &SD_Config;
      return ConfigManager::SD_ConfigPtr;
    }

    #define SDU_CONFIG_SD *SDU_SD_CONFIG_GET()


    inline ConfigManager::FS_Config_t* SDU_SD_GET()
    {
      static ConfigManager::FS_Config_t SD_FS_Config = {"sd", ConfigManager::SDU_SD_Ptr, ConfigManager::SD_ConfigPtr};
      return &SD_FS_Config;
    }


    inline bool SDU_SDBegin( uint8_t csPin )
    {
      return SD.begin( csPin );
    }

    inline bool SDU_SDBegin( ConfigManager::SD_FS_Config_t cfg=ConfigManager::SD_FS_Config_t() )
    {
      ConfigManager::SD_ConfigPtr = &cfg;
      log_d("SD will begin CS_PIN=%d, bus:%s, frq=%d", cfg.csPin, cfg.bus?"true":"false", cfg.bus?cfg.freq:0);
      return cfg.freq==0 || cfg.freq > 80000000
        ? SD.begin()
        : cfg.bus
          ? SD.begin(cfg.csPin, *cfg.bus, cfg.freq)
          : SD.begin(cfg.csPin);
    }


  };

#endif


