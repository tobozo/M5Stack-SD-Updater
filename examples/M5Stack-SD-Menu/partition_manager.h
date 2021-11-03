/*
 *
 * M5Stack SD Menu
 * Project Page: https://github.com/tobozo/M5Stack-SD-Updater
 *
 * Copyright 2019 tobozo http://github.com/tobozo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files ("M5Stack SD Updater"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*
static esp_image_metadata_t getSketchMeta( const esp_partition_t* running ) {
  esp_image_metadata_t data;
  if ( !running ) return data;
  const esp_partition_pos_t running_pos  = {
    .offset = running->address,
    .size = running->size,
  };
  data.start_addr = running_pos.offset;
  esp_image_verify( ESP_IMAGE_VERIFY, &running_pos, &data );
  return data;
}
*/

/*

bool compareFsPartition(const esp_partition_t* src1, fs::File* src2, size_t length) {
  size_t lengthLeft = length;
  const size_t bufSize = SPI_FLASH_SEC_SIZE;
  std::unique_ptr<uint8_t[]> buf1(new uint8_t[bufSize]);
  std::unique_ptr<uint8_t[]> buf2(new uint8_t[bufSize]);
  uint32_t offset = 0;
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
  }
  return true;
}


bool copyFsPartition(File* dst, const esp_partition_t* src, size_t length) {
  //tft.fillRect( 110, 112, 100, 20, 0);
  size_t lengthLeft = length;
  const size_t bufSize = SPI_FLASH_SEC_SIZE;
  std::unique_ptr<uint8_t[]> buf(new uint8_t[bufSize]);
  uint32_t offset = 0;
  uint32_t progress = 0, progressOld = 0;
  while( lengthLeft > 0) {
    size_t readBytes = (lengthLeft < bufSize) ? lengthLeft : bufSize;
    if (!ESP.flashRead(src->address + offset, reinterpret_cast<uint32_t*>(buf.get()), (readBytes + 3) & ~3)
    ) {
        return false;
    }
    if (dst) dst->write(buf.get(), (readBytes + 3) & ~3);
    lengthLeft -= readBytes;
    offset += readBytes;
    progress = 100 * offset / length;
    if (progressOld != progress) {
      progressOld = progress;
      //tft.progressBar( 110, 112, 100, 20, progress);
    }
  }
  return true;
}


void copyFsPartition( fs::FS &fs, const char* binfilename = PROGMEM {MENU_BIN} ) {
  const esp_partition_t *running = esp_ota_get_running_partition();
  size_t sksize = ESP.getSketchSize();
  fs::File dst = fs.open(binfilename, FILE_WRITE );
  //tft.drawString("Overwriting " MENU_BIN, 160, 38, 2);
  if (copyFsPartition( &dst, running, sksize)) {
    //tft.drawString("Done", 160, 52, 2);
  }
  dst.close();
}
*/

#pragma once
#include "core.h"


// from https://github.com/lovyan03/M5Stack_LovyanLauncher
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

// from https://github.com/lovyan03/M5Stack_LovyanLauncher
bool copyPartition(File* fs, const esp_partition_t* dst, const esp_partition_t* src, size_t length)
{
  tft.fillRect( 110, 112, 100, 20, 0);
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
    if (fs) fs->write(buf.get(), (readBytes + 3) & ~3);
    lengthLeft -= readBytes;
    offset += readBytes;
    progress = 100 * offset / length;
    if (progressOld != progress) {
      progressOld = progress;
      tft.progressBar( 110, 112, 100, 20, progress);
    }
  }
  return true;
}


