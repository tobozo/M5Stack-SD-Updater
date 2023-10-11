
//#include "./PartitionManager.hpp"
#include "../SDUpdater/SDUpdater_Class.hpp"


namespace SDUpdaterNS
{
  namespace PartitionManager
  {


    void createPartitions()
    {
      NVS::Partitions.clear();
      Flash::digest_t digests = Flash::digest_t();
      for( int i=0; i<Flash::Partitions.size(); i++ ) {
        auto part = &Flash::Partitions[i].part;
        auto meta = &Flash::Partitions[i].meta;
        if( Flash::partitionIsFactory( part ) ) continue;
        if( !Flash::partitionIsApp( part ) ) continue;
        NVS::PartitionDesc_t nvs_part;
        nvs_part.ota_num  = part->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
        snprintf( nvs_part.name, 39, "OTA %d",  nvs_part.ota_num );
        nvs_part.bin_size = 0;
        if( Flash::metadataHasDigest( meta ) ) {
          nvs_part.bin_size = meta->image_len;
          memcpy( nvs_part.digest, meta->image_digest, 32 );
          log_d("Added flash digest to NVS::Partitions[%d]: %s", NVS::Partitions.size(), digests.toString( nvs_part.digest ) );
        }
        NVS::Partitions.push_back( nvs_part );
      }
      NVS::savePartitions();
      //debugPartitions();
      log_i("Partition scheme has %d app slot(s)", NVS::Partitions.size() );
    }



    // update NVS blob with flash partition infos
    void updatePartitions()
    {
      log_v("Comparing Flash and NVS partitions");
      bool needs_saving = false;
      Flash::digest_t digests = Flash::digest_t();
      for( int i=0; i<Flash::Partitions.size(); i++ ) {
        auto sdu_flash_part = &Flash::Partitions[i];
        auto flash_part = &sdu_flash_part->part;
        auto meta = &sdu_flash_part->meta;
        if( !Flash::metadataHasDigest( meta ) ) continue;       // ignore empty partitions
        if( Flash::partitionIsFactory( flash_part ) ) continue; // ignore factory partition
        if( !Flash::partitionIsApp( flash_part ) ) continue;    // ignore non-app partitions
        auto sdu_nvs_part = NVS::findPartition(sdu_flash_part);

        if( sdu_nvs_part ) { // flash partition is documented in NVS, compare digests
          if( !digests.match( sdu_nvs_part->digest, sdu_flash_part->meta.image_digest ) ) { // image digests differ
            memcpy( sdu_nvs_part->digest, sdu_flash_part->meta.image_digest, 32 ); // update NVS digest
            needs_saving = true;
          }
        } else {
          log_e("No matching partition for ota #%d", flash_part->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
        }
      }

      if( needs_saving ) {
        NVS::savePartitions();
        //debugPartitions();
      }
    }


    // Called after factory firmware was flashed and copied to factory partition,
    // will erase the originating OTA partition to make it available for next flashing.
    void processPartitions()
    {
      Flash::digest_t digests = Flash::digest_t();
      for( int i=0; i<NVS::Partitions.size(); i++ ) {
        auto nvs_part = &NVS::Partitions[i];

        bool is_factory_dupe = Flash::FactoryPartition!=nullptr
                        && !digests.isEmpty(Flash::FactoryPartition->meta.image_digest)
                        && digests.match(nvs_part->digest, Flash::FactoryPartition->meta.image_digest);

        if( is_factory_dupe > 0 ) {
          // erase ota version of factory partition now that it's been duplicated
          if( Flash::erase( nvs_part->ota_num ) ) {
            nvs_part->bin_size = 0;
            memset( nvs_part->digest, 0, 32 );
            if( NVS::savePartitions() ) {
              //debugPartitions();
              // TODO: implement partitions reload instead of restart
              ESP.restart();
            }
          }
        }
      }
    }


    // copy firmware from SD/SPIFFS://path to OTA flash partition
    //bool Flash( fs::FS &fs, const char* path, const esp_partition_t *dstpart )
    bool flash( const esp_partition_t *dstpart, fs::FS *dstfs, const char* srcpath )
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
        auto nvs_part = NVS::findPartition( dstpart->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
        if( !nvs_part ) {
          log_e("FATAL: can't update nvs with new partition info");
          SDUpdater::_error("NVS persistence fail");
          return false;
        }
        snprintf(nvs_part->name, 39, srcpath );
        nvs_part->bin_size = fsize;
        if( esp_partition_get_sha256(dstpart, nvs_part->digest) != ESP_OK ) {
          log_e("WARN: partition has no sha");
          //return false;
        }
        ret = NVS::savePartitions();
        //debugPartitions();
      }
      return ret;
    }


