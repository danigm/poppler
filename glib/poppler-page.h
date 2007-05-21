/* poppler-page.h: glib interface to poppler
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

#ifndef __POPPLER_PAGE_H__
#define __POPPLER_PAGE_H__

#include <glib-object.h>
#include <gdk/gdkregion.h>
#include <gdk/gdkcolor.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#ifdef POPPLER_HAS_CAIRO
#include <cairo.h>
#endif

#include "poppler.h"

G_BEGIN_DECLS


#define POPPLER_TYPE_PAGE             (poppler_page_get_type ())
#define POPPLER_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_PAGE, PopplerPage))
#define POPPLER_IS_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_PAGE))


GType      	       poppler_page_get_type             (void) G_GNUC_CONST;
void                   poppler_page_render_to_pixbuf     (PopplerPage        *page,
							  int                 src_x,
							  int                 src_y,
							  int                 src_width,
							  int                 src_height,
							  double              scale,
							  int                 rotation,
							  GdkPixbuf          *pixbuf);

#ifdef POPPLER_HAS_CAIRO
void                   poppler_page_render               (PopplerPage        *page,
							  cairo_t            *cairo);
#endif	

void                   poppler_page_get_size             (PopplerPage        *page,
							  double             *width,
							  double             *height);
int                    poppler_page_get_index            (PopplerPage        *page);
double                 poppler_page_get_duration         (PopplerPage        *page);
PopplerPageTransition *poppler_page_get_transition       (PopplerPage        *page);
GdkPixbuf             *poppler_page_get_thumbnail        (PopplerPage        *page);
gboolean               poppler_page_get_thumbnail_size   (PopplerPage        *page,
							  int                *width,
							  int                *height);
GList     	      *poppler_page_find_text            (PopplerPage        *page,
							  const  char        *text);
void                   poppler_page_render_to_ps         (PopplerPage        *page,
							  PopplerPSFile      *ps_file);
char                  *poppler_page_get_text             (PopplerPage        *page,
							  PopplerRectangle   *rect);
GList                 *poppler_page_get_link_mapping     (PopplerPage        *page);
void                   poppler_page_free_link_mapping    (GList              *list);
GList                 *poppler_page_get_image_mapping    (PopplerPage        *page);
void                   poppler_page_free_image_mapping   (GList              *list);
GdkRegion             *poppler_page_get_selection_region (PopplerPage        *page,
							  gdouble             scale,
							  PopplerRectangle   *selection);
void                   poppler_page_render_selection     (PopplerPage        *page,
							  gdouble             scale,
							  int		      rotation,
							  GdkPixbuf          *pixbuf,
							  PopplerRectangle   *selection,
							  PopplerRectangle   *old_selection,
							  GdkColor           *glyph_color,
							  GdkColor           *background_color);
GList                 *poppler_page_get_form_fields      (PopplerPage        *page);
void 	               poppler_page_free_form_fields     (GList              *list);

void 		      poppler_page_get_crop_box 	 (PopplerPage        *page,
							  PopplerRectangle   *rect);


/* A rectangle on a page, with coordinates in PDF points. */
#define POPPLER_TYPE_RECTANGLE             (poppler_rectangle_get_type ())
struct _PopplerRectangle
{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
};

GType             poppler_rectangle_get_type (void) G_GNUC_CONST;
PopplerRectangle *poppler_rectangle_new      (void);
PopplerRectangle *poppler_rectangle_copy     (PopplerRectangle *rectangle);
void              poppler_rectangle_free     (PopplerRectangle *rectangle);



/* Mapping between areas on the current page and PopplerActions */
#define POPPLER_TYPE_LINK_MAPPING             (poppler_link_mapping_get_type ())
struct  _PopplerLinkMapping
{
  PopplerRectangle area;
  PopplerAction *action;
};

GType               poppler_link_mapping_get_type (void) G_GNUC_CONST;
PopplerLinkMapping *poppler_link_mapping_new      (void);
PopplerLinkMapping *poppler_link_mapping_copy     (PopplerLinkMapping *mapping);
void                poppler_link_mapping_free     (PopplerLinkMapping *mapping);

/* Page Transition */
#define POPPLER_TYPE_PAGE_TRANSITION                (poppler_page_transition_get_type ())
struct _PopplerPageTransition
{
  PopplerPageTransitionType type;
  PopplerPageTransitionAlignment alignment;
  PopplerPageTransitionDirection direction;
  gint duration;
  gint angle;
  gdouble scale;
  gboolean rectangular;
};

GType                  poppler_page_transition_get_type (void) G_GNUC_CONST;
PopplerPageTransition *poppler_page_transition_new      (void);
PopplerPageTransition *poppler_page_transition_copy     (PopplerPageTransition *transition);
void                   poppler_page_transition_free     (PopplerPageTransition *transition);

/* Mapping between areas on the current page and images */
#define POPPLER_TYPE_IMAGE_MAPPING             (poppler_image_mapping_get_type ())
struct  _PopplerImageMapping
{
  PopplerRectangle area;
  GdkPixbuf *image;	
};

/* FormField */
#define POPPLER_TYPE_FORM_FIELD                     (poppler_form_field_get_type ())
struct _PopplerTextField
{
  //flags
  char multiline:1;
  char password:1;
  char fileselect:1;
  char do_not_spell_check:1;
  char do_not_scroll:1;
  char comb:1;
  char rich_text:1;
  //content
  gchar *content;
  int length;
};

struct _PopplerButtonField
{
  //content
  gboolean state;
};

struct _PopplerChoiceField
{
  char combo:1;
  char edit:1;
  char multi_select:1;
  char do_not_spell_check:1;
  char commit_on_sel_change:1;
};

struct _PopplerFormField
{
  PopplerRectangle area;
  PopplerFormFieldType type;
  int id;
  double font_size;
  union {
    PopplerTextField text;
    PopplerButtonField button;
    PopplerChoiceField choice;
  };
};

GType               poppler_form_field_get_type (void) G_GNUC_CONST;
PopplerFormField   *poppler_form_field_new (void);
PopplerFormField   *poppler_form_field_copy (PopplerFormField *field);
void                poppler_form_field_free (PopplerFormField *field);

G_END_DECLS

#endif /* __POPPLER_PAGE_H__ */
