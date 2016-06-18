#include <libintl.h>
#include <locale.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include <libupower-glib/upower.h>
#include <libappindicator/app-indicator.h>

#include "butticon.h"


/* Default interval for refresh (in seconds).  */
#define DEFAULT_UPDATE_INTERVAL 5

/* Singleton GApplication to hold actions.  */
static GApplication *app;

/* Singleton upower interface.  */
static UpClient* up;

/* Singleton appindicator interface.  */
static AppIndicator *indicator;


/* Command line options.  */
static struct config_s {
  gboolean a_version;
  gint update_interval;
  gboolean symbolic;
} config;


static gint get_options (int argc, char **argv)
{
    GError *error = NULL;

    gchar *icon_type_string = NULL;
    GOptionContext *option_context;
    GOptionEntry option_entries[] = {
      { "version", 'v', 0, G_OPTION_ARG_NONE, &config.a_version,
	N_("Display the version"), NULL },
      { "update-interval", 'u', 0, G_OPTION_ARG_INT, &config.update_interval,
	N_("Set update interval (in seconds)"), NULL },
      { "symbolic", 's', 0, G_OPTION_ARG_NONE, &config.symbolic,
	N_("Use symbolic icons"), NULL },
      { NULL }
    };

    config.update_interval = DEFAULT_UPDATE_INTERVAL;

    option_context = g_option_context_new (NULL);
    g_option_context_add_main_entries (option_context, option_entries, BUTTICON_NAME);

    if (g_option_context_parse (option_context, &argc, &argv, &error) == FALSE) {
        g_printerr (_("Cannot parse command line arguments: %s\n"), error->message);
        g_error_free (error); error = NULL;
        return 1;
    }

    if (config.update_interval <= 0) {
        config.update_interval = DEFAULT_UPDATE_INTERVAL;
        g_printerr (_("Invalid update interval! It has been reset to default (%d seconds)\n"),
		    DEFAULT_UPDATE_INTERVAL);
    }

    g_option_context_free (option_context);

    /* option : display the version */

    if (config.a_version == TRUE) {
        g_print (_(BUTTICON_NAME ": the last battery tray icon you'll ever need\n"));
        g_print (_("version %s\n"), BUTTICON_VERSION_STRING);
        exit(0);
    }
    gtk_init (&argc, &argv);

    return 0;
}


/* Mpfh.  On i3bar the symbolic icons are not visible due to unthemed
   colors.  Map the icon names here.  Result is statically allocated
   and only valid until the next time the function is called.  */
const char *
map_icon_name(const char *icon_name)
{
  if (config.symbolic)
    return icon_name;

#define SYMBOLIC "-symbolic"
#define SYMBOLIC_LEN (sizeof (SYMBOLIC) - 1)
  int len = strlen(icon_name);
  if (!strcmp(&icon_name[len - SYMBOLIC_LEN], SYMBOLIC))
    {
      static char* new_name;
      if (new_name)
	free(new_name);
      new_name = strdup(icon_name);
      if (!new_name)
	return icon_name;
      new_name[len - SYMBOLIC_LEN] = '\0';
      return new_name;
    }
  return icon_name;	  
}


static void
cmd_lock (GtkButton* btn, gpointer user_data)
{
  g_warning("lock not implemented yet"); // dm-tool
}


static void
cmd_suspend (GtkButton* btn, gpointer user_data)
{
  g_warning("suspend not implemented yet"); // systemd
}


static void
cmd_hibernate (GtkButton* btn, gpointer user_data)
{
  g_warning("hibernate not implemented yet"); // systemd
}


static void
_menu_new_add_device(gpointer data, gpointer user_data)
{
  UpDevice* dev = (UpDevice*) data;
  GtkMenu* menu = (GtkMenu*) user_data;
  GtkWidget *item = NULL;

  up_device_refresh_sync(dev, NULL, NULL);
  item = gtk_menu_item_new_with_label(up_device_to_text(dev));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
#if 0
  g_signal_connect (G_OBJECT (item), "activate",
		    G_CALLBACK(cmd_battery),
		    NULL);
#endif
#if 0  
 Device: /org/freedesktop/UPower/devices/line_power_AC
    native-path:          AC
    power supply:         yes
   updated:              Sat 18 Jun 2016 12:25:11 AM CEST (5821 seconds ago)
   has history:          no
   has statistics:       no
     line-power
   warning-level:       none
   online:              yes
   icon-name:          'ac-adapter-symbolic'

   Device: /org/freedesktop/UPower/devices/battery_BAT0
   native-path:          BAT0
   vendor:               SMP
   model:                00HW023
   serial:               2928
   power supply:         yes
   updated:              Sat 18 Jun 2016 02:01:15 AM CEST (57 seconds ago)
   has history:          yes
   has statistics:       yes
     battery
   present:             yes
   rechargeable:        yes
   state:               fully-charged
   warning-level:       none
   energy:              23.5 Wh
   energy-empty:        0 Wh
   energy-full:         24.53 Wh
   energy-full-design:  23.54 Wh
   energy-rate:         0 W
   voltage:             12.815 V
   percentage:          95%
   capacity:            100%
   technology:          lithium-polymer
   icon-name:          'battery-full-charged-symbolic'

   Device: /org/freedesktop/UPower/devices/battery_BAT1
   native-path:          BAT1
   vendor:               SANYO
   model:                01AV405
   serial:               629
   power supply:         yes
   updated:              Sat 18 Jun 2016 02:01:11 AM CEST (62 seconds ago)
   has history:          yes
   has statistics:       yes
     battery
   present:             yes
   rechargeable:        yes
   state:               fully-charged
   warning-level:       none
   energy:              27.41 Wh
   energy-empty:        0 Wh
   energy-full:         27.41 Wh
   energy-full-design:  26.33 Wh
   energy-rate:         0 W
   voltage:             12.979 V
   percentage:          100%
   capacity:            100%
   technology:          lithium-ion
#endif      
}


