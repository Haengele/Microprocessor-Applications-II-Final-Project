# echo-server.py

import socket
import cv2
import sys
import time

HOST = "10.189.114.121"  # Standard loopback interface address (localhost)
PORT = 5555  # Port to listen on (non-privileged ports are > 1023)


if __name__=="__main__":
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()
        conn, addr = s.accept()
        with conn:
            print(f"Connected by {addr}")

            cv2.namedWindow("preview")
            vc = cv2.VideoCapture(0)

            if vc.isOpened(): # try to get the first frame
                rval, frame = vc.read()
            else:
                rval = False


            while True:
                #data = conn.recv(1024)
                gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
                scale_percent = 20 # percent of original size
                #width = int(gray.shape[1] * scale_percent / 100)
                #height = int(gray.shape[0] * scale_percent / 100)
                dim = (240, 280)

                # resize image
                resized = cv2.resize(frame[0:480,100:520], dim, interpolation = cv2.INTER_AREA)
                data = resized
                cv2.imshow("preview", resized)
                rval, frame = vc.read()
                
                
                key = cv2.waitKey(20)
                if key == 27: # exit on ESC
                    break
                elif key == ord(' '):
                    for row in data:
                     conn.sendall(row)

                
                print(data.shape)

                #conn.sendall(data)
                #time.sleep(1/20)
                #conn.recv(3)

    vc.release()
    cv2.destroyWindow("preview")


                