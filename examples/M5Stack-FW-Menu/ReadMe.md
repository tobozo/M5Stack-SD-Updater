## Factory Partition Firmware Launcher

/!\ This app launcher depends on special partition schemes that include the factory type and at least 4 OTA (app) partitions.

The intent of this launcher is to provide an application manager to hanle firmwares directly from the flash.

This is useful in the following situations:
- Some firmwares on the SD are frequently flashed with updateFromFS()
- updateFromFS() can be slow with big binaries
- Multiple apps needed but filesystem is empty of no filesystem is available
- Filesystem is variable, it can be one of: SPIFFS, LittleFS, FFat, SD


### Requirements

- M5Unified supported device (typically M5Stack, M5Fire, M5Core2, M5CoreS3)
- 16MB Flash
- Display module
- Buttons and/or touch


### Launcher Usage

On first run, the launcher firmware will copy itself to the factory partition and restart from there, it will take
some time but it only happens once.

The app launcher UI comes with a set of tools:
- Firmware launcher (from flash)
- Firmware launcher (from filesystem, like M5Stack-SD-Launcher but without the decorations)
- Partitions Manager
  - Add firmware (copy from filesystem to flash)
  - Backup firmware (copy from flash to filesystem)
  - Remove firmware (a.k.a. erase ota partition)


### Application Usage

#### Direct implementation of the factory loader in an application:

```cpp
if( Flash::hasFactory() ) {
   Flash::loadFactory()
}
```

#### Indirect implementation with `checkSDUpdater()`:

/!\ This is temporary as it uses a side effect of M5StackUpdater's legacy behaviour instead of a clean and separate
logic statement. This will change in the future.

Using this app launcher instead of M5Stack-SD-Menu launcher will require a couple of modifications in the call to `checkSDUpdater()`:


```cpp
    SDUCfg.rollBackToFactory = true; // ignore M5Stack-SD-Menu (menu.bin) hot-loading
    SDUCfg.setLabelMenu("FW Menu");  // Change the launcher button label
    checkSDUpdater( SD, "", 5000, TFCARD_CS_PIN ); // second argument must be empty
```