static GtkMenu*
menu_new(GPtrArray* devices)
{
  GtkMenu *menu = GTK_MENU(gtk_menu_new());
  GtkWidget *item = NULL;

  if (devices)
    {
      g_ptr_array_foreach(devices, _menu_new_add_device, menu);
    }
  
  item = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

  item = gtk_menu_item_new_with_label(_("Lock"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect (G_OBJECT (item), "activate",
		    G_CALLBACK(cmd_lock),
		    NULL);

  item = gtk_menu_item_new_with_label(_("Suspend"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect (G_OBJECT (item), "activate",
		    G_CALLBACK(cmd_suspend),
		    NULL);

  item = gtk_menu_item_new_with_label(_("hibernate"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  g_signal_connect (G_OBJECT (item), "activate",
		    G_CALLBACK(cmd_hibernate),
		    NULL);

  gtk_widget_show_all(GTK_WIDGET(menu));

  return menu;
}


static gboolean
indicator_update (void)
{
  UpDevice *dev = up_client_get_display_device(up);
  up_device_refresh_sync(dev, NULL, NULL);
  GValue val = G_VALUE_INIT;
  g_value_init(&val, G_TYPE_STRING);
  g_object_get_property(G_OBJECT(dev), "icon-name", &val);
  const char *icon_name = g_value_get_string(&val);
  icon_name = map_icon_name(icon_name);

  // No accessibility text in upower for now.
  app_indicator_set_icon_full (indicator, icon_name, icon_name);

  /* GtkBuilder GActions don't work with libappindicator :( */
  GtkMenu *menu = menu_new(up_client_get_devices(up));
  app_indicator_set_menu (indicator, GTK_MENU (menu));  

  return TRUE;
}


static void
indicator_init(void)
{
  indicator = app_indicator_new (BUTTICON_ID,
				 map_icon_name("battery-symbolic"),
				 APP_INDICATOR_CATEGORY_HARDWARE);
  app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_title (indicator, BUTTICON_NAME);

  /* GtkBuilder+GActions don't work with libappindicator :( */
  GtkMenu *menu = menu_new(NULL);  
  app_indicator_set_menu (indicator, menu);  
}


static gboolean
_indicator_change(gpointer data)
{
  gboolean* update_pending_ptr = (gboolean*) data;
  g_warning("UPDATE");

  if (update_pending_ptr)
    *update_pending_ptr = FALSE;

  indicator_update();
  return FALSE;
}  

static void
indicator_change(gpointer data, gpointer user_data)
{
  static gboolean update_pending = FALSE;

  if (!update_pending)
    {
      g_warning("REGISTER");
      update_pending = TRUE;
      g_timeout_add_seconds(5, _indicator_change, &update_pending);
    }
}


static void
create_tray_icon (void)
{
  indicator_init();
  indicator_update ();

  g_signal_connect (up, "notify", G_CALLBACK (indicator_change), NULL);

#if 0
  GPtrArray* devices = up_client_get_devices(up);
  for (int i = 0; i < devices->len; i++)
    {
      UpDevice* dev = g_ptr_array_index(devices, i);
      g_signal_connect (dev, "notify", G_CALLBACK (indicator_change), NULL);
    }
  #endif
  //g_timeout_add_seconds (config.update_interval, (GSourceFunc) indicator_update, NULL);
}


int
main(int argc, char *argv[])
{
  app = g_application_new(BUTTICON_ID, G_APPLICATION_IS_SERVICE);
  
  up = up_client_new();
  const gchar* up_version = up_client_get_daemon_version(up);
  g_debug("Connected to upower daemon version %s", up_version);
  
  gboolean lid_closed = up_client_get_lid_is_closed (up);
  gboolean on_battery = up_client_get_on_battery (up);
  g_debug("status: %s %s", lid_closed ? "lid_closed" : "", on_battery ? "on_battery" : "");

  setlocale (LC_ALL, "");
  // bindtextdomain (BUTTICON_NAME, NLSDIR);
  bind_textdomain_codeset (BUTTICON_NAME, "UTF-8");
  textdomain (BUTTICON_NAME);

  gint ret = get_options (argc, argv);
  if (ret)
    return ret;

  create_tray_icon();
  gtk_main();
  
  return 0;
}
