/* We first define enter_pressed, a callback function that gets called
 every time return is pressed. */

#include <gnome.h>

static void enter_pressed(GtkWidget *button, gpointer data)
{
    GtkWidget *text_entry = data;
    char *string = gtk_entry_get_text(GTK_ENTRY (text_entry));
    g_print(string);
}

/* Next we define variables, initialize GNOME and create a horizontal packing box. */

int main (int argc, char *argv[])
{
    GtkWidget *app;
    GtkWidget *text_entry;
    GtkWidget *label;
    GtkWidget *hbox;
    gchar *text;
    gnome_init ("example", "0.1", argc, argv);
    app = gnome_app_new ("example", "entry widget");
    gtk_container_border_width (GTK_CONTAINER (app), 5);
    hbox = gtk_hbox_new (FALSE, 0);

/* Now we create a label, set its alignment, and add it to the packing box. */

    label = gtk_label_new("Password: ");
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1.0);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

/* Next we create the entry box and set its visibility to false, which makes
 its contents appear as asterisks. */

    text_entry = gtk_entry_new();
    gtk_entry_set_visibility (GTK_ENTRY (text_entry), FALSE);
    gtk_box_pack_start (GTK_BOX (hbox), text_entry, FALSE, FALSE, 0);

/* Finally we set up the signal handlers, and place the packing box in the window. */

    gtk_signal_connect (GTK_OBJECT (app), "delete_event",
    GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_signal_connect (GTK_OBJECT (text_entry), "activate",
                        GTK_SIGNAL_FUNC (enter_pressed), text_entry);
    gnome_app_set_contents( GNOME_APP (app), hbox);
    gtk_widget_show_all (app);
    gtk_main ();
    return 0;
}

