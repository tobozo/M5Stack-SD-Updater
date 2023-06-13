// load all supported filesystems
#define SDU_USE_SD
#define SDU_USE_SPIFFS
#define SDU_USE_FFAT
#define SDU_USE_LITTLEFS
//#define SDU_ENABLE_GZ

#include <SD.h>
#include <SPIFFS.h>
#include <FFat.h>
#include <LittleFS.h>
//#include <ESP32-targz.h>
#include <Preferences.h> // lame but somehow reliable: store partitions info in NVS


#define SDU_USE_FACTORY

#include <M5Unified.h>
#include <M5StackUpdater.h>

#include <rom/rtc.h>
#define resetReason (int)rtc_get_reset_reason(0)



enum M5Btns_t
{
  M5_NOBTN = -1,
  M5_BTNA  =  0,
  M5_BTNB  =  1,
  M5_BTNC  =  2
};


struct sdu_filesystem_t
{
  fs::FS* fs;
  const char* name;
  bool enabled;
};


struct Item_t
{
  std::string name;
  void *item;
};


struct scrollClip_t
{
  int32_t x;
  int32_t y;
  uint32_t w;
  uint32_t h;
};


typedef void* resultGetter_t_type;
typedef resultGetter_t_type (*resultGetter_t)(std::vector<Item_t> items, uint8_t idx);
typedef void(*listRenderer_t)( std::vector<Item_t> items, uint8_t selected_idx );
typedef void(*cb_t)();

void lsFlashPartitions();


std::vector<sdu_filesystem_t> filesystems_enabled;
const int colors[] = { TFT_WHITE, TFT_CYAN, TFT_RED, TFT_YELLOW, TFT_BLUE, TFT_GREEN };
const char* const names[] = { "none", "wasHold", "wasClicked", "wasPressed", "wasReleased", "wasDeciedCount" };

std::string scrollableTitle = "";
scrollClip_t scrollClip = {0,0,0,0};
bool do_scroll = false;


namespace AppTheme
{
  using namespace SDU_UI;

  fontInfo_t Font0Size1 = {&Font0, 1};
  fontInfo_t Font0Size2 = {&Font0, 2};
  fontInfo_t Font2Size1 = {&Font2, 1};
  fontInfo_t DejaVu12Size1 = {&DejaVu12, 1};
  fontInfo_t DejaVu18Size1 = {&DejaVu18, 1};
  fontInfo_t FreeSans9pt7bSize1 = {&FreeSans9pt7b, 1};
  fontInfo_t FreeSans12pt7bSize1 = {&FreeSans12pt7b, 1};

  const uint16_t ListFgColor = TFT_WHITE;
  const uint16_t ListBgColor = M5.Lcd.color565( 0x20, 0x20, 0x20);

  const int32_t ListOffsetX=10;
  const int32_t ListOffsetY=40;
  int32_t ListPadding=6;


  const BtnStyle_t UpBtn     = { 0x73AE, 0x630C, TFT_WHITE, TFT_BLACK };
  const BtnStyle_t SelectBtn = { 0x73AE, 0x630C, TFT_WHITE, TFT_BLACK };
  const BtnStyle_t DownBtn   = { 0x73AE, 0x630C, TFT_WHITE, TFT_BLACK };
  const uint16_t MsgFontColors[2] = { ListFgColor, ListBgColor };
  const BtnStyles_t BtnStyles( UpBtn, SelectBtn, DownBtn, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HWIDTH, &FreeSans9pt7bSize1, &Font0Size2, MsgFontColors );
  SplashPageElementStyle_t TitleStyle      = { TFT_BLACK,     TFT_WHITE, &Font0Size2, MC_DATUM, TFT_LIGHTGREY, TFT_DARKGREY };
  SplashPageElementStyle_t AppNameStyle    = { TFT_LIGHTGREY, TFT_BLACK, &Font0Size2, BC_DATUM, 0, 0 };
  SplashPageElementStyle_t AuthorNameStyle = { TFT_LIGHTGREY, TFT_BLACK, &Font0Size2, BC_DATUM, 0, 0 };
  SplashPageElementStyle_t AppPathStyle    = { TFT_DARKGREY,  TFT_BLACK, &Font0Size1, BC_DATUM, 0, 0 };
  Theme_t Theme = { &BtnStyles, &TitleStyle, &AppNameStyle, &AuthorNameStyle, &AppPathStyle, &ProgressStyle };
};


