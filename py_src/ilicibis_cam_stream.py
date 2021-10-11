import cv2
import mediapipe as mp
import numpy as np
import socket
import base64
import sys
import errno
import glob
import random
import time
 
# config 
camera_cap = cv2.VideoCapture(0)
cam_stream_width = 640 #320
cam_stream_height = 480 #240
segmentation_confidence = 0.2
isStreaming = False
tcp_ip = "127.0.0.1"
tcp_port = 5011

# utils
def getRandomInteger(n):
    return round(random.random() * n)

# preload and process background images
backgrounds = []
for path in glob.glob("./backgrounds/*.jpg"):
    print(path)
    im = cv2.imread(path)
    print(im.shape)    
    if(im.shape[0] > cam_stream_height or im.shape[1] > cam_stream_width or
       im.shape[0] < cam_stream_height or im.shape[1] < cam_stream_width):
        im = cv2.resize(im, (cam_stream_width, cam_stream_height), interpolation=cv2.INTER_AREA)
    im = cv2.cvtColor(im, cv2.COLOR_BGR2RGB)
    backgrounds.append(im)

# set the first background image
background_index = getRandomInteger(len(backgrounds) - 1)
bg_image = backgrounds[background_index]

# make sure that ilicibis player is running
# connect to it's tcp server
sock = socket.socket()
sock.connect((tcp_ip, tcp_port))
sock.setblocking(0)      

mp_selfie_segmentation = mp.solutions.selfie_segmentation
with mp_selfie_segmentation.SelfieSegmentation(model_selection=1) as selfie_segmentation:
    #with mp_face_detection.FaceDetection(min_detection_confidence=0.6) as face_detection:
    if (camera_cap.isOpened()):
        while True:
            if(isStreaming):
                success, im = camera_cap.read()
                if not success:
                    print("Ignoring empty camera frame.")
                    # If loading a video, use 'break' instead of 'continue'.
                    continue
                im = cv2.resize(im, (cam_stream_width, cam_stream_height), interpolation=cv2.INTER_AREA)
                im = cv2.cvtColor(cv2.flip(im, 1), cv2.COLOR_BGR2RGB)
                im.flags.writeable = False
                segmentation_results = selfie_segmentation.process(im)
                mask = np.stack((segmentation_results.segmentation_mask,) * 3, axis=-1) > segmentation_confidence
                segmented_image = np.where(mask, im, bg_image)
                output_image = cv2.cvtColor(segmented_image, cv2.COLOR_BGR2RGB)
                #cv2.imshow('MediaPipe Selfie Segmentation', output_image)
                encoded, buffer = cv2.imencode('.jpg', output_image, [cv2.IMWRITE_JPEG_QUALITY, 80])
                message = base64.b64encode(buffer)

            # wait for a non blocking message from the ilicibis player
            try:
                if isStreaming:
                    sock.sendall(message)
                message = sock.recv(1024)
            except socket.error as e:
                err = e.args[0]
                if err == errno.EAGAIN or err == errno.EWOULDBLOCK:
                    continue
                else:
                    # a "real" error occurred
                    print(e)
                    sys.exit(1)
            else:
                message = message.decode()
                message = message.replace('[/TCP]\x00','')
                if(message == "startStream"):
                    print("start stream!");
                    isStreaming = True
                elif(message == "stopStream"):
                    print("stop stream!");                    
                    isStreaming = False
                    background_index = getRandomInteger(len(backgrounds) - 1)
                    bg_image = backgrounds[background_index]
                    segmentation_results = None
                elif(message == "exitApp"):
                    sys.exit(1)
            if not isStreaming:
                time.sleep(1)


            #if cv2.waitKey(5) & 0xFF == 27:
            #    break
    camera_cap.release()


