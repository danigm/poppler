//*********************************************************************************
//                               Movie.cc
//---------------------------------------------------------------------------------
// 
//---------------------------------------------------------------------------------
// Hugo Mercier <hmercier31[at]gmail.com> (c) 2008
// Pino Toscano <pino@kde.org> (c) 2008
// Carlos Garcia Campos <carlosgc@gnome.org> (c) 2010
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//*********************************************************************************

#include <math.h>
#include "Movie.h"
#include "FileSpec.h"

#include <GooList.h>

MovieWindowParameters::MovieWindowParameters() {
  // default values
  type = movieWindowEmbedded;
  width = -1;
  height = -1;
  relativeTo = windowRelativeToDocument;
  XPosition = 0.5;
  YPosition = 0.5;
  hasTitleBar = gTrue;
  hasCloseButton = gTrue;
  isResizeable = gTrue;
}

MovieWindowParameters::~MovieWindowParameters() {
}

void MovieWindowParameters::parseFWParams(Object* obj) {
  Object tmp;

  if (obj->dictLookup("D", &tmp)->isArray()) {
    Array * dim = tmp.getArray();
    
    if (dim->getLength() >= 2) {
      Object dd;
      if (dim->get(0, &dd)->isInt()) {
	width = dd.getInt();
      }
      dd.free();
      if (dim->get(1, &dd)->isInt()) {
	height = dd.getInt();
      }
      dd.free();
    }
  }
  tmp.free();

  if (obj->dictLookup("RT", &tmp)->isInt()) {
    int t = tmp.getInt();
    switch(t) {
    case 0: relativeTo = windowRelativeToDocument; break;
    case 1: relativeTo = windowRelativeToApplication; break;
    case 2: relativeTo = windowRelativeToDesktop; break;
    }
  }
  tmp.free();

  if (obj->dictLookup("P",&tmp)->isInt()) {
    int t = tmp.getInt();

    switch(t) {
    case 0: // Upper left
      XPosition = 0.0;
      YPosition = 0.0;
      break;
    case 1: // Upper Center
      XPosition = 0.5;
      YPosition = 0.0;
      break;
    case 2: // Upper Right
      XPosition = 1.0;
      YPosition = 0.0;
      break;
    case 3: // Center Left
      XPosition = 0.0;
      YPosition = 0.5;
      break;
    case 4: // Center
      XPosition = 0.5;
      YPosition = 0.5;
      break;
    case 5: // Center Right
      XPosition = 1.0;
      YPosition = 0.5;
      break;
    case 6: // Lower Left
      XPosition = 0.0;
      YPosition = 1.0;
      break;
    case 7: // Lower Center
      XPosition = 0.5;
      YPosition = 1.0;
      break;
    case 8: // Lower Right
      XPosition = 1.0;
      YPosition = 1.0;
      break;
    }
  }
  tmp.free();

  if (obj->dictLookup("T", &tmp)->isBool()) {
    hasTitleBar = tmp.getBool();
  }
  tmp.free();
  if (obj->dictLookup("UC", &tmp)->isBool()) {
    hasCloseButton = tmp.getBool();
  }
  tmp.free();
  if (obj->dictLookup("R", &tmp)->isInt()) {
    isResizeable = (tmp.getInt() != 0);
  }
  tmp.free();

}

MovieParameters::MovieParameters() {
  // instanciate to default values

  rotationAngle = 0;
  rate = 1.0;
  volume = 100;
  fittingPolicy = fittingUndefined;
  autoPlay = gTrue;
  repeatCount = 1.0;
  opacity = 1.0;
  showControls = gFalse;
  synchronousPlay = gFalse;
  repeatMode = repeatModeOnce;
  
  start.units = 0;
  duration.units = 0;  
}

MovieParameters::~MovieParameters() {
}

