#pragma once


#include "./NVS/NVSUtils.hpp"
#include "./Partitions/PartitionUtils.hpp"
#include <FS.h>


namespace SDUpdaterNS
{
  namespace PartitionManager
  {

    typedef void(*partitionFoundCb_t)( const esp_partition_t *part );
    typedef fs::FS*(*sdu_fs_picker_t)();
    typedef const char*(*sdu_file_picker_t)(fs::FS*);

    void createPartitions();
    void updatePartitions();
    void processPartitions();
    void debugPartitions();

    bool flashFactory();
    bool migrateSketch( const char* binFileName );
    bool canMigrateToFactory();

    bool verify( uint8_t ota_num );
    bool erase( uint8_t ota_num );

    bool flash( const esp_partition_t *dstpart, fs::FS *dstfs, const char* srcpath );
    bool flash( uint8_t ota_num, sdu_fs_picker_t fsPicker, sdu_file_picker_t filePicker );

    bool backup( uint8_t ota_num, sdu_fs_picker_t fsPicker );
    bool backup( NVS::PartitionDesc_t *src_nvs_part, sdu_fs_picker_t fsPicker );

    bool backupFlash( fs::FS* dstFs, const char* dstName );

    struct sdu_fs_copy_t
    {
      fs::FS* srcFs{nullptr};
      const char* name{nullptr};
      const esp_partition_t *dstPart{nullptr};
      bool commit()
      {
        return flash( dstPart, srcFs, name );
      }
    };

  };

};
