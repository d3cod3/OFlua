#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ///////////////////////////////////////////
    // OF Stuff
    ofSetEscapeQuitsApp(false);
    ofSetFrameRate(60);
    ofEnableAntiAliasing();
    ofSetLogLevel(PACKAGE, OF_LOG_NOTICE);

    initDataFolderFromBundle();
    ///////////////////////////////////////////

    // SYSTEM
    startTime   = ofGetElapsedTimeMillis();
    waitTime    = 200;

    // RETINA FIX
    isRetina = false;
    scaleFactor = 1.0f;
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){ // RETINA SCREEN
        isRetina = true;
        scaleFactor = 2.0f;
    }

    // LOGGER
    appLoggerChannel = shared_ptr<AppLoggerChannel>(new AppLoggerChannel());
    ofSetLoggerChannel(appLoggerChannel);

    // GUI
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ofFile fileToRead1(ofToDataPath(MAIN_FONT));
    string absPath1 = fileToRead1.getAbsolutePath();
    if(isRetina){
        io.Fonts->AddFontFromFileTTF(absPath1.c_str(),30.0f);
    }else{
        io.Fonts->AddFontFromFileTTF(absPath1.c_str(),18.0f);
    }

    ImFont* defaultfont = io.Fonts->Fonts[io.Fonts->Fonts.Size - 1];
    io.FontDefault = defaultfont;

    #ifdef TARGET_OSX
        shortcutFunc = "CMD";
    #else
        shortcutFunc = "CTRL";
    #endif

    // Main GUI
    mainTheme = new AppTheme();
    if(isRetina){
        mainTheme->fixForRetinaScreen();
    }
    mainGUI.setup();
    mainGUI.setTheme(mainTheme);

    // Threaded File Dialogs
    fileDialog.setup();
    ofAddListener(fileDialog.fileDialogEvent, this, &ofApp::onFileDialogResponse);


    output_width        = WINDOW_START_WIDTH;
    output_height       = WINDOW_START_HEIGHT;

    needToLoadScript    = true;

    scriptLoaded        = false;
    isError             = false;
    setupTrigger        = false;

    bShowLogger         = false;
    bShowScriptMenu     = false;

    initResolution();

    // load kuro
    kuro = new ofImage();
    kuro->load("images/kuro.jpg");

    // init lua
    lua.init(true);
    lua.addListener(this);
    watcher.start();

    // laod template script
    ofFile startScript(ofToDataPath("scripts/empty.lua",true));
    filepath = startScript.getAbsolutePath();

    // loaded flag
    loaded              = false;

}