void MovieParameters::parseMovieActivation(Object* aDict,
					   int width, int height,
					   int rotationAngleA) {
  Object obj1;

  if (!aDict->dictLookup("Start", &obj1)->isNull()) {
    if (obj1.isInt()) {
      // If it is representable as an integer (subject to the implementation limit for
      // integers, as described in Appendix C), it should be specified as such.

      start.units = obj1.getInt();
    }
    if (obj1.isString()) {
      // If it is not representable as an integer, it should be specified as an 8-byte
      // string representing a 64-bit twos-complement integer, most significant
      // byte first.

      // UNSUPPORTED
    }

    if (obj1.isArray()) {
      Array* a = obj1.getArray();

      Object tmp;
      a->get(0, &tmp);
      if (tmp.isInt()) {
        start.units = tmp.getInt();
      }
      if (tmp.isString()) {
        // UNSUPPORTED
      }
      tmp.free();

      a->get(1, &tmp);
      if (tmp.isInt()) {
        start.units_per_second = tmp.getInt();
      }
      tmp.free();
    }
  }
  obj1.free();

  if (!aDict->dictLookup("Duration", &obj1)->isNull()) {
    if (obj1.isInt()) {
      duration.units = obj1.getInt();
    }
    if (obj1.isString()) {
      // UNSUPPORTED
    }

    if (obj1.isArray()) {
      Array* a = obj1.getArray();

      Object tmp;
      a->get(0, &tmp);
      if (tmp.isInt()) {
        duration.units = tmp.getInt();
      }
      if (tmp.isString()) {
        // UNSUPPORTED
      }
      tmp.free();

      a->get(1, &tmp);
      if (tmp.isInt()) {
        duration.units_per_second = tmp.getInt();
      }
      tmp.free();
    }
  }
  obj1.free();

  if (aDict->dictLookup("Rate", &obj1)->isNum()) {
    rate = obj1.getNum();
  }
  obj1.free();

  rotationAngle = rotationAngleA;

  if (aDict->dictLookup("Volume", &obj1)->isNum()) {
    // convert volume to [0 100]
    volume = int((obj1.getNum() + 1.0) * 50);
  }
  obj1.free();

  if (aDict->dictLookup("ShowControls", &obj1)->isBool()) {
    showControls = obj1.getBool();
  }
  obj1.free();

  if (aDict->dictLookup("Synchronous", &obj1)->isBool()) {
    synchronousPlay = obj1.getBool();
  }
  obj1.free();

  if (aDict->dictLookup("Mode", &obj1)->isName()) {
    char* name = obj1.getName();
    if (!strcmp(name, "Once")) {
      repeatMode = repeatModeOnce;
    } else if (!strcmp(name, "Open")) {
      repeatMode = repeatModeOpen;
    } else if (!strcmp(name, "Repeat")) {
      repeatMode = repeatModeRepeat;
      repeatCount = 0.0;
    } else if (!strcmp(name,"Palindrome")) {
      repeatMode = repeatModePalindrome;
    }
  }
  obj1.free();

  windowParams.relativeTo = MovieWindowParameters::windowRelativeToDesktop;

  int znum = 1, zdenum = 1;
  if (aDict->dictLookup("FWScale", &obj1)->isArray()) {
    // the presence of that entry implies that the movie is to be played
    // in a floating window
    windowParams.type = MovieWindowParameters::movieWindowFloating;

    Array* scale = obj1.getArray();
    if (scale->getLength() >= 2) {
      Object tmp;
      if (scale->get(0, &tmp)->isInt()) {
        znum = tmp.getInt();
      }
      tmp.free();
      if (scale->get(1, &tmp)->isInt()) {
        zdenum = tmp.getInt();
      }
      tmp.free();
    }

    // detect fullscreen mode
    if (znum == 999 && zdenum == 1) {
      windowParams.type = MovieWindowParameters::movieWindowFullscreen;
    }
  }
  obj1.free();

  windowParams.width = int(width * double(znum) / zdenum);
  windowParams.height = int(height * double(znum) / zdenum);

  if (aDict->dictLookup("FWPosition", &obj1)->isArray()) {
    Array* pos = obj1.getArray();
    if (pos->getLength() >= 2) {
      Object tmp;
      if (pos->get(0, &tmp)->isNum()) {
        windowParams.XPosition = tmp.getNum();
      }
      tmp.free();
      if (pos->get(1, &tmp)->isNum()) {
        windowParams.YPosition = tmp.getNum();
      }
      tmp.free();
    }
  }
  obj1.free();
}

