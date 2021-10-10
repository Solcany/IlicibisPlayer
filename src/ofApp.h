#pragma once

#include "ofMain.h"
#include "IlicibisPlayer.hpp"

class ofApp : public ofBaseApp{

    public:
        IlicibisPlayer ilicibis;
        void setup();
        void update();
        void draw();
    
        void exit();
        void keyReleased(int key);
        void mouseReleased(int x, int y, int button);
        void windowResized(int w, int h);
        void gotMessage(ofMessage msg);
    
        
};
