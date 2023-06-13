#pragma once

#include <memory>
#include <vector>
#include <esp_partition.h>
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

    extern std::vector<Partition_t> Partitions; // all partitions (flash)
    extern Partition_t* FactoryPartition;
    extern const esp_partition_t* running_partition;
    extern const esp_partition_t* nextupd_partition;
    extern const esp_partition_t* factory_partition;

    bool comparePartition(const esp_partition_t* src1, const esp_partition_t* src2, size_t length);
    bool comparePartition(const esp_partition_t* src1, fs::File* src2, size_t length);

    bool copyPartition(fs::FS *fs, const char* binfilename); // copy from OTA0 to OTA1 and filesystem
    bool copyPartition(fs::File* dstFile, const esp_partition_t* src, size_t length); // copy from given partition to filesystem
    bool copyPartition(fs::File* dstFile, const esp_partition_t* dst, const esp_partition_t* src, size_t length); // copy from given partition to other partition and filesystem
    bool copyPartition(const esp_partition_t* dst, fs::FS *fs, const char* srcFilename); // copy from fs to given partition
    bool copyPartition(const esp_partition_t* dst, Stream* src, size_t length); // copy from stream to given partition
    bool copyPartition(const esp_partition_t* dst, const esp_partition_t* src, size_t length); // copy from one partition to another

    bool hasFactory();
    bool isRunningFactory(); // checks if running partition is the factory partition
    bool saveSketchToFactory();
    void loadFactory();

    bool bootPartition( uint8_t ota_num );

    const esp_partition_t* getFactoryPartition();
    esp_image_metadata_t getSketchMeta( const esp_partition_t* source_partition );

    const esp_partition_t* getPartition( uint8_t ota_num );
    Partition_t findPartition( uint8_t ota_num );
    const esp_partition_t* getNextAvailPartition();

    bool PartitionIsApp( const esp_partition_t *part );
    bool PartitionIsFactory( const esp_partition_t *part );
    bool MetadataHasDigest( const esp_image_metadata_t *meta );
    bool IsEmpty( Flash::Partition_t* sdu_partition );

    void Memoize( const esp_partition_t *part );
    void Scan();

    bool Erase( uint8_t ota_num );
    bool EraseRunning();

  };
};
