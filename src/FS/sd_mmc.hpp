#pragma once

#if defined SDU_HAS_SD_MMC

  #include "../misc/config.h"
  #include "../misc/types.h"
  #include <SD_MMC.h>

  namespace SDUpdaterNS
  {

    namespace ConfigManager
    {
      struct SD_MMC_Bus_Config_t
      {
        int freq{BOARD_MAX_SDMMC_FREQ};
        int clk{-1};
        int cmd{-1};
        int d0{-1};
        int d1{-1};
        int d2{-1};
        int d3{-1};
      };
      struct SD_MMC_FS_Config_t
      {
        const char * mountpoint{"/sdcard"};
        bool mode1bit{false};
        bool format_if_mount_failed{false};
        SD_MMC_Bus_Config_t busCfg{SD_MMC_Bus_Config_t()};
        uint8_t maxOpenFiles{5};
      };
      static SD_MMC_FS_Config_t *SD_MMC_ConfigPtr = nullptr;
      static fs::FS *SDU_SD_MMC_Ptr = &SD_MMC;
    };


    inline ConfigManager::SD_MMC_FS_Config_t* SDU_SD_MMC_CONFIG_GET()
    {
      if( ConfigManager::SD_MMC_ConfigPtr ) return ConfigManager::SD_MMC_ConfigPtr;
      static ConfigManager::SD_MMC_FS_Config_t SD_MMC_Config = ConfigManager::SD_MMC_FS_Config_t();
      ConfigManager::SD_MMC_ConfigPtr = &SD_MMC_Config;
      return ConfigManager::SD_MMC_ConfigPtr;
    }

    #define SDU_CONFIG_SD_MMC *SDU_SD_MMC_CONFIG_GET()


    inline ConfigManager::FS_Config_t* SDU_SD_MMC_GET()
    {
      static ConfigManager::FS_Config_t SD_MMC_FS_Config = {"sdmmc", ConfigManager::SDU_SD_MMC_Ptr, ConfigManager::SD_MMC_ConfigPtr};
      return &SD_MMC_FS_Config;
    }


    inline bool SDU_SD_MMC_Begin( ConfigManager::SD_MMC_FS_Config_t cfg=ConfigManager::SD_MMC_FS_Config_t() )
    {
      auto busCfg = cfg.busCfg;
      if( busCfg.clk!=-1 && busCfg.cmd!=-1 && busCfg.d0!=-1 ) {
        if( busCfg.d1!=-1 && busCfg.d2!=-1 && busCfg.d3!=-1 ) {
          SD_MMC.setPins( busCfg.clk, busCfg.cmd, busCfg.d0, busCfg.d1, busCfg.d2, busCfg.d3 );
        } else {
          SD_MMC.setPins( busCfg.clk, busCfg.cmd, busCfg.d0 );
        }
      }
      ConfigManager::SD_MMC_ConfigPtr = &cfg;
      return SD_MMC.begin(cfg.mountpoint, cfg.mode1bit, cfg.format_if_mount_failed, cfg.busCfg.freq, cfg.maxOpenFiles);
    }

  };

#endif
