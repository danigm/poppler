/*
 * Copyright (C) 2008 Inigo Martinez <inigomartinez@gmail.com>
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
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <gtk/gtk.h>
#include <string.h>

#include "annots.h"
#include "utils.h"

enum {
    ANNOTS_X1_COLUMN,
    ANNOTS_Y1_COLUMN,
    ANNOTS_X2_COLUMN,
    ANNOTS_Y2_COLUMN,
    ANNOTS_TYPE_COLUMN,
    ANNOTS_COLUMN,
    N_COLUMNS
};

typedef struct {
    PopplerDocument *doc;
    PopplerPage     *page;

    GtkListStore    *model;
    GtkWidget       *annot_view;
    GtkWidget       *timer_label;

    gint             num_page;
} PgdAnnotsDemo;

static void
pgd_annots_free (PgdAnnotsDemo *demo)
{
    if (!demo)
        return;

    if (demo->doc) {
        g_object_unref (demo->doc);
        demo->doc = NULL;
    }

    if (demo->page) {
        g_object_unref (demo->page);
        demo->page = NULL;
    }

    if (demo->model) {
        g_object_unref (demo->model);
        demo->model = NULL;
    }

    g_free (demo);
}

static GtkWidget *
pgd_annot_view_new (void)
{
    GtkWidget  *frame, *label;

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), "<b>Annot Properties</b>");
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_widget_show (label);

    return frame;
}

const gchar *
get_annot_type (PopplerAnnot *poppler_annot)
{
    switch (poppler_annot_get_annot_type (poppler_annot))
    {
      case POPPLER_ANNOT_TEXT:
        return "Text";
      case POPPLER_ANNOT_LINK:
        return "Link";
      case POPPLER_ANNOT_FREE_TEXT:
        return "Free Text";
      case POPPLER_ANNOT_LINE:
        return "Line";
      case POPPLER_ANNOT_SQUARE:
        return "Square";
      case POPPLER_ANNOT_CIRCLE:
        return "Circle";
      case POPPLER_ANNOT_POLYGON:
        return "Polygon";
      case POPPLER_ANNOT_POLY_LINE:
        return "Poly Line";
      case POPPLER_ANNOT_HIGHLIGHT:
        return "Highlight";
      case POPPLER_ANNOT_UNDERLINE:
        return "Underline";
      case POPPLER_ANNOT_SQUIGGLY:
        return "Squiggly";
      case POPPLER_ANNOT_STRIKE_OUT:
        return "Strike Out";
      case POPPLER_ANNOT_STAMP:
        return "Stamp";
      case POPPLER_ANNOT_CARET:
        return "Caret";
      case POPPLER_ANNOT_INK:
        return "Ink";
      case POPPLER_ANNOT_POPUP:
        return "Popup";
      case POPPLER_ANNOT_FILE_ATTACHMENT:
        return "File Attachment";
      case POPPLER_ANNOT_SOUND:
        return "Sound";
      case POPPLER_ANNOT_MOVIE:
        return "Movie";
      case POPPLER_ANNOT_WIDGET:
        return "Widget";
      case POPPLER_ANNOT_SCREEN:
        return "Screen";
      case POPPLER_ANNOT_PRINTER_MARK:
        return "Printer Mark";
      case POPPLER_ANNOT_TRAP_NET:
        return "Trap Net";
      case POPPLER_ANNOT_WATERMARK:
        return "Watermark";
      case POPPLER_ANNOT_3D:
        return "3D";
      default:
        break;
  }

  return "Unknown";
}

const gchar *
get_markup_reply_to (PopplerAnnotMarkup *poppler_annot)
{
    switch (poppler_annot_markup_get_reply_to (poppler_annot))
    {
      case POPPLER_ANNOT_MARKUP_REPLY_TYPE_R:
        return "Type R";
      case POPPLER_ANNOT_MARKUP_REPLY_TYPE_GROUP:
        return "Type Group";
      default:
        break;
    }

  return "Unknown";
}

const gchar *
get_markup_external_data (PopplerAnnotMarkup *poppler_annot)
{
    switch (poppler_annot_markup_get_external_data (poppler_annot))
    {
      case POPPLER_ANNOT_EXTERNAL_DATA_MARKUP_3D:
        return "Markup 3D";
      default:
        break;
    }

  return "Unknown";
}

static void
pgd_annot_view_set_annot (GtkWidget    *annot_view,
                          PopplerAnnot *annot)
{
    GtkWidget  *alignment;
    GtkWidget  *table;
    GEnumValue *enum_value;
    gint        row = 0;
    gchar      *text, *warning;

    alignment = gtk_bin_get_child (GTK_BIN (annot_view));
    if (alignment) {
        gtk_container_remove (GTK_CONTAINER (annot_view), alignment);
    }

    alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 5, 5, 12, 5);
    gtk_container_add (GTK_CONTAINER (annot_view), alignment);
    gtk_widget_show (alignment);

    if (!annot)
        return;

    table = gtk_table_new (10, 2, FALSE);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);

    text = poppler_annot_get_contents (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Contents:</b>", text, &row);
    g_free (text);

    text = poppler_annot_get_name (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Name:</b>", text, &row);
    g_free (text);

    text = poppler_annot_get_modified (annot);
    pgd_table_add_property (GTK_TABLE (table), "<b>Modified:</b>", text, &row);
    g_free (text);

    text = g_strdup_printf ("%d", poppler_annot_get_flags (annot));
    pgd_table_add_property (GTK_TABLE (table), "<b>Flags:</b>", text, &row);
    g_free (text);

    if (POPPLER_IS_ANNOT_MARKUP (annot)) {
        PopplerAnnotMarkup *markup = POPPLER_ANNOT_MARKUP (annot);

        text = poppler_annot_markup_get_label (markup);
        pgd_table_add_property (GTK_TABLE (table), "<b>Label:</b>", text, &row);
        g_free (text);

        pgd_table_add_property (GTK_TABLE (table), "<b>Popup is open:</b>",
                                poppler_annot_markup_get_popup_is_open (markup) ? "Yes" : "No", &row);

        text = g_strdup_printf ("%d", poppler_annot_markup_get_opacity (markup));
        pgd_table_add_property (GTK_TABLE (table), "<b>Opacity:</b>", text, &row);

        text = poppler_annot_markup_get_subject (markup);
        pgd_table_add_property (GTK_TABLE (table), "<b>Subject:</b>", text, &row);
        g_free (text);

        pgd_table_add_property (GTK_TABLE (table), "<b>Reply To:</b>", get_markup_reply_to (markup), &row);

        pgd_table_add_property (GTK_TABLE (table), "<b>External Data:</b>", get_markup_external_data (markup), &row);
    }

    gtk_container_add (GTK_CONTAINER (alignment), table);
    gtk_widget_show (table);
}

static void
pgd_annots_get_annots (GtkWidget     *button,
                       PgdAnnotsDemo *demo)
{
    GList       *mapping, *l;
    gint         n_fields;
    GTimer      *timer;

    gtk_list_store_clear (demo->model);
    pgd_annot_view_set_annot (demo->annot_view, NULL);

    if (demo->page) {
        g_object_unref (demo->page);
        demo->page = NULL;
    }

    demo->page = poppler_document_get_page (demo->doc, demo->num_page);
    if (!demo->page)
        return;

    timer = g_timer_new ();
    mapping = poppler_page_get_annot_mapping (demo->page);
    g_timer_stop (timer);

    n_fields = g_list_length (mapping);
    if (n_fields > 0) {
        gchar *str;

        str = g_strdup_printf ("<i>%d annots found in %.4f seconds</i>",
                               n_fields, g_timer_elapsed (timer, NULL));
        gtk_label_set_markup (GTK_LABEL (demo->timer_label), str);
        g_free (str);
    } else {
        gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No annots found</i>");
    }

    g_timer_destroy (timer);

    for (l = mapping; l; l = g_list_next (l)) {
        PopplerAnnotMapping *amapping;
        GtkTreeIter          iter;
        gchar               *x1, *y1, *x2, *y2;

        amapping = (PopplerAnnotMapping *) l->data;

        x1 = g_strdup_printf ("%.2f", amapping->area.x1);
        y1 = g_strdup_printf ("%.2f", amapping->area.y1);
        x2 = g_strdup_printf ("%.2f", amapping->area.x2);
        y2 = g_strdup_printf ("%.2f", amapping->area.y2);

        gtk_list_store_append (demo->model, &iter);
        gtk_list_store_set (demo->model, &iter,
                            ANNOTS_X1_COLUMN, x1, 
                            ANNOTS_Y1_COLUMN, y1,
                            ANNOTS_X2_COLUMN, x2,
                            ANNOTS_Y2_COLUMN, y2,
                            ANNOTS_TYPE_COLUMN, get_annot_type (amapping->annot),
                            ANNOTS_COLUMN, amapping->annot,
                           -1);
        g_free (x1);
        g_free (y1);
        g_free (x2);
        g_free (y2);
    }

    poppler_page_free_annot_mapping (mapping);
}

static void
pgd_annots_page_selector_value_changed (GtkSpinButton *spinbutton,
                                        PgdAnnotsDemo *demo)
{
    demo->num_page = (gint) gtk_spin_button_get_value (spinbutton) - 1;
}

static void
pgd_annots_selection_changed (GtkTreeSelection *treeselection,
                              PgdAnnotsDemo    *demo)
{
    GtkTreeModel *model;
    GtkTreeIter   iter;

    if (gtk_tree_selection_get_selected (treeselection, &model, &iter)) {
        PopplerAnnot *annot;

        gtk_tree_model_get (model, &iter,
                            ANNOTS_COLUMN, &annot,
                           -1);
        pgd_annot_view_set_annot (demo->annot_view, annot);
        g_object_unref (annot);
    }
}

GtkWidget *
pgd_annots_create_widget (PopplerDocument *document)
{
    PgdAnnotsDemo    *demo;
    GtkWidget        *label;
    GtkWidget        *vbox;
    GtkWidget        *hbox, *page_selector;
    GtkWidget        *button;
    GtkWidget        *hpaned;
    GtkWidget        *swindow, *treeview;
    GtkTreeSelection *selection;
    GtkCellRenderer  *renderer;
    gchar            *str;
    gint              n_pages;

    demo = g_new0 (PgdAnnotsDemo, 1);

    demo->doc = g_object_ref (document);

    n_pages = poppler_document_get_n_pages (document);

    vbox = gtk_vbox_new (FALSE, 12);

    hbox = gtk_hbox_new (FALSE, 6);

    label = gtk_label_new ("Page:");
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
    gtk_widget_show (label);

    page_selector = gtk_spin_button_new_with_range (1, n_pages, 1);
    g_signal_connect (G_OBJECT (page_selector), "value-changed",
                      G_CALLBACK (pgd_annots_page_selector_value_changed),
                      (gpointer) demo);
    gtk_box_pack_start (GTK_BOX (hbox), page_selector, FALSE, TRUE, 0);
    gtk_widget_show (page_selector);

    str = g_strdup_printf ("of %d", n_pages);
    label = gtk_label_new (str);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
    gtk_widget_show (label);
    g_free (str);

    button = gtk_button_new_with_label ("Get Annots");
    g_signal_connect (G_OBJECT (button), "clicked",
                      G_CALLBACK (pgd_annots_get_annots),
                      (gpointer) demo);
    gtk_box_pack_end (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_widget_show (button);

    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show (hbox);

    demo->timer_label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (demo->timer_label), "<i>No annots found</i>");
    g_object_set (G_OBJECT (demo->timer_label), "xalign", 1.0, NULL);
    gtk_box_pack_start (GTK_BOX (vbox), demo->timer_label, FALSE, TRUE, 0);
    gtk_widget_show (demo->timer_label);

    hpaned = gtk_hpaned_new ();

    demo->annot_view = pgd_annot_view_new ();

    swindow = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);

    demo->model = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING,
                                      G_TYPE_STRING, G_TYPE_STRING,
                                      G_TYPE_STRING, G_TYPE_STRING,
                                      G_TYPE_OBJECT);
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (demo->model));

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 0, "X1",
                                                 renderer,
                                                 "text", ANNOTS_X1_COLUMN,
                                                 NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 1, "Y1",
                                                 renderer,
                                                 "text", ANNOTS_Y1_COLUMN,
                                                 NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 2, "X2",
                                                 renderer,
                                                 "text", ANNOTS_X2_COLUMN,
                                                 NULL);
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 3, "Y2",
                                                 renderer,
                                                 "text", ANNOTS_Y2_COLUMN,
                                                 NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                                 4, "Annot Type",
                                                 renderer,
                                                 "text", ANNOTS_TYPE_COLUMN,
                                                 NULL);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
    g_signal_connect (G_OBJECT (selection), "changed",
                      G_CALLBACK (pgd_annots_selection_changed),
                      (gpointer) demo);

    gtk_container_add (GTK_CONTAINER (swindow), treeview);
    gtk_widget_show (treeview);

    gtk_paned_add1 (GTK_PANED (hpaned), swindow);
    gtk_widget_show (swindow);

    gtk_paned_add2 (GTK_PANED (hpaned), demo->annot_view);
    gtk_widget_show (demo->annot_view);

    gtk_paned_set_position (GTK_PANED (hpaned), 300);

    gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);
    gtk_widget_show (hpaned);

    g_object_weak_ref (G_OBJECT (vbox),
                       (GWeakNotify)pgd_annots_free,
                       demo);

    return vbox;
}
