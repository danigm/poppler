
/* Generated data (by glib-mkenums) */

#include "poppler-enums.h"
#include "poppler-document.h"


/* enumerations from "poppler-action.h" */
GType
poppler_action_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { POPPLER_ACTION_UNKNOWN, "POPPLER_ACTION_UNKNOWN", "unknown" },
      { POPPLER_ACTION_GOTO_DEST, "POPPLER_ACTION_GOTO_DEST", "goto-dest" },
      { POPPLER_ACTION_GOTO_REMOTE, "POPPLER_ACTION_GOTO_REMOTE", "goto-remote" },
      { POPPLER_ACTION_LAUNCH, "POPPLER_ACTION_LAUNCH", "launch" },
      { POPPLER_ACTION_URI, "POPPLER_ACTION_URI", "uri" },
      { POPPLER_ACTION_NAMED, "POPPLER_ACTION_NAMED", "named" },
      { POPPLER_ACTION_MOVIE, "POPPLER_ACTION_MOVIE", "movie" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PopplerActionType", values);
  }
  return etype;
}

GType
poppler_dest_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { POPPLER_DEST_UNKNOWN, "POPPLER_DEST_UNKNOWN", "unknown" },
      { POPPLER_DEST_XYZ, "POPPLER_DEST_XYZ", "xyz" },
      { POPPLER_DEST_FIT, "POPPLER_DEST_FIT", "fit" },
      { POPPLER_DEST_FITH, "POPPLER_DEST_FITH", "fith" },
      { POPPLER_DEST_FITV, "POPPLER_DEST_FITV", "fitv" },
      { POPPLER_DEST_FITR, "POPPLER_DEST_FITR", "fitr" },
      { POPPLER_DEST_FITB, "POPPLER_DEST_FITB", "fitb" },
      { POPPLER_DEST_FITBH, "POPPLER_DEST_FITBH", "fitbh" },
      { POPPLER_DEST_FITBV, "POPPLER_DEST_FITBV", "fitbv" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PopplerDestType", values);
  }
  return etype;
}


/* enumerations from "poppler-document.h" */
GType
poppler_page_layout_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { POPPLER_PAGE_LAYOUT_UNSET, "POPPLER_PAGE_LAYOUT_UNSET", "unset" },
      { POPPLER_PAGE_LAYOUT_SINGLE_PAGE, "POPPLER_PAGE_LAYOUT_SINGLE_PAGE", "single-page" },
      { POPPLER_PAGE_LAYOUT_ONE_COLUMN, "POPPLER_PAGE_LAYOUT_ONE_COLUMN", "one-column" },
      { POPPLER_PAGE_LAYOUT_TWO_COLUMN_LEFT, "POPPLER_PAGE_LAYOUT_TWO_COLUMN_LEFT", "two-column-left" },
      { POPPLER_PAGE_LAYOUT_TWO_COLUMN_RIGHT, "POPPLER_PAGE_LAYOUT_TWO_COLUMN_RIGHT", "two-column-right" },
      { POPPLER_PAGE_LAYOUT_TWO_PAGE_LEFT, "POPPLER_PAGE_LAYOUT_TWO_PAGE_LEFT", "two-page-left" },
      { POPPLER_PAGE_LAYOUT_TWO_PAGE_RIGHT, "POPPLER_PAGE_LAYOUT_TWO_PAGE_RIGHT", "two-page-right" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PopplerPageLayout", values);
  }
  return etype;
}

GType
poppler_page_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { POPPLER_PAGE_MODE_UNSET, "POPPLER_PAGE_MODE_UNSET", "unset" },
      { POPPLER_PAGE_MODE_NONE, "POPPLER_PAGE_MODE_NONE", "none" },
      { POPPLER_PAGE_MODE_USE_OUTLINES, "POPPLER_PAGE_MODE_USE_OUTLINES", "use-outlines" },
      { POPPLER_PAGE_MODE_USE_THUMBS, "POPPLER_PAGE_MODE_USE_THUMBS", "use-thumbs" },
      { POPPLER_PAGE_MODE_FULL_SCREEN, "POPPLER_PAGE_MODE_FULL_SCREEN", "full-screen" },
      { POPPLER_PAGE_MODE_USE_OC, "POPPLER_PAGE_MODE_USE_OC", "use-oc" },
      { POPPLER_PAGE_MODE_USE_ATTACHMENTS, "POPPLER_PAGE_MODE_USE_ATTACHMENTS", "use-attachments" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PopplerPageMode", values);
  }
  return etype;
}