void MovieParameters::parseMediaPlayParameters(Object* obj) {
  
  Object tmp;

  if (obj->dictLookup("V", &tmp)->isInt()) {
    volume = tmp.getInt();
  }
  tmp.free();

  if (obj->dictLookup("C", &tmp)->isBool()) {
    showControls = tmp.getBool();
  }
  tmp.free();

  if (obj->dictLookup("F", &tmp)->isInt()) {
    int t = tmp.getInt();
    
    switch(t) {
    case 0: fittingPolicy = fittingMeet; break;
    case 1: fittingPolicy = fittingSlice; break;
    case 2: fittingPolicy = fittingFill; break;
    case 3: fittingPolicy = fittingScroll; break;
    case 4: fittingPolicy = fittingHidden; break;
    case 5: fittingPolicy = fittingUndefined; break;
    }
  }
  tmp.free();

  // duration parsing
  // duration's default value is set to 0, which means : intrinsinc media duration
  if (obj->dictLookup("D", &tmp)->isDict()) {
    Object oname, ddict, tmp2;
    if (tmp.dictLookup("S", &oname)->isName()) {
      char* name = oname.getName();
      if (!strcmp(name, "F"))
	duration.units = -1; // infinity
      else if (!strcmp(name, "T")) {
	if (tmp.dictLookup("T", &ddict)->isDict()) {
	  if (ddict.dictLookup("V", &tmp2)->isNum()) {
	    duration.units = Gulong(tmp2.getNum());
	  }
	  tmp2.free();
	}
	ddict.free();
      }
    }
    oname.free();
  }
  tmp.free();


  if (obj->dictLookup("A", &tmp)->isBool()) {
    autoPlay = tmp.getBool();
  }
  tmp.free();

  if (obj->dictLookup("RC", &tmp)->isNum()) {
    repeatCount = tmp.getNum();
  }
  tmp.free();

}

void MovieParameters::parseMediaScreenParameters(Object* obj) {
  Object tmp;

  if (obj->dictLookup("W", &tmp)->isInt()) {
    int t = tmp.getInt();
    
    switch(t) {
    case 0: windowParams.type = MovieWindowParameters::movieWindowFloating; break;
    case 1: windowParams.type = MovieWindowParameters::movieWindowFullscreen; break;
    case 2: windowParams.type = MovieWindowParameters::movieWindowHidden; break;
    case 3: windowParams.type = MovieWindowParameters::movieWindowEmbedded; break;
    }
  }
  tmp.free();

  // background color
  if (obj->dictLookup("B", &tmp)->isArray()) {
    Array* color = tmp.getArray();

    Object component;
    
    color->get(0, &component);
    bgColor.r = component.getNum();
    component.free();

    color->get(1, &component);
    bgColor.g = component.getNum();
    component.free();

    color->get(2, &component);
    bgColor.b = component.getNum();
    component.free();
  }
  tmp.free();


  // opacity
  if (obj->dictLookup("O", &tmp)->isNum()) {
    opacity = tmp.getNum();
  }
  tmp.free();

  if (windowParams.type == MovieWindowParameters::movieWindowFloating) {
    Object winDict;
    if (obj->dictLookup("F",&winDict)->isDict()) {
      windowParams.parseFWParams(&winDict);
    }
  }
}

void Movie::initialize() {
  fileName = NULL;
  contentType = NULL;
  isEmbedded = gFalse;
  embeddedStream = NULL;
  width = -1;
  height = -1;
  showPoster = gFalse;
}

Movie::~Movie() {
  if (fileName)
    delete fileName;
  if (contentType)
    delete contentType;

  if (embeddedStream && (!embeddedStream->decRef())) {
    delete embeddedStream;
  }
  poster.free();
}