// ersatz for M5.update(), modified to also work with CoreS3's touch
void HIDUpdate()
{
  if( M5.getBoard()==lgfx::boards::board_M5StackCoreS3 && M5.Touch.isEnabled() ) {
    // M5Unified doesn't handle Touch->M5.BtnX translation for CoreS3, give it a little help
    uint8_t edge = 220;
    auto ms = m5gfx::millis();
    M5.Touch.update(ms);
    uint_fast8_t btn_bits = 0;
    int i = M5.Touch.getCount();
    if( i>0 ) {
      while (--i >= 0) {
        auto raw = M5.Touch.getTouchPointRaw(i);
        if (raw.y > edge) {
          auto det = M5.Touch.getDetail(i);
          if (det.state & m5::touch_state_t::touch) {
            if (M5.BtnA.isPressed()) { btn_bits |= 1 << 0; }
            if (M5.BtnB.isPressed()) { btn_bits |= 1 << 1; }
            if (M5.BtnC.isPressed()) { btn_bits |= 1 << 2; }
            if (btn_bits || !(det.state & m5::touch_state_t::mask_moving)) {
              btn_bits |= 1 << ((raw.x - 2) / 107);
            }
          }
        }
      }
    }
    M5.BtnA.setRawState(ms, btn_bits & 1);
    M5.BtnB.setRawState(ms, btn_bits & 2);
    M5.BtnC.setRawState(ms, btn_bits & 4);
  } else { // trust M5Unified
    M5.update();
  }
}


M5Btns_t getPressedButton(cb_t cb=nullptr)
{
  using namespace AppTheme;
  SDUCfg.onButtonDraw( "Up",     0, BtnStyles.Load.BorderColor, BtnStyles.Load.FillColor, BtnStyles.Load.TextColor, BtnStyles.Load.ShadowColor );
  SDUCfg.onButtonDraw( "Select", 1, BtnStyles.Skip.BorderColor, BtnStyles.Skip.FillColor, BtnStyles.Skip.TextColor, BtnStyles.Skip.ShadowColor );
  SDUCfg.onButtonDraw( "Down",   2, BtnStyles.Save.BorderColor, BtnStyles.Save.FillColor, BtnStyles.Save.TextColor, BtnStyles.Save.ShadowColor );

  while(true) {
    HIDUpdate();
    if( M5.BtnA.wasPressed() ) return M5_BTNA;
    if( M5.BtnB.wasPressed() ) return M5_BTNB;
    if( M5.BtnC.wasPressed() ) return M5_BTNC;
    if( cb ) cb();
  }
  return M5_NOBTN;
}


void scrollTitle()
{
  using namespace AppTheme;
  const uint32_t delay_ms = 120;
  static uint32_t last_scroll_ms = millis();
  uint32_t now = millis();
  if( last_scroll_ms+delay_ms>now ) return;
  std::rotate(scrollableTitle.begin(), scrollableTitle.begin() + 1, scrollableTitle.end());
  M5.Lcd.setTextDatum( TL_DATUM ); // text coords are top-left based
  setFontInfo( SDU_GFX, &DejaVu18Size1 );
  M5.Lcd.setTextColor( MsgFontColors[1], MsgFontColors[0] );
  //M5.Lcd.setClipRect(scrollClip.x, scrollClip.y, scrollClip.w, scrollClip.h);
  M5.Lcd.drawString( scrollableTitle.c_str(), scrollClip.x, scrollClip.y );
  //M5.Lcd.clearClipRect();
  last_scroll_ms = now;
}


