import cv2
import mediapipe as mp
import numpy as np
import socket
import time
import imutils
import base64
import sys
import errno
import os
import random
 
def getRandomIndex(n):
    return round(random.random() * n)

isStreaming = False

mp_selfie_segmentation = mp.solutions.selfie_segmentation
cap = cv2.VideoCapture(0)
# W = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
# H = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)

NEW_W = 640
NEW_H = 480

backgrounds = []
backgrounds.append(cv2.imread("./backgrounds/1.jpg"))
backgrounds.append(cv2.imread("./backgrounds/2.jpg"))
backgrounds.append(cv2.imread("./backgrounds/3.jpg"))
backgrounds[0] = cv2.cvtColor(backgrounds[0], cv2.COLOR_BGR2RGB)
backgrounds[1] = cv2.cvtColor(backgrounds[1], cv2.COLOR_BGR2RGB)
backgrounds[2] = cv2.cvtColor(backgrounds[2], cv2.COLOR_BGR2RGB)
background_index = getRandomIndex(len(backgrounds) - 1)

TCP_IP = "127.0.0.1"
TCP_PORT = 5011

sock = socket.socket()
sock.connect((TCP_IP, TCP_PORT))
sock.setblocking(0)      



with mp_selfie_segmentation.SelfieSegmentation(model_selection=1) as selfie_segmentation:
    #with mp_face_detection.FaceDetection(min_detection_confidence=0.6) as face_detection:
    bg_image = None
    if (cap.isOpened()):
        while True:
            if(isStreaming):
                success, image = cap.read()
                if not success:
                    print("Ignoring empty camera frame.")
                    # If loading a video, use 'break' instead of 'continue'.
                    continue

                image = cv2.resize(image, (NEW_W, NEW_H), interpolation=cv2.INTER_AREA)

                # Flip the image horizontally for a later selfie-view display, and convert
                # the BGR image to RGB.
                image = cv2.cvtColor(cv2.flip(image, 1), cv2.COLOR_BGR2RGB)
                # To improve performance, optionally mark the image as not writeable to
                # pass by reference.
                image.flags.writeable = False
                segmentation_results = selfie_segmentation.process(image)

                # mask = segmentation_results.segmentation_mask
                mask = np.stack((segmentation_results.segmentation_mask,) * 3, axis=-1) > 0.25

                segmented_image = np.where(mask, image, backgrounds[background_index])
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
                    background_index = getRandomIndex(len(backgrounds) - 1)
                    isStreaming = True
                elif(message == "stopStream"):
                    print("stop stream!");                    
                    isStreaming = False
                elif(message == "exitApp"):
                    sys.exit(1)

            if cv2.waitKey(5) & 0xFF == 27:
                break
    cap.release()


