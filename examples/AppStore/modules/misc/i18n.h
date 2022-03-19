#pragma once

#define LANG_EN 0xff
#define LANG_JP 0x12
#define LANG_CN 0x07
#define LANG_KR 0x14

#define I18N_LANG LANG_EN

#if I18N_LANG == LANG_CN

  #if !defined ARDUINO_M5STACK_Core2 && !defined ARDUINO_M5STACK_FIRE
    #error "Chinese font require M5Core2 or M5Fire"
  #endif

  #include "lang/i18n.cn.h"
  #define ButtonFont     &efontCN_10
  #define HeaderFont     &efontCN_10
  #define LIFont         &efontCN_10
  #define InfoWindowFont &efontCN_10

#elif I18N_LANG == LANG_EN

  #include "lang/i18n.en.h"
  #define ButtonFont     &Font2
  #define HeaderFont     &FreeMono9pt7b
  #define LIFont         &Font2
  #define InfoWindowFont &FreeMono9pt7b

#elif I18N_LANG == LANG_JP

  #include "lang/i18n.jp.h"
  #define ButtonFont     &lgfxJapanGothic_12
  #define HeaderFont     &lgfxJapanGothic_12
  #define LIFont         &lgfxJapanGothic_12
  #define InfoWindowFont &lgfxJapanGothic_12

#elif I18N_LANG == LANG_KR

  #include "lang/i18n.kr.h"
  #define ButtonFont     &efontKR_10
  #define HeaderFont     &efontKR_12_b
  #define LIFont         &efontKR_10_b
  #define InfoWindowFont &efontKR_12

#else

  #error "Unsupported language"

#endif


#define SCROLL_SEPARATOR "   ***   "
