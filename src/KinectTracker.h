//
//  KinectTracker.h
//  Relief2
//
//  Created by Sean Follmer on 3/24/13.
//
//

#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxKCore.h"
#include "Constants.h"
#include "Cube.h"


#ifndef __Relief2__KinectTracker__
#define __Relief2__KinectTracker__

#include <iostream>


class ColorBand {
public:
    ColorBand(int _hueTarget, int _hueTolerance, int _saturationThreshold) {
        hueTarget = min(_hueTarget, 255);
        hueTolerance = min(_hueTolerance, 128);
        saturationThreshold = min(_saturationThreshold, 255);
    };

    int hueTarget;
    int hueTolerance;
    int saturationThreshold;
};


class KinectTracker {
public:
    ofxKinect kinect;
    
    void setup();
    void exit();
    void draw(int x, int y, int width, int height, int probe_x=NULL, int probe_y=NULL);
    void drawColorImage(int x, int y, int width, int height);
    void drawDepthImage(int x, int y, int width, int height);
    void drawDetectedObjects(int x, int y, int width, int height);
    void drawDepthThresholdedColorImage(int x, int y, int width, int height);
    void drawCornerLikelihoods(int x, int y, int width, int height);
    void update();
    
    void findBlobs(ColorBand blobColor, vector<Blob>& blobs);
    void findFingers(vector<ofPoint>& points);
    void findFingersAboveSurface(vector<ofPoint>& points);

    void saveDepthImage();
    void loadDepthBackground();

    vector<Blob>  redBlobs;                     // red blobs detected
    vector<Cube>  redCubes;                     // cube objects using red blobs
    vector<ofPoint>  fingers;                   // fingers detected (z is relative above height map)
    vector<ofPoint>  absFingers;                // fingers detected (z is absolute)

    ofPoint src[4], dst[4];

    int frameWidth = 190;
    int frameHeight = 190;

    ofxCvColorImage colorImgRaw;                // color straight from kinect
    ofxCvColorImage colorImg;                   // color restricted to inFORM ROI
    ofxCvColorImage depthThresholdC;            // depth threshold as a color image
    ofxCvColorImage depthThresholdDilatedC;     // dilated depth threshold as a color image
    ofxCvColorImage dThresholdedColor;          // depth-thresholded color
    ofxCvColorImage dThresholdedColorDilated;   // dilated depth-thresholded color

    ofxCvGrayscaleImage depthImgRaw;            // depth straight from kinect
    ofxCvGrayscaleImage depthImg;               // depth restricted to inFORM ROI
    ofxCvGrayscaleImage depthNearThreshold;     // helper for removing depths that are too close
    ofxCvGrayscaleImage depthThreshold;         // threshold rejecting pixels of uninteresting depth
    ofxCvGrayscaleImage depthThresholdDilated;  // dilated depth threshold
    ofxCvGrayscaleImage dThresholdedColorDilatedG; // depth-thresholded color as a grayscale image
    ofxCvGrayscaleImage depthBG;                // used by finger tracking
    ofxCvGrayscaleImage depthBGPlusSurface;     // used by finger tracking
    ofxCvGrayscaleImage depthFiltered;          // used by finger tracking
    ofxCvGrayscaleImage cornerLikelihoods;      // map of each pixel's probability of being a corner

    ofxCvFloatImage depthThresholdF;            // depth threshold as a float image
    ofxCvFloatImage cornerLikelihoodsRawF;      // pre-normalization corner likelihoods map

    ofImage depthDisplayImage;
    ofFbo detectedObjectsDisplayFbo;
    ofImage cornerLikelihoodsDisplayImage;

    vector<ofPoint> corners;

    // blob tracking output
    Blob currentBlob;
    Cube currentCube;
    bool cubeIsSquareAligned;

    // blob tracking images
    ofxCvColorImage hsvImage;                   // input image converted to hsv
    ofxCvGrayscaleImage hue;                    // hue component
    ofxCvGrayscaleImage sat;                    // saturation component
    ofxCvGrayscaleImage bri;                    // brilliance component
    ofxCvGrayscaleImage hueThreshHigh;          // hue high boundary threshold
    ofxCvGrayscaleImage hueThreshLow;           // hue low boundary threshold
    ofxCvGrayscaleImage hueThresh;              // hue band threshold
    ofxCvGrayscaleImage satThresh;              // saturation threshold
    ofxCvGrayscaleImage hueSatThresh;           // combined hue and saturation threshold

    // tracking objects
    ContourFinder finger_contourFinder;
    ContourFinder ball_contourFinder;
    BlobTracker finger_tracker;
    BlobTracker ball_tracker;
    Calibration calib;
    
    ofTrueTypeFont verdana;
    
private:
    void updateInputImages();
    void updateDepthThresholds();
    void generateBlobDescriptors(vector<Cube> cubes);
    void detectCorners(ofxCvGrayscaleImage &imageIn, vector<ofPoint>& cornersOut);
    void updateDetectionStatus();

};

#endif /* defined(__Relief2__KinectTracker__) */
