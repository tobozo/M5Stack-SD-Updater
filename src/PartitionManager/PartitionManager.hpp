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

    bool Save();
    void Create();
    void Delete();
    void Update();
    void Process();

    bool Flash( const esp_partition_t *dstpart, fs::FS *dstfs, const char* srcpath );
    bool Flash( uint8_t ota_num, sdu_fs_picker_t fsPicker, sdu_file_picker_t filePicker );

    bool Backup( uint8_t ota_num, sdu_fs_picker_t fsPicker );
    bool Backup( NVS::PartitionDesc_t *src_nvs_part, sdu_fs_picker_t fsPicker );

    bool Verify( uint8_t ota_num );

    bool Erase( uint8_t ota_num );

    bool NeedsMigration();
    bool FlashFactory();

    struct sdu_fs_copy_t
    {
      fs::FS* srcFs{nullptr};
      const char* name{nullptr};
      const esp_partition_t *dstPart{nullptr};
      bool commit()
      {
        return Flash( dstPart, srcFs, name );
      }
    };

  };

};