void copyPartition( const char* binfilename = PROGMEM {MENU_BIN} )
{
  const esp_partition_t *running = esp_ota_get_running_partition();
  const esp_partition_t *nextupdate = esp_ota_get_next_update_partition(NULL);
  //const char* menubinfilename PROGMEM {MENU_BIN} ;
  size_t sksize = ESP.getSketchSize();
  bool flgSD = M5_FS.begin();
  File dst;
  if (flgSD) {
    dst = (M5_FS.open(binfilename, FILE_WRITE ));
    tft.drawString("Overwriting " MENU_BIN, 160, 38, 2);
  }
  if (copyPartition( flgSD ? &dst : NULL, nextupdate, running, sksize)) {
    tft.drawString("Done", 160, 52, 2);
  }
  if (flgSD) dst.close();
}



struct {
  String toString( uint8_t dig[32] ) {
    static String digest;
    digest = "";
    char hex[3] = {0};
    for(int i=0;i<32;i++) {
      snprintf( hex, 3, "%02x", dig[i] );
      digest += String(hex);
    }
    return digest;
  }
} digest;


void lsPart()
{
  #if defined ESP_PARTITION_TYPE_ANY
    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    log_w("Partition  Type   Subtype    Address   PartSize   ImgSize   Digest");
    log_w("---------+------+---------+----------+----------+---------+--------");
    while(pi != NULL) {
      const esp_partition_t* part = esp_partition_get(pi);
      esp_image_metadata_t meta;
      bool isOta = (part->label[3]=='1' || part->label[3] == '0');
      if( isOta ) meta  = sdUpdater->getSketchMeta( part );
      log_w("%-8s   0x%02x      0x%02x   0x%06x   %8d  %8s   %s",
        String( part->label ),
        part->type,
        part->subtype,
        part->address,
        part->size,
        isOta ? String(meta.image_len) : "n/a",
        isOta ? digest.toString(meta.image_digest).c_str() : "n/a"
      );
      pi = esp_partition_next( pi );
    }
    esp_partition_iterator_release(pi);
  #endif
}


// from https://github.com/lovyan03/M5Stack_LovyanLauncher
void checkMenuStickyPartition( const char* menubinfilename = PROGMEM {MENU_BIN} )
{
  const esp_partition_t* running = esp_ota_get_running_partition();
  const esp_partition_t* nextupdate = esp_ota_get_next_update_partition(NULL);

  if (!running) {
    log_e( "Can't fetch running partition info !!" );
    return;
  }
  if (!nextupdate) {
    log_e( "Can't fetch nextupdate partition info !!" );
    return;
  }

  lsPart();

  //const char* menubinfilename PROGMEM {MENU_BIN} ;
  tft.setTextDatum(MC_DATUM);
  tft.setCursor(0,0);
  if (!nextupdate) {
    // tft.setTextFont(4);
    tft.print("! WARNING !\r\nNo OTA partition.\r\nCan't use SD-Updater.");
    delay(3000);
  } else if (running && running->label[3] == '0' && nextupdate->label[3] == '1') {
    tft.drawString("Checking 'app0' partition", 160, 10, 2);
    size_t sksize = ESP.getSketchSize();
    if (!comparePartition(running, nextupdate, sksize)) {
      // TODO: handle SPIFFS
      bool flgSD = M5_FS.begin( /*TFCARD_CS_PIN, SPI, 40000000*/ );
      tft.drawString("Synchronizing to 'app1' partition", 160, 24, 2);
      File dst;
      if (flgSD) {
        dst = (M5_FS.open(menubinfilename, FILE_WRITE ));
        tft.drawString("Overwriting " MENU_BIN, 160, 38, 2);
      }
      if (copyPartition( flgSD ? &dst : NULL, nextupdate, running, sksize)) {
        tft.drawString("Done", 160, 52, 2);
      }
      if (flgSD) dst.close();
    }
    SDUpdater::updateNVS();
    tft.drawString("Hot-loading 'app1' partition", 160, 66, 2);
    if (Update.canRollBack()) {
      Update.rollBack();
      ESP.restart();
    } else {
      tft.print("! WARNING !\r\nUpdate.rollBack() failed.");
      log_e("Failed to rollback after copy");
    }
  }
  tft.setTextDatum(TL_DATUM);
}
