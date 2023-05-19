#pragma once

#if defined SDU_HAS_FFAT

  #include "../misc/config.h"
  #include "../misc/types.h"
  #include <FFat.h>

  namespace SDUpdaterNS
  {

    namespace ConfigManager
    {
      struct FFat_FS_Config_t
      {
        bool formatOnFail{false};
        const char * basePath{"/ffat"};
        uint8_t maxOpenFiles{10};
        const char * partitionLabel{FFAT_PARTITION_LABEL};
      };

      static FFat_FS_Config_t *FFat_ConfigPtr = nullptr;
      static fs::FS *SDU_FFat_Ptr = &FFat;
    };

    inline ConfigManager::FFat_FS_Config_t* SDU_FFat_CONFIG_GET()
    {
      if( ConfigManager::FFat_ConfigPtr ) return ConfigManager::FFat_ConfigPtr;
      static ConfigManager::FFat_FS_Config_t FFat_Config = ConfigManager::FFat_FS_Config_t();
      ConfigManager::FFat_ConfigPtr = &FFat_Config;
      return ConfigManager::FFat_ConfigPtr;
    }

    #define SDU_CONFIG_FFat *SDU_FFat_CONFIG_GET()

    inline ConfigManager::FS_Config_t* SDU_FFAT_GET()
    {
      static ConfigManager::FS_Config_t FFat_FS_Config = {"ffat", ConfigManager::SDU_FFat_Ptr, ConfigManager::FFat_ConfigPtr};
      return &FFat_FS_Config;
    }


    inline bool SDU_FFat_Begin(ConfigManager::FFat_FS_Config_t cfg=ConfigManager::FFat_FS_Config_t())
    {
      ConfigManager::FFat_ConfigPtr = &cfg;
      return FFat.begin( cfg.formatOnFail, cfg.basePath, cfg.maxOpenFiles, cfg.partitionLabel );
    }


  };

#endif
