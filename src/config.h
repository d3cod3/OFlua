#pragma mark once

#define PACKAGE	"OFlua"
#define	VERSION	"0.1.1"
#define DESCRIPTION "Live Visual Patching Creative-Coding Platform"
#define DEVELOPERS "Emanuele Mazza"
#define TAGS "openframeworks,macos,linux,windows,creative-coding,video,audio,graphics,live-coding,creative-coding"

#define WINDOW_TITLE "OFlua 0.1.1"

#define WINDOW_START_WIDTH                  1280
#define WINDOW_START_HEIGHT                 720

// choose modifier key based on platform
#ifdef TARGET_OSX
    #define MOD_KEY OF_KEY_SUPER
#else
    #define MOD_KEY OF_KEY_CONTROL
#endif

#define RETINA_MIN_WIDTH                    2560
#define RETINA_MIN_HEIGHT                   1600

#define OUTPUT_TEX_MAX_WIDTH                4800
#define OUTPUT_TEX_MAX_HEIGHT               4800

#define STANDARD_PROJECTOR_WINDOW_WIDTH     1280
#define STANDARD_PROJECTOR_WINDOW_HEIGHT    720

#define STANDARD_TEXTURE_WIDTH              1280
#define STANDARD_TEXTURE_HEIGHT             720

#define CAM_MAX_WIDTH                       1920
#define CAM_MAX_HEIGHT                      1080

#define LOGGER_HEIGHT                       200

#define MAIN_FONT                           "fonts/SpaceMono-Regular.ttf"