void renderList( std::vector<Item_t> items, uint8_t selected_idx )
{
  using namespace AppTheme;
  if( selected_idx >= items.size() ) return;

  M5.Lcd.setTextDatum( TL_DATUM ); // text coords are top-left based
  setFontInfo( SDU_GFX, &DejaVu18Size1 );

  // figure out pagination variables
  uint16_t max_avail_width = M5.Lcd.width()-ListOffsetX;
  uint8_t box_height = M5.Lcd.height() - (ListOffsetY + ListPadding + BtnStyles.height);
  uint8_t line_height = M5.Lcd.fontHeight()*1.5;
  uint8_t max_items_per_page = box_height / line_height;
  uint8_t page_num = selected_idx/max_items_per_page;
  uint8_t item_idx = selected_idx%max_items_per_page;
  uint8_t item_start = page_num*max_items_per_page;
  uint8_t item_end = item_start+(max_items_per_page);
  if( item_end >items.size() ) item_end = items.size();
  uint8_t items_visible = item_end-item_start;

  M5.Lcd.setClipRect( 0, ListOffsetY-ListPadding, M5.Lcd.width(), max_items_per_page*line_height+ListPadding  );
  M5.Lcd.fillRoundRect( 0, ListOffsetY-ListPadding, M5.Lcd.width(), max_items_per_page*line_height+ListPadding, ListPadding*2, ListBgColor );

  for( int i=item_start; i<item_end; i++ ) {
    int32_t ypos = i%max_items_per_page * line_height;
    if( selected_idx == i ) { // highlighted
      M5.Lcd.setTextColor( MsgFontColors[1], MsgFontColors[0] );
      uint32_t tw = M5.Lcd.textWidth(items[i].name.c_str());
      if( tw > max_avail_width ) {
        scrollClip = { ListOffsetX, ListOffsetY+ypos, max_avail_width, line_height };
        scrollableTitle = " " + items[i].name + " ";
        do_scroll = true;
      } else {
        do_scroll = false;
      }
    } else { // regular
      M5.Lcd.setTextColor( MsgFontColors[0], MsgFontColors[1] );
    }

    M5.Lcd.drawString( items[i].name.c_str(), ListOffsetX, ListOffsetY+ypos );
  }
  M5.Lcd.clearClipRect();
}


void* paginateLoop( uint8_t& menu_idx, std::vector<Item_t> items, resultGetter_t getter, listRenderer_t renderList )
{
  while(1) {
    renderList( items, menu_idx ); // render paginated list
    auto btn_idx = getPressedButton( do_scroll?scrollTitle:nullptr ); // poll BtnA/B/C
    switch( btn_idx ) { // paginate
      case M5_BTNA: menu_idx = (menu_idx>0) ? menu_idx-1: items.size()-1; break; // btn up
      case M5_BTNB: return getter( items, menu_idx ); break; // btn action
      case M5_BTNC: menu_idx = (menu_idx<items.size()-1) ? menu_idx+1: 0; break; // btn down
      default: return nullptr;
    }
  }
}


void *fsGetter(std::vector<Item_t> items, uint8_t idx)
{
  if( idx==0 || idx >= filesystems_enabled.size() ) return nullptr;
  auto fsitem = (sdu_filesystem_t*) &filesystems_enabled[idx];
  return (void*)fsitem->fs;
}


fs::FS* fsPicker()
{
  using namespace AppTheme;
  using namespace SDU_UI;
  M5.Lcd.setClipRect( 0, 0, 0, 0 ); // ignore SdUpdater UI messages as it is only a scan
  SDUpdater sdUpdater;
  M5.Lcd.clearClipRect();
  SDUpdater::_message("Scanning filesystems...");
  M5.Lcd.setClipRect( 0, 0, 0, 0 );

  sdu_filesystem_t filesystems[] =
  {
    { nullptr,   "..",       true                                         },
    { &SD,       "SD",       ConfigManager::hasFS( &sdUpdater, SD )       },
    { &LittleFS, "LittleFS", ConfigManager::hasFS( &sdUpdater, LittleFS ) },
    { &SPIFFS,   "SPIFFS",   ConfigManager::hasFS( &sdUpdater, SPIFFS )   },
    { &FFat,     "FFat",     ConfigManager::hasFS( &sdUpdater, FFat )     }
  };

  M5.Lcd.clearClipRect();

  filesystems_enabled.clear();
  //std::vector<std::string> filesystems_names;
  std::vector<Item_t> filesystems_vect;
  size_t fscount = sizeof(filesystems)/sizeof(sdu_filesystem_t);
  uint8_t menu_idx = 0;

  for( int i=0; i<fscount; i++ ) {
    if( filesystems[i].enabled ) {
      filesystems_enabled.push_back( filesystems[i] );
      //filesystems_names.push_back( std::string(filesystems[i].name) );
      filesystems_vect.push_back( { std::string(filesystems[i].name), filesystems[i].fs } );
    }
  }

  if( filesystems_vect.size() == 1 ) return nullptr; // no readable FS found
  if( filesystems_vect.size() == 2 ) return (fs::FS*)filesystems_vect[1].item; // only one real fs found, no need to pick

  drawSDUSplashElement( "Choose a filesystem", M5.Lcd.width()/2, 0, &TitleStyle );

  fs::FS* ret = (fs::FS*)paginateLoop( menu_idx, filesystems_vect, fsGetter, renderList );
  return ret;
}


