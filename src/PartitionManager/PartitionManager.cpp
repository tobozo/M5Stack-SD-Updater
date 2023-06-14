
//#include "./PartitionManager.hpp"
#include "../SDUpdater/SDUpdater_Class.hpp"


namespace SDUpdaterNS
{
  namespace PartitionManager
  {

    bool Save()
    {
      bool ret = true;
      if( NVS::Partitions.size() > 0 ) {
        log_d("Saving partitions");
        size_t blob_size = (sizeof(NVS::PartitionDesc_t)*NVS::Partitions.size()) + 1;
        char* blob = (char*)calloc(blob_size, sizeof(char));
        if( !blob) {
          log_e("Can't allocate %d bytes for blob", blob_size );
          return false;
        }
        size_t idx = 0;
        for( int i=0; i<NVS::Partitions.size(); i++ ) {
          idx = i*sizeof(NVS::PartitionDesc_t);
          auto part = &NVS::Partitions[i];
          memcpy( &blob[idx], part, sizeof(NVS::PartitionDesc_t) );
        }

        auto err = nvs_open(SDU_PARTITION_NS, NVS_READWRITE, &NVS::handle);
        if( err != ESP_OK ) {
          log_e("Cannote create NVS Namespace %s", SDU_PARTITION_NS);
          return false;
        }

        err = nvs_set_blob(NVS::handle, SDU_PARTITION_KEY, blob, blob_size);
        if( err != ESP_OK ) {
          log_e("Failed to save blob");
          ret = false;
        } else {
          log_d("Blob save success (%d bytes)", blob_size);
          nvs_commit( NVS::handle );
        }

        nvs_close( NVS::handle );
        free( blob );
      }
      return ret;
    }


    void Create()
    {
      NVS::Partitions.clear();
      Flash::digest_t digests = Flash::digest_t();
      for( int i=0; i<Flash::Partitions.size(); i++ ) {
        auto part = &Flash::Partitions[i].part;
        auto meta = &Flash::Partitions[i].meta;
        if( Flash::PartitionIsFactory( part ) ) continue;
        if( !Flash::PartitionIsApp( part ) ) continue;
        NVS::PartitionDesc_t nvs_part;
        nvs_part.ota_num  = part->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
        snprintf( nvs_part.name, 39, "OTA %d",  nvs_part.ota_num );
        nvs_part.bin_size = 0;
        if( Flash::MetadataHasDigest( meta ) ) {
          nvs_part.bin_size = meta->image_len;
          memcpy( nvs_part.digest, meta->image_digest, 32 );
          log_d("Added flash digest to NVS::Partitions[]: %s", digests.toString( nvs_part.digest ) );
        }
        NVS::Partitions.push_back( nvs_part );
      }
      Save();
      log_i("Found %d application(s)", NVS::Partitions.size() );
    }


    void Delete()
    {
      auto err = nvs_open(SDU_PARTITION_NS, NVS_READWRITE, &NVS::handle);
      if( err != ESP_OK ) {
        log_e("Cannot open NVS Namespace %s for writing", SDU_PARTITION_NS);
        return;
      }
      err = nvs_erase_key(NVS::handle, SDU_PARTITION_KEY);
      if( err != ESP_OK ) {
        log_e("Failed to erase blob");
      } else {
        log_d("Blob erase success (%d bytes)");
        nvs_commit( NVS::handle );
      }
      nvs_close( NVS::handle );
    }


    // update NVS blob with flash partition infos
    void Update()
    {
      log_v("Comparing Flash and NVS partitions");
      bool needs_saving = false;
      Flash::digest_t digests = Flash::digest_t();
      for( int i=0; i<Flash::Partitions.size(); i++ ) {
        auto sdu_flash_part = &Flash::Partitions[i];
        auto flash_part = &sdu_flash_part->part;
        auto meta = &sdu_flash_part->meta;
        if( !Flash::MetadataHasDigest( meta ) ) continue;       // ignore empty partitions
        if( Flash::PartitionIsFactory( flash_part ) ) continue; // ignore factory partition
        if( !Flash::PartitionIsApp( flash_part ) ) continue;    // ignore non-app partitions
        auto sdu_nvs_part = NVS::FindPartition(sdu_flash_part);

        if( sdu_nvs_part ) { // flash partition is documented in NVS, compare digests
          if( !digests.match( sdu_nvs_part->digest, sdu_flash_part->meta.image_digest ) ) { // image digests differ
            memcpy( sdu_nvs_part->digest, sdu_flash_part->meta.image_digest, 32 ); // update NVS digest
            needs_saving = true;
          }
        } else {
          [[maybe_unused]] auto ota_num = flash_part->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
          log_e("No matching partition for ota #%d", ota_num );
        }
      }

      if( needs_saving ) {
        log_d("Updating NVS partitions");
        Save();
      }
    }


    // Called after factory firmware was flashed and copied to factory partition,
    // will erase the originating OTA partition to make it available for next
    // flashing.
    void Process()
    {
      Flash::digest_t digests = Flash::digest_t();
      for( int i=0; i<NVS::Partitions.size(); i++ ) {
        auto nvs_part = &NVS::Partitions[i];

        bool is_factory_dupe = Flash::FactoryPartition!=nullptr
                        && !digests.isEmpty(Flash::FactoryPartition->meta.image_digest)
                        && digests.match(nvs_part->digest, Flash::FactoryPartition->meta.image_digest);

        if( is_factory_dupe > 0 ) {
          // erase ota version of factory partition now that it's been duplicated
          if( Flash::Erase( nvs_part->ota_num ) ) {
            nvs_part->bin_size = 0;
            memset( nvs_part->digest, 0, 32 );
            if( Save() ) {
              log_d("TODO: implement partitions reload instead of restart");
              ESP.restart();
            }
          }
        }

        if( nvs_part->bin_size > 0 ) {
          Serial.printf("[%d] %s %s %s\n", nvs_part->ota_num, nvs_part->name, digests.toString( nvs_part->digest ), is_factory_dupe ? "(dupe of factory)" : "" );
        } else {
          Serial.printf("[%d] Available slot\n", nvs_part->ota_num );
        }
      }
    }


