/*\
 *
 * NVS Dumper
 *
 * Copyleft tobozo 2020
 *
 * Inspired from the following implementations:
 *
 *   - https://github.com/Edzelf/ESP32-Show_nvs_keys/
 *   - https://gist.github.com/themadsens/026d38f432727567c3c456fb4396621b
 *
\*/

#include "NVSUtils.hpp"


namespace SDUpdaterNS
{

  namespace NVS
  {

    nvs_handle_t handle;
    std::vector<PartitionDesc_t> Partitions; // filled by NVS


    PartitionDesc_t* findPartition( uint8_t ota_num )
    {
      log_v("Find by ota number %d", ota_num);
      for( int i=0; i<Partitions.size(); i++ ) {
        if( Partitions[i].ota_num == ota_num ) {
          log_d("OTA %d found", ota_num );
          return &Partitions[i];
        }
      }
      log_w("OTA %d not found", ota_num );
      return nullptr;
    }


    PartitionDesc_t* findPartition( Flash::Partition_t* flash_partition )
    {
      assert( flash_partition );
      auto ota_num = flash_partition->part.subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
      auto nvs_part = findPartition( ota_num );

      if( nvs_part ) {
        if( Flash::metadataHasDigest( &flash_partition->meta ) ) {
          return nvs_part;
        }
      }
      return nullptr;
    }


    int Erase()
    {
      esp_err_t result = nvs_flash_erase();
      if( result != ESP_OK ) return -1;
      return 0;
    }


    bool getBLobPartitions()
    {
      size_t blob_size;
      char* out = nullptr;
      bool ret = false;
      auto err = nvs_open(SDU_PARTITION_NS, NVS_READONLY, &handle);
      if( err != ESP_OK ) {
        log_i("NVS Namespace not created yet");
        return false;
      }

      err = nvs_get_blob(handle, SDU_PARTITION_KEY, NULL, &blob_size);
      if( err != ESP_OK ) {
        log_i("NVS key not created yet");
        goto _nvs_close;
      }

      out = (char*)calloc(blob_size+1, sizeof(char));
      if (!out ) {
        log_e("Could not alloc %d bytes", blob_size+1 );
        goto _nvs_close;
      }
      err = nvs_get_blob(handle, SDU_PARTITION_KEY, out, &blob_size);
      if( err != ESP_OK ) {
        log_e("Could not read blob");
        goto _nvs_close;
      }

      ret = parseBlobPartitions( out, blob_size-1 );
      free( out );

      _nvs_close:
      nvs_close( handle );
      return ret;
    }


    bool parseBlobPartitions( const char* blob, size_t size )
    {
      size_t idx = 0;
      size_t found = 0;
      do {
        PartitionDesc_t* sdu_part = (PartitionDesc_t*)&blob[idx];
        auto sdu_part_ref = PartitionDesc_t();
        memcpy( &sdu_part_ref, sdu_part, sizeof(PartitionDesc_t) );
        Partitions.push_back( sdu_part_ref );
        found++;
        idx += sizeof(PartitionDesc_t);
      } while( idx < size );

      log_d("Found %d items in NVS partitions blob", found );

      return found>0;
    }

  }; // end namespace NVS

}; // end namespace SDUpdaterNS