void *fileGetter(std::vector<Item_t> items, uint8_t idx)
{
  if( idx == 0 ) return nullptr;
  static String outFile = "";
  outFile = String(items[idx].name.c_str());
  return (void*)outFile.c_str();
}


const char* filePicker( fs::FS *fs )
{
  using namespace AppTheme;
  using namespace SDU_UI;
  SDUpdater::_message("Scanning files...");

  static String outFile = "";
  File root = fs->open( "/" );
  if( !root ){
    SDUpdater::_error( "Opendir '/' failed" );
    return nullptr;
  }
  auto file = root.openNextFile();

  std::vector<Item_t> files_vec;
  files_vec.push_back({"..", nullptr});
  uint8_t menu_idx = 0;

  while( file ){
    auto fname = String( SDUpdater::fs_file_path(&file) );
    if( fname.endsWith(".bin") || fname.endsWith(".gz") ) {
      files_vec.push_back( { std::string( fname.c_str() ), nullptr } );
    }
    file = root.openNextFile();
  }
  root.close();

  drawSDUSplashElement( "Choose a binary file", M5.Lcd.width()/2, 0, &TitleStyle );

  if( files_vec.size() == 1 ) return nullptr; // no readable FS found

  const char* ret = (const char*)paginateLoop( menu_idx, files_vec, fileGetter, renderList );

  return ret;
}


std::vector<Item_t> GetSlotItems( bool show_hidden )
{
  std::vector<Item_t> slots;
  slots.push_back({"..",nullptr});
  for( int i=0; i<NVS::Partitions.size(); i++ ) {
    auto nvs_part = &NVS::Partitions[i];
    if( nvs_part->bin_size > 0 ) {
      String slotNameStr = String(nvs_part->name);
      if( slotNameStr.endsWith(".bin") ) {
        slotNameStr.replace(".bin", ""); // lose the trailing extension
      }
      if( slotNameStr[0] == '/' ) {
        slotNameStr = slotNameStr.substring( 1, slotNameStr.length());  // lose the leading slash
      }
      slotNameStr = "[" + String(nvs_part->ota_num) + "] " + slotNameStr; // prepend partition number
      std::string slotname = std::string( slotNameStr.c_str() );
      slots.push_back( {slotname,nvs_part} );
    } else {
      if( show_hidden && nvs_part->ota_num!=0 ) { // slot 0 must always be available
        std::string slotname = std::string("Slot ") + std::to_string( nvs_part->ota_num ) + std::string(" available");
        slots.push_back( {slotname,nvs_part} );
      }
    }
  }
  return slots;
}


void *slotGetter(std::vector<Item_t> items, uint8_t idx)
{
  static int8_t ret;
  if( idx>=items.size() ) return nullptr;
  return items[idx].item;
}


int8_t slotPicker( const char* label,  bool show_hidden=true )
{
  using namespace AppTheme;
  using namespace SDU_UI;
  uint8_t menu_idx = 0;
  std::vector<Item_t> slots_vec;
  slots_vec = GetSlotItems( show_hidden );
  drawSDUSplashElement( label, M5.Lcd.width()/2, 0, &TitleStyle );
  if( slots_vec.size() == 1 ) {
    if( show_hidden ) {
      SDUpdater::_error("No OTA slots found :-(");
    } else {
      SDUpdater::_message("All OTA slots are free");
      delay(1000);
    }
    return -1;
  }
  auto nvs_part = (NVS::PartitionDesc_t*)paginateLoop( menu_idx, slots_vec, slotGetter, renderList );
  return nvs_part ? nvs_part->ota_num : -1;
}


