#!/usr/local/bin/python2.7
import numpy as np
import argparse
import cv


def channel_processing(channel):
    pass
    cv.AdaptiveThreshold(channel, channel, 255, adaptive_method=cv.CV_ADAPTIVE_THRESH_MEAN_C, thresholdType=cv.CV_THRESH_BINARY, blockSize=55, param1=7)
    #mop up the dirt
    cv.Dilate(channel, channel, None, 1)
    cv.Erode(channel, channel, None, 1)

def inter_centre_distance(x1,y1,x2,y2):
    return ((x1-x2)**2 + (y1-y2)**2)**0.5

def colliding_circles(circles):
    for index1, circle1 in enumerate(circles):
        for circle2 in circles[index1+1:]:
            x1, y1, Radius1 = circle1[0]
            x2, y2, Radius2 = circle2[0]
            #collision or containment:
            if inter_centre_distance(x1,y1,x2,y2) < Radius1 + Radius2:
                return True

def find_circles(processed, storage, LOW):
    try:
        print("YO")
        cv.HoughCircles(processed, storage, cv.CV_HOUGH_GRADIENT, 2, 32.0, 30, LOW)#, 0, 100) great to add circle constraint sizes.
    except:
        LOW += 1
        print 'try'
        find_circles(processed, storage, LOW)
    circles = np.asarray(storage)
    print 'number of circles:', len(circles)
    if colliding_circles(circles):
        LOW += 1
        storage = find_circles(processed, storage, LOW)
    print 'c', LOW
    return storage

def draw_circles(storage, output):
    circles = np.asarray(storage)
    print len(circles), 'circles found'
    for circle in circles:
        Radius, x, y = int(circle[0][2]), int(circle[0][0]), int(circle[0][1])
        print(Radius, x, y)
        cv.Circle(output, (x, y), 1, cv.CV_RGB(0, 255, 0), -1, 8, 0)
        cv.Circle(output, (x, y), Radius, cv.CV_RGB(255, 0, 0), 3, 8, 0)

def get_circles(storage):
    circles = np.asarray(storage)
    ret = []
    for circle in circles:
        Radius, x, y = int(circle[0][2]), int(circle[0][0]), int(circle[0][1])
        print(Radius, x, y)
        ret.append(((Radius, x, y)))
    return ret

def process_image(filename):
    print(filename)
    orig = cv.LoadImage(filename)
    output = cv.LoadImage(filename)
    print("Not filename error")
    # create tmp images
    rrr=cv.CreateImage((orig.width,orig.height), cv.IPL_DEPTH_8U, 1)
    ggg=cv.CreateImage((orig.width,orig.height), cv.IPL_DEPTH_8U, 1)
    bbb=cv.CreateImage((orig.width,orig.height), cv.IPL_DEPTH_8U, 1)
    processed = cv.CreateImage((orig.width,orig.height), cv.IPL_DEPTH_8U, 1)
    storage = cv.CreateMat(orig.width, 1, cv.CV_32FC3)
    print("Checkpoint 1", storage)
    #split image into RGB components
    cv.Split(orig,rrr,ggg,bbb,None)
    #process each component
    channel_processing(rrr)
    channel_processing(ggg)
    channel_processing(bbb)
    #combine images using logical 'And' to avoid saturation
    cv.And(rrr, ggg, rrr)
    cv.And(rrr, bbb, processed)
    #cv.ShowImage('before canny', processed)
    #cv.SaveImage('case3_processed.jpg',processed)
    #use canny, as HoughCircles seems to prefer ring like circles to filled ones.
    cv.Canny(processed, processed, 5, 70, 3)
    #smooth to reduce noise a bit more
    cv.Smooth(processed, processed, cv.CV_GAUSSIAN, 7, 7)
    print("Checkpoint 2")
    #cv.ShowImage('processed', processed)
    #find circles, with parameter search
    storage = find_circles(processed, storage, 100)
    print("Checkpoint 3")
    cir     = get_circles(storage)

    if(len(cir) == 1):
        draw_circles(storage, output)
        cv.ShowImage("original with circles", output)
        cv.SaveImage('output_'+filename,output)
        cv.WaitKey(0)
        return cir[0]
    # show images
print("This is what is returned: (%s,%s,%s)" %process_image("cnc_Cal_2_0.jpg"))