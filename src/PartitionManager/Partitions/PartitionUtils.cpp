#include "./PartitionUtils.hpp"
#include "../NVS/NVSUtils.hpp"
#include "../../ConfigManager/ConfigManager.hpp"


#if !defined SPI_FLASH_SEC_SIZE
  #define SPI_FLASH_SEC_SIZE 4096
#endif

namespace SDUpdaterNS
{

  namespace Flash
  {

    std::vector<Partition_t> Partitions; // all partitions (flash)
    Partition_t* FactoryPartition = nullptr;
    const esp_partition_t* running_partition = nullptr;
    const esp_partition_t* factory_partition = nullptr;
    const esp_partition_t* nextupd_partition = nullptr;


    const char* digest_t::toString( const uint8_t dig[32] )
    {
      str = "";
      char hex[3] = {0};
      for(int i=0;i<32;i++) {
        snprintf( hex, 3, "%02x", dig[i] );
        str += String(hex);
      }
      return str.c_str();
    }


    bool digest_t::match( const uint8_t d1[32], const uint8_t d2[32] )
    {
      for(int i=0; i<32; i++) {
        if (d1[i] != d2[i]) return false;
      }
      return true;
    }


    bool digest_t::isEmpty( const uint8_t d1[32] )
    {
      int sum = -1;
      for( int i=0; i<32; i++ ) {
        sum += d1[i];
      }
      return sum<=0;
    }


    bool comparePartition(const esp_partition_t* src1, const esp_partition_t* src2, size_t length)
    {
      size_t lengthLeft = length;
      const size_t bufSize = SPI_FLASH_SEC_SIZE;
      std::unique_ptr<uint8_t[]> buf1(new uint8_t[bufSize]);
      std::unique_ptr<uint8_t[]> buf2(new uint8_t[bufSize]);
      uint32_t offset = 0;
      size_t i;
      while( lengthLeft > 0) {
        size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
        if (!ESP.flashRead(src1->address + offset, reinterpret_cast<uint32_t*>(buf1.get()), (readBytes + 3) & ~3)
        || !ESP.flashRead(src2->address + offset, reinterpret_cast<uint32_t*>(buf2.get()), (readBytes + 3) & ~3)) {
            return false;
        }
        for (i = 0; i < readBytes; ++i) if (buf1[i] != buf2[i]) return false;
        lengthLeft -= readBytes;
        offset += readBytes;
      }
      return true;
    }


    bool comparePartition(const esp_partition_t* src1, fs::File* src2, size_t length)
    {
      size_t lengthLeft = length;
      const size_t bufSize = SPI_FLASH_SEC_SIZE;
      std::unique_ptr<uint8_t[]> buf1(new uint8_t[bufSize]);
      std::unique_ptr<uint8_t[]> buf2(new uint8_t[bufSize]);
      uint32_t offset = 0;
      uint32_t progress = 0, progressOld = 1;
      size_t i;
      while( lengthLeft > 0) {
        size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
        if (!ESP.flashRead(src1->address + offset, reinterpret_cast<uint32_t*>(buf1.get()), (readBytes + 3) & ~3)
        || !src2->read(                           reinterpret_cast<uint8_t*>(buf2.get()), (readBytes + 3) & ~3)
        ) {
          return false;
        }
        for (i = 0; i < readBytes; ++i) if (buf1[i] != buf2[i]) return false;
        lengthLeft -= readBytes;
        offset += readBytes;
        if( SDUCfg.onProgress ) {
          progress = 100 * offset / length;
          if (progressOld != progress) {
            progressOld = progress;
            SDUCfg.onProgress( (uint8_t)progress, 100 );
          }
        }
      }
      return true;
    }


