import socket
import cv2
import pickle
import struct
import serial
import socket
import numpy as np

from struct import *

from face_detect import face_detect

HOST = "10.189.114.121"  # The server's hostname or IP address
PORT = 5555  # The port used by the server

if __name__=="__main__":
    # your code here# echo-client.py

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        #s.sendall(b"Hello, world")
        #faceCascade = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')
            #ser = serial.Serial(port='/dev/ttyS4', baudrate=115200, timeout=1,parity=serial.PARITY_NONE,stopbits=serial.STOPBITS_ONE,bytesize=serial.EIGHTBITS)
        with serial.Serial(port='/dev/ttyS4', baudrate=1500000, timeout=1,parity=serial.PARITY_NONE,stopbits=serial.STOPBITS_ONE,bytesize=serial.EIGHTBITS) as ser:
            while(True):
                data = bytearray(s.recv(240*3))
                for i in range(0,279):
                    data += bytearray(s.recv(240*3))
                #s.sendall("REC")
                print(len(data))
                nparr = np.fromstring(bytes(data), np.uint8)
                while(len(nparr) < 280*240*3):
                    nparr = np.append(nparr, [1,1,1,1,1,1,1,1,1,1])
                    print(len(nparr))
                print(type(nparr))
                img_np = nparr.reshape((280,240,3) )#, order='F')
                #print(type(img_np))
                #faces = faceCascade.detectMultiScale(img_np, scaleFactor=1.1, minNeighbors=3, minSize=(15, 15), flags = cv2.CASCADE_SCALE_IMAGE)
                #cv2.imwrite("image.jpg", img_np)
                i = 0
                j = 0
                rgb = [0,0,0]
                for pix in nparr:
                    rgb[i] = pix
                    i+=1
                    if(i == 3 and j < len(nparr)-1):
                        msg = pack("<BBB",rgb[0], rgb[1], rgb[2])
                    
                        ser.write(msg)
                        i=0
                    j+=1
                print(rgb)
                print("Finished")    
                for i in range(0, 850):
                    msg = pack("<BBB",0,0,0)
                    ser.write(msg)
                    
                while(True):
                        
                    x1 = int.from_bytes(ser.read(1), byteorder='little')
                    while(x1 == 0):
                        x1 = int.from_bytes(ser.read(1), byteorder='little')
                    x1 -=1
                    #x1 = int(x1)
                    x2 = int.from_bytes(ser.read(1), byteorder='little')
                    y1 = int.from_bytes(ser.read(1), byteorder='little')
                    y2 = int.from_bytes(ser.read(1), byteorder='little')
                    
                    x = (x1 << 8) + x2
                    y = (y1 << 8) + y2
                    """print("X1: " + str(x1))
                    print("X2: " + str(x2))
                    print("Y1: " + str(y1))
                    print("Y2: " + str(y2))
                    print()
                    print("X: " + str(x))
                    print("Y: " + str(y))"""
                    
                    if(x == 255 and y == 255):
                        break
                    msg = pack("<BBB",img_np[y][x][0], img_np[y][x][1], img_np[y][x][2])
                    ser.write(msg)
                    
                    
                   
            #ser.write(-1)
                
            #print(data)