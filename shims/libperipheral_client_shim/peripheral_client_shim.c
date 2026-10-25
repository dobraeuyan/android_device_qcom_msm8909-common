typedef void (*pm_event_notifier)(void *client_data, int event);

int pm_client_register(pm_event_notifier notifier, void *client_data,
                       const char *img_name, const char *client_name,
                       int *subsys_state, void **pm_handle)
{
    (void)notifier;
    (void)client_data;
    (void)img_name;
    (void)client_name;

    if (subsys_state != 0)
        *subsys_state = 0;
    if (pm_handle != 0)
        *pm_handle = 0;

    return 1;
}

int pm_client_connect(void *pm_handle) { (void)pm_handle; return 0; }
int pm_client_disconnect(void *pm_handle) { (void)pm_handle; return 0; }
int pm_client_unregister(void *pm_handle) { (void)pm_handle; return 0; }
int pm_client_event_acknowledge(void *pm_handle) { (void)pm_handle; return 0; }
