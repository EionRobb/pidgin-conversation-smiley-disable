/*
 * Conversation Smiley Disable Plugin
 * Copyright (C) 2024 Eion Robb
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 */

#define PLUGIN_ID			"gtk-eionrobb-conversation-smiley-disable"
#define PLUGIN_NAME			N_("Per-Conversation Smiley Disable")
#define PLUGIN_STATIC_NAME	conversation_smiley_disable
#define PLUGIN_SUMMARY		N_("Disable smiley/emoticons on a per-conversation basis.")
#define PLUGIN_DESCRIPTION	N_("Adds an option to the Options menu of the conversation window to disable all smileys for that conversation.")
#define PLUGIN_AUTHOR		"Eion Robb <eionrobb@gmail.com>"
#define PLUGIN_WEBSITE      "https://github.com/EionRobb/pidgin-conversation-smiley-disable"
#define PLUGIN_VERSION      "0.1"

#ifndef PURPLE_PLUGINS
#	define PURPLE_PLUGINS
#endif

/* System headers */
#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>

/* Purple headers */
#include <plugin.h>
#include <gtkconv.h>
#include <gtkimhtml.h>
#include <gtkthemes.h>
#include <gtkplugin.h>

static void
disable_smileys_for_gtkconv(PidginConversation *gtkconv)
{
	GtkIMHtml *imhtml;

	imhtml = GTK_IMHTML(gtkconv->imhtml);
	gtk_imhtml_remove_smileys(imhtml);

	gtk_imhtml_set_format_functions(imhtml,
			gtk_imhtml_get_format_functions(imhtml) & ~GTK_IMHTML_SMILEY);
}

static void
enable_smileys_for_gtkconv(PidginConversation *gtkconv)
{
	PurpleConversation *conv = gtkconv->active_conv;
	GtkIMHtml *imhtml;

	imhtml = GTK_IMHTML(gtkconv->imhtml);
	gtk_imhtml_set_format_functions(imhtml,
			gtk_imhtml_get_format_functions(imhtml) | GTK_IMHTML_SMILEY);

	if (conv->features & PURPLE_CONNECTION_ALLOW_CUSTOM_SMILEY) {
		pidgin_themes_smiley_themeize_custom(GTK_WIDGET(gtkconv->imhtml));
	} else {
		pidgin_themes_smiley_themeize(GTK_WIDGET(gtkconv->imhtml));
	}
}

static void
conv_created(PidginConversation *gtkconv, gpointer null)
{
	
}

static gboolean
conv_has_smileys(PidginConversation *gtkconv)
{
	GtkIMHtml *imhtml;
	//guint smileys_count;

	if (!gtkconv)
		return TRUE;

	imhtml = GTK_IMHTML(gtkconv->imhtml);
	if (!imhtml)
		return TRUE;
	
	return gtk_imhtml_get_format_functions(imhtml) & GTK_IMHTML_SMILEY;

	//smileys_count = g_hash_table_size(imhtml->smiley_data);
	//gtk_imhtml_set_format_functions(GTK_IMHTML(gtkconv->entry), buttons);
	//imhtml->format_functions & GTK_IMHTML_SMILEY
	//numsmileys_total = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(imhtml), "gtkimhtml_numsmileys_total"));
	//if (numsmileys_total >= 300) {
	//return smileys_count > 0;
}

static void
win32_gtk_menu_item_set_label(GtkMenuItem *menu_item, const gchar *label)
{
	g_return_if_fail(GTK_IS_MENU_ITEM(menu_item));

	if (GTK_IS_LABEL(GTK_BIN(menu_item)->child)) {
		gtk_label_set_label(GTK_LABEL(GTK_BIN(menu_item)->child),
				    label ? label : "");
	}
}

static void toggle_setting(PurpleConversation *conv, gpointer null);

static void
refresh_setting_label(PidginConversation *gtkconv)
{
	PidginWindow *win;
	GList *action_items;
	GtkWidget *menuitem;
	gboolean has_smileys;

	if (!gtkconv)
		return;

	win = gtkconv->win;
	if (!win || !win->window)
		return;

	has_smileys = conv_has_smileys(gtkconv);

	action_items = g_object_get_data(G_OBJECT(win->window), "plugin-actions");
	for(; action_items; action_items = action_items->next) {
		menuitem = action_items->data;
		gpointer callback = g_object_get_data(G_OBJECT(menuitem), "purplecallback");

		if (callback != toggle_setting)
			continue;

		win32_gtk_menu_item_set_label(GTK_MENU_ITEM(menuitem),
				!has_smileys ? _("Enable smileys") : _("Disable smileys"));
	}
}

static void
toggle_setting(PurpleConversation *conv, gpointer null)
{
	PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);

	if (!gtkconv)
		return;

	if (conv_has_smileys(gtkconv)) {
		disable_smileys_for_gtkconv(gtkconv);
	} else {
		enable_smileys_for_gtkconv(gtkconv);
	}

	refresh_setting_label(gtkconv);
}

static void
conv_menu_cb(PurpleConversation *conv, GList **list)
{
	PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);
	GtkIMHtml *imhtml;
	gboolean has_smileys;

	if (!gtkconv)
		return;

	imhtml = GTK_IMHTML(gtkconv->imhtml);
	if (!imhtml)
		return;

	has_smileys = conv_has_smileys(gtkconv);
	PurpleMenuAction *action = purple_menu_action_new(!has_smileys ? _("Enable smileys") : _("Disable smileys"),
			PURPLE_CALLBACK(toggle_setting), NULL, NULL);
	*list = g_list_append(*list, action);
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
	purple_signal_connect(pidgin_conversations_get_handle(), "conversation-displayed",
						plugin, PURPLE_CALLBACK(conv_created), NULL);

	purple_signal_connect(purple_conversations_get_handle(), "conversation-extended-menu",
						plugin, PURPLE_CALLBACK(conv_menu_cb), NULL);
	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	return TRUE;
}

static PurplePluginUiInfo prefs_info = {
	NULL,
	0,
	NULL,

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info = {
	PURPLE_PLUGIN_MAGIC,		/* Magic				*/
	2,		                    /* Purple Major Version	*/
	0,			                /* Purple Minor Version	*/
	PURPLE_PLUGIN_STANDARD,		/* plugin type			*/
	PIDGIN_PLUGIN_TYPE,			/* ui requirement		*/
	0,							/* flags				*/
	NULL,						/* dependencies			*/
	PURPLE_PRIORITY_DEFAULT,	/* priority				*/

	PLUGIN_ID,					/* plugin id			*/
	PLUGIN_NAME,				/* name					*/
	PLUGIN_VERSION,				/* version				*/
	PLUGIN_SUMMARY,				/* summary				*/
	PLUGIN_DESCRIPTION,			/* description			*/
	PLUGIN_AUTHOR,				/* author				*/
	PLUGIN_WEBSITE,				/* website				*/

	plugin_load,				/* load					*/
	plugin_unload,				/* unload				*/
	NULL,						/* destroy				*/

	NULL,						/* ui_info				*/
	NULL,						/* extra_info			*/
	&prefs_info,				/* prefs_info			*/
	NULL,						/* actions				*/

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(PLUGIN_STATIC_NAME, init_plugin, info)
