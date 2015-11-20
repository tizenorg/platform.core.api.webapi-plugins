#include "auto_gen_interface.h"
#include "stdio.h"


#define BUS_NAME    "org.tizen.system.deviced"
#define OBJECT_PATH "/Org/Tizen/System/DeviceD/Display"


static _auto_genOrgTizenSystemDevicedDisplayIface interface;


gboolean handle_current_brightness_cb(_auto_genOrgTizenSystemDevicedDisplay *object,
                                  GDBusMethodInvocation *invocation)
{
    g_print("handle_current_brightness_cb-\n");

    _auto_gen_org_tizen_system_deviced_display_complete_current_brightness(object, invocation, 123456);

    return TRUE;
}


gboolean handle_custom_brightness_cb(_auto_genOrgTizenSystemDevicedDisplay *object,
                                    GDBusMethodInvocation *invocation){

    _auto_gen_org_tizen_system_deviced_display_complete_custom_brightness(object, invocation, 987);

    g_print("\n-----------------------------\n");
    g_print("handle_custom_brightness_cb\n");
    return TRUE;
}


gboolean handle_hold_brightness_cb(
  _auto_genOrgTizenSystemDevicedDisplay *object,
  GDBusMethodInvocation *invocation,
  gint arg_brightness){


     g_print("handle_hold_brightness_cb\n");

    _auto_gen_org_tizen_system_deviced_display_complete_hold_brightness(object, invocation, 8882);
    return TRUE;
}


gboolean handle_release_brightness_cb (
  _auto_genOrgTizenSystemDevicedDisplay *object,
  GDBusMethodInvocation *invocation){



    _auto_gen_org_tizen_system_deviced_display_complete_release_brightness(object, invocation, 991);


    return TRUE;
}


gboolean handle_unlock_state_cb (
  _auto_genOrgTizenSystemDevicedDisplay *object,
  GDBusMethodInvocation *invocation,
  const gchar *arg_state,
  const gchar *arg_option){


    g_print("\n-----------------------------\n");
    g_print("handle_unlock_state_cb\n");

    g_print("arg_state: %s\n", arg_state);
    g_print("arg_option: %s\n", arg_option);


    _auto_gen_org_tizen_system_deviced_display_complete_unlock_state(object, invocation, 9999);

    return TRUE;


}


gboolean handle_lock_state_cb (   _auto_genOrgTizenSystemDevicedDisplay *object,
                                  GDBusMethodInvocation *invocation,
                                  const gchar *arg_state,
                                  const gchar *arg_option1,
                                  const gchar *arg_option2,
                                  gint arg_timeout){


    g_print("arg_state  : %s\n", arg_state);
    g_print("arg_option1: %s\n", arg_option1);
    g_print("arg_option2: %s\n", arg_option2);
    g_print("arg_timeout: %d\n", arg_timeout);


    _auto_gen_org_tizen_system_deviced_display_complete_lock_state(object,invocation,34343);

    return TRUE;

}




static void on_bus_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
    g_print("Bus acquired: %s\n", name);

    // Step 2 -> create object path

    _auto_genOrgTizenSystemDevicedDisplay *object = NULL;
    GError *error = NULL;

    object = _auto_gen_org_tizen_system_deviced_display_skeleton_new();

    if (!g_dbus_interface_skeleton_export (G_DBUS_INTERFACE_SKELETON (object),
                                            connection,
                                            OBJECT_PATH,
                                            &error))
    {
        printf("error");
        return;
     /* handle error */
    }

    g_signal_connect(object, "handle-current-brightness", G_CALLBACK(interface.handle_current_brightness), NULL);
    g_signal_connect(object, "handle-unlock-state",       G_CALLBACK(interface.handle_unlock_state), NULL);
    g_signal_connect(object, "handle-custom-brightness",  G_CALLBACK(interface.handle_custom_brightness), NULL);
    g_signal_connect(object, "handle-hold-brightness",    G_CALLBACK(interface.handle_hold_brightness ), NULL);
    g_signal_connect(object, "handle-release-brightness", G_CALLBACK(interface.handle_release_brightness), NULL);
    g_signal_connect(object, "handle-lock-state",         G_CALLBACK(interface.handle_lock_state ), NULL);



}

static void on_name_acquired (GDBusConnection *connection,
                  const gchar     *name,
                  gpointer         user_data)
{
  g_print ("Acquired the name %s\n", name);
}

static void on_name_lost (GDBusConnection *connection,
              const gchar     *name,
              gpointer         user_data)
{
  g_print ("Lost the name %s\n", name);
}

int main(){

    interface.handle_current_brightness = handle_current_brightness_cb;
    interface.handle_custom_brightness  = handle_custom_brightness_cb;
    interface.handle_hold_brightness    = handle_hold_brightness_cb;
    interface.handle_release_brightness = handle_release_brightness_cb;
    interface.handle_unlock_state       = handle_unlock_state_cb;
    interface.handle_lock_state         = handle_lock_state_cb;



    GMainLoop *loop;
    guint id;
    loop = g_main_loop_new (NULL, FALSE);

    id = g_bus_own_name (G_BUS_TYPE_SYSTEM,
                            BUS_NAME,
                            G_BUS_NAME_OWNER_FLAGS_NONE,
                            on_bus_acquired,
                            on_name_acquired,
                            on_name_lost,
                            loop,
                            NULL);

    g_main_loop_run (loop);

    g_bus_unown_name (id);
    g_main_loop_unref (loop);

    return 0;
}

