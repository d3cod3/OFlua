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

    isFullscreen                        = false;
    thposX = thposY = thdrawW = thdrawH = 0.0f;

    output_width        = STANDARD_TEXTURE_WIDTH;
    output_height       = STANDARD_TEXTURE_HEIGHT;

    temp_width              = output_width;
    temp_height             = output_height;

    window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH;
    window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT;

    needReset           = false;
    needToLoadScript    = true;

    scriptLoaded        = false;
    isError             = false;
    setupTrigger        = false;

    loadLuaScriptFlag   = false;
    saveLuaScriptFlag   = false;

    bShowLogger         = false;
    bShowScriptMenu     = false;

    // RETINA FIX
    isRetina = false;
    scaleFactor = 1.0f;
    if(ofGetScreenWidth() >= RETINA_MIN_WIDTH && ofGetScreenHeight() >= RETINA_MIN_HEIGHT){ // RETINA SCREEN
        isRetina = true;
        scaleFactor = 2.0f;
        if(ofGetScreenWidth() > 3360 && ofGetScreenHeight() > 2100){
            window_actual_width     = STANDARD_PROJECTOR_WINDOW_WIDTH*2;
            window_actual_height    = STANDARD_PROJECTOR_WINDOW_HEIGHT*2;
        }
    }
    ofSetWindowShape(window_actual_width, window_actual_height);
    // setup drawing  dimensions
    asRatio = reduceToAspectRatio(output_width,output_height);
    window_asRatio = reduceToAspectRatio(ofGetWindowWidth(),ofGetWindowHeight());
    scaleTextureToWindow(ofGetWindowWidth(),ofGetWindowHeight());

    // LOGGER
    appLoggerChannel = shared_ptr<AppLoggerChannel>(new AppLoggerChannel());
    ofSetLoggerChannel(appLoggerChannel);

    // GUI
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

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

    if(needReset){
        needReset = false;
        resetOutputResolution();
    }

    if(loadLuaScriptFlag){
        loadLuaScriptFlag = false;
        fileDialog.openFile("load lua script","Select a lua script");
    }

    if(saveLuaScriptFlag){
        saveLuaScriptFlag = false;
        string newFileName = "luaScript_"+ofGetTimestampString("%y%m%d")+".lua";
        fileDialog.saveFile("save lua script","Save new Lua script as",newFileName);
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
    ofBackground(0);

    ofSetColor(255);
    fbo->draw(thposX, thposY, thdrawW, thdrawH);

    // GUI
    ofSetColor(255,255,255);
    drawImGuiInterface();
}

