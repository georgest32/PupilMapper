#pragma once


#include "ofMain.h"
#include "ofxGui.h"
#include "ofxCv.h"
#include "ofxBox2d.h"
#include "ofxOsc.h"

#define HOST "localhost"
#define PORT 12001
#define WEKPORT 12002
#define NUM_MSG_STRINGS 20

class ofApp: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    void mousePressed(int x, int y, int button);
    void mouseMoved(int x, int y );
        
    ofVideoPlayer video;
    ofPixels binaryPixels;
    ofTexture binaryTexture;
    ofPolyline line;
    ofxCv::ContourFinder contourFinder;

    ofxPanel gui;
    ofParameter<float> blurLevel;
    ofParameter<int> threshold;
    ofParameter<bool> invert;
    ofParameter<int> erodeIterations;
    ofParameter<int> dilateIterations;
    ofParameter<float> contourMinArea;
    ofParameter<float> contourMaxArea;
    ofParameter<float> contourThreshold;
    ofParameter<float> alphaPOT;
    
    ofTrueTypeFont font;
    ofxOscSender sender;
    ofxOscSender wekSender;
    ofxOscReceiver receiver;
    ofBuffer imgAsBuffer;
    ofImage img;
    
    std::vector<std::pair<float, float>> pupilDilationVec;
    
    string msg_strings[NUM_MSG_STRINGS];
    bool recordPDF = false;
    int current_msg_string;
    int counter = 0;
    int mouseX = 0;
    int mouseY = 0;
    int centroidX;
    int centroidY;
    int circleResolution = 10;
    float colorR = 0;
    float colorG = 0;
    float colorB = 0;
    float pupilArea = 0;
    float pupilDilation = 0;
    float pupilDilationOverTime = 0;
    float pupilAreaOverTime = 0;
    float counter1 = 0;
    float counter2 = 0;
    float timers[NUM_MSG_STRINGS];
};