    // Copy firmware from filesystem to ota slot in transaction style.
    // Implements a picker for source filesystem and source filename.
    bool flash( uint8_t ota_num, sdu_fs_picker_t fsPicker, sdu_file_picker_t filePicker )
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


    bool backupFlash( fs::FS* dstFs, const char* dstName )
    {
      SDUpdater::_message("Backing up full Flash");
      bool ret = Flash::dumpFW( dstFs, "/full_dump.fw" );
      if( ! ret ) SDUpdater::_error("Backup failed");
      return ret;
    }



    bool backup( uint8_t ota_num, sdu_fs_picker_t fsPicker )
    {
      auto nvs_part = NVS::findPartition( ota_num );
      if( !nvs_part ) return false;
      return backup( nvs_part, fsPicker );
    }


    bool backup( NVS::PartitionDesc_t *src_nvs_part, sdu_fs_picker_t fsPicker )
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


    bool verify( uint8_t ota_num )
    {
      auto part = Flash::getPartition( ota_num );
      if( !part ) return false;
      auto meta = Flash::getSketchMeta( part );
      Flash::digest_t digests = Flash::digest_t();
      if( digests.isEmpty( meta.image_digest ) ) return false;

      auto nvs_part = NVS::findPartition( ota_num );
      if( !nvs_part ) return false;

      uint8_t sha256[32];

      auto err = bootloader_common_get_sha256_of_partition( part->address, part->size, part->type, sha256 );

      if( err != ESP_OK ) return false;

      return digests.match(sha256, nvs_part->digest) && digests.match(meta.image_digest, nvs_part->digest);
    }


    // void debugPartitions()
    // {
    //   log_d("OTA, size, digest, name, desc");
    //   Flash::digest_t digests;
    //   for( int i=0;i<NVS::Partitions.size();i++ ) {
    //     log_d("%d %s %s %s %d bytes",
    //       NVS::Partitions[i].ota_num, // OTA partition number
    //       digests.toString( NVS::Partitions[i].digest ), // firmware digest
    //       NVS::Partitions[i].name, // firmware name
    //       NVS::Partitions[i].desc,  // firmware desc
    //       NVS::Partitions[i].bin_size // firmware size
    //     );
    //   }
    //
    //   log_d("Partition  Type   Subtype    Address   PartSize   ImgSize    Info    Digest");
    //   log_d("---------+------+---------+----------+----------+---------+--------+--------");
    //   for( int i=0; i<Flash::Partitions.size(); i++ ) {
    //     //printFlashPartition( &Flash::Partitions[i] );
    //     Flash::digest_t digests = Flash::digest_t();
    //
    //     auto sdu_partition = &Flash::Partitions[i];
    //     auto part = sdu_partition->part;
    //     auto meta = sdu_partition->meta;
    //
    //     String AppName = "n/a";
    //
    //     if( Flash::partitionIsApp( &part ) ) {
    //       if( Flash::partitionIsFactory( &part ) ) {
    //         AppName = "Factory";
    //       } else {
    //         AppName = "OTA" + String( part.subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
    //       }
    //     }
    //
    //     log_d("%-8s   0x%02x      0x%02x   0x%06x   %8d  %8s %8s %8s",
    //       String( part.label ).c_str(),
    //       part.type,
    //       part.subtype,
    //       part.address,
    //       part.size,
    //       meta.image_len>0 ? String(meta.image_len).c_str() : "n/a",
    //       AppName.c_str(),
    //       Flash::partitionIsApp(&part)&&Flash::metadataHasDigest(&meta) ? digests.toString(meta.image_digest) : "n/a"
    //     );
    //   }
    // }