//--------------------------------------------------------------
void ofApp::update(){

    windowTitle = filepath+" - "+WINDOW_TITLE;
    ofSetWindowTitle(windowTitle);

    while(watcher.waitingEvents()) {
        pathChanged(watcher.nextEvent());
    }

    if(needToLoadScript){
        needToLoadScript = false;
        loadScript();
        setupTrigger = false;
    }

    ///////////////////////////////////////////
    // LUA UPDATE
    if(scriptLoaded && !isError){
        if(!setupTrigger){
            setupTrigger = true;
            lua.scriptSetup();
        }

        // update lua state
        ofSoundUpdate();
        lua.scriptUpdate();
    }
    ///////////////////////////////////////////

    ///////////////////////////////////////////
    // LUA DRAW
    fbo->begin();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    ofPushView();
    ofPushStyle();
    ofPushMatrix();
    if(scriptLoaded && !isError){
        lua.scriptDraw();
    }else{
        kuro->draw(0,0,fbo->getWidth(),fbo->getHeight());
    }
    ofPopMatrix();
    ofPopStyle();
    ofPopView();
    glPopAttrib();
    fbo->end();
    ///////////////////////////////////////////

    if(!loaded && ofGetElapsedTimeMillis()-startTime > waitTime){
        loaded = true;
        initGuiPositions();
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(30);

    ofSetColor(255);
    fbo->draw(0,0);

    // GUI
    ofSetColor(255,255,255);
    drawImGuiInterface();
}

//--------------------------------------------------------------
void ofApp::drawImGuiInterface(){
    mainGUI.begin();
    {


        // logger
        // ---------------------------------------------
        if(bShowLogger){
            ImGui::SetNextWindowSize(ImVec2(loggerRect.width,loggerRect.height), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(loggerRect.x,loggerRect.y), ImGuiCond_Always);
            appLoggerChannel->Draw("Logger");
        }
        // ---------------------------------------------
    }

    mainGUI.end();
}

//--------------------------------------------------------------
void ofApp::initGuiPositions(){
    float loggerH = max(LOGGER_HEIGHT,ofGetWindowHeight()/3);
    loggerRect.set(0,ofGetWindowHeight()-(loggerH*scaleFactor),ofGetWindowWidth(),loggerH*scaleFactor);
}

//--------------------------------------------------------------
void ofApp::exit() {
    ///////////////////////////////////////////
    // LUA EXIT
    lua.scriptExit();
    // clear the lua state
    lua.clear();
    ///////////////////////////////////////////

    fileDialog.stop();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    lua.scriptKeyPressed(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    lua.scriptKeyReleased(key);

    if(key == 'l' || key == 'L'){
        bShowLogger = !bShowLogger;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    lua.scriptMouseMoved(x, y);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    lua.scriptMouseDragged(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    lua.scriptMousePressed(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    lua.scriptMouseReleased(x, y, button);
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){
    lua.scriptMouseScrolled(x,y,scrollX,scrollY);
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    if(loaded){
        initGuiPositions();
    }
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    if( dragInfo.files.size() == 1 ){
        ofFile file (dragInfo.files[0]);
        openScript(file.getAbsolutePath());
    }
}

//--------------------------------------------------------------
void ofApp::onFileDialogResponse(ofxThreadedFileDialogResponse &response){
    if(response.id == "open script"){
        ofFile file(response.filepath);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "LUA") {
                filepath = file.getAbsolutePath();
                reloadScript();
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::pathChanged(const PathWatcher::Event &event) {
    switch(event.change) {
        case PathWatcher::CREATED:
            //ofLogVerbose(PACKAGE) << "path created " << event.path;
            break;
        case PathWatcher::MODIFIED:
            //ofLogVerbose(PACKAGE) << "path modified " << event.path;
            filepath = event.path;
            reloadScript();
            break;
        case PathWatcher::DELETED:
            //ofLogVerbose(PACKAGE) << "path deleted " << event.path;
            return;
        default: // NONE
            return;
    }

}

//--------------------------------------------------------------
void ofApp::errorReceived(std::string& msg) {
    isError = true;

    if(!msg.empty()){
        size_t found = msg.find_first_of("\n");
        if(found == string::npos){
            ofLog(OF_LOG_ERROR,"LUA SCRIPT error: %s",msg.c_str());
        }
    }
}

//--------------------------------------------------------------
void ofApp::initResolution(){
    fbo = new ofFbo();
    fbo->allocate(output_width,output_height,GL_RGBA32F_ARB,4);
    fbo->begin();
    ofClear(0,0,0,255);
    fbo->end();
}

//--------------------------------------------------------------
void ofApp::resetResolution(int newWidth, int newHeight){
    if(output_width!=newWidth || output_height!=newHeight){
        output_width    = newWidth;
        output_height   = newHeight;

        initResolution();

        tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
        lua.doString(tempstring);
        tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
        lua.doString(tempstring);
        ofFile tempFileScript(filepath);
        tempstring = "SCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'";

#ifdef TARGET_WIN32
        std::replace(tempstring.begin(),tempstring.end(),'\\','/');
#endif

        lua.doString(tempstring);
    }
}

//--------------------------------------------------------------
void ofApp::openScript(string scriptFile){
    ofFile file (scriptFile);
    if (file.exists()){
        string fileExtension = ofToUpper(file.getExtension());
        if(fileExtension == "LUA") {
            filepath = file.getAbsolutePath();
            reloadScript();
        }
    }
}

//--------------------------------------------------------------
void ofApp::loadScript(){
    lua.doScript(filepath, true);

    tempstring = "OUTPUT_WIDTH = "+ofToString(output_width);
    lua.doString(tempstring);
    tempstring = "OUTPUT_HEIGHT = "+ofToString(output_height);
    lua.doString(tempstring);
    ofFile tempFileScript(filepath);
    tempstring = "SCRIPT_PATH = '"+tempFileScript.getEnclosingDirectory().substr(0,tempFileScript.getEnclosingDirectory().size()-1)+"'";

#ifdef TARGET_WIN32
    std::replace(tempstring.begin(),tempstring.end(),'\\','/');
#endif

    lua.doString(tempstring);

    scriptLoaded = lua.isValid();

    ///////////////////////////////////////////
    // LUA SETUP
    if(scriptLoaded  && !isError){
        watcher.removeAllPaths();
        watcher.addPath(filepath);
        ofLog(OF_LOG_NOTICE,"[verbose] lua script: %s loaded & running!",filepath.c_str());
    }
    ///////////////////////////////////////////
}

//--------------------------------------------------------------
void ofApp::unloadScript(){
    lua.scriptExit();
    lua.init(true);
}

//--------------------------------------------------------------
void ofApp::clearScript(){
    unloadScript();

    lua.doString("function draw() of.background(0) end");

    scriptLoaded = lua.isValid();

}

//--------------------------------------------------------------
void ofApp::reloadScript(){

    unloadScript();

    scriptLoaded = false;
    needToLoadScript = true;
    isError = false;
}

//--------------------------------------------------------------
void ofApp::initDataFolderFromBundle(){

    #ifdef TARGET_OSX

    string _bundleDataPath;

    CFURLRef appUrl = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef appPath = CFURLCopyFileSystemPath(appUrl, kCFURLPOSIXPathStyle);

    const CFIndex kCStringSize = 128;
    char temporaryCString[kCStringSize];
    bzero(temporaryCString,kCStringSize);
    CFStringGetCString(appPath, temporaryCString, kCStringSize, kCFStringEncodingUTF8);
    std::string *appPathStr = new std::string(temporaryCString);
    CFRelease(appUrl);
    CFRelease(appPath);

    CFURLRef resourceUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    CFStringRef resourcePath = CFURLCopyFileSystemPath(resourceUrl, kCFURLPOSIXPathStyle);

    bzero(temporaryCString,kCStringSize);
    CFStringGetCString(resourcePath, temporaryCString, kCStringSize, kCFStringEncodingUTF8);
    std::string *resourcePathStr = new std::string(temporaryCString);
    CFRelease(resourcePath);
    CFRelease(resourceUrl);

    _bundleDataPath = *appPathStr + "/" + *resourcePathStr + "/"; // the absolute path to the resources folder

    const char *homeDir = getenv("HOME");

    if(!homeDir){
        struct passwd* pwd;
        pwd = getpwuid(getuid());
        if (pwd){
            homeDir = pwd->pw_dir;
        }
    }

    string _AppDataPath(homeDir);
    userHome = _AppDataPath;

    _AppDataPath += "/Documents/OFlua/data";

    std::filesystem::path tempPath(_AppDataPath.c_str());

    ofluaAppPath = tempPath;

    ofDirectory appDir;

    // data directory
    if(!appDir.doesDirectoryExist(ofluaAppPath)){
        appDir.createDirectory(ofluaAppPath,true,true);

        std::filesystem::path dataPath(_bundleDataPath.c_str());

        ofDirectory dataDir(dataPath);
        dataDir.copyTo(ofluaAppPath,true,true);
    }else{
        string relfilepath = _AppDataPath+"/release.txt";
        std::filesystem::path releasePath(relfilepath.c_str());
        ofFile relFile(releasePath);

        if(relFile.exists()){
            string actualRel = relFile.readToBuffer().getText();

            if(VERSION != actualRel){
                std::filesystem::path dataPath(_bundleDataPath.c_str());

                ofDirectory dataDir(dataPath);
                dataDir.copyTo(ofluaAppPath,true,true);
            }
        }
    }

    ofSetDataPathRoot(ofluaAppPath); // tell OF to look for resources here

    #else



    #endif

}

