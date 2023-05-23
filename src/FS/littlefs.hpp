#pragma once

#if defined SDU_HAS_LITTLEFS

  #include "../misc/config.h"
  #include "../misc/types.h"
  #include <LittleFS.h>

  namespace SDUpdaterNS
  {

    namespace ConfigManager
    {
      struct LittleFS_FS_Config_t
      {
        bool formatOnFail{false};
        const char * basePath{"/littlefs"};
        uint8_t maxOpenFiles{10};
        const char * partitionLabel{"spiffs"};
      };
      static LittleFS_FS_Config_t *LittleFS_ConfigPtr = nullptr;
      static fs::FS *SDU_LittleFS_Ptr = &LittleFS;
    };


    inline ConfigManager::LittleFS_FS_Config_t* SDU_LittleFS_CONFIG_GET()
    {
      if( ConfigManager::LittleFS_ConfigPtr ) return ConfigManager::LittleFS_ConfigPtr;
      static ConfigManager::LittleFS_FS_Config_t LittleFS_Config = ConfigManager::LittleFS_FS_Config_t();
      ConfigManager::LittleFS_ConfigPtr = &LittleFS_Config;
      return ConfigManager::LittleFS_ConfigPtr;
    }

    #define SDU_CONFIG_LittleFS *SDU_LittleFS_CONFIG_GET()


    inline ConfigManager::FS_Config_t* SDU_LITTLEFS_GET()
    {
      static ConfigManager::FS_Config_t LittleFS_FS_Config = {"littlefs", ConfigManager::SDU_LittleFS_Ptr, ConfigManager::LittleFS_ConfigPtr};
      return &LittleFS_FS_Config;
    }


    inline bool SDU_LittleFS_Begin(ConfigManager::LittleFS_FS_Config_t cfg=ConfigManager::LittleFS_FS_Config_t())
    {
      ConfigManager::LittleFS_ConfigPtr = &cfg;
      return LittleFS.begin( cfg.formatOnFail, cfg.basePath, cfg.maxOpenFiles, cfg.partitionLabel );
    }


  };

#endif