    bool erase( uint8_t ota_num )
    {
      NVS::PartitionDesc_t* nvs_part = NVS::findPartition(ota_num);
      if( !nvs_part ) {
        log_e("Cannot erase partition,  NVS::findPartition(%d) found nothing", ota_num);
        return false;
      }
      auto flash_part = Flash::getPartition( ota_num );
      if( !flash_part ) {
        log_e("Cannot erase partition,  Flash::getPartition(%d) found nothing", ota_num);
        return false;
      }
      if( Flash::erase(ota_num) ) {

        nvs_part->bin_size = 0;
        nvs_part->name[0] = 0;
        //nvs_part->desc[0] = 0;
        for( int i=0;i<32;i++ ) nvs_part->digest[i] = 0;

        Flash::scan();
        //debugPartitions();

        if( NVS::savePartitions() ) {
          log_d("TODO: implement partitions reload instead of restart");
          //debugPartitions();
          ESP.restart(); // force partition reload
        }
      } else {
        log_e("Could erase partition,  Flash::erase(%d) failed", ota_num);
      }
      return false;
    }


    bool canMigrateToFactory()
    {
      if( !Flash::hasFactory() ) return false;
      if( Flash::isRunningFactory() ) return false;
      return true;
    }


    bool flashFactory()
    {
      if( !canMigrateToFactory() ) return false; // need a factory partition scheme
      SDUpdater::_message( String("Migrating to factory") );
      auto ret = Flash::saveSketchToFactory();
      if( !ret ) {
        SDUpdater::_error( String("Migration failed :(") );
      }
      return ret;
    }