    // copy firmware from SD/SPIFFS://path to OTA flash partition
    //bool Flash( fs::FS &fs, const char* path, const esp_partition_t *dstpart )
    bool Flash( const esp_partition_t *dstpart, fs::FS *dstfs, const char* srcpath )
    {
      if( !srcpath ) return false;
      if( !dstpart ) return false;
      if( !dstfs   ) return false;

      // - check if file exists on SD
      if( !dstfs->exists( srcpath ) ) return false;
      auto file = dstfs->open( srcpath );
      if( !file || file.size()==0 ) return false;
      auto fsize = file.size();
      SDUpdater::_message("Copying FS to Flash");
      // - flash binary
      bool ret = Flash::copyPartition(dstpart, &file, fsize );
      file.close();
      if( ret ) {
        auto nvs_part = NVS::FindPartition( dstpart->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
        if( !nvs_part ) {
          log_e("FATAL: can't update nvs with new partition info");
          return false;
        }
        snprintf(nvs_part->name, 39, srcpath );
        nvs_part->bin_size = fsize;
        if( esp_partition_get_sha256(dstpart, nvs_part->digest) != ESP_OK ) {
          log_e("WARN: partition has no sha");
          //return false;
        }
        return Save();
      }
      return ret;
    }


    // Copy firmware from filesystem to ota slot in transaction style.
    // Implements a picker for source filesystem and source filename.
    bool Flash( uint8_t ota_num, sdu_fs_picker_t fsPicker, sdu_file_picker_t filePicker )
    {
      auto dest_part = Flash::findPartition( ota_num );
      //if( !dest_part.part ) return false; // invalid ota number
      sdu_fs_copy_t fsCopy;
      fsCopy.dstPart = &dest_part.part;
      fsCopy.srcFs = fsPicker(); // select a source filesystem
      if( !fsCopy.srcFs ) return false; // action cancelled
      fsCopy.name = filePicker( fsCopy.srcFs );
      if( !fsCopy.name ) return false; // action cancelled
      return fsCopy.commit();
    }


    bool Backup( uint8_t ota_num, sdu_fs_picker_t fsPicker )
    {
      auto nvs_part = NVS::FindPartition( ota_num );
      if( !nvs_part ) return false;
      return Backup( nvs_part, fsPicker );
    }


    bool Backup( NVS::PartitionDesc_t *src_nvs_part, sdu_fs_picker_t fsPicker )
    {
      if( !src_nvs_part ) return false;
      if( src_nvs_part->bin_size == 0 ) return false;
      String name = String(src_nvs_part->name);
      auto flash_part = Flash::getPartition( src_nvs_part->ota_num );
      if( !flash_part ) return false;
      if( !name.startsWith("/") ) name = "/" + name;
      if( !name.endsWith(".bin") ) name = name + ".bin";
      auto fs = fsPicker();
      if( !fs ) return false;
      auto destfile = fs->open( name, "w" );
      if(!destfile) return false;
      bool ret = Flash::copyPartition(&destfile, flash_part, flash_part->size);
      destfile.close();
      return ret;
    }


    bool Verify( uint8_t ota_num )
    {
      auto part = Flash::getPartition( ota_num );
      if( !part ) return false;
      auto meta = Flash::getSketchMeta( part );
      Flash::digest_t digests = Flash::digest_t();
      if( digests.isEmpty( meta.image_digest ) ) return false;

      auto nvs_part = NVS::FindPartition( ota_num );
      if( !nvs_part ) return false;

      uint8_t sha256[32];

      auto err = bootloader_common_get_sha256_of_partition( part->address, part->size, part->type, sha256 );

      if( err != ESP_OK ) return false;

      // TODO: use bootloader util to compute sha256sum and compare with meta image digest
      //esp_err_t bootloader_common_get_sha256_of_partition(uint32_t address, uint32_t size, int type, uint8_t *out_sha_256);

      //nvs_part->digest
      //meta.image_digest

      return digests.match(sha256, nvs_part->digest) && digests.match(meta.image_digest, nvs_part->digest);
    }


    bool Erase( uint8_t ota_num )
    {
      NVS::PartitionDesc_t* nvs_part = NVS::FindPartition(ota_num);
      if( !nvs_part ) return false;
      auto flash_part = Flash::getPartition( ota_num );
      if( !flash_part ) return false;
      if( Flash::Erase(ota_num) ) {
        nvs_part->bin_size = 0;
        nvs_part->name[0] = 0;
        nvs_part->desc[0] = 0;
        for( int i=0;i<32;i++ ) nvs_part->digest[i] = 0;
        if( Save() ) {
          log_d("TODO: implement partitions reload instead of restart");
          ESP.restart(); // force partition reload
        }
      }
      return false;
    }


    bool NeedsMigration()
    {
      if( !Flash::hasFactory() ) return false;
      if( Flash::isRunningFactory() ) return false;
      return true;
    }


    bool FlashFactory()
    {
      if( NeedsMigration() ) {
        SDUpdater::_message( String("Migrating to factory") );
        auto ret = Flash::saveSketchToFactory();
        if( !ret ) {
          SDUpdater::_message( String("Migration failed :(") );
          delay(1000);
        }
        return ret;
      }
      return false;
    }



  };

};