//--------------------------------------------------------------
void ofApp::drawImGuiInterface(){
    mainGUI.begin();
    {

        // script menu
        // ---------------------------------------------
        if(bShowScriptMenu){
            ImGui::SetNextWindowSize(ImVec2(scriptMenuRect.width,scriptMenuRect.height), ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(scriptMenuRect.x,scriptMenuRect.y), ImGuiCond_Always);
            if (!ImGui::Begin("Lua Script Config",nullptr,ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize))
            {
                ImGui::End();
                return;
            }

            // Lua Script Config content
            ImGui::Spacing();

            if(ImGui::InputInt("Width",&temp_width)){
                if(temp_width > OUTPUT_TEX_MAX_WIDTH){
                    temp_width = output_width;
                }
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()){
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("You can set the output resolution WxH (limited for now at max. 4800x4800)");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            if(ImGui::InputInt("Height",&temp_height)){
                if(temp_height > OUTPUT_TEX_MAX_HEIGHT){
                    temp_height = output_height;
                }
            }
            ImGui::Spacing();
            if(ImGui::Button("APPLY",ImVec2(-1,26))){
                needReset = true;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Loaded File:");
            ImGui::Text("%s",currentScriptFile.getFileName().c_str());
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",currentScriptFile.getAbsolutePath().c_str());

            ImGui::Spacing();
            if(ImGui::Button("New",ImVec2(-1,26))){
                saveLuaScriptFlag = true;
            }
            ImGui::Spacing();
            if(ImGui::Button("Open",ImVec2(-1,26))){
                loadLuaScriptFlag = true;
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Separator();
            ImGui::Spacing();
            if(ImGui::Button("Clear Script",ImVec2(-1,26))){
                clearScript();
            }
            ImGui::Spacing();
            if(ImGui::Button("Reload Script",ImVec2(-1,26))){
                reloadScript();
            }

            ImGui::End();

        }
        // ---------------------------------------------

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
    scriptMenuRect.set(0,0,ofGetWindowWidth()/4,ofGetWindowHeight()-(loggerH*scaleFactor));
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
void ofApp::keyReleased(ofKeyEventArgs &e){

    // OSX: CMD-F, WIN/LINUX: CTRL-F    (FULLSCREEN)
    if(e.hasModifier(MOD_KEY) && e.keycode == 70){
        toggleWindowFullscreen();
    // OSX: CMD-L, WIN/LINUX: CTRL-L    (SHOW LOGGER)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 76){
        bShowLogger = !bShowLogger;
    // OSX: CMD-G, WIN/LINUX: CTRL-G    (SHOW GUI)
    }else if(e.hasModifier(MOD_KEY) && e.keycode == 71){
        bShowScriptMenu = !bShowScriptMenu;
    }

    //ofLog(OF_LOG_NOTICE,"%i",e.keycode);

    lua.scriptKeyReleased(e.key);
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

        window_asRatio = reduceToAspectRatio(ofGetWindowWidth(),ofGetWindowHeight());
        scaleTextureToWindow(ofGetWindowWidth(),ofGetWindowHeight());
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
    if(response.id == "load lua script"){
        ofFile file(response.filepath);
        if (file.exists()){
            string fileExtension = ofToUpper(file.getExtension());
            if(fileExtension == "LUA") {
                filepath = file.getAbsolutePath();
                reloadScript();
            }
        }
    }else if(response.id == "save lua script"){
        ofFile fileToRead(ofToDataPath("scripts/empty.lua"));

        ofFile tempPF(response.filepath);
        string preSanitizeFN = tempPF.getFileName();
        sanitizeFilename(preSanitizeFN);
        string sanitizedPatchFile = tempPF.getEnclosingDirectory()+preSanitizeFN;
        ofFile tempFile(sanitizedPatchFile);
        string tempFileName = tempFile.getFileName();
        string finalTempFileName = tempFile.getFileName().substr(0,tempFileName.find_last_of('.'));

        // create containing folder
        ofDirectory::createDirectory(tempPF.getEnclosingDirectory()+finalTempFileName+"/");

        ofFile newLuaFile (tempPF.getEnclosingDirectory()+finalTempFileName+"/"+finalTempFileName+".lua");

        // copy file
        ofFile::copyFromTo(fileToRead.getAbsolutePath(),newLuaFile.getAbsolutePath(),true,true);

        // create data/ folder
        ofDirectory::createDirectory(newLuaFile.getEnclosingDirectory()+"data/");

        filepath = newLuaFile.getAbsolutePath();

        reloadScript();
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

    currentScriptFile.open(filepath);

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
glm::vec2 ofApp::reduceToAspectRatio(int _w, int _h){
    glm::vec2 _res;
    int temp = _w*_h;
    if(temp>0){
        for(int tt = temp; tt>1; tt--){
            if((_w%tt==0) && (_h%tt==0)){
                _w/=tt;
                _h/=tt;
            }
        }
    }else if (temp<0){
        for (int tt = temp; tt<-1; tt++){
            if ((_w%tt==0) && (_h%tt==0)){
                _w/=tt;
                _h/=tt;
            }
        }
    }
    _res = glm::vec2(_w,_h);
    return _res;
}

//--------------------------------------------------------------
void ofApp::scaleTextureToWindow(int theScreenW, int theScreenH){
    // wider texture than screen
    if(asRatio.x/asRatio.y >= window_asRatio.x/window_asRatio.y){
        thdrawW           = theScreenW;
        thdrawH           = (output_height*theScreenW) / output_width;
        thposX            = 0;
        thposY            = (theScreenH-thdrawH)/2.0f;
    // wider screen than texture
    }else{
        thdrawW           = (output_width*theScreenH) / output_height;
        thdrawH           = theScreenH;
        thposX            = (theScreenW-thdrawW)/2.0f;
        thposY            = 0;
    }
    //ofLog(OF_LOG_NOTICE,"Window: %ix%i, Texture; %fx%f at %f,%f",theScreenW,theScreenH,thdrawW,thdrawH,thposX,thposY);
}

//--------------------------------------------------------------
void ofApp::toggleWindowFullscreen(){
    isFullscreen = !isFullscreen;
    ofToggleFullscreen();

    if(!isFullscreen){
        ofSetWindowShape(window_actual_width, window_actual_height);
        scaleTextureToWindow(window_actual_width, window_actual_height);
    }else{
        scaleTextureToWindow(ofGetScreenWidth(),ofGetScreenHeight());
    }
}

//--------------------------------------------------------------
void ofApp::resetOutputResolution(){

    if(output_width != temp_width || output_height != temp_height){
        output_width = temp_width;
        output_height = temp_height;

        resetResolution(output_width,output_height);

        asRatio = reduceToAspectRatio(output_width,output_height);

        if(!isFullscreen){
            window_asRatio = reduceToAspectRatio(ofGetWindowWidth(),ofGetWindowHeight());
            scaleTextureToWindow(ofGetWindowWidth(),ofGetWindowHeight());
        }else{
            window_asRatio = reduceToAspectRatio(ofGetScreenWidth(),ofGetScreenHeight());
            scaleTextureToWindow(ofGetScreenWidth(),ofGetScreenHeight());
        }


        ofLog(OF_LOG_NOTICE,"RESOLUTION CHANGED TO %ix%i",static_cast<int>(output_width),static_cast<int>(output_height));
    }

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

