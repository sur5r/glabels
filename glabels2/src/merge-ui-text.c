/*
 *  (GLABELS) Label and Business Card Creation program for GNOME
 *
 *  merge_ui_text.c:  text-file document merge user interface backend module
 *
 *  Copyright (C) 2002  Jim Evins <evins@snaught.com>.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include <config.h>

#include "merge-ui-text.h"
#include "merge-text.h"
#include "marshal.h"

#include "debug.h"

/*===========================================*/
/* Private types                             */
/*===========================================*/

enum {
	CHANGED,
	LAST_SIGNAL
};

typedef struct {
	gchar *loc;
	GtkWidget *entry;
} EntryNode;

/*===========================================*/
/* Private globals                           */
/*===========================================*/

static GtkContainerClass *parent_class;

static gint merge_ui_text_src_signals[LAST_SIGNAL] = { 0 };
static gint merge_ui_text_field_ws_signals[LAST_SIGNAL] = { 0 };

/*===========================================*/
/* Local function prototypes                 */
/*===========================================*/

static void gl_merge_ui_text_src_class_init (glMergeUITextSrcClass * class);
static void gl_merge_ui_text_src_instance_init (glMergeUITextSrc * src);
static void gl_merge_ui_text_src_finalize (GObject * object);
static void gl_merge_ui_text_src_construct (glMergeUITextSrc * src,
					    glMergeType type);

static void src_changed_cb (glMergeUITextSrc * src);

static void gl_merge_ui_text_field_ws_class_init (glMergeUITextFieldWSClass * class);
static void gl_merge_ui_text_field_ws_instance_init (glMergeUITextFieldWS * field_ws);
static void gl_merge_ui_text_field_ws_finalize (GObject * object);
static void gl_merge_ui_text_field_ws_construct (glMergeUITextFieldWS * field_ws,
						 glMergeType type,
						 gchar * src);

static void field_ws_changed_cb (glMergeUITextFieldWS * field_ws);

/*****************************************************************************/
/* Boilerplate Object stuff.                                                 */
/*****************************************************************************/
guint
gl_merge_ui_text_src_get_type (void)
{
	static guint merge_ui_text_src_type = 0;

	if (!merge_ui_text_src_type) {
		GTypeInfo merge_ui_text_src_info = {
			sizeof (glMergeUITextSrcClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_merge_ui_text_src_class_init,
			NULL,
			NULL,
			sizeof (glMergeUITextSrc),
			0,
			(GInstanceInitFunc) gl_merge_ui_text_src_instance_init,
		};

		merge_ui_text_src_type =
			g_type_register_static (gtk_vbox_get_type (),
						"glMergeUITextSrc",
						&merge_ui_text_src_info, 0);
	}

	return merge_ui_text_src_type;
}

static void
gl_merge_ui_text_src_class_init (glMergeUITextSrcClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	object_class->finalize = gl_merge_ui_text_src_finalize;

	merge_ui_text_src_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glMergeUITextSrcClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);
}

static void
gl_merge_ui_text_src_instance_init (glMergeUITextSrc * src)
{
	src->type = GL_MERGE_NONE;
	src->entry = NULL;
}

static void
gl_merge_ui_text_src_finalize (GObject * object)
{
	glMergeUITextSrc *src;
	glMergeUITextSrcClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MERGE_UI_TEXT_SRC (object));

	src = GL_MERGE_UI_TEXT_SRC (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_merge_ui_text_src_new (glMergeType type)
{
	glMergeUITextSrc *src;

	src = g_object_new (gl_merge_ui_text_src_get_type (), NULL);

	gl_merge_ui_text_src_construct (src, type);

	return GTK_WIDGET (src);
}

/*---------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                     */
/*---------------------------------------------------------------------------*/
static void
gl_merge_ui_text_src_construct (glMergeUITextSrc * src,
				glMergeType type)
{
	GtkWidget *wvbox, *wentry;

	wvbox = GTK_WIDGET (src);

	src->type = type;

	src->entry =
	    gnome_file_entry_new (NULL, _("Select merge-database source"));
	gtk_box_pack_start (GTK_BOX (wvbox), src->entry, TRUE, TRUE, 0);

	wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (src->entry));
	g_signal_connect_swapped (G_OBJECT (wentry), "changed",
				  G_CALLBACK (src_changed_cb),
				  G_OBJECT (src));
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when our backend widget has changed.              */
/*--------------------------------------------------------------------------*/
static void
src_changed_cb (glMergeUITextSrc * src)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (src), merge_ui_text_src_signals[CHANGED], 0);
}

