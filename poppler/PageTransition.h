#ifndef PAGE_TRANSITION_H
#define PAGE_TRANSITION_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"

//------------------------------------------------------------------------
// PageTransition
//------------------------------------------------------------------------

// if changed remember to keep in sync with frontend enums
enum PageTransitionType {
  transitionReplace = 0,
  transitionSplit,
  transitionBlinds,
  transitionBox,
  transitionWipe,
  transitionDissolve,
  transitionGlitter,
  transitionFly,
  transitionPush,
  transitionCover,
  transitionUncover,
  transitionFade
};

// if changed remember to keep in sync with frontend enums
enum PageTransitionAlignment {
  transitionHorizontal = 0,
  transitionVertical
};

// if changed remember to keep in sync with frontend enums
enum PageTransitionDirection {
  transitionInward = 0,
  transitionOutward
};

class PageTransition {
public:
  // Construct a Page Transition.
  PageTransition (Object *trans);

  // Destructor.
  ~PageTransition ();

  // Was the Page Transition created successfully?
  GBool isOk() { return ok; }

  // Get type
  PageTransitionType getType() { return type; }

  // Get duration
  int getDuration() { return duration;}

  // Get alignment
  PageTransitionAlignment getAlignment() { return alignment; }

  // Get direction
  PageTransitionDirection getDirection() { return direction; }

  // Get angle
  int getAngle() { return angle; }

  // Get scale
  double getScale() { return scale; }

  // Is rectangular?
  GBool isRectangular() { return rectangular; }

private:
  
  PageTransitionType type;           // transition style
  int duration;                      // duration of the effect in seconds
  PageTransitionAlignment alignment; // dimension of the effect
  PageTransitionDirection direction; // direction of motion
  int angle;                         // direction in degrees
  double scale;                      // scale
  GBool rectangular;                 // is the area to be flown in rectangular?
  GBool ok;                          // set if created successfully
};

#endif /* PAGE_TRANSITION_H */
