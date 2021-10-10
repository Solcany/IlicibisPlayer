#ifndef _ILICIBISPLAYER
#define _ILICIBISPLAYER

#include <cmath>
#include "ofMain.h"
#include "ofxYAML.h"
#include "ofxNetwork.h"
#include "ofxIO.h"

class IlicibisPlayer {
    public:
        void setup();
        void start();
        void update();
        void draw();
        void triggerScheduledVideo();
        void toggleCameraStream();
        void closeServer();
        //ofTexture udpTexture;
        IlicibisPlayer();
    private:
        ofxYAML yamlReader;

        // config
        float audioVolume;
        int nFramesToSkip;
        int screenWidth;
        int screenHeight;
        int camStreamWidth;
        int camStreamHeight;
        int camStreamBytesSize;
        int tcpPort;
        bool playScheduledVideos;
        
        ofxTCPServer tcpServer;
    
        ofTrueTypeFont appFont;
        vector<string> videoPaths;
        vector<string> scheduledVideoPaths;
        int videosTotal = 0;
        int scheduledVideosTotal = 0;
        int nextScheduledVideoIndex = 0;
        ofVideoPlayer player0;
        bool player0TriggeredNext = false;
        ofVideoPlayer player1;
        bool player1TriggeredNext = false;
        ofVideoPlayer playerScheduled;
        bool playerScheduledTriggeredNext = false;
        ofTexture camStreamTexture;
        ofPixels camStreamPixels;

        enum PlayerStates {PAUSED, RUNNING};
        enum PlayerSources {PLAYERZERO, PLAYERONE, PLAYERSCHEDULED, CAMSTREAM};
        PlayerStates playerState = PAUSED;
        PlayerSources playerSource = PLAYERZERO;
        PlayerSources lastPlayerSource;

        void setupAppFromYamlConfig();
        void setPlayerState(PlayerStates newState);
        void setPlayerSource(PlayerSources newSource);
        void setLastPlayerSource(PlayerSources source);
        PlayerSources getLastPlayerSource();
        PlayerSources getCurrentPlayerSource();
        string getRandomVideoPath();
    
};

#endif
