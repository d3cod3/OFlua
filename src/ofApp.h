#pragma once

#include "ofMain.h"
#include "ofxLua.h"

class ofApp : public ofBaseApp, ofxLuaListener {

public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    void keyReleased(int key);

    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);

    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);

    // ofxLua error callback
    void errorReceived(std::string& msg);

    ofxLua      lua;
    string      testScript;

};
