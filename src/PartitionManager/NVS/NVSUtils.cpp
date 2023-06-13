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


    PartitionDesc_t* FindPartition( uint8_t ota_num )
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


    PartitionDesc_t* FindPartition( Flash::Partition_t* flash_partition )
    {
      assert( flash_partition );
      auto ota_num = flash_partition->part.subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN;
      auto nvs_part = FindPartition( ota_num );

      if( nvs_part ) {
        if( Flash::MetadataHasDigest( &flash_partition->meta ) ) {
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


    char *IterateTypeToString(struct IterType_t *nityp, char *buf)
    {
      strcpy(buf, nityp->nvs_type == T_SIGNED   ? "int" :
                  nityp->nvs_type == T_UNSIGNED ? "uint" :
                  nityp->nvs_type == T_STRING   ? "string" :
                  nityp->nvs_type == T_BLOB     ? "blob" : "<unknown>"
      );
      if (nityp->nvs_type == T_SIGNED || nityp->nvs_type == T_UNSIGNED) {
        sprintf(buf + strlen(buf), "%d", 8 * nityp->bytewid);
      }
      return buf;
    }


    uint64_t IterateGetData(const char *key)
    {
      return *((uint64_t *) (key + 16));
    }


    char *IterateGetNamespace(struct Iterator_t *it, const char *key)
    {
      size_t off = offsetof(struct Entry_t, Key);
      struct Entry_t *ent = (struct Entry_t *) (key - off);
      return NULL == it->nsindex ? (char*)"<NONE>" : (it->nsindex->buf + it->nsindex->names[ent->Ns]);
    }


    // Find the namespace ID for the namespace passed as parameter.                                    *
    uint8_t FindNsID(const esp_partition_t* nvs, const char* ns, struct NSIndex_t **indexP)
    {
      esp_err_t      result = ESP_OK; // Result of reading partition
      uint32_t       offset = 0;      // Offset in nvs partition
      uint8_t        bm;              // Bitmap for an entry
      uint8_t        res = 0xFF;      // Function result
      struct Page_t *page = (struct Page_t*)malloc(sizeof(struct Page_t));
      struct NSIndex_t *index = NULL;

      if ( NULL == ns ) {
        index = (struct NSIndex_t *) malloc(sizeof(struct NSIndex_t));
        strcpy(index->buf, "<OVF>");
        index->next = 6;
        *indexP = index;
      }

      while ( offset < nvs->size ) {
        // Read 1 page in nvs partition
        result = esp_partition_read ( nvs, offset,  page, sizeof(struct Page_t) );
        log_v("NSREAD: %d/%d -> %d", offset, nvs->size, result);
        if ( result != ESP_OK ) {
          log_e( "Error reading NVS!" );
          break;
        }
        int i = 0;
        while ( i < 126 ) {
          // Get bitmap for this entry
          bm = ( page->Bitmap[i/4] >> ( ( i % 4 ) * 2 ) ) & 0x03;
          if ( ( bm == 2 ) && ( page->Entry[i].Ns == 0 ) && ( NULL == ns || strcmp ( ns, page->Entry[i].Key ) == 0 ) )  {
            // Return the ID
            res = page->Entry[i].Data & 0xFF;
            if ( NULL == ns ) {
              int len = strlen(page->Entry[i].Key) + 1;
              log_v("NS(%d): %s", res, page->Entry[i].Key);
              if (index->next + len > 2048) {
                index->names[res] = 0;
              } else {
                index->names[res] = index->next;
                strcpy(index->buf + index->next, page->Entry[i].Key);
                index->next += len;
              }
              i += page->Entry[i].Span;// Next entry
            } else {
              log_v("Found NS(%d): %s", res, page->Entry[i].Key);
              offset = nvs->size;  // Stop outer loop as well
              break;
            }
          } else {
            if ( bm == 2 ) {
              i += page->Entry[i].Span;                          // Next entry
              log_v("KEY(%d): %s", page->Entry[i].Ns, page->Entry[i].Ns == 0xFF ? "<ANY>" : page->Entry[i].Key);
            } else {
              i++;
            }
          }
        }
        offset += sizeof(struct Page_t); // Prepare to read next page in nvs
      }
      free(page);
      return res;
    }


    IteratorPtr_t IterateNew(const char *strNamespace, const char *prefix)
    {
      esp_partition_iterator_t  pi;  // Iterator for find
      const esp_partition_t*    nvs; // Pointer to partition struct
      struct Iterator_t *it;
      struct NSIndex_t *index;
      uint8_t namespace_ID;
      // Get partition iterator for this partition
      pi = esp_partition_find ( ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "nvs" );
      if ( pi ) {
        nvs = esp_partition_get ( pi );        // Get partition struct
        esp_partition_iterator_release ( pi ); // Release the iterator
      } else {
        log_e( "NVS Partition not found!" );
        return NULL;
      }
      if ( strNamespace && *strNamespace ) {
        namespace_ID = FindNsID ( nvs, strNamespace, NULL );
        index = NULL;
        if ( namespace_ID == 0xFF ) {
          log_e( "NVS namespace not found!" );
          return NULL;
        }
      } else {
        namespace_ID = 0xFF;
        FindNsID ( nvs, NULL, &index );
      }

      it = (struct Iterator_t *) malloc(sizeof(struct Iterator_t));
      assert(it);

      it->namespace_ID = namespace_ID;
      strcpy(it->prefix, prefix ? prefix : "");
      it->nvs = nvs;
      it->ix = -1;
      it->offset = 0;
      it->nsindex = index;
      log_v("OFF:%d/%d IX:%d NS:%d", it->offset, it->nvs->size, it->ix, it->namespace_ID);
      return it;
    }


    void IterateDone(struct Iterator_t *it)
    {
      if ( NULL != it ) {
        if ( it->nsindex ) free(it->nsindex);
        free(it);
      }
    }


    bool IterateNext(struct Iterator_t *it, const char **key, IteratorTyped_t *typ)
    {
      esp_err_t result = ESP_OK;

      log_v("OFF:%d/%d IX:%d", it->offset, it->nvs->size, it->ix);
      while ( it->offset < it->nvs->size ) {
        if (it->ix < 0 || it->ix >= 126) {
          // Read 1 page in nvs partition
          result = esp_partition_read ( it->nvs, it->offset, &it->page, sizeof(struct Page_t) );
          log_v("NSREAD: %d/%d -> %d", it->offset, it->nvs->size, result);
          if ( result != ESP_OK ) {
            log_e( "Error reading NVS!" );
            return  false;
          }
          it->ix = 0;
          it->offset += sizeof(struct Page_t); // Prepare to read next page in nvs
        }

        while ( it->ix < 126 ) {
          uint8_t bm = ( it->page.Bitmap[it->ix/4] >> ( ( it->ix % 4 ) * 2 ) ) & 0x03;  // Get bitmap for this entry
          if ( bm == 2 ) {
            const char *ky = it->page.Entry[it->ix].Key;
            uint8_t tp = it->page.Entry[it->ix].Type;
            int ns = it->page.Entry[it->ix].Ns;
            it->ix += it->page.Entry[it->ix].Span; // Next entry
            // Show all if ID = 0xFF  otherwise just my namespace
            if(  ( ( it->namespace_ID == 0xFF && ns > 0 && ns < 0xFF ) ||  ns == it->namespace_ID )
              && ( ! it->prefix[0] || 0 == strncmp(it->prefix, ky, strlen(it->prefix)) )
            ) {
              typ->nvs_type = (Types_t)((tp >> 4) & 0xf);
              typ->bytewid = tp & 0xf;
              *key = ky;
              return true;
            }
          } else {
            it->ix++;
          }
        }
      }
      return false;
    }


    void printBlobData( const char* data, size_t len, Stream *out, uint32_t max_read=32 )
    {
      bool isPrintable = true;
      for( int i=0; i<len; i++ ) {
        if( data[i] !=0x00 && data[i] !=0x0a && !isprint(data[i]) ) {
          isPrintable = false;
          break;
        }
      }
      out->printf("%s(%d): %s", isPrintable?"String":"Blob", len, isPrintable?"":"0x");
      for( int i=0; i<len; i++ ) {
        if( !isPrintable && i >= max_read ) { // max bytes to read in the blob
          out->print("...[truncated]");
          break;
        }
        if( isPrintable ) {
          char sym[2] = {data[i],0};
          if( isprint( data[i] ) ) out->printf( "%s", sym );
        } else {
          out->printf("%02x", data[i]);
        }
      }
    }


    bool GetBLobPartitions()
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

      ret = ParseBlobPartitions( out, blob_size-1 );
      free( out );

      _nvs_close:
      nvs_close( handle );
      return ret;
    }


    bool ParseBlobPartitions( const char* blob, size_t size )
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


    int Dump( const char* searchStr, EntryCb_t entryCb )
    {
      const char *key;
      IteratorTyped_t typ;

      IteratorPtr_t it = IterateNew( NULL, NULL );

      if (!it) {
        printf("Can not iterate nvs\n");
        return -1;
      }

      printf("*********************************\n  NVS Dump\n*********************************\n");

      nvs_stats_t nvs_stats;
      nvs_get_stats(NULL, &nvs_stats);
      auto usedpercent = (float(nvs_stats.used_entries) / float(nvs_stats.total_entries)) * 100;
      //printf("Used: %.2f%%, Free: %.2f%%\n", usedpercent, 100.0-usedpercent );
      printf("Used: %.2f%% (%d Entries), Free: %.2f%% (%d Entries), Total avail: %d Entries\n",
        usedpercent,
        nvs_stats.used_entries,
        100.0-usedpercent,
        nvs_stats.free_entries,
        nvs_stats.total_entries
      );

      char buf[10];
      while (IterateNext(it, &key, &typ)) {
        char val[200];
        uint64_t      ival = 0;
        uint64_t      data = IterateGetData(key);
        memcpy(&ival, &data, typ.bytewid);
        const char*  nsStr = IterateGetNamespace(it, key);
        const char* typStr = IterateTypeToString(&typ, buf);
        nvs_handle_t handle;

        switch( typ.nvs_type ) {
          case T_SIGNED:
          case T_UNSIGNED: {
            sprintf(val, "%llu", ival);
          } break;
          case T_STRING: {
            auto err = nvs_open(nsStr, NVS_READONLY, &handle);
            if( err == ESP_OK ) {
              size_t required_size;
              err = nvs_get_str(handle, key, NULL, &required_size);
              if( err == ESP_OK ) {
                char* string_out = (char*)malloc(required_size);
                nvs_get_str(handle, key, string_out, &required_size);
                snprintf( val, 199, "%s", string_out );
                free( string_out );
              } else {
                log_e("Failed to get string for %s::%s (errcode=%d)", nsStr, key, err );
              }
              nvs_close( handle );
            } else {
              log_e("Failed to nvs_open %s::%s (errcode=%d)", nsStr, key, err );
            }
          } break;
          case T_BLOB: {
            if( typ.bytewid == 2 ) { // blob data
              continue;
            } else { // blob index

              struct blobIndex_t
              {
                uint32_t size; // BlobSize
                uint8_t count; // ChunkCount
                uint8_t start; // ChunkStart
              };
              blobIndex_t *blobIdx = (blobIndex_t*)&data;

              auto err = nvs_open(nsStr, NVS_READONLY, &handle);
              if( err == ESP_OK ) {
                char* out = (char*)calloc(blobIdx->size+1, sizeof(char));
                err = nvs_get_blob(handle, key, out, &blobIdx->size);
                if( err == ESP_OK ) {
                  StreamString blah;
                  printBlobData( out, blobIdx->size, &blah );
                  String outstr = blah;
                  snprintf( val, 199, "%s", outstr.c_str() );
                } else {
                  snprintf( val, 199, "BLOB READ FAIL (%u bytes, %d chunks, start=%d)", blobIdx->size, blobIdx->count, blobIdx->start );
                }
                free( out );
              } else {
                snprintf( val, 199, "BLOB OPEN FAIL (%u bytes, %d chunks, start=%d)", blobIdx->size, blobIdx->count, blobIdx->start );
              }
              nvs_close( handle );
            }
          } break;
        }

        if(entryCb ) {
          entryCb( typStr, typ.bytewid, nsStr, key, val );
        } else {
          printf("%-6s(%d) %15s:%-15s = %s\n", typStr, typ.bytewid, nsStr, key, val );
        }
      }

      IterateDone(it);
      return 0;
    }

  }; // end namespace NVS

}; // end namespace SDUpdaterNS
