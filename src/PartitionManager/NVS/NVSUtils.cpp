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
      if( Partitions.size()==0 ) {
        if( !getPartitions() ) return nullptr;
      }
      for( int i=0; i<Partitions.size(); i++ ) {
        if( Partitions[i].ota_num == ota_num ) {
          log_v("OTA %d found", ota_num );
          return &Partitions[i];
        }
      }
      log_w("OTA %d not found", ota_num );
      return nullptr;
    }


    PartitionDesc_t* findPartition( const char* name )
    {
      assert(name);
      if( Partitions.size()==0 ) {
        if( !getPartitions() ) return nullptr;
      }
      log_v("Find by name %s", name);
      for( int i=0; i<Partitions.size(); i++ ) {
        if( strcmp( Partitions[i].name, name ) == 0 ) {
          log_v("OTA name '%s' found", name );
          return &Partitions[i];
        }
      }
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




    bool getPartitions()
    {
      if( Partitions.size()>0 )
        Partitions.clear();
      size_t blob_size;
      bool ret = false;
      blob_partition_t *bPart = nullptr;
      auto err = nvs_open(PARTITION_NS, NVS_READONLY, &handle);
      if( err != ESP_OK ) {
        log_i("NVS Namespace not created yet");
        return false;
      }

      err = nvs_get_blob(handle, PARTITION_KEY, NULL, &blob_size);
      if( err != ESP_OK ) {
        log_i("NVS key not created yet");
        goto _nvs_close;
      }

      bPart = new blob_partition_t( blob_size );
      if (!bPart->blob ) {
        log_e("Could not alloc %d bytes", blob_size );
        goto _nvs_close;
      }

      err = nvs_get_blob(handle, PARTITION_KEY, bPart->blob, &blob_size);
      if( err != ESP_OK ) {
        log_e("Could not read blob");
        goto _nvs_close;
      }

      ret = parsePartitions( bPart->blob, blob_size );

      _nvs_close:
      nvs_close( handle );
      if( bPart!=nullptr ) delete bPart;
      return ret;
    }


    bool parsePartitions( const char* blob, size_t size )
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

      log_d("Found %d items", found );

      return found>0;
    }



    bool savePartitions()
    {
      bool ret = true;
      if( Partitions.size() > 0 ) {
        log_d("Saving partitions");
        size_t blob_size = (sizeof(PartitionDesc_t)*Partitions.size());
        blob_partition_t *bPart = new blob_partition_t(blob_size);

        if( !bPart->blob) {
          log_e("Can't allocate %d bytes for blob", blob_size );
          return false;
        }
        size_t idx = 0;
        for( int i=0; i<Partitions.size(); i++ ) {
          idx = i*sizeof(PartitionDesc_t);
          auto part = &Partitions[i];
          memcpy( &bPart->blob[idx], part, sizeof(PartitionDesc_t) );
        }

        auto err = nvs_open(PARTITION_NS, NVS_READWRITE, &handle);
        if( err != ESP_OK ) {
          log_e("Cannote create NVS Namespace %s", PARTITION_NS);
          return false;
        }

        err = nvs_set_blob(handle, PARTITION_KEY, bPart->blob, blob_size);
        if( err != ESP_OK ) {
          log_e("Failed to save blob");
          ret = false;
        } else {
          log_v("Blob save success (%d bytes)", blob_size);
          nvs_commit( handle );
        }

        nvs_close( handle );
        delete bPart;
      }
      return ret;
    }


    void deletePartitions()
    {
      auto err = nvs_open(PARTITION_NS, NVS_READWRITE, &handle);
      if( err != ESP_OK ) {
        log_e("Cannot open NVS Namespace %s for writing", PARTITION_NS);
        return;
      }
      err = nvs_erase_key(handle, PARTITION_KEY);
      if( err != ESP_OK ) {
        log_e("Failed to erase blob");
      } else {
        log_v("Blob erase success (%d bytes)");
        nvs_commit( handle );
      }
      nvs_close( handle );
    }


    // Rollback helper: save menu.bin meta info in NVS
    bool saveMenuPrefs()
    {
      const esp_partition_t* update_partition = esp_ota_get_next_update_partition( NULL );
      if (!update_partition) {
        log_e( "Partition scheme does not support OTA" );
        return false;
      }
      esp_image_metadata_t nusketchMeta = Flash::getSketchMeta( update_partition );
      uint32_t nuSize = nusketchMeta.image_len;

      auto err = nvs_open(MENU_PREF_NS, NVS_READWRITE, &handle);
      if( err != ESP_OK ) {
        log_i("NVS Namespace %s not created yet", MENU_PREF_NS );
        return false;
      }

      bool ret = true;

      err = nvs_set_blob(handle, DIGEST_KEY, nusketchMeta.image_digest, 32);
      if( err != ESP_OK ) {
        log_e("NVS failed to save %s::%s", MENU_PREF_NS, DIGEST_KEY);
        ret = false;
      }

      err = nvs_set_i32(handle, MENUSIZE_KEY, nuSize);
      if( err != ESP_OK ) {
        log_e("NVS failed to save %s::%s", MENU_PREF_NS, MENUSIZE_KEY);
        ret = false;
      }

      if( ret) nvs_commit( handle );

      nvs_close( handle );
      return ret;
    }



    bool getMenuPrefs( uint32_t *menuSize, uint8_t *image_digest )
    {
      if( nvs_open(MENU_PREF_NS, NVS_READONLY, &handle) != ESP_OK ) {
        log_i("NVS Namespace %s not created yet", MENU_PREF_NS );
        return false;
      }

      bool ret = false;
      size_t blob_size;

      if( nvs_get_blob(handle, DIGEST_KEY, NULL, &blob_size) != ESP_OK ) {
        log_i("NVS key %s::%s not created yet", MENU_PREF_NS, DIGEST_KEY);
        goto _nvs_close;
      }

      if( blob_size != 32 ) {
        log_e("NVS key %s::%s has invalid size( expect=32, got=%d)", MENU_PREF_NS, DIGEST_KEY, blob_size);
        goto _nvs_close;
      }

      if( nvs_get_blob(handle, DIGEST_KEY, image_digest, &blob_size) != ESP_OK ) {
        log_i("NVS key %s::%s not created yet", MENU_PREF_NS, DIGEST_KEY);
        goto _nvs_close;
      }

      if( nvs_get_i32(handle, MENUSIZE_KEY, (int32_t*)menuSize) != ESP_OK ) {
        log_i("NVS key %s::%s not created yet", MENU_PREF_NS, MENUSIZE_KEY);
      }

      ret = true;

      _nvs_close:
      nvs_close( handle );
      return ret;
    }




  }; // end namespace NVS

}; // end namespace SDUpdaterNS
