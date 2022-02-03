#pragma once

#include "CertsManager.hpp"

namespace TLS
{

  const char* updateWallet( String host, const char* ca)
  {
    int8_t idx = -1;
    uint8_t sizeOfWallet = sizeof( TLSWallet ) / sizeof( TLSWallet[0] );
    for(uint8_t i=0; i<sizeOfWallet; i++) {
      if( TLSWallet[i].host==NULL ) {
        if( idx == -1 ) {
          idx = i;
        }
        continue;
      }
      if( String( TLSWallet[i].host ) == host ) {
        log_v("[WALLET SKIP UPDATE] Wallet #%d exists ( %s )\n", i, TLSWallet[i].host );
        return TLSWallet[i].ca;
      }
    }
    if( idx > -1 ) {
      int hostlen = host.length() + 1;
      int certlen = String(ca).length() + 1;
      char *newhost = (char*)malloc( hostlen );
      char *newcert = (char*)malloc( certlen );
      memcpy( newhost, host.c_str(), hostlen );
      memcpy( newcert, ca, certlen);
      TLSWallet[idx] = { (const char*)newhost , (const char*)newcert };
      log_v("[WALLET UPDATE] Wallet #%d loaded ( %s )\n", idx, TLSWallet[idx].host );
      return TLSWallet[idx].ca;
    }
    return ca;
  }


  const char* fetchLocalCert( String host )
  {
    String certPath = SD_CERT_PATH PATH_SEPARATOR + host;
    File certFile = M5_FS.open( certPath );
    if(! certFile ) { // failed to open the cert file
      log_w("[WARNING] Failed to open the cert file %s, TLS cert checking therefore disabled", certPath.c_str() );
      return NULL;
    }
    String certStr = "";
    while( certFile.available() ) {
      certStr += certFile.readStringUntil('\n') + "\n";
    }
    certFile.close();
    log_v("\n%s\n", certStr.c_str() );
    const char* certChar = updateWallet( host, certStr.c_str() );
    return certChar;
  }


  const char* getWalletCert( String host )
  {
    uint8_t sizeOfWallet = sizeof( TLSWallet ) / sizeof( TLSWallet[0] );
    log_v("\nChecking wallet (%d items)",  sizeOfWallet );
    for(uint8_t i=0; i<sizeOfWallet; i++) {
      if( TLSWallet[i].host==NULL ) continue;
      if( String( TLSWallet[i].host ) == host ) {
        log_v("Wallet #%d ( %s ) : [OK]", i, TLSWallet[i].host );
        //log_d(" [OK]");
        return TLSWallet[i].ca;
      } else {
        log_v("Wallet #%d ( %s ) : [KO]", i, TLSWallet[i].host );
        //log_d(" [KO]");
      }
    }
    const char* nullcert = NULL;
    return nullcert;
  }


  const char* fetchCert( String host, bool checkWallet, bool checkFS )
  {
    //const char* nullcert = NULL;
    if( checkWallet ) {
      const char* walletCert = getWalletCert( host );
      if( walletCert != NULL ) {
        log_v("[FETCHED WALLET CERT] -> %s", host.c_str() );
        return walletCert;
      } else {
        //
      }
    }
    String certPath = SD_CERT_PATH PATH_SEPARATOR + host;
    String certURL = certProvider + host;
    if( !checkFS || !M5_FS.exists( certPath ) ) {
      //log_d("[FETCHING REMOTE CERT] -> ");
      //wget(certURL , certPath );
      return fetchLocalCert( host );
    } else {
      log_v("[FETCHING LOCAL (SD) CERT] -> %s", certPath.c_str() );
    }
    return fetchLocalCert( host );
  }


  bool isInWallet( String host )
  {
    uint8_t sizeOfWallet = sizeof( TLSWallet ) / sizeof( TLSWallet[0] );
    for(uint8_t i=0; i<sizeOfWallet; i++) {
      if( TLSWallet[i].host==NULL ) continue;
      if( String( TLSWallet[i].host ) == host ) {
        return true;
      }
    }
    return false;
  }


  bool init( String host )
  {
    if( fetchLocalCert( host ) != NULL ) {
      return true;
    }
    bool ret = false;
    int hidState = HID_INERT;
    String certPath = SD_CERT_PATH PATH_SEPARATOR + host;
    String certURL = certProvider + host;
    if( wget == nullptr ) {
      log_e("ERROR: wget has not been inited");
      return false;
    }

    if( !wget( certURL , certPath ) ) {
      log_e( MODAL_TLSCERT_FETCHINGFAILED_MSG );
      hidState = modalConfirm ? modalConfirm( MODAL_CANCELED_TITLE, MODAL_TLSCERT_FETCHINGFAILED_MSG, WGET_MSG_FAIL, MENU_BTN_RESTART, MENU_BTN_CONTINUE, MENU_BTN_CANCEL ) : HID_BTN_A;
      goto finally;
    }
    if( fetchLocalCert( host ) == NULL ) {
      log_e( MODAL_TLSCERT_INSTALLFAILED_MSG );
      hidState = modalConfirm ? modalConfirm( MODAL_CANCELED_TITLE, MODAL_TLSCERT_INSTALLFAILED_MSG, FS_MSG_FAIL, MENU_BTN_RESTART, MENU_BTN_CONTINUE, MENU_BTN_CANCEL ) : HID_BTN_A;
      goto finally;
    }
    log_w( NEW_TLS_CERTIFICATE_TITLE );
    ret = true;

    finally:

    if( hidState == HID_BTN_C ) { // BTN_CANCEL, explicitely return false
      ret = false;
    } else if( hidState == HID_BTN_A ) { // BTN_RESTART
      ESP.restart();
    }
    return ret;
  }

};

