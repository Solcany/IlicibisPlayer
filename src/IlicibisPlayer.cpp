#include "IlicibisPlayer.hpp"

IlicibisPlayer::IlicibisPlayer(){
    }
    void IlicibisPlayer::setupAppFromYamlConfig() {
        yamlReader.load("config.yaml");
        float _audioVolume = yamlReader["audioVolume"].as<float>();
        float _nFramesToSkip = yamlReader["nFramesToSkip"].as<int>();
        int _camStreamWidth = yamlReader["camStreamWidth"].as<int>();
        int _camStreamHeight = yamlReader["camStreamHeight"].as<int>();
        int _tcpPort = yamlReader["tcpPort"].as<int>();
        float _camStreamDelay = yamlReader["camStreamDelay"].as<float>();
        float _camStreamDuration = yamlReader["camStreamDuration"].as<float>();
        string _videosDirPath = yamlReader["videosDirPath"].as<string>();
        string _playerName = yamlReader["playerName"].as<string>();
        string _videosExtension = yamlReader["videosExtension"].as<string>();
        string _playScheduledVideos = yamlReader["playScheduledVideos"].as<string>();
        string _isCameraStreamScheduled = yamlReader["isCameraStreamScheduled"].as<string>();
        
        YAML::Node scheduledVideoNames = yamlReader["scheduledVideos"];
        // convert yaml nodes to vectors
        vector<string> _scheduledVideoPaths;
        for(int i = 0; i < scheduledVideoNames.size(); i++) {
            string name = scheduledVideoNames[i].as<string>();
            string path = "./videos/" + name;
            _scheduledVideoPaths.push_back(path);
        }
        // set up the app
        audioVolume = _audioVolume;
        nFramesToSkip = _nFramesToSkip;
        camStreamWidth = _camStreamWidth;
        camStreamHeight = _camStreamHeight;
        camStreamBytesSize = camStreamWidth * camStreamHeight * 3;
        camStreamDelay = _camStreamDelay;
        camStreamDuration = _camStreamDuration;
        tcpPort = _tcpPort;
        playerName = _playerName;
        videosDirPath = _videosDirPath;
        videosExtension = _videosExtension;
        if(_playScheduledVideos == "true") { playScheduledVideos = true; } else { playScheduledVideos = false; }
        if(_isCameraStreamScheduled == "true") { isCameraStreamScheduled = true; } else { isCameraStreamScheduled = false; }
        scheduledVideosTotal = _scheduledVideoPaths.size();
        scheduledVideoPaths = _scheduledVideoPaths;
    }

    void IlicibisPlayer::setup() {
        setupAppFromYamlConfig();
        tcpServer.setup(tcpPort);
        videosDir.open(videosDirPath);
        videosDir.allowExt(videosExtension);
        videosDir.listDir();
        videosDir.sort();
        
        appFont.load("./font/IBMPlexMono-Regular.ttf", 18);
        playerWidth = ofGetWindowWidth();
        playerHeight = ofGetWindowHeight();
        
        ofSetBackgroundColor(0,0,0);
        //system("python3 /Users/m/documents/_DIGITAL/openframeworks112/apps/myApps/ilicibisPlayer3/py_src/ilicibis_cam_stream.py");
        player0.load(getRandomVideoPath());
        player0.setLoopState(OF_LOOP_NONE);
        player0.setVolume(audioVolume);
        player0.setPaused(true);
        
        player1.load(getRandomVideoPath());
        player1.setLoopState(OF_LOOP_NONE);
        player1.setVolume(audioVolume);
        player1.setPaused(true);
        
        playerScheduled.loadAsync(scheduledVideoPaths[nextScheduledVideoIndex]);
        playerScheduled.setLoopState(OF_LOOP_NONE);
        playerScheduled.setVolume(audioVolume);
        playerScheduled.setPaused(true);
        ofLog() << playerName <<  ": press S to start";
    }

    void IlicibisPlayer::start() {
        ofLog() << playerName <<  " is starting...";
        lastCamStreamTime = ofGetElapsedTimef();
        //lastCameraStreamTime = ofGetElapsedTimef() + cameraStreamDelay;
        switch(playerSource)
        {
            case PLAYERZERO:
                ofLog() << playerName <<  " is playing: " << player0.getMoviePath();
                player0.play();
                break;
            case PLAYERONE:
                ofLog() << playerName <<  " is playing: " << player1.getMoviePath();
                player1.play();
                break;
        }
        setPlayerState(RUNNING);
    }

    void IlicibisPlayer::update() {
        
        if(playerState == RUNNING) {
             switch(playerSource)
             {
                 case PLAYERZERO:
                     if(player0.getCurrentFrame() > player0.getTotalNumFrames()-nFramesToSkip && !player0TriggeredNext) {
                        player1.play();
                        ofLog() << playerName <<  " is playing: " << player1.getMoviePath();
                        setPlayerSource(PLAYERONE);
                        setLastPlayerSource(PLAYERONE);
                        player0.closeMovie();
                        player0.load(getRandomVideoPath());
                        player0.setLoopState(OF_LOOP_NONE);
                        player0.setVolume(audioVolume);
                        // skip 2 frames to avoid black background when transitioning to the next video
                        player0.setFrame(2);
                        player0.setPaused(true);
                        // block this code block from running again
                        player0TriggeredNext = true;
                        // unblock code block of the other player
                        if(player1TriggeredNext) {
                            player1TriggeredNext = false;
                        }
                     }
                     player0.update();
                     break;
                 case PLAYERONE:
                    if(player1.getCurrentFrame() > player1.getTotalNumFrames()-nFramesToSkip && !player1TriggeredNext) {
                        player0.play();
                        ofLog() << playerName <<  " is playing: " << player0.getMoviePath();
                        setPlayerSource(PLAYERZERO);
                        setLastPlayerSource(PLAYERZERO);
                        player1.closeMovie();
                        player1.load(getRandomVideoPath());
                        player1.setLoopState(OF_LOOP_NONE);
                        player1.setVolume(audioVolume);
                        // skip 2 frames to avoid black background when transitioning to the next video
                        player1.setFrame(2);
                        player1.setPaused(true);
                        // block this code block from running again
                        player1TriggeredNext = true;
                        // unblock code block of the other player
                        if(player0TriggeredNext) {
                            player0TriggeredNext = false;
                        }
                    }
                    player1.update();
                    break;
                     
                 case CAMSTREAM:
                     if( tcpServer.isClientConnected(0)) {
                         char tcpMesssage[camStreamBytesSize];
                         tcpServer.receiveRawBytes(0, tcpMesssage, camStreamBytesSize);
                         ofBuffer decodedBuffer;
                         decodedBuffer.set(ofxIO::Base64Encoding::decode(tcpMesssage));
                         ofLoadImage(camStreamPixels, decodedBuffer);
                         camStreamTexture.loadData(camStreamPixels);
                     }
                     break;
                 case PLAYERSCHEDULED:
                     if(playScheduledVideos) {
                         if(playerScheduled.getCurrentFrame() > playerScheduled.getTotalNumFrames()-nFramesToSkip && !playerScheduledTriggeredNext) {
                             playerScheduled.closeMovie();
                             // move to the next video in the schedule
                             nextScheduledVideoIndex += 1;
                             int nextIndex = nextScheduledVideoIndex % scheduledVideosTotal;
                             // preload the scheduled video
                             playerScheduled.load(scheduledVideoPaths[nextIndex]);
                             playerScheduled.setLoopState(OF_LOOP_NONE);
                             playerScheduled.setVolume(audioVolume);
                             playerScheduled.setPaused(true);
                             PlayerSources lastSource = getLastPlayerSource();
                             // after the scheduled video is finished
                             // return to the last loaded random video player and play its loaded video
                             if(lastSource == PLAYERZERO) {
                                 player1TriggeredNext = false;
                                 player1.play();
                                 setPlayerSource(PLAYERONE);
                             } else if (lastSource == PLAYERONE) {
                                 player0TriggeredNext = false;
                                 player0.play();
                                 setPlayerSource(PLAYERZERO);
                             }
                         }
                         playerScheduled.update();
                     }
                     break;
                 }
        }
        
        if(isCameraStreamScheduled) {
            float currentTime = ofGetElapsedTimef();
            // start cam stream every camStreamDelay seconds
            if(currentTime >  lastCamStreamTime + camStreamDelay) {
                toggleCameraStream();
                isCamStreamOpen = true;
                lastCamStreamTime = currentTime;
            }
            // end cam stream after camStreamDuration seconds
            if(isCamStreamOpen && currentTime > lastCamStreamTime + camStreamDuration) {
                toggleCameraStream();
                isCamStreamOpen = false;
            }
        }
    }
    
    void IlicibisPlayer::draw() {
        if(playerState == RUNNING) {
            switch(playerSource)
            {
                 case PLAYERZERO:
                    player0.draw(0,0, playerWidth, playerHeight);
                    break;
                 case PLAYERONE:
                     player1.draw(0,0, playerWidth, playerHeight);
                     break;
                    
                case CAMSTREAM:
                    if( tcpServer.isClientConnected(0) && camStreamTexture.isAllocated()) {
                        camStreamTexture.draw(0,0, playerWidth, playerHeight);
                    } else {
                        //ofLogError() << "Cam stream texture is not allocated!";
                    }
                    break;
                 case PLAYERSCHEDULED:
                     playerScheduled.draw(0,0, playerWidth, playerHeight);
                     break;
                 }
        } else {
            appFont.drawString(playerName, 80,80);
            if(isCameraStreamScheduled) {
                appFont.drawString("scheduled mode", 80,115);
            } else {
                appFont.drawString("interactive mode", 80,115);
            }

        }
    }

    void IlicibisPlayer::toggleFullscreen() {
            ofToggleFullscreen();
            playerWidth = ofGetWindowWidth();
            playerHeight = ofGetWindowHeight();
    }

    void IlicibisPlayer::setPlayerSource(PlayerSources newSource) {
        playerSource = newSource;
    }

    void IlicibisPlayer::setLastPlayerSource(PlayerSources source) {
        lastPlayerSource = source;
    }

    void IlicibisPlayer::setPlayerState(PlayerStates newState) {
        playerState = newState;
    }

    IlicibisPlayer::PlayerSources IlicibisPlayer::getLastPlayerSource() {
        return lastPlayerSource;
    }

    IlicibisPlayer::PlayerSources IlicibisPlayer::getCurrentPlayerSource() {
        return playerSource;
    }

    void IlicibisPlayer::triggerScheduledVideo() {
        // pause and close the currently playing random player
        // preload video for the closed random player
        PlayerSources lastSource = getLastPlayerSource();
        if(lastSource == PLAYERZERO) {
            player0.setPaused(true);
            player0.closeMovie();
            player0.loadAsync(getRandomVideoPath());
            player0.setLoopState(OF_LOOP_NONE);
            player0.setVolume(audioVolume);
            player0.setPaused(true);
        } else if(lastSource == PLAYERONE) {
            player1.setPaused(true);
            player1.closeMovie();
            player1.loadAsync(getRandomVideoPath());
            player1.setLoopState(OF_LOOP_NONE);
            player1.setVolume(audioVolume);
            player1.setPaused(true);
        }
        if(playerScheduledTriggeredNext) {
            playerScheduledTriggeredNext = false;
        }
        playerScheduled.play();
        setPlayerSource(PLAYERSCHEDULED);
    }

    void IlicibisPlayer::toggleCameraStream() {
        if(playerState == RUNNING) {
            PlayerSources source = getCurrentPlayerSource();
            if(source == CAMSTREAM) {
                // exit cam stream
                tcpServer.send(0, "stopStream");
                PlayerSources lastSource = getLastPlayerSource();
                if(lastSource == PLAYERONE) {
                    player1.play();
                    setPlayerSource(PLAYERONE);
                    setLastPlayerSource(PLAYERONE);
                } else if(lastSource == PLAYERZERO) {
                    player0.play();
                    setPlayerSource(PLAYERZERO);
                    setLastPlayerSource(PLAYERZERO);
                }
                // erase old pixels from the camera stream pixel buffer
                camStreamTexture.clear();
                camStreamPixels.clear();
            } else {
                // start cam stream
                ofLog() << playerName << " is playing: camera stream";
                tcpServer.send(0, "startStream");
                PlayerSources lastSource = getLastPlayerSource();
                if(lastSource == PLAYERZERO) {
                    player0.setPaused(true);
                    player0.closeMovie();
                    player0.load(getRandomVideoPath());
                    player0.setLoopState(OF_LOOP_NONE);
                    player0.setVolume(audioVolume);
                    player0.setPaused(true);
                } else if(lastSource == PLAYERONE) {
                    player1.setPaused(true);
                    player1.closeMovie();
                    player1.load(getRandomVideoPath());
                    player1.setLoopState(OF_LOOP_NONE);
                    player1.setVolume(audioVolume);
                    player1.setPaused(true);
                }
                setPlayerSource(CAMSTREAM);
            }
        } else {
            ofLogWarning() << "Can't start the camera stream, start the app first with S key";
        }
    }




    void IlicibisPlayer::closeServer() {
        tcpServer.send(0, "exitApp");
        tcpServer.disconnectAllClients();
        tcpServer.close();
    }

    string IlicibisPlayer::getRandomVideoPath()  {
        int randomIndex = (int) std::round(ofRandom(0, videosDir.size()-1));
        string path = videosDir.getPath(randomIndex);
        return path;
    }
    
    