GType
poppler_viewer_preferences_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { POPPLER_VIEWER_PREFERENCES_UNSET, "POPPLER_VIEWER_PREFERENCES_UNSET", "unset" },
      { POPPLER_VIEWER_PREFERENCES_HIDE_TOOLBAR, "POPPLER_VIEWER_PREFERENCES_HIDE_TOOLBAR", "hide-toolbar" },
      { POPPLER_VIEWER_PREFERENCES_HIDE_MENUBAR, "POPPLER_VIEWER_PREFERENCES_HIDE_MENUBAR", "hide-menubar" },
      { POPPLER_VIEWER_PREFERENCES_HIDE_WINDOWUI, "POPPLER_VIEWER_PREFERENCES_HIDE_WINDOWUI", "hide-windowui" },
      { POPPLER_VIEWER_PREFERENCES_FIT_WINDOW, "POPPLER_VIEWER_PREFERENCES_FIT_WINDOW", "fit-window" },
      { POPPLER_VIEWER_PREFERENCES_CENTER_WINDOW, "POPPLER_VIEWER_PREFERENCES_CENTER_WINDOW", "center-window" },
      { POPPLER_VIEWER_PREFERENCES_DISPLAY_DOC_TITLE, "POPPLER_VIEWER_PREFERENCES_DISPLAY_DOC_TITLE", "display-doc-title" },
      { POPPLER_VIEWER_PREFERENCES_DIRECTION_RTL, "POPPLER_VIEWER_PREFERENCES_DIRECTION_RTL", "direction-rtl" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("PopplerViewerPreferences", values);
  }
  return etype;
}

GType
poppler_permissions_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { POPPLER_PERMISSIONS_PRINT, "POPPLER_PERMISSIONS_PRINT", "print" },
      { POPPLER_PERMISSIONS_MODIFY, "POPPLER_PERMISSIONS_MODIFY", "modify" },
      { POPPLER_PERMISSIONS_COPY, "POPPLER_PERMISSIONS_COPY", "copy" },
      { POPPLER_PERMISSIONS_EXTRACT_TEXT, "POPPLER_PERMISSIONS_EXTRACT_TEXT", "extract-text" },
      { POPPLER_PERMISSIONS_ANNOTATIONS_AND_FORMS, "POPPLER_PERMISSIONS_ANNOTATIONS_AND_FORMS", "annotations-and-forms" },
      { POPPLER_PERMISSIONS_FORMS, "POPPLER_PERMISSIONS_FORMS", "forms" },
      { POPPLER_PERMISSIONS_PRINT_LOW_QUALITY, "POPPLER_PERMISSIONS_PRINT_LOW_QUALITY", "print-low-quality" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("PopplerPermissions", values);
  }
  return etype;
}


/* enumerations from "poppler.h" */
GType
poppler_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { POPPLER_ERROR_INVALID, "POPPLER_ERROR_INVALID", "invalid" },
      { POPPLER_ERROR_ENCRYPTED, "POPPLER_ERROR_ENCRYPTED", "encrypted" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PopplerError", values);
  }
  return etype;
}

GType
poppler_orientation_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { POPPLER_ORIENTATION_DOCUMENT, "POPPLER_ORIENTATION_DOCUMENT", "document" },
      { POPPLER_ORIENTATION_PORTRAIT, "POPPLER_ORIENTATION_PORTRAIT", "portrait" },
      { POPPLER_ORIENTATION_LANDSCAPE, "POPPLER_ORIENTATION_LANDSCAPE", "landscape" },
      { POPPLER_ORIENTATION_UPSIDEDOWN, "POPPLER_ORIENTATION_UPSIDEDOWN", "upsidedown" },
      { POPPLER_ORIENTATION_SEASCAPE, "POPPLER_ORIENTATION_SEASCAPE", "seascape" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PopplerOrientation", values);
  }
  return etype;
}

GType
poppler_backend_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { POPPLER_BACKEND_UNKNOWN, "POPPLER_BACKEND_UNKNOWN", "unknown" },
      { POPPLER_BACKEND_SPLASH, "POPPLER_BACKEND_SPLASH", "splash" },
      { POPPLER_BACKEND_CAIRO, "POPPLER_BACKEND_CAIRO", "cairo" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("PopplerBackend", values);
  }
  return etype;
}

#define __POPPLER_ENUMS_C__


/* Generated data ends here */

