#pragma once
#define SDU_NVSUTILS_HPP

//#include <Arduino.h>
//#include <Preferences.h>
#include <cstring>
#include <ctype.h>
#include <stdio.h>
#include <vector>
#include <esp32-hal-log.h>
#include <esp_partition.h>
#include <nvs_flash.h>
#include <Stream.h>
#include <StreamString.h>

#include "../Partitions/PartitionUtils.hpp"

#define SDU_PARTITION_NS "sdu"
#define SDU_PARTITION_KEY "partitions"

namespace SDUpdaterNS
{

  namespace NVS
  {

    typedef enum { T_SIGNED=0, T_UNSIGNED=1, T_STRING=2, T_BLOB=4 } Types_t;

    typedef void (*EntryCb_t)(const char*typStr, uint8_t bytewid, const char*nsStr, const char*key, const char*val);

    struct Entry_t
    {
      uint8_t  Ns ;         // Namespace ID
      uint8_t  Type ;       // Type of value
      uint8_t  Span ;       // Number of entries used for this item
      uint8_t  Rvs ;        // Reserved, should be 0xFF
      uint32_t CRC ;        // CRC
      char     Key[16] ;    // Key in Ascii
      uint64_t Data ;       // Data in entry
    };

    struct Page_t // For nvs entries
    {              // 1 page is 4096 bytes
      uint32_t  State ;
      uint32_t  Seqnr ;

      uint32_t  Unused[5] ;
      uint32_t  CRC ;
      uint8_t   Bitmap[32] ;
      struct Entry_t Entry[126] ;
    };

    struct NSIndex_t
    {
      uint16_t names[256];
      uint16_t next;
      char     buf[2048];
    };

    struct Iterator_t
    {
      struct Page_t          page;        // Holds current page in partition
      const esp_partition_t* nvs;         // Pointer to partition struct
      char                   prefix[16];
      struct NSIndex_t*      nsindex;     // Holds namespace names
      uint8_t                namespace_ID;
      uint32_t               offset;      // Offset in nvs partition
      int32_t                ix;          // Index in Entry 0..125
    };

    typedef struct Iterator_t* IteratorPtr_t;

    typedef struct IterType_t
    {
      Types_t nvs_type;
      uint8_t bytewid;
    } IteratorTyped_t;


    // NVS representation of flash partition
    struct __attribute__((__packed__)) PartitionDesc_t
    {
      uint8_t ota_num{0};    // OTA partition number
      size_t  bin_size{0};   // firmware size
      uint8_t digest[32]{0}; // firmware digest
      char    name[40]{0};   // firmware name
      char    desc[40]{0};   // firmware desc
    };

    extern nvs_handle_t handle;
    extern std::vector<NVS::PartitionDesc_t> Partitions; // filled by NVS

    PartitionDesc_t* FindPartition( uint8_t ota_num );
    PartitionDesc_t* FindPartition( Flash::Partition_t* flash_partition );

    int         Dump( const char* searchStr = nullptr, EntryCb_t entryCb=NULL );
    int         Erase();
    bool        GetBLobPartitions();
    bool        ParseBlobPartitions( const char* blob, size_t size );

    char*       IterateGetNamespace( struct Iterator_t *it, const char *key );
    char*       IterateTypeToString( IteratorTyped_t *nityp, char *buf );
    uint64_t    IterateGetData( const char *key );
    void        IterateDone( struct Iterator_t *it );
    bool        IterateNext( struct Iterator_t *it, const char **key, IteratorTyped_t *typ );
    IteratorPtr_t IterateNew( const char *nmspace, const char *prefix );
    uint8_t     FindNsID( const esp_partition_t* nvs, const char* ns, struct NVS::NSIndex_t **indexP );

  };

}; // end namespace SDUpdaterNS

