#pragma once

namespace SDUpdaterNS
{

  struct SplashPageElementStyle_t
  {
    const uint16_t textColor;
    const uint16_t bgColor;
    const uint16_t fontSize;
    const uint16_t textDatum;
    const uint16_t colorStart; // gradient color start
    const uint16_t colorEnd;   // gradient color end
  };

  struct ProgressBarStyle_t
  {
    const int      width;
    const int      height;
    const bool     clipText;
    const uint16_t borderColor;
    const uint16_t fillColor;
    const uint8_t  fontNumber;
    const uint8_t  textDatum;
    const uint8_t  textSize;
    const uint16_t textColor;
    const uint16_t bgColor;
  };

  struct BtnStyle_t
  {
    const uint16_t BorderColor;
    const uint16_t FillColor;
    const uint16_t TextColor;
    const uint16_t ShadowColor;
  };

  struct SDUTextStyle_t // somehow redundant with LGFX's textstyle_t
  {
    bool frozen           = false;
    uint8_t textsize      = 0;
    uint8_t textdatum     = 0;
    uint32_t textcolor    = 0;
    uint32_t textbgcolor  = 0;
  };

  struct BtnStyles_t
  {
    BtnStyles_t() { };
    BtnStyles_t(
      const BtnStyle_t _Load, const BtnStyle_t _Skip, const BtnStyle_t _Save,
      const uint16_t _height, const uint16_t _width, const uint16_t _hwidth,
      const uint8_t _FontSize, const uint8_t _BtnFontNumber, const uint8_t _MsgFontSize, const uint16_t _MsgFontColor[2]
    ) :
      Load(_Load), Skip(_Skip), Save(_Save),
      height(_height), width(_width), hwidth(_hwidth),
      FontSize(_FontSize), BtnFontNumber(_BtnFontNumber), MsgFontSize(_MsgFontSize)
    {
      MsgFontColor[0] = _MsgFontColor[0];
      MsgFontColor[1] = _MsgFontColor[1];
    }
    // 16bit colors: Border  Fill    Text    Shadow
    const BtnStyle_t Load{ 0x73AE, 0x630C, 0xFFFF, 0x0000 };
    const BtnStyle_t Skip{ 0x73AE, 0x4208, 0xFFFF, 0x0000 };
    const BtnStyle_t Save{ 0x73AE, 0x2104, 0xFFFF, 0x0000 };
    const uint16_t height{28};
    const uint16_t width{68};
    const uint16_t hwidth{34};
    const uint8_t  FontSize{1}; // buttons font size
    const uint8_t  BtnFontNumber{1};
    const uint8_t  MsgFontSize{2}; // welcome message font size
    uint16_t MsgFontColor[2]{0xFFFF, 0x0000}; // foreground, background
  };


  struct Theme_t
  {
    Theme_t() { };
    Theme_t(
      const BtnStyles_t*        _buttons,
      SplashPageElementStyle_t* _SplashTitleStyle,
      SplashPageElementStyle_t* _SplashAppNameStyle,
      SplashPageElementStyle_t* _SplashAuthorNameStyle,
      SplashPageElementStyle_t* _SplashAppPathStyle,
      ProgressBarStyle_t      * _ProgressStyle
    ) :
      buttons              (_buttons              ),
      SplashTitleStyle     (_SplashTitleStyle     ),
      SplashAppNameStyle   (_SplashAppNameStyle   ),
      SplashAuthorNameStyle(_SplashAuthorNameStyle),
      SplashAppPathStyle   (_SplashAppPathStyle   ),
      ProgressStyle        (_ProgressStyle        )
    { };

    const BtnStyles_t* buttons{nullptr};
    SplashPageElementStyle_t* SplashTitleStyle     {nullptr};
    SplashPageElementStyle_t* SplashAppNameStyle   {nullptr};
    SplashPageElementStyle_t* SplashAuthorNameStyle{nullptr};
    SplashPageElementStyle_t* SplashAppPathStyle   {nullptr};
    ProgressBarStyle_t *ProgressStyle{nullptr};
  };

};
