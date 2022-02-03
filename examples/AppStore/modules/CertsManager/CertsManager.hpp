#pragma once

#include "../misc/config.h"

namespace TLS
{
  struct TLSCert
  {
    const char* host;
    const char* ca;
  };

  String certProvider; // URL where the certificate chains are fetched
  const char* nullHost = nullptr; // dummy host
  const char* nullCa   = nullptr; // dummy CA
  TLSCert NULLCert = { nullHost, nullCa }; // dummy cert
  TLSCert TLSWallet[8] = {NULLCert, NULLCert, NULLCert, NULLCert, NULLCert, NULLCert, NULLCert, NULLCert }; // dummy wallet

  const char* fetchCert( String host, bool checkWallet = true , bool checkFS = true );
  const char* fetchLocalCert( String host );
  const char* getWalletCert( String host );
  const char* updateWallet( String host, const char* ca);

  bool isInWallet( String host );
  bool init( String host ); // cert provider url, host

  int (*modalConfirm)( const char* question, const char* title, const char* body, const char* labelA, const char* labelB, const char* labelC) = nullptr;
  bool (*wget)(String url, String path) = nullptr;


};
