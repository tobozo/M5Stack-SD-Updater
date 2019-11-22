// left in a separate file for easy editing / overwriting

//see #define DEFAULT_REGISTRY_BOARD in menu.h

#define DEFAULT_REGISTRY_NAME "SDUpdater"
#define DEFAULT_REGISTRY_DESC "Tobozo's " PLATFORM_NAME " Application registry @ phpsecu.re"
#define DEFAULT_REGISTRY_URL "https://phpsecu.re/" DEFAULT_REGISTRY_BOARD "/registry/phpsecu.re.json" // should exist as "/.registry/default.json" on SD Card
#define DEFAULT_REGISTRY_CHANNEL "unstable" // "master" or "unstable"

#define DEFAULT_MASTER_DESC "Master channel at phpsecu.re/" DEFAULT_REGISTRY_BOARD " registry"
#define DEFAULT_MASTER_URL "https://phpsecu.re/" DEFAULT_REGISTRY_BOARD "/sd-updater/"
#define DEFAULT_MASTER_API_HOST "phpsecu.re"
#define DEFAULT_MASTER_API_PATH "/" DEFAULT_REGISTRY_BOARD
#define DEFAULT_MASTER_API_CERT_PATH "/cert/"
#define DEFAULT_MASTER_UPDATER_PATH "/sd-updater"
#define DEFAULT_MASTER_CATALOG_ENDPOINT "/catalog.json"

#define DEFAULT_UNSTABLE_DESC "Unstable channel at phpsecu.re/" DEFAULT_REGISTRY_BOARD " registry"
#define DEFAULT_UNSTABLE_URL "https://phpsecu.re/" DEFAULT_REGISTRY_BOARD "/sd-updater/unstable/"
#define DEFAULT_UNSTABLE_API_HOST "phpsecu.re"
#define DEFAULT_UNSTABLE_API_PATH "/" DEFAULT_REGISTRY_BOARD
#define DEFAULT_UNSTABLE_API_CERT_PATH "/cert/"
#define DEFAULT_UNSTABLE_UPDATER_PATH "/sd-updater/unstable"
#define DEFAULT_UNSTABLE_CATALOG_ENDPOINT "/catalog.json"