void menuItemLoadFW()
{
  auto ota_num = slotPicker("Run Flash App", false); // load apps
  if( ota_num > 0 ) {
    SDUpdater::_message("Booting partition");
    if( !Flash::bootPartition( ota_num ) ) {
      SDUpdater::_error("Partition unbootable :(");
    }
  }
}


void menuItemLoadFS()
{
  auto fs = fsPicker();
  if( !fs ) return;
  auto file = filePicker(fs);
  if( !file ) return;
  String fsBin = String(file);
  log_w("Will overwrite next OTA slot");
  SDUCfg.use_rollback = false;
  updateFromFS( *fs, fsBin );
}

void menuItemPartitionsList()
{
  lsFlashPartitions();
}


void handleResult( bool res, const char* msg )
{
  String message = String(msg) + " ";
  if(!res ) {
    message +="Fail";
    SDUpdater::_error(message.c_str());
  } else {
    message +="Success";
    SDUpdater::_message(message.c_str());
  }
  delay(1000);
}


void menuItemPartitionFlash()
{
  auto ret = PartitionManager::Flash( slotPicker("Flash Slot", true), fsPicker, filePicker );
  handleResult( ret, "Flash");
}


void menuItemPartitionBackup()
{
  auto ret = PartitionManager::Backup( slotPicker("Backup Slot", true), fsPicker );
  handleResult( ret, "Backup");
}


void menuItemPartitionErase()
{
  auto ret = PartitionManager::Erase(slotPicker("Erase Slot"));
  handleResult( ret, "Erase");
}


void menuItemPartitionVerify()
{
  auto ret = PartitionManager::Verify(slotPicker("Verify Slot"));
  handleResult( ret, "Verify");
}


void *menuItemGetter(std::vector<Item_t> items, uint8_t idx)
{
  if( idx >= items.size() ) return nullptr; // first menu item is always null
  static int8_t ret;
  ret = idx;
  return (void*)&ret;
}


void toolsPicker()
{
  using namespace AppTheme;
  using namespace SDU_UI;
  std::vector<Item_t> labels_vec;
  labels_vec.push_back({"..", nullptr});
  labels_vec.push_back({"Print Partition Info", nullptr});
  labels_vec.push_back({"Add firmware", nullptr});
  labels_vec.push_back({"Backup firmware", nullptr});
  labels_vec.push_back({"Remove firmware", nullptr});
  labels_vec.push_back({"Verify firmware", nullptr});

  while(1) {
    drawSDUSplashElement( "Partition Tools", M5.Lcd.width()/2, 0, &TitleStyle );
    uint8_t menu_idx = 0;
    int8_t* ret = (int8_t*)paginateLoop( menu_idx, labels_vec, menuItemGetter, renderList );

    if( ret ) {
      switch( *ret ) {
        case 1: menuItemPartitionsList() ; break;
        case 2: menuItemPartitionFlash() ; break;
        case 3: menuItemPartitionBackup(); break;
        case 4: menuItemPartitionErase() ; break;
        case 5: menuItemPartitionVerify(); break;
        default: log_d("Leaving toolsPicker"); return; break;
      }
    }
  }
}


void launcherPicker()
{
  using namespace AppTheme;
  using namespace SDU_UI;

  std::vector<Item_t> labels_vec;
  labels_vec.push_back({"..", nullptr});
  labels_vec.push_back({"Load Firmware (Flash)", nullptr});
  labels_vec.push_back({"Load Firmware (Filesystem)", nullptr});
  labels_vec.push_back({"Manage Partitions (this is a super long, scrollable title)", nullptr});

  while(1) {
    drawSDUSplashElement( "Firmware Launcher", M5.Lcd.width()/2, 0, &TitleStyle );
    uint8_t menu_idx = 0;
    int8_t* ret = (int8_t*)paginateLoop( menu_idx, labels_vec, menuItemGetter, renderList );
    if( ret ) {
      switch( *ret ) {
        case 1: menuItemLoadFW(); break;
        case 2: menuItemLoadFS(); break;
        case 3: toolsPicker(); break;
        default: log_d("Leaving launcherPicker"); return; break;
      }
    }
  }
}


