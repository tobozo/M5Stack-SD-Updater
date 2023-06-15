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

#define SDU_PARTITION_NS "sdu"
#define SDU_PARTITION_KEY "partitions"

namespace SDUpdaterNS
{

  namespace NVS
  {

    // NVS representation of flash partition
    struct __attribute__((__packed__)) PartitionDesc_t
    {
      uint8_t ota_num{0};    // OTA partition number
      size_t  bin_size{0};   // firmware size
      uint8_t digest[32]{0}; // firmware digest
      char    name[40]{0};   // firmware name
      char    desc[40]{0};   // firmware desc
    };

    extern nvs_handle_t handle;
    extern std::vector<NVS::PartitionDesc_t> Partitions; // filled by NVS

    PartitionDesc_t* findPartition( uint8_t ota_num );
    PartitionDesc_t* findPartition( Flash::Partition_t* flash_partition );

    int         erase();
    bool        getPartitions();
    bool        parsePartitions( const char* blob, size_t size );

  };

}; // end namespace SDUpdaterNS