    bool migrateSketch( const char* binFileName )
    {
      assert(binFileName);
      log_v("Checking %s for migration", binFileName);
      Flash::digest_t digests = Flash::digest_t();

      if( !canMigrateToFactory() ) {
        return false; // need a factory partition scheme
      }

      if( !NVS::getPartitions() ) {
        return false; // need a NVS partition management already set
      }

      esp_image_metadata_t running_meta;
      esp_image_metadata_t dst_meta;

      NVS::PartitionDesc_t* NVSPart = nullptr;
      Flash::Partition_t *FlashPart = nullptr;

      uint8_t ota_num = 0;
      size_t sksize = 0;

      String error = "";
      String msg = "";

      Flash::running_partition = esp_ota_get_running_partition();
      if( !Flash::running_partition ) {
        log_e("Flash inconsistency: running partition not found" );
        return false; // uh-oh
      }

      if( Flash::running_partition->subtype != ESP_PARTITION_SUBTYPE_APP_OTA_MIN ) {

      }


      running_meta = Flash::getSketchMeta( Flash::running_partition );

      NVSPart = NVS::findPartition( binFileName );

      if( !NVSPart /*|| (NVSPart && digests.isEmpty( NVSPart->digest))*/ ) {
        // No NVS partition found named [binFilename], but NVS may be wrong so try to also find by digest
        FlashPart = Flash::findDupePartition( &running_meta, Flash::running_partition->type, Flash::running_partition->subtype );
        if( FlashPart ) {
          //log_d("Duplicate partition found: NVS has no entry named %s but partition meta %d exists", binFileName, FlashPart->part.subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
          error = "Error: NVS dupe found, please erase NVS";
          goto _error_nvs;
        }
        log_d("NVS Partition named %s not found", binFileName );
        // new slot, get next flashable partition
        Flash::nextupd_partition = Flash::getNextAvailPartition( Flash::running_partition->type, Flash::running_partition->subtype );
        if( !Flash::nextupd_partition ) {
          error = "Migration canceled: partitions full";//migration to new slot is not possible
          goto _error_nvs;
        }
        // store NVS app number
        ota_num = Flash::nextupd_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
        sksize = ESP.getSketchSize();

        msg = String("Migrating to new slot #") + String(ota_num);
        SDUpdater::_message( msg );

        // copy to next partition
        if( !Flash::copyPartition( Flash::nextupd_partition, Flash::running_partition, sksize) ) {
          error = "Migration failed";
          goto _error_nvs;
        }

        NVSPart = NVS::findPartition(ota_num);
        if( !NVSPart ) {
          error = "Error: please erase NVS";
          goto _error_nvs;
        }

        goto _update_nvs;
      }

      // NVSPart exists with similar binFile name, figure out if overwrite is needed

      ota_num = NVSPart->ota_num;
      // get the destination slot according to NVS
      Flash::nextupd_partition = Flash::getPartition( ota_num );
      if( !Flash::nextupd_partition ) {
        // log_e("NVS inconsistency: destination slot #%d not found", ota_num );
        error = "Error: please erase Flash";
        goto _error_nvs;
      }
      dst_meta = Flash::getSketchMeta( Flash::nextupd_partition );

      // health check: compare NVSPart with the flash partition it represents
      if( ota_num != Flash::nextupd_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN ) {
        //log_e("NVS inconsistency: slot mismatch (%d vs %d)", ota_num, Flash::nextupd_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
        error = "Error: please erase Flash";
        goto _error_nvs;
      }
      // health check: compare NVSPart with the flash partition it represents
      if( ! digests.match( NVSPart->digest, dst_meta.image_digest ) ) {
        log_w("NVS inconsistency: digest mismatch, will be overwritten");
      } else {
        // log_d("NVS data is consistent");
      }
      // functional check: verify that source and destination differ
      if( ota_num == Flash::running_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN ) {
        // check for duplicate meta
        FlashPart = Flash::findDupePartition( &running_meta, Flash::running_partition->type, Flash::running_partition->subtype );
        if( FlashPart ) {
          // erase duplicate (will trigger a restart on success)
          msg = String("Erasing old partition");
          SDUpdater::_message( msg );
          if( !PartitionManager::erase( FlashPart->part.subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN ) ) {
            error = "Erasing failed!";
            goto _error_nvs;
          }

        }
        log_i("this sketch is already running from the right partition (%d) according to NVS", ota_num);
        return false;
      }

      log_w("Current sketch is not running from its assigned partition (NVS want=%d, has=%d)", ota_num, Flash::running_partition->subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );

      // overwrite if digests differ
      if( ! digests.match( NVSPart->digest, running_meta.image_digest ) ) {
        msg  = "Overwriting slot (in=";
        msg += digests.toString( NVSPart->digest );
        msg += ", out=";
        msg += digests.toString( running_meta.image_digest );
        msg += ")";
        SDUpdater::_message( String("Overwriting slot") );
        sksize = ESP.getSketchSize();
        if( !Flash::copyPartition( Flash::nextupd_partition, Flash::running_partition, sksize) ) {
          error = "Overwriting failed!";
          goto _error_nvs;
        }
        goto _update_nvs;
      } else {
        log_d("names and digests match, no overwrite, just switch");
        goto _boot_partition;
      }

      _error_nvs:
        SDUpdater::_error( error );
        return false;

      _update_nvs:

        NVSPart->bin_size = running_meta.image_len;
        memcpy( NVSPart->digest, running_meta.image_digest, 32 );
        snprintf( NVSPart->name, 39, "%s", binFileName );
        log_d("Updated NVSPart->name %s=%s", NVSPart->name, binFileName);
        NVS::savePartitions();
        //debugPartitions();

      _boot_partition:

        Flash::bootPartition( ota_num );

      return false;
    }


  };

};