    bool copyPartition( fs::FS* fs, const char* binfilename )
    {
      bool ret = false;
      running_partition = esp_ota_get_running_partition();
      nextupd_partition = esp_ota_get_next_update_partition(NULL);
      size_t sksize = ESP.getSketchSize();
      bool flgSD = fs?true:false;
      File dst;
      if (flgSD) {
        dst = (fs->open(binfilename, FILE_WRITE ));
      }
      ret = copyPartition( flgSD ? &dst : NULL, nextupd_partition, running_partition, sksize);
      if (flgSD) dst.close();
      return ret;
    }


    bool copyPartition(fs::File* dst, const esp_partition_t* src, size_t length)
    {
      size_t lengthLeft = length;
      const size_t bufSize = SPI_FLASH_SEC_SIZE;
      std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
      uint32_t offset = 0;
      uint32_t progress = 0, progressOld = 1;
      while( lengthLeft > 0) {
        size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
        if (!ESP.flashRead(src->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3) ) {
          return false;
        }
        if (dst) dst->write(buf.get(), (readBytes + 3) & ~3);
        lengthLeft -= readBytes;
        offset += readBytes;
        if( SDUCfg.onProgress ) {
          progress = 100 * offset / length;
          if (progressOld != progress) {
            progressOld = progress;
            SDUCfg.onProgress( (uint8_t)progress, 100 );
            vTaskDelay(10);
          }
        }
      }
      return true;
    }


    bool copyPartition(fs::File* dstFile, const esp_partition_t* dst, const esp_partition_t* src, size_t length)
    {
      if( dst->size < length ) {
        log_e("data won't fit in destination partition (available: %d, needed: %d)", dst->size, length );
        return false;
      }
      size_t lengthLeft = length;
      const size_t bufSize = SPI_FLASH_SEC_SIZE;
      std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
      uint32_t offset = 0;
      uint32_t progress = 0, progressOld = 0;
      while( lengthLeft > 0) {
        size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
        if (!ESP.flashRead(src->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)
        || !ESP.flashEraseSector((dst->address + offset) / bufSize)
        || !ESP.flashWrite(dst->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)) {
          return false;
        }
        if (dstFile) dstFile->write(buf.get(), (readBytes + 3) & ~3);
        lengthLeft -= readBytes;
        offset += readBytes;
        if( SDUCfg.onProgress ) {
          progress = 100 * offset / length;
          if (progressOld != progress) {
            progressOld = progress;
            SDUCfg.onProgress( (uint8_t)progress, 100 );
          }
        }
      }
      return true;
    }


    bool copyPartition(const esp_partition_t* dst, const esp_partition_t* src, size_t length)
    {
      if( dst->size < length ) {
        log_e("data won't fit in destination partition (available: %d, needed: %d)", dst->size, length );
        return false;
      }
      size_t lengthLeft = length;
      const size_t bufSize = SPI_FLASH_SEC_SIZE;
      std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
      uint32_t offset = 0;
      uint32_t progress = 0, progressOld = 0;
      while( lengthLeft > 0) {
        size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
        if (!ESP.flashRead(src->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)
        || !ESP.flashEraseSector((dst->address + offset) / bufSize)
        || !ESP.flashWrite(dst->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)) {
          return false;
        }
        lengthLeft -= readBytes;
        offset += readBytes;
        if( SDUCfg.onProgress ) {
          progress = 100 * offset / length;
          if (progressOld != progress) {
            progressOld = progress;
            SDUCfg.onProgress( (uint8_t)progress, 100 );
          }
        }
      }
      return true;
    }


