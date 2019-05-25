#include <gnome.h>

int main (int argc, char *argv[])
{
    GtkWidget *app;
    gnome_init ("example", "0.1", argc, argv);
    app = gnome_app_new ("example", "Window Title");
    gtk_widget_show (app);
    gtk_main ();
    return 0;
}

