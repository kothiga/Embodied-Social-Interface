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

import time
import webbrowser

import yarp

import argparse

def getArgs():
    parser = argparse.ArgumentParser(description='yarpWebOpener')
    parser.add_argument('-n', '--name',   default='/webOpener',    help='Name for the module.                  (default: {})'.format('/webOpener'))
    parser.add_argument('-r', '--rate',   type=float, default=0.1, help='Check for new messages per r seconds. (default: {})'.format(0.1))
    parser.add_argument('-s', '--survey', default='https://forms.gle/mqUBCaR5a4PKdXbN7',    help='Survey link')
    args = parser.parse_args()
    return args


# Init yarp network.
yarp.Network.init()


class webOpener(object):
    '''
    Receive requests to open urls.
    '''
    def __init__(self, args):

        # Get some args.
        self.name = args.name
        self.rate = args.rate
        self.link = args.survey
        
        self.url_buffer = []

        # Open the yarp port.
        self.port = yarp.BufferedPortBottle()
        self.port.open(self.name + ":i")

        return


    def run(self):

        while True:
            
            self.checkPort()

            if len(self.url_buffer):

                # Get the url to open.
                url = self.url_buffer[0]
                self.url_buffer.pop(0)

                print("Opening url: {}".format(self.link))
                webbrowser.open_new(self.link)

            else:
                time.sleep(self.rate)

        return

    
    def checkPort(self):
        
        # See if we have mail.
        b = self.port.read(False)
        if b is None:
            return

        # Get the contents of the bottle.
        new_url = b.get(0).toString()

        # Check if we're told to quit.
        if new_url == "exit":
            self.cleanup()
            exit(0)

        # Add this video to the queue.
        self.url_buffer.append(new_url)

        return


    def cleanup(self):
        print("Closing yarp ports.")
        self.port.interrupt()
        self.port.close()
        return


def main():

    # Parse the arguments.
    args = getArgs()

    player = webOpener(args)

    try:
        player.run()
    finally:
        player.cleanup()

    return


if __name__ == '__main__':
    main()