    bool copyPartition(const esp_partition_t* dst, Stream* src, size_t length)
    {
      if( dst->size < length ) {
        log_e("data won't fit in destination partition (available: %d, needed: %d)", dst->size, length );
        return false;
      }
      size_t lengthLeft = length;
      const size_t bufSize = SPI_FLASH_SEC_SIZE;
      std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
      uint32_t offset = 0;
      uint32_t progress = 0, progressOld = 0;
      while( lengthLeft > 0) {
        size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
        if (!src->readBytes( reinterpret_cast<char*>(buf.get()), (readBytes + 3) & ~3)
        || !ESP.flashEraseSector((dst->address + offset) / bufSize)
        || !ESP.flashWrite(dst->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)) {
            return false;
        }
        lengthLeft -= readBytes;
        offset += readBytes;
        if( SDUCfg.onProgress ) {
          progress = 100 * offset / length;
          if (progressOld != progress) {
            progressOld = progress;
            SDUCfg.onProgress( (uint8_t)progress, 100 );
          }
        }
      }
      return true;
    }


    bool copyPartition(const esp_partition_t* dst, fs::FS *fs, const char* srcFilename)
    {
      if( !dst || !fs || !srcFilename ) return false;
      auto file = fs->open( srcFilename );
      if( !file ) return false;
      bool ret = copyPartition( dst, &file, file.size() );
      if( ret ) {
        // TODO: update NVS
      }
      file.close();
      return ret;
    }



    bool dumpFW( fs::FS *fs, const char* fw_name )
    {
      const uint32_t fw_size = ESP.getFlashChipSize();//0x1000000; // 0x1000000 = 16MB
      //ESP.getFlashChipSize();


      uint8_t buffer[SPI_FLASH_SEC_SIZE];

      size_t size_left = fw_size;
      uint32_t page_addr = 0;
      uint32_t total_written = 0;

      auto file = fs->open( fw_name, "w" );
      if( !file ) return false;

      log_w("Start dumping (estimated 0x%06x bytes to read)", fw_size );

      uint32_t progress = 0, progressOld = 0;

      while( size_left ) {
        #if ESP_IDF_VERSION_MAJOR < 5
          esp_err_t ret = spi_flash_read(page_addr, buffer, SPI_FLASH_SEC_SIZE);
        #else
          esp_err_t ret = esp_flash_read(NULL, buffer, page_addr, SPI_FLASH_SEC_SIZE);
        #endif
        if( ret ) return false;
        total_written += file.write( buffer, SPI_FLASH_SEC_SIZE );
        size_left -= SPI_FLASH_SEC_SIZE;
        page_addr += SPI_FLASH_SEC_SIZE;
        if( SDUCfg.onProgress ) {
          progress = 100 * page_addr / fw_size;
          if (progressOld != progress) {
            progressOld = progress;
            SDUCfg.onProgress( (uint8_t)progress, 100 );
          }
        }
      }

      log_w("Dumping finished, wrote 0x%06x bytes", total_written);

      file.close();
      return fw_size == total_written;
    }


    //***********************************************************************************************
    //                                B A C K T O F A C T O R Y                                     *
    //***********************************************************************************************
    // https://www.esp32.com/posting.php?mode=quote&f=2&p=19066&sid=5ba5f33d5fe650eb8a7c9f86eb5b61b8
    // Return to factory version.                                                                   *
    // This will set the otadata to boot from the factory image, ignoring previous OTA updates.     *
    //***********************************************************************************************
    void loadFactory()
    {
      factory_partition = getFactoryPartition();

      if( !factory_partition ) {
        log_e( "Failed to find factory partition" );
        return;
      }

      auto err = esp_ota_set_boot_partition ( factory_partition ) ; // Set partition for boot

      if ( err != ESP_OK ) {                         // Check error
        log_e( "Failed to set boot partition" ) ;
      } else {
        log_i("Will reboot to factory partition");
        esp_restart() ;                              // Restart ESP
      }
    }


