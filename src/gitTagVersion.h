#define SDU_VERSION_MAJOR 1
#define SDU_VERSION_MINOR 2
#define SDU_VERSION_PATCH 1
#define _SDU_STR(x) #x
#define SDU_STR(x) _SDU_STR(x)
// Macro to convert library version number into an integer
#define VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
// current library version as a string
#define M5_SD_UPDATER_VERSION SDU_STR(SDU_VERSION_MAJOR) "." SDU_STR(SDU_VERSION_MINOR) "." SDU_STR(SDU_VERSION_PATCH)
// current library version as an int, to be used in comparisons, such as M5_SD_UPDATER_VERSION_INT >= VERSION_VAL(2, 0, 0)
#define M5_SD_UPDATER_VERSION_INT VERSION_VAL(SDU_VERSION_MAJOR, SDU_VERSION_MINOR, SDU_VERSION_PATCH)
