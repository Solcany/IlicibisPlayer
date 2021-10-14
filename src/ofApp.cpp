#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetVerticalSync(false);
    ilicibis.setup();
}

//--------------------------------------------------------------
void ofApp::update(){
    ilicibis.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    ilicibis.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
    std::cout << ilicibis.playerName << " exit" << std::endl;
    ilicibis.closeServer();
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    if(!ilicibis.isCameraStreamScheduled) {
        ilicibis.toggleCameraStream();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    switch(key) {
        case('s'):
            ilicibis.start();
            break;
        case('f'):
            ilicibis.toggleFullscreen();
            break;
        case('q'):
            ofExit();
            break;
    }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}