    esp_image_metadata_t getSketchMeta( const esp_partition_t* src )
    {
      esp_image_metadata_t data;
      if ( !src ) {
        log_e("No source partition provided");
        return data;
      }
      const esp_partition_pos_t src_pos  = {
        .offset = src->address,
        .size = src->size,
      };
      data.start_addr = src_pos.offset;

      esp_app_desc_t app_desc;
      if( esp_ota_get_partition_description(src, &app_desc) != ESP_OK ) {
        // nothing flashed here
        memset( data.image_digest, 0, sizeof(data.image_digest) );
        data.image_len = 0;
        return data;
      }

      if( partitionIsOTA(src) ) { // only verify OTA partitions
        esp_err_t ret = esp_image_verify( ESP_IMAGE_VERIFY, &src_pos, &data );
        if( ret != ESP_OK ) {
          log_e("Failed to verify image %s at addr %x", String( src->label ), src->address );
        } else {
          log_v("Successfully verified image %s at addr %x", String( src->label[3] ), src->address );
        }
      } else if( partitionIsFactory(src)  ) { // compute the digest
        if( esp_partition_get_sha256(src, data.image_digest) != ESP_OK ) {
          memset( data.image_digest, 0, sizeof(data.image_digest) );
          data.image_len = 0;
        }
      }
      return data;
    }


    bool hasFactory()
    {
      factory_partition = getFactoryPartition();
      if(!factory_partition) return false;
      return true;
    }


    bool hasFactoryApp()
    {
      if( !hasFactory() ) return false;
      auto meta = getSketchMeta( factory_partition );
      digest_t digests = digest_t();
      if( digests.isEmpty( meta.image_digest ) ) return false;
      return true;
    }


    bool isRunningFactory()
    {
      running_partition = esp_ota_get_running_partition();
      if( !hasFactory() ) return false;
      return factory_partition==running_partition;
    }



    bool saveSketchToPartition( const esp_partition_t* dst_partition )
    {
      if (!running_partition) {
        log_e( "Can't fetch running partition info !!" );
        return false;
      }

      size_t sksize = ESP.getSketchSize();

      if (!comparePartition(running_partition, dst_partition, sksize)) {
        if( copyPartition( dst_partition, running_partition, sksize) ) {
          log_d("Sketch successfully propagated to destination partition");
          return true;
        } else {
          log_e("Sketch propagation to destination partition failed");
          return false;
        }
      } else {
        log_i("Current sketch and destination partition already match");
        return true;
      }
      return false;
    }



    bool saveSketchToFactory()
    {
      if( isRunningFactory() ) {
        log_d("Sketch is running from factory partition, no need to propagate");
        return false;
      }

      if( !hasFactory() ) {
        log_w( "This flash has no factory partition" );
        return false;
      }

      if (!running_partition) {
        log_e( "Can't fetch running partition info !!" );
        return false;
      }

      size_t sksize = ESP.getSketchSize();

      if (!comparePartition(running_partition, factory_partition, sksize)) {
        if( copyPartition( factory_partition, running_partition, sksize) ) {
          log_d("Sketch successfully propagated to factory partition");
          return true;
        } else {
          log_e("Sketch propagation to factory partition failed");
          return false;
        }
      } else {
        log_i("Current sketch and factory partition already match");
        return true;
      }
      return false;
    }


    const esp_partition_t* getFactoryPartition()
    {
      auto factorypi = esp_partition_find( ESP_PARTITION_TYPE_APP,  ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL );
      if( factorypi != NULL ) {
        return esp_partition_get(factorypi);
      }
      return NULL;
    }


    const esp_partition_t* getPartition( uint8_t ota_num )
    {
      esp_partition_subtype_t subtype = (esp_partition_subtype_t)(ESP_PARTITION_SUBTYPE_APP_OTA_MIN+ota_num);
      esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_APP, subtype, NULL);
      if( pi != NULL ) {
        const esp_partition_t* part = esp_partition_get(pi);
        esp_partition_iterator_release(pi);
        return part;
      }
      return nullptr;
    }


    Partition_t findPartition( uint8_t ota_num )
    {
      auto part = getPartition( ota_num );
      auto meta = esp_image_metadata_t();
      if( part ) {
        meta = getSketchMeta(part);
      }
      return {*part, meta};
    }


