#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(false);
    ofSetFrameRate(60);
    ofSetLogLevel("ofxLua", OF_LOG_VERBOSE);

    // init the lua state
    lua.init(true); // true because we want to stop on an error

    // listen to error events
    lua.addListener(this);

    // run a script
    // true = change working directory to the script's parent dir
    // so lua will find scripts with relative paths via require
    // note: changing dir does *not* affect the OF data path
    testScript = ofToDataPath("scripts/testing.lua");
    lua.doScript(testScript, true);

    // call the script's setup() function
    lua.scriptSetup();
}

//--------------------------------------------------------------
void ofApp::update(){
    // call the script's update() function
    lua.scriptUpdate();
}

//--------------------------------------------------------------
void ofApp::draw(){
    //ofBackground(30);

    // call the script's draw() function
    lua.scriptDraw();
}

//--------------------------------------------------------------
void ofApp::exit() {
    // call the script's exit() function
    lua.scriptExit();

    // clear the lua state
    lua.clear();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    lua.scriptKeyPressed(key);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}

//--------------------------------------------------------------
void ofApp::errorReceived(std::string& msg) {
    ofLogNotice() << "got a script error: " << msg;
}