// decorator for serial output
void printFlasPartition( Flash::Partition_t* sdu_partition )
{
  Flash::digest_t digests = Flash::digest_t();

  auto part = sdu_partition->part;
  auto meta = sdu_partition->meta;

  String AppName = "n/a";

  if( Flash::PartitionIsApp( &part ) ) {
    if( Flash::PartitionIsFactory( &part ) ) {
      AppName = "Factory";
    } else {
      // TODO: match digest with application meta (binary name, initial path, picture, description)
      AppName = "OTA" + String( part.subtype - ESP_PARTITION_SUBTYPE_APP_OTA_MIN );
    }
  }

  Serial.printf("%-8s   0x%02x      0x%02x   0x%06x   %8d  %8s %8s %8s\n",
    String( part.label ).c_str(),
    part.type,
    part.subtype,
    part.address,
    part.size,
    meta.image_len>0 ? String(meta.image_len).c_str() : "n/a",
    AppName.c_str(),
    Flash::PartitionIsApp(&part)&&Flash::MetadataHasDigest(&meta) ? digests.toString(meta.image_digest) : "n/a"
  );
}


void lsFlashPartitions()
{
  Serial.println("Partition  Type   Subtype    Address   PartSize   ImgSize    Info    Digest");
  Serial.println("---------+------+---------+----------+----------+---------+--------+--------");
  for( int i=0; i<Flash::Partitions.size(); i++ ) {
    printFlasPartition( &Flash::Partitions[i] );
  }
}


bool checkFactoryStickyPartition()
{
  Flash::Scan();

  if( Flash::Partitions.size() == 0 ) {
    log_e("No flash partitions found");
    return false;
  }

  if( Flash::FactoryPartition == nullptr ) {
    log_e("No factory partition found");
    return false;
  }

  // will compare running partition with factory, and update if necessary
  if( SDUpdater::saveSketchToFactory() ) {
    // sketch was just saved to factory partition, mark it as bootable and restart
    Flash::loadFactory(); // will trigger a restart on success
    log_e("Switching to factory app failed :-(");
    return false;
  }

  if( !NVS::GetBLobPartitions() ) {
    log_w("Partitions not found on NVS, creating"); // first visit!
    PartitionManager::Create();
  } else {
    PartitionManager::Update(); // refresh
  }

  if( NVS::Partitions.size() == 0 ) {
    log_e("Failed to create shadow copy of partitions table in NVS");
    return false;
  }

  // cleanup/update shadow copy of partitions table in NVS
  PartitionManager::Process();
  return Flash::FactoryPartition != nullptr;
}



void setup()
{
  using namespace AppTheme;
  using namespace SDU_UI;

  if( resetReason == 12 /*RTC_SW_CPU_RST*/ ) {
    log_d("software reset detected, delaying");
    delay(1000);
  }

  M5.begin();
  Serial.setTimeout(100);

  SDUCfg.display = &M5.Lcd;
  SDUCfg.setErrorCb( DisplayErrorUI );
  SDUCfg.setMessageCb( DisplayUpdateUI );
  SDUCfg.setButtonDrawCb( drawSDUPushButton );
  SDUCfg.setButtonsTheme( &Theme );

  SDUpdater::_message("Booting factory...");

  if( ! checkFactoryStickyPartition() ) {
    // print partitions for debug
    lsFlashPartitions();
    // NVSUtil::Dump(); // more debug
    SDUpdater::_error("No factory context");
    SDUpdater::_error("Halting"); // TODO: load SDUpdater lobby ?
    while(1);
  }

  SDUpdater::_message("Enumerating slots...");

  lsFlashPartitions();
}


void loop()
{
  auto ota_num = slotPicker("Run Application", false); // load apps
  if( ota_num > 0 ) {
    SDUpdater::_message("Booting partition");
    if( !Flash::bootPartition( ota_num ) ) {
      SDUpdater::_error("Partition unbootable :(");
    }
  }
  launcherPicker();
}
