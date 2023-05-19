#pragma once

#if defined SDU_HAS_SPIFFS

  #include "../misc/config.h"
  #include "../misc/types.h"
  #include <SPIFFS.h>

  namespace SDUpdaterNS
  {

    namespace ConfigManager
    {
      struct SPIFFS_FS_Config_t
      {
        bool formatOnFail{false};
        const char * basePath{"/spiffs"};
        uint8_t maxOpenFiles{10};
        const char * partitionLabel{NULL};
      };

      static SPIFFS_FS_Config_t *SPIFFS_ConfigPtr = nullptr;
      static fs::FS *SDU_SPIFFS_Ptr = &SPIFFS;

    };

    inline ConfigManager::SPIFFS_FS_Config_t* SDU_SPIFFS_CONFIG_GET()
    {
      if( ConfigManager::SPIFFS_ConfigPtr ) return ConfigManager::SPIFFS_ConfigPtr;
      static ConfigManager::SPIFFS_FS_Config_t SPIFFS_Config = ConfigManager::SPIFFS_FS_Config_t();
      ConfigManager::SPIFFS_ConfigPtr = &SPIFFS_Config;
      return ConfigManager::SPIFFS_ConfigPtr;
    }

    #define SDU_CONFIG_SPIFFS *SDU_SPIFFS_CONFIG_GET()


    inline ConfigManager::FS_Config_t* SDU_SPIFFS_GET()
    {
      static ConfigManager::FS_Config_t SPIFFS_FS_Config = {"spiffs", ConfigManager::SDU_SPIFFS_Ptr, ConfigManager::SPIFFS_ConfigPtr};
      return &SPIFFS_FS_Config;
    }


    inline bool SDU_SPIFFS_Begin(ConfigManager::SPIFFS_FS_Config_t cfg=ConfigManager::SPIFFS_FS_Config_t())
    {
      ConfigManager::SPIFFS_ConfigPtr = &cfg;
      return SPIFFS.begin( cfg.formatOnFail, cfg.basePath, cfg.maxOpenFiles, cfg.partitionLabel );
    }

  };

#endif



