#pragma once
#define SDU_NVSUTILS_HPP

//#include <Arduino.h>
//#include <Preferences.h>
#include <cstring>
#include <ctype.h>
#include <stdio.h>
#include <vector>
#include <esp32-hal-log.h>
#include <esp_partition.h>
#include <nvs_flash.h>
#include <Stream.h>
#include <StreamString.h>

#include "../Partitions/PartitionUtils.hpp"


namespace SDUpdaterNS
{

  namespace NVS
  {

    // NVS namespace/key for virtual partitions array (may hold fw-menu partition)
    constexpr const char* PARTITION_NS  = "sdu";
    constexpr const char* PARTITION_KEY = "partitions";
    // NVS namespace/keys for sd-menu partition (blob digest + size)
    constexpr const char* MENU_PREF_NS  = "sd-menu";
    constexpr const char* DIGEST_KEY    = "digest";
    constexpr const char* MENUSIZE_KEY  = "menusize";


    // NVS representation of flash partition
    struct __attribute__((__packed__)) PartitionDesc_t
    {
      uint8_t ota_num{0};    // OTA partition number
      size_t  bin_size{0};   // firmware size
      uint8_t digest[32]{0}; // firmware digest
      char    name[40]{0};   // firmware name
      //char    desc[40]{0};   // firmware desc
    };

    struct blob_partition_t
    {
      char* blob{nullptr};
      bool needs_free{false};
      blob_partition_t() : blob(nullptr), needs_free(false) { }
      blob_partition_t( size_t blob_size ) : blob(nullptr), needs_free(false)
      {
        this->blob = (char*)calloc(blob_size+1, sizeof(char));
        if (!this->blob ) {
          log_e("Could not alloc %d bytes", blob_size );
        } else {
          needs_free = true;
        }
      }
      ~blob_partition_t()
      {
        if( needs_free )
          free(this->blob);
      }
    };

    extern nvs_handle_t handle;
    extern std::vector<PartitionDesc_t> Partitions; // filled by NVS

    PartitionDesc_t* findPartition( uint8_t ota_num );
    PartitionDesc_t* findPartition( const char* name );
    PartitionDesc_t* findPartition( Flash::Partition_t* flash_partition );

    int  erase();
    bool getPartitions();
    void deletePartitions();
    bool savePartitions();
    bool parsePartitions( const char* blob, size_t size );

    // save menu.bin meta info in NVS
    bool saveMenuPrefs();
    bool getMenuPrefs( uint32_t *menuSize, uint8_t *image_digest );

  };

}; // end namespace SDUpdaterNS