Movie::Movie(Object *movieDict, Object *aDict) {
  initialize();

  Object obj1, obj2;
  if (getFileSpecNameForPlatform(movieDict->dictLookup("F", &obj1), &obj2)) {
    fileName = obj2.getString()->copy();
    obj2.free();
  }
  obj1.free();

  if (movieDict->dictLookup("Aspect", &obj1)->isArray()) {
    Array* aspect = obj1.getArray();
    if (aspect->getLength() >= 2) {
      Object tmp;
      if( aspect->get(0, &tmp)->isNum() ) {
        width = (int)floor( aspect->get(0, &tmp)->getNum() + 0.5 );
      }
      tmp.free();
      if( aspect->get(1, &tmp)->isNum() ) {
        height = (int)floor( aspect->get(1, &tmp)->getNum() + 0.5 );
      }
      tmp.free();
    }
  }
  obj1.free();

  int rotationAngle = 0;
  if (movieDict->dictLookup("Rotate", &obj1)->isInt()) {
    // round up to 90°
    rotationAngle = (((obj1.getInt() + 360) % 360) % 90) * 90;
  }
  obj1.free();

  //
  // movie poster
  //
  if (!movieDict->dictLookupNF("Poster", &poster)->isNull()) {
    if (poster.isRef() || poster.isStream()) {
      showPoster = gTrue;
    } else if (poster.isBool()) {
      showPoster = obj1.getBool();
      poster.free();
    } else {
      poster.free();
    }
  }

  if (aDict->isDict()) {
    MH.parseMovieActivation(aDict, width, height, rotationAngle);
    // deep copy of MH to BE
    // (no distinction is made with AnnotMovie)
    memcpy(&BE, &MH, sizeof(MH));
  }
}

Movie::Movie(Object* obj) {
  Object tmp, tmp2;

  initialize();
  if (obj->dictLookup("S", &tmp)->isName()) {
    if (!strcmp(tmp.getName(), "MR")) { // it's a media rendition

      //
      // parse Media Play Parameters
      if (obj->dictLookup("P", &tmp2)->isDict()) { // media play parameters
	Object params;
	if (tmp2.dictLookup("MH", &params)->isDict()) {
	  MH.parseMediaPlayParameters(&params);
	}
	params.free();
	if (tmp2.dictLookup("BE", &params)->isDict()) {
	  BE.parseMediaPlayParameters(&params);
	}
	params.free();
      }
      tmp2.free();
      //
      // parse Media Screen Parameters
      if (obj->dictLookup("SP", &tmp2)->isDict()) { // media screen parameters
	Object params;
	if (tmp2.dictLookup("MH", &params)->isDict()) {
	  MH.parseMediaScreenParameters(&params);
	}
	params.free();
	if (tmp2.dictLookup("BE", &params)->isDict()) {
	  BE.parseMediaScreenParameters(&params);
	}
	params.free();
      }
      tmp2.free();

      //
      // Parse media clip data
      //
      if (obj->dictLookup("C", &tmp2)->isDict()) { // media clip
	
	tmp.free();
	if (tmp2.dictLookup("S", &tmp)->isName()) {
	  if (!strcmp(tmp.getName(), "MCD")) { // media clip data
	    Object obj1, obj2;
	    if (tmp2.dictLookup("D", &obj1)->isDict()) {
	      if (obj1.dictLookup("F", &obj2)->isString()) {
		fileName = obj2.getString()->copy();
	      }
	      obj2.free();
	      
	      if (!obj1.dictLookup("EF", &obj2)->isNull()) {
		tmp.free();
		Object mref;
		if (!obj2.dictLookupNF("F", &mref)->isNull()) {
		  isEmbedded = gTrue;
		  Object embedded;
		  obj2.dictLookup("F", &embedded);
		  if (embedded.isStream()) {
		    embeddedStream = embedded.getStream();
		    // "copy" stream
		    embeddedStream->incRef();
		  }
		  embedded.free();
		}
		mref.free();
	      }
	      obj2.free();
	    }
	    obj1.free();
	    
	    if (tmp2.dictLookup("CT", &obj1)->isString()) {
	      contentType = obj1.getString()->copy();
	    }
	    obj1.free();
	  }
	}
	tmp.free();
      }
      tmp2.free();
    }
  }
}

Movie *Movie::fromMovie(Object *objMovie, Object *objAct)
{
  return new Movie (objMovie, objAct);
}

Movie *Movie::fromMediaRendition(Object *objRend)
{
  return new Movie (objRend);
}

void Movie::outputToFile(FILE* fp) {
  embeddedStream->reset();

  while (1) {
    int c = embeddedStream->getChar();
    if (c == EOF)
      break;
    
    fwrite(&c, 1, 1, fp);
  }
  
}

Movie* Movie::copy() {

  // call default copy constructor
  Movie* new_movie = new Movie(*this);

  if (contentType)
    new_movie->contentType = contentType->copy();
  if (fileName)
    new_movie->fileName = fileName->copy();

  if (new_movie->embeddedStream)
    new_movie->embeddedStream->incRef();

  poster.copy(&new_movie->poster);

  return new_movie;
}
