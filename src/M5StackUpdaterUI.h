#ifndef _M5UPDATER_UI_
#define _M5UPDATER_UI_

#if defined _CHIMERA_CORE_|| defined LOVYANGFX_HPP_ || defined _M5STACK_H_


  static bool assertStartUpdateFromButton()
  {
    M5.update();
    return M5.BtnA.isPressed();
  }


  static void checkSDUpdaterUI( fs::FS &fs, String fileName ) {
    auto &tft = M5.Lcd;
    tft.setCursor(0,0);
    tft.print("SDUpdater\npress BtnA");
    tft.setCursor(0,0);
    auto msec = millis();
    do {
      if ( assertStartUpdateFromButton() ) {
        Serial.println("Will Load menu binary");
        updateFromFS( fs, fileName );
        ESP.restart();
      }
    } while (millis() - msec < 512);
    tft.fillScreen(TFT_BLACK);
  }



  static void SDMenuProgressUI( int state, int size )
  {
    static int SD_UI_Progress;
    int percent = ( state * 100 ) / size;
    if( percent == SD_UI_Progress ) {
      // don't render twice the same value
      return;
    }
    auto &tft = M5.Lcd;
    //Serial.printf("percent = %d\n", percent); // this is spammy
    SD_UI_Progress = percent;
    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (tft.width() - progress_w) >> 1;
    int progress_y = (tft.height()- progress_h) >> 1;
    if ( percent >= 0 && percent < 101 ) {
      tft.fillRect( progress_x+1, progress_y+1, percent, 18, TFT_GREEN );
      tft.fillRect( progress_x+1+percent, progress_y+1, 100-percent, 18, TFT_BLACK );
      Serial.print( "." );
    } else {
      tft.fillRect( progress_x+1, progress_y+1, 100, 18, TFT_BLACK );
      Serial.println();
    }
    String percentStr = " " + String( percent ) + "% ";
    tft.drawCentreString( percentStr , tft.width() >> 1, progress_y+progress_h+5, 0); // trailing space is important
    if ( percent >= 0 && percent < 101 ) {
      Serial.print( "." );
    } else {
      Serial.println();
    }
  };



  static void DisplayUpdateUI( const String& label )
  {
    auto &tft = M5.Lcd;
    if (tft.width() < tft.height()) tft.setRotation(tft.getRotation() ^ 1);

    tft.fillScreen( TFT_BLACK );
    tft.setTextColor( TFT_WHITE, TFT_BLACK );
    tft.setTextFont( 0 );
    tft.setTextSize( 2 );
    // attemtp to center the text
    int16_t xpos = ( tft.width() / 2) - ( tft.textWidth( label ) / 2 );
    if ( xpos < 0 ) {
      // try with smaller size
      tft.setTextSize(1);
      xpos = ( tft.width() / 2 ) - ( tft.textWidth( label ) / 2 );
      if( xpos < 0 ) {
        // give up
        xpos = 0 ;
      }
    }

    int progress_w = 102;
    int progress_h = 20;
    int progress_x = (tft.width() - progress_w) >> 1;
    int progress_y = (tft.height()- progress_h) >> 1;
    tft.setCursor( xpos, progress_y - 20 );
    tft.print( label );
    tft.drawRect( progress_x, progress_y, progress_w, progress_h, TFT_WHITE );
  }

#else

  #undef USE_DISPLAY

#endif




#endif //  _M5UPDATER_UI_
