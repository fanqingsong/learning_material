#include <gnome.h>

static void button_clicked(GtkWidget *button, gpointer data)
{
    char *string = data;
    g_print(string);
}

int main (int argc, char *argv[])
{
    GtkWidget *app;
    GtkWidget *button;

    gnome_init ("example", "0.1", argc, argv);
    app = gnome_app_new ("example", "Window Title");
    button = gtk_button_new_with_label ("Hello,\n GNOME world!");
    gtk_signal_connect (GTK_OBJECT (app), "delete_event",
                        GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_signal_connect (GTK_OBJECT (button), "clicked",
                        GTK_SIGNAL_FUNC (button_clicked), "Ouch!\n");
    gnome_app_set_contents (GNOME_APP (app), button);
    gtk_widget_show_all (app);
    gtk_main ();
    return 0;
}

