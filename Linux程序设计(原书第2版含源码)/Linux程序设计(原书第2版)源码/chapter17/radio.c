/* First we declare the widgets, and create an empty radio group using GSList. */

#include <gnome.h>

int main (int argc, char *argv[])
{
    GtkWidget *app;
    GtkWidget *button1, *button2;
    GtkWidget *radio1, *radio2, *radio3, *radio4;
    GtkWidget *vbox1, *vbox2;
    GtkWidget *hbox;
    GSList *group = NULL;
    gnome_init ("example", "0.1", argc, argv);
    app = gnome_app_new ("example", "Music choices");

/* We give the window a border and create our packing boxes. */

    gtk_container_border_width (GTK_CONTAINER (app), 20);
    vbox1 = gtk_vbox_new (FALSE, 0);
    vbox2 = gtk_vbox_new (FALSE, 0);
    hbox =  gtk_hbox_new (FALSE, 0);

/* We add two check buttons to the first vertical packing box:. */

    button1 = gtk_check_button_new_with_label( "Orchestra");
    gtk_box_pack_start (GTK_BOX (vbox1), button1, FALSE, FALSE, 0);
    button2 = gtk_check_button_new_with_label ("Conductor");
    gtk_box_pack_start (GTK_BOX (vbox1), button2, FALSE, FALSE, 0);

/* And add four radio buttons to the second box, adding each new button to the group list. */

    radio1 = gtk_radio_button_new_with_label (group, "Strings");
    gtk_box_pack_start (GTK_BOX (vbox2), radio1, FALSE, FALSE, 0);
    group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
    radio2 = gtk_radio_button_new_with_label (group, "Wind");
    gtk_box_pack_start (GTK_BOX (vbox2), radio2, FALSE, FALSE, 0);
    group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio2));
    radio3 = gtk_radio_button_new_with_label (group, "Brass");
    gtk_box_pack_start (GTK_BOX (vbox2), radio3, FALSE, FALSE, 0);
    group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio3));
    radio4 = gtk_radio_button_new_with_label (group, "Percussion");
    gtk_box_pack_start (GTK_BOX (vbox2), radio4, FALSE, FALSE, 0);
    group = gtk_radio_button_group (GTK_RADIO_BUTTON (radio4));

/* Lastly, we add an exit handler and put the packing boxes together. */

    gtk_signal_connect (GTK_OBJECT (app), "delete_event",
                        GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_container_add (GTK_CONTAINER (hbox), vbox1);
    gtk_container_add (GTK_CONTAINER (hbox), vbox2);
    gnome_app_set_contents (GNOME_APP (app), hbox);
    gtk_widget_show_all (app);
    gtk_main ();
    return 0;
}

