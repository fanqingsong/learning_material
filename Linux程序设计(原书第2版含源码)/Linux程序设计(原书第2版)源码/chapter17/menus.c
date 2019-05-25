/* We create a callback function to output text when an item is selected. */

#include <gnome.h>

static void callback (GtkWidget *button, gpointer data)
{
    g_print ("Item Selected");
}

/* Now we create an array of two items to be placed in the file menu, an option
 that calls callback and an exit option. */

GnomeUIInfo file_menu[] = {
    GNOMEUIINFO_ITEM_NONE ("A Menu Item","This is the statusbar info", callback),
    GNOMEUIINFO_MENU_EXIT_ITEM(gtk_main_quit, NULL),
    GNOMEUIINFO_END
};

/* Then we create the menu structure with one top level menu, the file menu,
 that points to the array we've just created. */

GnomeUIInfo menubar[] = {
    GNOMEUIINFO_MENU_FILE_TREE(file_menu),
    GNOMEUIINFO_END
};

/* Similarly for the toolbar, we create an array that holds two items,
 a print and exit button. */

GnomeUIInfo toolbar[] = {
    GNOMEUIINFO_ITEM_STOCK("Print", "This is another tooltip",
                           callback, GNOME_STOCK_PIXMAP_PRINT),
    GNOMEUIINFO_ITEM_STOCK("Exit", "Exit the application",
                           gtk_main_quit, GNOME_STOCK_PIXMAP_EXIT),
    GNOMEUIINFO_END
};

/* Finally we create the menus and toolbar and add them to the window. */

int main(int argc, char *argv[])
{
    GtkWidget *app;

    gnome_init ("example", "0.1", argc, argv);
    app = gnome_app_new ("example", "simple toolbar and menu");
    gnome_app_create_menus (GNOME_APP (app), menubar);
    gnome_app_create_toolbar (GNOME_APP (app), toolbar);
    gtk_widget_show_all (app);
    gtk_main ();
    return 0;
}
