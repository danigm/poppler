/* poppler.h: glib interface to poppler
 * Copyright (C) 2004, Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __POPPLER_GLIB_H__
#define __POPPLER_GLIB_H__

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

GQuark poppler_error_quark (void);

#define POPPLER_ERROR poppler_error_quark ()

typedef enum
{
  POPPLER_ERROR_INVALID,
  POPPLER_ERROR_ENCRYPTED
} PopplerError;

typedef enum
{
	POPPLER_ORIENTATION_PORTRAIT,
	POPPLER_ORIENTATION_LANDSCAPE,
	POPPLER_ORIENTATION_UPSIDEDOWN,
	POPPLER_ORIENTATION_SEASCAPE
} PopplerOrientation;

typedef struct _PopplerDocument    PopplerDocument;
typedef struct _PopplerIndexIter   PopplerIndexIter;
typedef struct _PopplerFontsIter   PopplerFontsIter;
typedef struct _PopplerRectangle   PopplerRectangle;
typedef struct _PopplerLinkMapping PopplerLinkMapping;
typedef struct _PopplerPage        PopplerPage;
typedef struct _PopplerFontInfo    PopplerFontInfo;
typedef struct _PopplerPSFile      PopplerPSFile;
typedef union  _PopplerAction      PopplerAction;
typedef struct _PopplerDest        PopplerDest;


typedef enum
{
  POPPLER_BACKEND_UNKNOWN,
  POPPLER_BACKEND_SPLASH,
  POPPLER_BACKEND_CAIRO
} PopplerBackend;

PopplerBackend poppler_get_backend (void);
const char *   poppler_get_version (void);

G_END_DECLS

#include "poppler-features.h"
#include "poppler-document.h"
#include "poppler-page.h"
#include "poppler-action.h"
#include "poppler-enums.h"
#include "poppler-attachment.h"

#endif /* __POPPLER_GLIB_H__ */
