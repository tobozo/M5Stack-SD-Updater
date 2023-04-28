#pragma once

// fs::FS layer for SdFat
// Inspired by @ockernuts https://github.com/ockernuts
// See https://github.com/greiman/SdFat/issues/148#issuecomment-1464448806

#if defined USE_SDFATFS

  #include <FS.h>
  #include <FSImpl.h>
  #include <SdFat.h>

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


  class SdFatFile32Impl : public fs::FileImpl
  {
    private:
      mutable File32 _file;
    public:
      SdFatFile32Impl(File32 file) : _file(file) {}
      virtual ~SdFatFile32Impl() { }

      virtual size_t write(const uint8_t *buf, size_t size) { return _file.write(buf, size); }
      virtual size_t read(uint8_t* buf, size_t size) { return _file.read(buf, size); }
      virtual void flush() { return _file.flush(); }
      virtual size_t position() const { return _file.curPosition(); }
      virtual size_t size() const { return _file.size(); }
      virtual bool setBufferSize(size_t size) { /* don't know how to implement... */ return false; }
      virtual void close() { _file.close(); }
      virtual time_t getLastWrite() { /* too lazy to implement ... */ return 0; }
      virtual const char* path() const { /* too lazy to implement ... */ return nullptr; }
      virtual boolean isDirectory(void) { return _file.isDirectory(); }
      virtual fs::FileImplPtr openNextFile(const char* mode) { return  std::make_shared<SdFatFile32Impl>(_file.openNextFile(_convert_access_mode_to_flag(mode))); }
      virtual boolean seekDir(long position) { return _file.seek(position); }
      virtual String getNextFileName(void) { return String("Unimplemented"); }
      virtual String getNextFileName(bool*) { return String("Unimplemented"); }
      virtual void rewindDirectory(void) { return; }
      virtual operator bool() { return _file.operator bool(); }
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
  };


  class SdFat32FSImpl : public fs::FSImpl
  {
    SdFat32& sd;
    public:
      SdFat32FSImpl(SdFat32& sd) : sd(sd) { }
      virtual ~SdFat32FSImpl() {}
      virtual fs::FileImplPtr open(const char* path, const char* mode, const bool create) {
          return std::make_shared<SdFatFile32Impl>(sd.open(path, _convert_access_mode_to_flag(mode, create)));
      }
      virtual bool exists(const char* path) { return sd.exists(path); }
      virtual bool rename(const char* pathFrom, const char* pathTo) { return sd.rename(pathFrom, pathTo); }
      virtual bool remove(const char* path) { return sd.remove(path); }
      virtual bool mkdir(const char *path) { return sd.mkdir(path); }
      virtual bool rmdir(const char *path) { return sd.rmdir(path); }
  };


  static SdFat32 *SDU_SdFatPtr = nullptr;
  static fs::FS *SDU_SdFat32FsPtr = nullptr;
  inline bool SDU_SDFatBegin( int cs, int freq )
  {
    if( !SDU_SdFatPtr ) {
      log_e("SDFat is not set");
      return false;
    }
    bool ret = SDU_SdFatPtr->begin( cs, freq );
    if (SDU_SdFatPtr->card()->errorCode()) {
      log_e( "SDFat init failed with error code: 0x%x, Error Data:0x%x", SDU_SdFatPtr->card()->errorCode(), int(SDU_SdFatPtr->card()->errorData()) );
      return false;
    }
    return ret;
  }

#endif
