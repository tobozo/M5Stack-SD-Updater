#pragma once


#if defined SDU_HAS_TARGZ // binary may or may not be gzipped

  // some macros with if(gz) logic
  #define GzUpdate GzUpdateClass::getInstance()
  #define GzUpdateEnd() (mode_z ? GzUpdate.endgz() : GzUpdate.end())
  #define GzAbort() if (mode_z) GzUpdate.abortgz(); else GzUpdate.abort()
  #define GzWriteStream(updateSource,updateSize) (mode_z ? GzUpdate.writeGzStream(updateSource,updateSize) : GzUpdate.writeStream(updateSource))
  #define GzCanBegin( usize ) (mode_z ? GzUpdate.begingz(UPDATE_SIZE_UNKNOWN) : GzUpdate.begin(usize))
  #define GzEnd() (mode_z ? GzUpdate.endgz() : GzUpdate.end() )

  namespace SDUpdaterNS
  {
    namespace ConfigManager
    {
      using namespace UpdateInterfaceNS;
      inline UpdateManagerInterface_t *GetUpdateInterface()
      {
        static UpdateManagerInterface_t Iface =
        {
          .begin       = [](size_t s)->bool{ return GzCanBegin(s); },
          .writeStream = [](Stream &data,size_t size)->size_t{ return GzWriteStream(data, size); },
          .abort       = [](){ GzAbort(); },
          .end         = []()->bool{ return GzEnd(); },
          .isFinished  = []()->bool{ return GzUpdate.isFinished(); },
          .canRollBack = []()->bool{ return GzUpdate.canRollBack(); },
          .rollBack    = []()->bool{ return GzUpdate.rollBack(); },
          .onProgress  = [](UpdateClass::THandlerFunction_Progress fn){ GzUpdate.onProgress(fn); },
          .getError    = []()->uint8_t{ return GzUpdate.getError(); },
          .setBinName  = []( String& fileName, Stream* stream ) { mode_z=fileName.endsWith(".gz")?(stream->peek()==0x1f):false; log_d("Compression %s", mode_z?"enabled":"disabled"); }
        };
        return &Iface;
      }

    };
  };

#else // no gzip support, only native firmwares

  namespace SDUpdaterNS
  {
    namespace ConfigManager
    {
      using namespace UpdateInterfaceNS;
      inline UpdateManagerInterface_t *GetUpdateInterface()
      {
        static UpdateManagerInterface_t Iface =
        {
          .begin       = [](size_t s)->bool{ return Update.begin(s); },
          .writeStream = [](Stream &data,size_t size)->size_t{ return Update.writeStream(data); },
          .abort       = [](){ Update.abort(); },
          .end         = []()->bool{ return Update.end(); },
          .isFinished  = []()->bool{ return Update.isFinished(); },
          .canRollBack = []()->bool{ return Update.canRollBack(); },
          .rollBack    = []()->bool{ return Update.rollBack(); },
          .onProgress  = [](UpdateClass::THandlerFunction_Progress fn){ Update.onProgress(fn); },
          .getError    = []()->uint8_t{ return Update.getError(); },
          .setBinName  = [](String&fileName, Stream* stream) { if(fileName.endsWith(".gz")) log_e("Gz file detected but gz support is disabled!"); }
        };
        return &Iface;
      }

    };
  }

#endif
