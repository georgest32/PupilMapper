#include "ofApp.h"

void ofApp::setup()
{
    receiver.setup(PORT);
    sender.setup(HOST, PORT);
    wekSender.setup(HOST, WEKPORT);
    imgAsBuffer = ofBufferFromFile("of-logo.png", true);
    ofSetWindowTitle("Pupil Detector");
    video.load("movies/eye2.mp4");
    current_msg_string = 0;

    video.play();
    
    binaryPixels.allocate(video.getWidth()/2,
                          video.getHeight()/2,
                          OF_PIXELS_GRAY);
    binaryTexture.loadData(binaryPixels);
    
    gui.setup();
    gui.add(blurLevel.set("Blur Level", 5, 0, 100));
    gui.add(threshold.set("Threshold", 55, 0, 100));
    gui.add(erodeIterations.set("Erode Iterations",0, 0, 30));
    gui.add(dilateIterations.set("Dilate Iterations", 0, 0, 30));
    gui.add(contourMaxArea.set("Contour Max Area", 200, 1, 500));
    gui.add(contourMinArea.set("Contour Min Area", 25, 1, 500));
    gui.add(alphaPOT.set("alpha POT", 0.99, 0, 1));
    gui.add(invert.set("Invert", true));

    auto it = pupilDilationVec.begin();
    pupilDilationVec.insert(it, std::make_pair(0, 0));
}

void ofApp::update()
{
// hide old messages
    for(int i = 0; i < NUM_MSG_STRINGS; i++){
        if(timers[i] < ofGetElapsedTimef()){
            msg_strings[i] = "";
        }
    }
// check for waiting messages
    while(receiver.hasWaitingMessages()){
// get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        string msg_string;
        msg_string = m.getAddress();
        msg_string += ":";
        for(int i = 0; i < m.getNumArgs(); i++){
// get the argument type
            msg_string += " ";
            msg_string += m.getArgTypeName(i);
            msg_string += ":";
// display the argument - make sure we get the right type
            if(m.getArgType(i) == OFXOSC_TYPE_INT32){
                msg_string += ofToString(m.getArgAsInt32(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
                msg_string += ofToString(m.getArgAsFloat(i));
            }
            else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
                msg_string += m.getArgAsString(i);
            }
            else{
                msg_string += "unknown";
            }
        }
// add to the list of strings to display
        msg_strings[current_msg_string] = msg_string;
        timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
        current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
// clear the next line
        msg_strings[current_msg_string] = "";
        }
    
    video.update();
    
    if (video.isFrameNew())
    {
        ofxCv::convertColor(video, binaryPixels, CV_RGB2GRAY);
        ofxCv::blur(binaryPixels, binaryPixels, blurLevel);
        ofxCv::threshold(binaryPixels, binaryPixels, threshold, invert);
        ofxCv::erode(binaryPixels, binaryPixels, erodeIterations);
        ofxCv::dilate(binaryPixels, binaryPixels, dilateIterations);

        contourFinder.setMinAreaRadius(contourMinArea);
        contourFinder.setMaxAreaRadius(contourMaxArea);
        contourFinder.setThreshold(contourThreshold);
        contourFinder.findContours(binaryPixels);

        binaryTexture.loadData(binaryPixels);
    }
    
    ofxOscMessage m;
    
    m.setAddress("/p2/inputs");
    m.addFloatArg(pupilArea);
    m.addFloatArg(centroidX);
    m.addFloatArg(centroidY);
    m.addFloatArg(ofMap(pupilDilationOverTime, -200, 200, 0, 1, true));
    
//  ****Wekinator Messaging****

    ofxOscMessage m2;
    
    m2.setAddress("/wek/inputs");
    m2.addFloatArg(ofMap(pupilDilationOverTime, -200, 200, 0, 100.f, true));
    m2.addFloatArg(ofMap(pupilAreaOverTime, 35000, 70000, 0, 100.f, true));

//  **Training**
//
//    m2.addFloatArg(ofMap(200, -200, 200, 0, 100.f, true));
//    m2.addFloatArg(ofMap(70000, 35000, 70000, 0, 100.f, true));
    
    sender.sendMessage(m, false);
    wekSender.sendMessage(m2, false);
}

void ofApp::mouseMoved(int x, int y){
    mouseX = x;
    mouseY = y;
}

void ofApp::draw()
{
    ofFill();
    colorG = ofMap(pupilAreaOverTime, 20000, 100000, 0, 255);
    colorR = ofMap(pupilDilationOverTime, -200, 200, 0, 255);

    ofSetColor(255);

    video.draw(0,0);
    
    ofPushStyle();
    ofSetLineWidth(3);
    
    if(contourFinder.size() > 0 && video.isPlaying() == true)
    {
        auto i = 0;
        auto it = pupilDilationVec.begin();
        float radius = glm::distance(contourFinder.getPolyline(i)[0],
                                     contourFinder.getPolyline(i).getCentroid2D());
        
        centroidX = contourFinder.getPolyline(i).getCentroid2D().x;
        centroidY = contourFinder.getPolyline(i).getCentroid2D().y;

        ofSetColor(0,255,0,80);

        contourFinder.getPolyline(i).draw();
        ofDrawCircle(centroidX, centroidY, radius);
        
        pupilArea = PI * (radius * radius);
        
        pupilDilation = pupilArea - (it + 1)->first;
        
        pupilDilationVec.insert (it, std::make_pair(pupilArea, pupilDilation));
        
        pupilAreaOverTime = pupilAreaOverTime * alphaPOT + pupilArea * (1 - alphaPOT);
        pupilDilationOverTime = pupilDilationOverTime * alphaPOT + pupilDilation * (1 - alphaPOT);
        
        if(pupilDilationVec.size() >= 1000)
            pupilDilationVec.erase(it + 999);
        
        cout << "Pupil Area: " << pupilArea << std::endl;
        cout << "Pupil Area Over Time: " << pupilAreaOverTime << std::endl;
        cout << "Pupil Dilation: " << pupilDilation << std::endl;
        cout << "Pupil Dilation Over Time: " << pupilDilationOverTime << std::endl;
        cout << "Centroid X: " << centroidX << std::endl;
        cout << "Centroid Y: " << centroidY << std::endl;
        cout << "- - - - - - - - - - - - - - -" << std::endl;
    }

    ofPopStyle();
    binaryTexture.draw(640, 0);
    gui.draw();
    ofFill();
}

void ofApp::mousePressed(int x, int y, int button){
//  ****Wekinator Training****
//
//    if(video.isPlaying() == true){
//        video.stop();
//        video.previousFrame();
//    }
//    else
//        video.play();
//
//    ofxOscMessage m;
//    
//    m.setAddress("/wek/inputs");
//    
//    m.addFloatArg(ofMap(pupilAreaOverTime, 20000, 100000, 0.f, 1.f, true));
//    m.addFloatArg(ofMap(pupilDilationOverTime, -200, 200, 0.f, 1.f, true));
//    
//    sender.sendMessage(m, false);
}
