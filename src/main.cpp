#include "ofMain.h"
#include "ofApp.h"
#include "ofxYAML.h"

//========================================================================
int main( ){
    ofxYAML yamlReader;
    yamlReader.load("config.yaml");
    int screenWidth = yamlReader["screenWidth"].as<int>();
    int screenHeight = yamlReader["screenHeight"].as<int>();
    
    ofSetupOpenGL(screenWidth, screenHeight,OF_WINDOW);
        
	ofRunApp(new ofApp());

}
