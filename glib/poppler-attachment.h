/* poppler-attachment.h: glib interface to poppler
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

#ifndef __POPPLER_ATTACHMENT_H__
#define __POPPLER_ATTACHMENT_H__

#include <time.h>
#include <glib-object.h>

#include "poppler.h"

G_BEGIN_DECLS


#define POPPLER_TYPE_ATTACHMENT             (poppler_attachment_get_type ())
#define POPPLER_ATTACHMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), POPPLER_TYPE_ATTACHMENT, PopplerAttachment))
#define POPPLER_IS_ATTACHMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), POPPLER_TYPE_ATTACHMENT))


typedef gboolean (*PopplerAttachmentSaveFunc) (const gchar  *buf,
					       gsize         count,
					       gpointer      data,
					       GError      **error);

typedef struct _PopplerAttachment
{
  GObject parent;

  const char *name;
  const char *description;
  time_t mtime;
  time_t ctime;
} PopplerAttachment;

typedef struct _PopplerAttachmentClass
{
  GObjectClass parent_class;
} PopplerAttachmentClass;


GType     poppler_attachment_get_type         (void) G_GNUC_CONST;
gboolean  poppler_attachment_save             (PopplerAttachment          *attachment,
					       const char                 *filename,
					       GError                    **error);
gboolean  poppler_attachment_save_to_callback (PopplerAttachment          *attachment,
					       PopplerAttachmentSaveFunc   save_func,
					       gpointer                    user_data,
					       GError                    **error);


G_END_DECLS

#endif /* __POPPLER_ATTACHMENT_H__ */
