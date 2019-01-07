#pragma once
#include "ofMain.h"
#include "ofxMidiParams.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofParameterGroup padParams;
    ofParameter<bool> pad1, pad2, pad3, pad4, pad5, pad6, pad7, pad8;
    ofParameterGroup kParams;
    ofParameter<float> k1, k2, k3, k4, k5, k6, k7, k8;
    
    ofxMidiParams mMidiParams;
		
};
