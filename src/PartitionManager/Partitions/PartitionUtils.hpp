#pragma once

#include <memory>
#include <vector>
#include <esp_partition.h>
#include <esp_flash.h>
extern "C" {
  #include "esp_ota_ops.h"
  #include "esp_image_format.h"
  #include "bootloader_common.h"
}
#include <Stream.h>
#include <FS.h>
#include <Stream.h>


namespace SDUpdaterNS
{

  namespace Flash
  {

    struct digest_t
    {
      String str{"0000000000000000000000000000000000000000000000000000000000000000"};
      const char* toString( const uint8_t dig[32] );
      bool match( const uint8_t d1[32], const uint8_t d2[32] );
      bool isEmpty( const uint8_t d1[32] );
    };

    struct Partition_t
    {
      const esp_partition_t part;
      const esp_image_metadata_t meta;
    };

    extern Partition_t* FactoryPartition;
    extern std::vector<Partition_t> Partitions; // all partitions (flash)
    extern const esp_partition_t* running_partition;
    extern const esp_partition_t* nextupd_partition;
    extern const esp_partition_t* factory_partition;

    void loadFactory();
    void memoize( const esp_partition_t *part );
    void scan();

    bool bootPartition( uint8_t ota_num );
    bool erase( uint8_t ota_num );
    bool erase( const esp_partition_t *part );

    bool copyPartition(fs::FS *fs, const char* binfilename); // copy from OTA0 to OTA1 and filesystem
    bool copyPartition(fs::File* dstFile, const esp_partition_t* src, size_t length); // copy from given partition to filesystem
    bool copyPartition(fs::File* dstFile, const esp_partition_t* dst, const esp_partition_t* src, size_t length); // copy from given partition to other partition and filesystem
    bool copyPartition(const esp_partition_t* dst, fs::FS *fs, const char* srcFilename); // copy from fs to given partition
    bool copyPartition(const esp_partition_t* dst, Stream* src, size_t length); // copy from stream to given partition
    bool copyPartition(const esp_partition_t* dst, const esp_partition_t* src, size_t length); // copy from one partition to another

    bool comparePartition(const esp_partition_t* src1, const esp_partition_t* src2, size_t length);
    bool comparePartition(const esp_partition_t* src1, fs::File* src2, size_t length);

    bool hasFactory();
    bool hasFactoryApp();
    bool isRunningFactory(); // checks if running partition is the factory partition
    bool saveSketchToFactory();
    bool saveSketchToPartition( const esp_partition_t* dst_partition );
    bool dumpFW( fs::FS *fs, const char* fw_name );
    bool partitionIsApp( const esp_partition_t *part );
    bool partitionIsOTA( const esp_partition_t *part );
    bool partitionIsFactory( const esp_partition_t *part );
    bool metadataHasDigest( const esp_image_metadata_t *meta );
    bool isEmpty( Flash::Partition_t* sdu_partition );

    const esp_partition_t* getPartition( uint8_t ota_num );
    const esp_partition_t* getFactoryPartition();
    const esp_partition_t* getNextAvailPartition( esp_partition_type_t type=ESP_PARTITION_TYPE_APP, esp_partition_subtype_t filter_subtype=ESP_PARTITION_SUBTYPE_APP_OTA_MIN );

    Partition_t findPartition( uint8_t ota_num );
    Partition_t *findDupePartition( esp_image_metadata_t *meta, esp_partition_type_t type=ESP_PARTITION_TYPE_APP, esp_partition_subtype_t filter_subtype=ESP_PARTITION_SUBTYPE_APP_OTA_MIN );

    esp_image_metadata_t getSketchMeta( const esp_partition_t* source_partition );

  };
};
