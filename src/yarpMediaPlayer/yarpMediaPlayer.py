#!/usr/bin/python3

## ================================================================================
## Copyright: (C) 2022, SIRRL Social and Intelligent Robotics Research Laboratory, 
##     University of Waterloo, All rights reserved.
## 
## Authors: 
##     Austin Kothig <austin.kothig@uwaterloo.ca>
## 
## CopyPolicy: Released under the terms of the MIT License. 
##     See the accompanying LICENSE file for details.
## ================================================================================

import numpy as np
import cv2
import time

import yarp

import argparse

def getArgs():
    parser = argparse.ArgumentParser(description='yarpMediaPlayer')
    parser.add_argument('-n', '--name',    default='/mediaPlayer',     help='Name for the module.          (default: {})'.format('/mediaPlayer'))
    parser.add_argument('-x',              default=0,     type=int,    help='Position for window to be.    (default: {})'.format(0))
    parser.add_argument('-y',              default=0,     type=int,    help='Position for window to be.    (default: {})'.format(0))
    parser.add_argument('-H',              default=540,   type=int,    help='Height for the window to be.  (default: {})'.format(540))
    parser.add_argument('-W',              default=960,   type=int,    help='Width for the window to be.   (default: {})'.format(960))
    parser.add_argument('-d', '--default', default=None,               help='Default video to play.        (default: {})'.format(None))
    parser.add_argument('-g', '--goal',    default=None,               help='Goal image to send.           (default: {})'.format(None))
    parser.add_argument('-s', '--speed',   default=10.,   type=float,  help='Speed mutl when jobs todo.    (default: {})'.format(10.))
    parser.add_argument('-b', '--breako',  default=False,              help='Allow ESC and Q to break out? (default: {})'.format(False))
    args = parser.parse_args()
    return args


# Init yarp network.
yarp.Network.init()


class mediaPlayer(object):
    '''
    Receive requests to open videos. Play them through opencv.
    '''
    def __init__(self, args):

        # Get some args.
        self.name       = args.name
        self.default    = args.default
        self.goal       = args.goal
        self.speedup    = args.speed
        self.break_outs = args.breako
        self.xpos       = args.x
        self.ypos       = args.y
        self.height     = args.H
        self.width      = args.W
        
        self.file_buffer = []

        # Open the audio and video streams.
        self.video = cv2.VideoCapture(self.default)
        self.fps = self.video.get(cv2.CAP_PROP_FPS)

        #if self.goal != None:
        #    goal_img = cv2.imread(self.goal)
        #    cv2.namedWindow('goal state')        # Create a named window
        #    cv2.moveWindow('goal state', self.height//3, self.width)  # Move it to (40,30)
        #    cv2.imshow('goal state', goal_img)

        # Init a flag for completion of current media.
        self.playing = True

        # Open the yarp port.
        self.port = yarp.BufferedPortBottle()
        self.port.open(self.name + ":i")

        return


    def run(self):

        while True:
            
            self.checkPort()

            if self.playing:
                
                while self.video.isOpened():
                    
                    ret, frame = self.video.read()

                    if not ret:
                        # End of video.
                        self.playing = False
                        self.video.release()
                        break
                    
                    # Wait based on the frames-per-second of the video,
                    # but if we have multiple videos in the queue, speed
                    # things up so that we're not waiting too for the
                    # most recent video clips.
                    wait_for = int((1000 / self.fps) / (self.speedup if len(self.file_buffer) > 1 else 1.))
                    key = cv2.waitKey(wait_for)
                    
                    # Allow breaking out?
                    if (key == 27 or key == 1048603) and self.break_outs:
                        self.cleanup()
                        exit(0)
                        break
                    
                    # Modify the frame.
                    frame = cv2.resize(frame,
                        (self.width, self.height),
                        fx=0, fy=0,
                        interpolation=cv2.INTER_CUBIC
                    )
                    
                    cv2.imshow(self.name, frame)
                    cv2.moveWindow(self.name, self.xpos, self.ypos)

                    self.checkPort()


            # If there is something in the beffer, load it.
            elif len(self.file_buffer):

                # Get the file at the front.
                file = self.file_buffer[0]
                self.file_buffer.pop(0)

                if file == "none":
                    file = self.default
                    if file == None: continue

                # Load the file.
                print("Loading file: {}".format(file))
                self.video.open(file)

                if not self.video.isOpened():
                    print("Could not open ``{}``!!".format(file))

                else:
                    self.fps = self.video.get(cv2.CAP_PROP_FPS)
                    self.playing = True


            # Else wait for some input.
            else:
                print("Waiting for video . . .")
                time.sleep(0.1)

        return

    
    def checkPort(self):
        
        # See if we have mail.
        b = self.port.read(False)
        if b is None:
            return

        # Get the contents of the bottle.
        new_file = b.get(0).toString()
        if new_file == "none" and new_file in self.file_buffer:
            # Ensure only one copy of "none" in buffer.
            return

        # Check if we're told to quit.
        if new_file == "exit":
            self.cleanup()
            exit(0)

        # Add this video to the queue.
        self.file_buffer.append(new_file)

        return


    def cleanup(self):

        print("Closing media streams.")
        if self.video.isOpened():
            self.video.release()
            cv2.destroyAllWindows()

        print("Closing yarp ports.")
        self.port.interrupt()
        self.port.close()
        return


def main():

    # Parse the arguments.
    args = getArgs()

    player = mediaPlayer(args)

    try:
        player.run()
    finally:
        player.cleanup()

    return


if __name__ == '__main__':
    main()
