#pragma once

// fs::FS layer for SdFat
// Inspired by @ockernuts https://github.com/ockernuts
// See https://github.com/greiman/SdFat/issues/148#issuecomment-1464448806

#if defined SDU_HAS_SDFS

  #if !defined SDFAT_FILE_TYPE
    #define SDFAT_FILE_TYPE 3 // tell SdFat.h to support all filesystem types (fat16/fat32/ExFat)
  #endif
  #if SDFAT_FILE_TYPE!=3
    #error "SD Updater only supports SdFs"
  #endif

  #include "../misc/config.h"
  #include "../misc/types.h"

  #undef __has_include // tell SdFat to define 'File' object needed to convert access mode to flag
  #include <FS.h>
  #include <FSImpl.h>
  #include <SdFat.h>
  #define __has_include(STR)  __has_include__(STR) // kudos to @GOB52 for this trick

  // cfr https://en.cppreference.com/w/c/io/fopen + guesses
  inline oflag_t _convert_access_mode_to_flag(const char* mode, const bool create = false)
  {
    int mode_chars = strlen(mode);
    if (mode_chars==0) return O_RDONLY;
    if (mode_chars==1) {
      if (mode[0]=='r') return O_RDONLY;
      if (mode[0]=='w') return O_WRONLY | create ? O_CREAT : 0;
      if (mode[0]=='a') return O_APPEND | create ? O_CREAT : 0;
    }
    if (mode_chars==2) {
      if (mode[1] ==  '+') {
        if (mode[0] == 'r') return O_RDWR;
        if (mode[0] == 'w') return O_RDWR | O_CREAT;
        if (mode[0] == 'a') return O_RDWR | O_APPEND | O_CREAT;
      }
    }
    return O_RDONLY;
  }


  class SdFsFileImpl : public fs::FileImpl
  {
    private:
      mutable FsFile _file;
    public:
      SdFsFileImpl(FsFile file) : _file(file) {}
      virtual ~SdFsFileImpl() { }

      virtual size_t write(const uint8_t *buf, size_t size) { return _file.write(buf, size); }
      virtual size_t read(uint8_t* buf, size_t size) { return _file.read(buf, size); }
      virtual void flush() { return _file.flush(); }
      virtual size_t position() const { return _file.curPosition(); }
      virtual size_t size() const { return _file.size(); }
      virtual void close() { _file.close(); }
      virtual operator bool() { return _file.operator bool(); }
      virtual boolean isDirectory(void) { return _file.isDirectory(); }
      virtual fs::FileImplPtr openNextFile(const char* mode) { return  std::make_shared<SdFsFileImpl>(_file.openNextFile(_convert_access_mode_to_flag(mode))); }
      virtual boolean seekDir(long position) { return _file.seek(position); }
      virtual bool seek(uint32_t pos, fs::SeekMode mode)
      {
        if (mode == fs::SeekMode::SeekSet) {
          return _file.seek(pos);
        } else if (mode == fs::SeekMode::SeekCur) {
          return _file.seek(position()+ pos);
        } else if (mode == fs::SeekMode::SeekEnd) {
          return _file.seek(size()-pos);
        }
        return false;
      }
      virtual const char* name() const
      {
        // static, so if one asks the name of another file the same buffer will be used.
        // so we assume here the name ptr is not kept. (anyhow how would it be dereferenced and then cleaned...)
        static char _name[256];
        _file.getName(_name, sizeof(_name));
        return _name;
      }

      virtual String getNextFileName(void) { /* not implemented and not needed */ return String("Unimplemented"); }
      virtual String getNextFileName(bool*) { /* not implemented and not needed */ return String("Unimplemented"); }
      virtual time_t getLastWrite() { /* not implemented and not needed */  return 0; }
      virtual const char* path() const { /* not implemented and not needed */ return nullptr; }
      virtual bool setBufferSize(size_t size) { /* not implemented and not needed */ return false; }
      virtual void rewindDirectory(void) { /* not implemented and not needed */  }
  };


  class SdFsFSImpl : public fs::FSImpl
  {
    SdFs& sd;
    public:
      SdFsFSImpl(SdFs& sd) : sd(sd) { }
      virtual ~SdFsFSImpl() {}
      virtual fs::FileImplPtr open(const char* path, const char* mode, const bool create)
      {
          return std::make_shared<SdFsFileImpl>(sd.open(path, _convert_access_mode_to_flag(mode, create)));
      }
      virtual bool exists(const char* path) { return sd.exists(path); }
      virtual bool rename(const char* pathFrom, const char* pathTo) { return sd.rename(pathFrom, pathTo); }
      virtual bool remove(const char* path) { return sd.remove(path); }
      virtual bool mkdir(const char *path) { return sd.mkdir(path); }
      virtual bool rmdir(const char *path) { return sd.rmdir(path); }
  };



  namespace SDUpdaterNS
  {

    namespace ConfigManager
    {
      static void *SDU_SdFatPtr = nullptr;
      static fs::FS *SDU_SdFatFsPtr = nullptr;
      static SdSpiConfig *SDU_SdSpiConfigPtr = nullptr;
    };

    inline fs::FS* getSdFsFs( SdFs &sd )
    {
      static fs::FS _fs = fs::FS(fs::FSImplPtr(new SdFsFSImpl(sd)));
      return &_fs;
    }


    inline ConfigManager::FS_Config_t* SDU_SDFAT_GET()
    {
      static ConfigManager::FS_Config_t SDFAT_FS_Config = {"sdfat", ConfigManager::SDU_SdFatPtr, ConfigManager::SDU_SdSpiConfigPtr};
      return &SDFAT_FS_Config;
    }

    //#define SDU_CONFIG_SDFAT ConfigManager::SDU_SdSpiConfigPtr


    inline bool SDU_SDFat_Begin( SdSpiConfig *SdFatCfg )
    {
      using namespace ConfigManager;
      if( !SDU_SdFatPtr ) {
        log_e("SDFat is not set");
        return false;
      }

      bool ret = false;
      int errcode = 0, errdata = 0;
      auto _fat = (SdFs*)SDU_SdFatPtr;
      ret = _fat->begin( *SdFatCfg );
      errcode = _fat->card()->errorCode();
      errdata = int(_fat->card()->errorData());
      (void)errcode;
      (void)errdata;
      if (!ret) {
        log_e( "SDFat init failed with error code: 0x%x, Error Data:0x%x", errcode, errdata );
        return false;
      }
      return ret;
    }


  };


#endif

