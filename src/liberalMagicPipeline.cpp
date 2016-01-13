//
//  liberalMagicPipeline.cpp
//  PolyMouse
//
//  Created by Tristan Hume on 2016-01-07.
//
//

#include "liberalMagicPipeline.h"
#include "ofxRemoteUIServer.h"
#include "ofGraphics.h"

static const int kLiberalMagicMinimumJump = 100;
static const float kSpeedExpandFactor = 10000.0;
static const float kSmallJumpFactor = 0.75;
static const float kHeadSmoothingFactor = 0.05;

liberalMagicPipeline::liberalMagicPipeline(inputProcess<ofVec2f> *gaze, inputProcess<ofVec2f> *head)
  : rawGaze(gaze), headInp(head, "head velocity"), speedExpandFactor(kSpeedExpandFactor),
  gazeInp(gaze, "fixation detection"), minJump(kLiberalMagicMinimumJump),
  smoothedHeadVel(0.0), headSmoothingFactor(kHeadSmoothingFactor) {}

void liberalMagicPipeline::setup() {
  gazeInp.setup();
  headInp.setup();
  RUI_NEW_GROUP("Liberal MAGIC");
  RUI_SHARE_PARAM(minJump, 0, 800);
  RUI_SHARE_PARAM(speedExpandFactor, 0, 50000);
  RUI_SHARE_PARAM(headSmoothingFactor, 0, 1);
}

void liberalMagicPipeline::update() {
  gazeInp.update();
  headInp.update();

  // teleport to gaze if gaze is too far away
  // Note: cursor must also be away from the last teleport location
  smoothedHeadVel = smoothedHeadVel*(1-headSmoothingFactor) +
                    headInp.rawVel.length()*headSmoothingFactor;
  float jumpRadius = calcJumpRadius();
  float smallJump = jumpRadius*kSmallJumpFactor;
  if(val.distance(gazeInp.val) > jumpRadius &&
     lastJumpDestination.distance(gazeInp.val) > smallJump) {
    val = gazeInp.val;
    lastJumpDestination = gazeInp.val;
  }

  val += headInp.val; // use relative motion from head
}

void liberalMagicPipeline::draw() {
  gazeInp.draw();

  ofSetColor(255,0,255);
  ofNoFill();
  float jumpRadius = calcJumpRadius();
  ofDrawCircle(lastJumpDestination.x, lastJumpDestination.y, jumpRadius*kSmallJumpFactor);
  ofDrawCircle(val.x, val.y, jumpRadius);
  ofFill();
}

float liberalMagicPipeline::calcJumpRadius() const {
  return minJump + smoothedHeadVel*speedExpandFactor;
}