    Partition_t* findDupePartition( esp_image_metadata_t *meta, esp_partition_type_t type, esp_partition_subtype_t filter_subtype )
    {
      if( Partitions.size()==0 ) {
        scan();
      }
      digest_t digests = digest_t();
      for( int i=0; i<Partitions.size(); i++ ) {
        if( Partitions[i].part.type==type
         && Partitions[i].part.subtype!=filter_subtype
         && digests.match( meta->image_digest, Partitions[i].meta.image_digest ) ) {
          return &Partitions[i];
        }
      }
      return nullptr;
    }


    bool bootPartition( uint8_t ota_num )
    {
      const esp_partition_t* part = getPartition( ota_num );
      if ( !part ) {
        log_e( "Failed to find partition OTA%d", ota_num ) ;
        return false;
      }
      esp_err_t err = esp_ota_set_boot_partition ( part );
      if ( err != ESP_OK ) {
        log_e( "Failed to set OTA%d as boot partition", ota_num );
        return false;
      }
      log_i("Will reboot to partition OTA%d", ota_num);
      esp_restart() ;
      return true;
    }


    bool partitionIsApp( const esp_partition_t *part )
    {
      return part->type==ESP_PARTITION_TYPE_APP;
    }


    bool partitionIsFactory( const esp_partition_t *part )
    {
      return partitionIsApp( part ) && part->subtype==ESP_PARTITION_SUBTYPE_APP_FACTORY;
    }


    bool partitionIsOTA( const esp_partition_t *part )
    {
      return partitionIsApp( part ) && (part->subtype>=ESP_PARTITION_SUBTYPE_APP_OTA_MIN && part->subtype<ESP_PARTITION_SUBTYPE_APP_OTA_MAX);
    }


    bool metadataHasDigest( const esp_image_metadata_t *meta )
    {
      digest_t digests = digest_t();
      return meta && meta->image_digest ? !digests.isEmpty( meta->image_digest ) : false;
    }


    bool isEmpty( Partition_t* sdu_partition )
    {
      return partitionIsApp( &sdu_partition->part ) && !partitionIsFactory( &sdu_partition->part ) && !metadataHasDigest( &sdu_partition->meta );
    }


    const esp_partition_t* getNextAvailPartition(esp_partition_type_t type, esp_partition_subtype_t filter_subtype)
    {
      if( Partitions.size()==0 ) {
        scan();
      }
      for( int i=0; i<Partitions.size(); i++ ) {
        if( Partitions[i].part.type !=type ) continue;
        if( Partitions[i].part.subtype ==filter_subtype ) continue;
        if( isEmpty( &Partitions[i] ) ) {
          return &Partitions[i].part;
        }
      }
      return nullptr;
    }


    void memoize( const esp_partition_t *part )
    {
      esp_image_metadata_t meta = esp_image_metadata_t();

      if( partitionIsApp(part) ) {
        meta  = getSketchMeta( part );
      }
      Partitions.push_back({*part,meta});
      if( partitionIsFactory( part ) ) {
        log_v("Found factory partition");
        FactoryPartition = &Partitions[Partitions.size()-1];
      }
    }


    void scan()
    {
      if( Partitions.size()>0 ) {
        Partitions.clear(); // reset last scan results
      }
      esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
      while(pi != NULL) {
        const esp_partition_t* part = esp_partition_get(pi);
        memoize( part );
        pi = esp_partition_next( pi );
      }
      esp_partition_iterator_release(pi);
    }


    bool erase( const esp_partition_t *part )
    {
      assert(part);
      log_d("Erasing ota partition %#x", part->subtype );
      if( part && ESP.partitionEraseRange(part, 0, part->size ) ) {
        return true;
      }
      log_e("FATAL: can't erase partition");
      return false;
    }


    bool erase( uint8_t ota_num )
    {
      auto part = getPartition( ota_num );
      return erase( part );
    }

  };
};
