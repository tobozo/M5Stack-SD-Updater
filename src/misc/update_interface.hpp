#pragma once


#if defined SDU_HAS_TARGZ

  using namespace SDUpdaterNS::UpdateInterfaceNS;

  #define F_Update GzUpdateClass::getInstance()
  #define F_UpdateEnd() (mode_z ? F_Update.endgz() : F_Update.end())
  #define F_abort() if (mode_z) F_Update.abortgz(); else F_Update.abort()
  #define F_writeStream(updateSource,updateSize) (mode_z ? F_Update.writeGzStream(updateSource,updateSize) : F_Update.writeStream(updateSource))
  #define F_canBegin( usize ) (mode_z ? F_Update.begingz(UPDATE_SIZE_UNKNOWN) : F_Update.begin(usize))
  #define F_end() (mode_z ? F_Update.endgz() : F_Update.end() )

  namespace SDUpdaterNS
  {
    namespace ConfigManager
    {
      static UpdateInterfaceNS::UpdateManagerInterface_t Iface =
      {
        .begin=[](size_t s)->bool{ return F_canBegin(s); },
        .writeStream=[](Stream &data,size_t size)->size_t{ return F_writeStream(data, size); },
        .abort=[]() { F_abort(); },
        .end=[]()->bool{ return F_end(); },
        .isFinished=[]()->bool{ return F_Update.isFinished(); },
        .canRollBack=[]()->bool{ return F_Update.canRollBack(); },
        .rollBack=[]()->bool{ return F_Update.rollBack(); },
        .onProgress=[](UpdateClass::THandlerFunction_Progress fn){ F_Update.onProgress(fn); },
        .getError=[]()->uint8_t{ return F_Update.getError(); },
        .setBinName=[]( String& fileName, Stream* stream ) {
          if( !fileName.endsWith(".gz") ) {
            log_d("Not a gz file");
            return;
          }
          mode_z = stream->peek() == 0x1f; // magic zlib byte
          log_d("compression: %s", mode_z ? "enabled" : "disabled" );
        }
      };
    };
  };

#else
  namespace SDUpdaterNS
  {
    namespace ConfigManager
    {
      static UpdateInterfaceNS::UpdateManagerInterface_t Iface =
      {
        .begin=[](size_t s)->bool{ return Update.begin(s); },
        .writeStream=[](Stream &data,size_t size)->size_t{ return Update.writeStream(data); },
        .abort=[]() { Update.abort(); },
        .end=[]()->bool{ return Update.end(); },
        .isFinished=[]()->bool{ return Update.isFinished(); },
        .canRollBack=[]()->bool{ return Update.canRollBack(); },
        .rollBack=[]()->bool{ return Update.rollBack(); },
        .onProgress=[](UpdateClass::THandlerFunction_Progress fn){ Update.onProgress(fn); },
        .getError=[]()->uint8_t{ return Update.getError(); },
        .setBinName=[](String&fileName, Stream* stream){
          if( fileName.endsWith(".gz") ) {
            log_e("Gz file detected but gz support is disabled!");
          }
        }
      };
    };
  }
#endif
