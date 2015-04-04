//
//  Cube.cpp
//  Relief2
//
//  Created by Daniel Windham on 3/25/15.
//
//

#include "Cube.h"

//           w                 w    .*1
//  0+---------------+1         . ' | \                        ^ -y
//   |               |      0*'     |  \   . '                 |
// h |       +       |        \     |  .\'   )theta     -x <---+---> +x
//   |       ^center |       --\----+'---\----                 |
//  3+---------------+2       h \   |     \                    v +y
//
// all cube distances are in fractions of a containing unit-square; the image
// frame the cube comes from is interpreted as having units width = height = 1


// Constructors

Cube::Cube() : candidateUpdates(*new CubeUpdatesBuffer) {
    candidateUpdates.blob = NULL;
    candidateUpdates.hasMarker = false;
}

Cube::Cube(Blob *_blob, bool _update) : candidateUpdates(*new CubeUpdatesBuffer) {
    candidateUpdates.hasMarker = false;
    setBlob(_blob, _update);
}

Cube::Cube(Blob *_blob, ofPoint _marker, bool _update) : candidateUpdates(*new CubeUpdatesBuffer) {
    setBlobAndMarker(_blob, _marker, _update);
}

Cube::~Cube() {
    candidateUpdates.~CubeUpdatesBuffer();
}

bool Cube::isValid() {
    return blob != NULL;
}


// Setters

void Cube::setBlob(Blob *_blob, bool _update) {
    candidateUpdates.blob = _blob;
    if (_update) {
        update();
    }
}

void Cube::setMarker(ofPoint _marker, bool _update) {
    candidateUpdates.rawMarker = _marker;
    candidateUpdates.hasMarker = true;
    if (_update) {
        update();
    }
}

void Cube::setBlobAndMarker(Blob *_blob, ofPoint _marker, bool _update) {
    candidateUpdates.blob = _blob;
    candidateUpdates.rawMarker = _marker;
    candidateUpdates.hasMarker = true;
    if (_update) {
        update();
    }
}

void Cube::clearMarker(bool _update) {
    candidateUpdates.hasMarker = false;
    if (_update) {
        update();
    }
}


// Update

void Cube::calculateCandidateUpdates() {
    // locally use a shorthand alias for the candidate updates object
    CubeUpdatesBuffer &cand = candidateUpdates;

    // a blob's angleBoundingRect height and width variables are flipped. furthermore, blob
    // units are scaled by the size of the image they were found in. fix these mistakes.
    cand.normalizationVector.set(1.0 / cand.blob->widthScale, 1.0 / cand.blob->heightScale);
    cand.width = cand.blob->angleBoundingRect.height * cand.normalizationVector.x;
    cand.height = cand.blob->angleBoundingRect.width * cand.normalizationVector.y;
    cand.center.set(cand.blob->angleBoundingRect.x, cand.blob->angleBoundingRect.y);
    cand.center *= cand.normalizationVector;

    // normalized marker position relative to center
    if (cand.hasMarker) {
        cand.marker = cand.rawMarker * cand.normalizationVector - cand.center;
    }

    // range is 0 <= rawTheta < 90; raw theta does not take cube orientation into account
    cand.rawTheta = -cand.blob->angle;
    cand.rawThetaRadians = cand.rawTheta * pi / 180;

    // relative corner coordinates using raw theta value
    cand.rawCorners[0].x = -cand.width/2 * cos(cand.rawThetaRadians) - cand.height/2 * sin(cand.rawThetaRadians);
    cand.rawCorners[0].y = cand.width/2 * sin(cand.rawThetaRadians) - cand.height/2 * cos(cand.rawThetaRadians);
    cand.rawCorners[1].x = cand.width/2 * cos(cand.rawThetaRadians) - cand.height/2 * sin(cand.rawThetaRadians);
    cand.rawCorners[1].y = -cand.width/2 * sin(cand.rawThetaRadians) - cand.height/2 * cos(cand.rawThetaRadians);
    cand.rawCorners[2].x = -cand.rawCorners[0].x;
    cand.rawCorners[2].y = -cand.rawCorners[0].y;
    cand.rawCorners[3].x = -cand.rawCorners[1].x;
    cand.rawCorners[3].y = -cand.rawCorners[1].y;

    // if the cube is marked, use this to determine its orientation. cornerA becomes corners[0], cornerB
    // becomes corners[1], and the rest follow. if there is no marker, leave corners as they are.
    int cornerA = 0;
    int cornerB = 1;
    if (cand.hasMarker) {
        // determine which two cube corners the marker is closest to
        vector<pair<float, int> > distances; // (distance, index) tuple vector
        for (int i = 0; i < 4; i++) {
            float distance = cand.marker.squareDistance(cand.rawCorners[i]);
            distances.push_back(pair<float, int>(distance, i));
        }

        // sort by distance value, then extract matching indices
        sort(distances.begin(), distances.end());
        cornerA = min(distances[0].second, distances[1].second);
        cornerB = max(distances[0].second, distances[1].second);
        if (cornerA == 0 && cornerB == 3) {
            cornerA = 3;
            cornerB = 0;
        }
    }

    // adjust the cube angle appropriately
    cand.theta = cand.rawTheta + (360 - 90 * cornerA) % 360;
    cand.thetaRadians = cand.theta * pi / 180;

    // relative corner coordinates, determined by cycling indices of raw corners
    for (int i = 0; i < 4; i++) {
        cand.corners[i] = cand.rawCorners[(i + cornerA) % 4];
    }
}

void Cube::update() {
    if (!candidateUpdates.blob) {
        return;
    }

    // to filter out image noise, only update cube values when the blob changes substantially.
    // therefore, calculate updates into a candidate buffer and only propagating them if their
    // difference compared to current values passes a hysteresis threshold
    calculateCandidateUpdates();

    // locally use a shorthand alias for the candidate updates object
    CubeUpdatesBuffer &cand = candidateUpdates;

    blob = cand.blob;
    hasMarker = cand.hasMarker;
    normalizationVector = cand.normalizationVector;
    width = cand.width;
    height = cand.height;
    center = cand.center;
    marker = cand.marker;
    theta = cand.theta;
    thetaRadians = cand.thetaRadians;
    for (int i = 0; i < 4; i++) {
        corners[i] = cand.corners[i];
    }

    // derive additional values for convenience

    // absolute corner coordinates
    absCorners[0] = center + corners[0];
    absCorners[1] = center + corners[1];
    absCorners[2] = center + corners[2];
    absCorners[3] = center + corners[3];

    // cube boundary descriptors
    minX = absCorners[0].x;
    maxX = absCorners[2].x;
    minY = absCorners[1].y;
    maxY = absCorners[3].y;
}

Blob * Cube::getCandidateBlob() {
    return candidateUpdates.blob;
}