/*****************************************************************************/
/* Set src name.                                                             */
/*****************************************************************************/
void
gl_merge_ui_text_src_set_value (glMergeUITextSrc * src,
				gchar * text)
{
	GtkWidget *wentry;

	wentry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (src->entry));

	gtk_entry_set_text (GTK_ENTRY (wentry), text);
}

/*****************************************************************************/
/* Get src name.                                                             */
/*****************************************************************************/
gchar *
gl_merge_ui_text_src_get_value (glMergeUITextSrc * src)
{
	return gnome_file_entry_get_full_path (GNOME_FILE_ENTRY (src->entry),
					       TRUE);
}

/*****************************************************************************/
/* Boilerplate Object stuff.                                                 */
/*****************************************************************************/
guint
gl_merge_ui_text_field_ws_get_type (void)
{
	static guint merge_ui_text_field_ws_type = 0;

	if (!merge_ui_text_field_ws_type) {
		GTypeInfo merge_ui_text_field_ws_info = {
			sizeof (glMergeUITextFieldWSClass),
			NULL,
			NULL,
			(GClassInitFunc) gl_merge_ui_text_field_ws_class_init,
			NULL,
			NULL,
			sizeof (glMergeUITextFieldWS),
			0,
			(GInstanceInitFunc) gl_merge_ui_text_field_ws_instance_init,
		};

		merge_ui_text_field_ws_type =
		    g_type_register_static (gtk_hbox_get_type (),
					    "glMergeUITextFieldWS",
					    &merge_ui_text_field_ws_info, 0);
	}

	return merge_ui_text_field_ws_type;
}

