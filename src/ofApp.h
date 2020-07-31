#pragma once

#include "ofMain.h"

#if !defined(TARGET_WIN32)
#include <pwd.h>
#endif

#include <unistd.h>

#include "ofxImGui.h"
#include "ofxLua.h"
#include "ofxThreadedFileDialog.h"

#include "AppTheme.h"
#include "PathWatcher.h"

#include "config.h"
#include "utils.h"

class ofApp : public ofBaseApp, ofxLuaListener {

public:
    void setup();
    void update();
    void draw();
    void exit();

    // GUI
    void initGuiPositions();
    void drawImGuiInterface();

    // Keyboard Events
    void keyPressed(int key);
    void keyReleased(ofKeyEventArgs &e);

    // Mouse Events
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);

    // Misc Events
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);

    // threaded file dialog callback
    void onFileDialogResponse(ofxThreadedFileDialogResponse &response);

    // Filepath watcher callback
    void pathChanged(const PathWatcher::Event &event);

    // ofxLua error callback
    void errorReceived(std::string& msg);


    // custom methods
    void            initResolution();
    void            resetResolution(int newWidth, int newHeight);

    void            openScript(string scriptFile);
    void            loadScript();
    void            unloadScript();
    void            clearScript();
    void            reloadScript();

    glm::vec2       reduceToAspectRatio(int _w, int _h);
    void            scaleTextureToWindow(int theScreenW, int theScreenH);
    void            toggleWindowFullscreen();
    void            resetOutputResolution();

    void            initDataFolderFromBundle();

    ofxLua                      lua;
    string                      filepath;


    ofxThreadedFileDialog       fileDialog;
    PathWatcher                 watcher;
    ofFile                      currentScriptFile;
    bool                        loadLuaScriptFlag;
    bool                        saveLuaScriptFlag;
    bool                        scriptLoaded;
    bool                        isError;
    bool                        setupTrigger;
    bool                        loaded;

    ofFbo                       *fbo;
    ofImage                     *kuro;

    bool                        isFullscreen;
    int                         temp_width, temp_height;
    float                       output_width, output_height;
    int                         window_actual_width, window_actual_height;
    glm::vec2                   asRatio;
    glm::vec2                   window_asRatio;
    float                       thposX, thposY, thdrawW, thdrawH;
    bool                        needReset;

    // GUI
    ofxImGui::Gui               mainGUI;
    AppTheme*                   mainTheme;
    string                      shortcutFunc;
    bool                        isRetina;
    float                       scaleFactor;
    ofRectangle                 loggerRect;
    ofRectangle                 scriptMenuRect;

    bool                        bShowLogger;
    bool                        bShowScriptMenu;

protected:

    shared_ptr<AppLoggerChannel>        appLoggerChannel;

    size_t                      startTime;
    size_t                      waitTime;

    // BUNDLE
    std::filesystem::path       ofluaAppPath;
    string                      userHome;

    string                      windowTitle;
    string                      tempstring;
    bool                        needToLoadScript;

};