static void
gl_merge_ui_text_field_ws_class_init (glMergeUITextFieldWSClass * class)
{
	GObjectClass *object_class;

	object_class = (GObjectClass *) class;

	parent_class = gtk_type_class (gtk_hbox_get_type ());

	object_class->finalize = gl_merge_ui_text_field_ws_finalize;

	merge_ui_text_field_ws_signals[CHANGED] =
	    g_signal_new ("changed",
			  G_OBJECT_CLASS_TYPE(object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (glMergeUITextFieldWSClass, changed),
			  NULL, NULL,
			  gl_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);
}

static void
gl_merge_ui_text_field_ws_instance_init (glMergeUITextFieldWS * field_ws)
{
	field_ws->type = GL_MERGE_NONE;
	field_ws->entry_list = NULL;
}

static void
gl_merge_ui_text_field_ws_finalize (GObject * object)
{
	glMergeUITextFieldWS *field_ws;
	glMergeUITextFieldWSClass *class;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GL_IS_MERGE_UI_TEXT_FIELD_WS (object));

	field_ws = GL_MERGE_UI_TEXT_FIELD_WS (object);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GtkWidget *
gl_merge_ui_text_field_ws_new (glMergeType type,
			       gchar * src)
{
	glMergeUITextFieldWS *field_ws;

	field_ws = g_object_new (gl_merge_ui_text_field_ws_get_type (), NULL);

	gl_merge_ui_text_field_ws_construct (field_ws, type, src);

	return GTK_WIDGET (field_ws);
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Construct composite widget.                                    */
/*--------------------------------------------------------------------------*/
static void
gl_merge_ui_text_field_ws_construct (glMergeUITextFieldWS * field_ws,
				     glMergeType type,
				     gchar * src)
{
	GtkWidget *whbox, *wtable, *wlabel, *whline, *wentry;
	glMergeInput *mp;
	glMergeRawField *sample_field;
	GList *sample_field_list=NULL, *p;
	gint n_fields, i;
	EntryNode *entry_node;

	field_ws->type = type;
	field_ws->entry_list = NULL;

	mp = gl_merge_open( type, NULL, src );
	sample_field_list = gl_merge_get_raw_record (mp);
	gl_merge_close(mp);
	n_fields = g_list_length( sample_field_list );

	whbox = GTK_WIDGET (field_ws);

	wtable = gtk_table_new (n_fields + 2, 3, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (wtable), 20);
	gtk_table_set_row_spacings (GTK_TABLE (wtable), 10);
	gtk_box_pack_start (GTK_BOX (whbox), wtable, FALSE, FALSE, GNOME_PAD);

	wlabel = gtk_label_new (_("Column"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 1, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 0, 1, 0, 1);

	wlabel = gtk_label_new (_("Custom field key"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 1, 2, 0, 1);

	wlabel = gtk_label_new (_("Sample data"));
	gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel, 2, 3, 0, 1);

	whline = gtk_hseparator_new ();
	gtk_table_attach_defaults (GTK_TABLE (wtable), whline, 0, 1, 1, 2);
	whline = gtk_hseparator_new ();
	gtk_table_attach_defaults (GTK_TABLE (wtable), whline, 1, 2, 1, 2);
	whline = gtk_hseparator_new ();
	gtk_table_attach_defaults (GTK_TABLE (wtable), whline, 2, 3, 1, 2);

	for (p = sample_field_list, i = 0; p != NULL; p = p->next, i++) {
		sample_field = p->data;

		wlabel = gtk_label_new (sample_field->loc);
		gtk_misc_set_alignment (GTK_MISC (wlabel), 1, 0.5);
		gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel,
					   0, 1, i + 2, i + 3);

		wentry = gtk_entry_new ();
		gtk_entry_set_text (GTK_ENTRY (wentry), sample_field->loc);
		gtk_widget_set_usize (wentry, 100, 0);
		gtk_table_attach_defaults (GTK_TABLE (wtable), wentry,
					   1, 2, i + 2, i + 3);

		g_signal_connect_swapped (G_OBJECT (wentry), "changed",
					  G_CALLBACK(field_ws_changed_cb),
					  G_OBJECT (field_ws));

		wlabel = gtk_label_new (sample_field->value);
		gtk_misc_set_alignment (GTK_MISC (wlabel), 0, 0.5);
		gtk_label_set_justify (GTK_LABEL(wlabel), GTK_JUSTIFY_LEFT);
		gtk_table_attach_defaults (GTK_TABLE (wtable), wlabel,
					   2, 3, i + 2, i + 3);

		entry_node = g_new0 (EntryNode, 1);
		entry_node->loc = g_strdup (sample_field->loc);
		entry_node->entry = wentry;
		field_ws->entry_list =
		    g_list_append (field_ws->entry_list, entry_node);

	}

	gl_merge_free_raw_record (&sample_field_list);

}

/*****************************************************************************/
/* Set field definitions.  (associate key with locator)                      */
/*****************************************************************************/
void
gl_merge_ui_text_field_ws_set_field_defs (glMergeUITextFieldWS * field_ws,
					  GList * field_defs)
{
	GList *p_entry, *p_defs;
	EntryNode *entry;
	glMergeFieldDefinition *def;

	for (p_entry = field_ws->entry_list; p_entry != NULL;
	     p_entry = p_entry->next) {
		entry = (EntryNode *) p_entry->data;

		for (p_defs = field_defs; p_defs != NULL; p_defs = p_defs->next) {
			def = (glMergeFieldDefinition *) p_defs->data;

			if (strcmp (entry->loc, def->loc) == 0) {
				gtk_entry_set_text (GTK_ENTRY (entry->entry),
						    def->key);
			}
		}
	}

}

/*****************************************************************************/
/* Get field definitions.  (associate key with locator)                      */
/*****************************************************************************/
GList *
gl_merge_ui_text_field_ws_get_field_defs (glMergeUITextFieldWS * field_ws)
{
	GList *p_entry, *defs_list = NULL;
	EntryNode *entry;
	glMergeFieldDefinition *def;

	for (p_entry = field_ws->entry_list; p_entry != NULL;
	     p_entry = p_entry->next) {
		entry = (EntryNode *) p_entry->data;

		def = g_new0 (glMergeFieldDefinition, 1);

		def->loc = g_strdup (entry->loc);
		def->key =
		    gtk_editable_get_chars (GTK_EDITABLE (entry->entry), 0, -1);

		defs_list = g_list_append (defs_list, def);

	}

	return defs_list;
}

/*--------------------------------------------------------------------------*/
/* PRIVATE.  Callback for when our backend widget has changed.              */
/*--------------------------------------------------------------------------*/
static void
field_ws_changed_cb (glMergeUITextFieldWS * field_ws)
{
	/* Emit our "changed" signal */
	g_signal_emit (G_OBJECT (field_ws),
		       merge_ui_text_field_ws_signals[CHANGED], 0);
}
