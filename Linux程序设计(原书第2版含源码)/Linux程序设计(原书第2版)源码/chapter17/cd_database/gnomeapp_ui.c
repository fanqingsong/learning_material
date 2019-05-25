/* GNOME front end to the CD database 
   developed in Beginning Linux Programming 2nd edition */

#include <gnome.h>
#include <stdlib.h>
#include <stdio.h>

#include "cd_data.h"


/* define local prototypes */ 
static void update_clist(void);
static void add_cd_dialog(void);
static void delete_cd_dialog(void);
static void add_track_dialog(void);
static void delete_track_dialog(void);
static void find_cat_entry(void);
static gint get_row(gchar *catalog);
static void update_track_list(void);
static gint count_track_entries(void);
static void create_main_window(void);
static void set_insensitive(void);
static void show_about_dialog(void);

/* define global variables - a data structure would be better */
GtkWidget *cd_list;
GtkWidget *track_list;
GtkWidget *track_frame;
cdc_entry selected_cdc_entry;
gint selected_track;


/* Menu and toolbar definitions */
GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_MENU_EXIT_ITEM(gtk_main_quit, NULL),
	GNOMEUIINFO_END
};


GnomeUIInfo edit_menu[] = {
	GNOMEUIINFO_MENU_FIND_ITEM(find_cat_entry, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_MENU_ABOUT_ITEM(show_about_dialog, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo menubar[] = {
	GNOMEUIINFO_MENU_FILE_TREE(file_menu),
	GNOMEUIINFO_MENU_EDIT_TREE(edit_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
	GNOMEUIINFO_END
};

GnomeUIInfo toolbar[] = {
	GNOMEUIINFO_ITEM_STOCK("New", "Add a Cd",
						add_cd_dialog,
						GNOME_STOCK_PIXMAP_CDROM),
	GNOMEUIINFO_ITEM_STOCK("Delete", "Delete a Cd",
						delete_cd_dialog,
						GNOME_STOCK_PIXMAP_CLOSE),
	GNOMEUIINFO_ITEM_STOCK("Find", "Find a Cd",
					find_cat_entry,
					GNOME_STOCK_PIXMAP_SEARCH),
	GNOMEUIINFO_SEPARATOR,
	
	GNOMEUIINFO_ITEM_STOCK("Exit", "Exit the application",
					gtk_main_quit,
					GNOME_STOCK_PIXMAP_EXIT),
	GNOMEUIINFO_END
};

static void show_about_dialog()
{
    GtkWidget *about;
    const gchar *authors[] = { "Wrox Press et al", NULL};
    
    about = gnome_about_new("CD Database", "0.1",
			    "GNU Public License",
			    authors,
			    "The GNOME front end to the CD Database developed in 'Beginning Linux Programming 2nd edition'",
			    NULL);
    gtk_widget_show(about);
}

static void select_row_callback(GtkWidget *cd_list, 
					gint row,
					gint column,
					GdkEventButton *event,
					gpointer data)
{	
    gchar *row_text;

    gtk_widget_set_sensitive(track_frame, TRUE); /* unshade the track frame */
    gtk_clist_get_text (GTK_CLIST (cd_list), row, 0, &row_text); /* get the catalog of the selected cd */
    selected_cdc_entry = get_cdc_entry(row_text); /* update our global variable */
    update_track_list(); /* redraw the track list to show the tracks for the selected cd */
}
	
static void update_track_list () /* redraw the track list based on the currently selected cd */
{
    gchar *text[2];
    cdt_entry entry_found;
    gint track_no = 1;
    gchar buffer[70];
	
    gtk_clist_clear (GTK_CLIST (track_list)); /* clear the track list */
    text[0] = buffer;
		
	
    do {
	entry_found = get_cdt_entry(selected_cdc_entry.catalog, track_no);
	if (entry_found.catalog[0]) {
	    sprintf(text[0], "%d", entry_found.track_no); /* place the track number in the first column */
	    text[1] = entry_found.track_txt; /* place the track text in the second column */
	    gtk_clist_append (GTK_CLIST(track_list), text);   /* add the row */
	    track_no++; /* do the next track */
	}
    } while (entry_found.catalog[0]); 
}	

void select_track_row_callback(GtkWidget *cd_list,
				   gint row,
				   gint column,
				   GdkEventButton *event,
				   gpointer data)
{		
	selected_track = row + 1; /* update the selected track global variable */
				  /* rows start from zero, so add one to get the track number */
}
	

static void update_clist() /* update the cd list */
{
    cdc_entry item_found;
    gint first_call = 1;
    gchar *text[4];
    gchar *null_text = ""; /* we get all the cds from the database by searching for a null string */
    gint row;
   	
    gtk_clist_clear (GTK_CLIST (cd_list)); /* clear the list */
  	
    item_found = search_cdc_entry(null_text, &first_call);
    while (item_found.catalog[0] != '\0') { /* add cds row by row */
	    text[0] = item_found.catalog;
	    text[1] = item_found.title;
	    text[2] = item_found.type;
	    text[3] = item_found.artist;
	
	    row = gtk_clist_append (GTK_CLIST(cd_list), text); 
	    item_found = search_cdc_entry(null_text, &first_call);
    }
    gtk_clist_sort(GTK_CLIST(cd_list)); 
}	
 	
static void add_cd_dialog()
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *catalog, *title, *type, *artist;
    cdc_entry new_entry;
    gint i;	

    memset(&new_entry ,'\0', sizeof(new_entry));

    dialog = gnome_dialog_new("Add new CD",
		 GNOME_STOCK_BUTTON_OK,
		 GNOME_STOCK_BUTTON_CLOSE,
		 NULL);
    hbox = gtk_hbox_new (FALSE, 0);
    label = gtk_label_new ("Catalog Entry: ");
    catalog = gtk_entry_new_with_max_length (CAT_CAT_LEN);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), catalog, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    label = gtk_label_new ("Title: ");
    title = gtk_entry_new_with_max_length (CAT_TITLE_LEN);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), title, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    label = gtk_label_new ("Type: ");
    type = gtk_entry_new_with_max_length (CAT_TYPE_LEN);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), type, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    label = gtk_label_new ("Artist: ");
    artist = gtk_entry_new_with_max_length (CAT_ARTIST_LEN);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), artist, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

    gtk_widget_show_all (dialog);

    i = gnome_dialog_run(GNOME_DIALOG (dialog));
 		
    if (i==0) {
 	strncpy(new_entry.catalog, gtk_entry_get_text (GTK_ENTRY (catalog)), CAT_CAT_LEN - 1);
 	strncpy(new_entry.title, gtk_entry_get_text (GTK_ENTRY(title)), CAT_TITLE_LEN - 1);
 	strncpy(new_entry.type, gtk_entry_get_text (GTK_ENTRY(type)), CAT_TYPE_LEN - 1);
    	strncpy(new_entry.artist, gtk_entry_get_text (GTK_ENTRY(artist)), CAT_ARTIST_LEN - 1);
    	
    	add_cdc_entry(new_entry);
    	update_clist();
    }
    gnome_dialog_close (GNOME_DIALOG (dialog));    
}

static void delete_cd_dialog()  
{
    GtkWidget *dialog;
    gint track_no = 1;
    gint i;
    gint delete_ok;
	
    dialog = gnome_message_box_new ("Are you sure you want to delete this entry and all its tracks?",
								GNOME_MESSAGE_BOX_QUESTION,
								GNOME_STOCK_BUTTON_YES,
								GNOME_STOCK_BUTTON_CANCEL,
								NULL);
    gtk_widget_show_all (dialog);
    i = gnome_dialog_run(GNOME_DIALOG (dialog));
    if (i==0) {
	 do {
	      delete_ok = del_cdt_entry(selected_cdc_entry.catalog, track_no);
	      track_no++;
	 } while (delete_ok);
	 del_cdc_entry(selected_cdc_entry.catalog);
	 update_clist();
	 gtk_clist_clear (GTK_CLIST (track_list));
	 set_insensitive();	 
    }

}

static void add_track_dialog()
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *hbox;
    GtkWidget *spin_button, *description;
    GtkObject *spinner_adj;
    cdt_entry new_track;
    gint next_track;
    gint i;
    
    memset(&new_track, '\0', sizeof(new_track));
	
    dialog = gnome_dialog_new("Add a track",
		 GNOME_STOCK_BUTTON_OK,
		 GNOME_STOCK_BUTTON_CLOSE,
		 NULL);
    next_track = count_track_entries() + 1; /* we count the number of tracks for this cd      */
					    /* so we can only add a track with a track number */
					    /* 1 greater than the current number of tracks    */	
    spinner_adj = gtk_adjustment_new((gfloat) (next_track), 1.0, (gfloat) (next_track), 1.0, 1.0, 1.0);
	
    hbox = gtk_hbox_new (FALSE, 2);
    label = gtk_label_new ("Track number: ");
	
    spin_button = gtk_spin_button_new (GTK_ADJUSTMENT (spinner_adj), 1.0, 0.0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spin_button), TRUE);
	
	
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), spin_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    label = gtk_label_new ("Description: ");
    description = gtk_entry_new_with_max_length (CAT_TITLE_LEN);
    gtk_box_pack_end (GTK_BOX (hbox), description, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);

    gtk_widget_show_all (dialog);
    i = gnome_dialog_run(GNOME_DIALOG (dialog));
 
    if (i==0) { /* i==0 when we click ok, so here we add the new track */
	new_track.track_no = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON (spin_button));
					
	strncpy(new_track.track_txt, gtk_entry_get_text (GTK_ENTRY (description)), TRACK_TTEXT_LEN - 1);
	strcpy(new_track.catalog, selected_cdc_entry.catalog);
	add_cdt_entry(new_track);	 
    }
	
    gnome_dialog_close(GNOME_DIALOG (dialog)); /* close the dialog */ 
    update_track_list();  /* and update the track list to show the new addition */
}
	
static void delete_track_dialog()
{
    GtkWidget *dialog;
    gint i;
 	
    dialog = gnome_message_box_new ("Are you sure you want to delete this track and all the higher numbered tracks?",
								GNOME_MESSAGE_BOX_QUESTION,
								GNOME_STOCK_BUTTON_YES,
								GNOME_STOCK_BUTTON_CANCEL,
								NULL);
    gtk_widget_show_all (dialog);
    i = gnome_dialog_run(GNOME_DIALOG (dialog));
    if (i==0) {
	while (del_cdt_entry(selected_cdc_entry.catalog, selected_track)) {
  	selected_track++;
	}
	update_track_list();
	 /* we must delete all higher numbered tracks than the one we select to delete, to ensure */
	 /* that row 'n' contains track 'n+1', which our update functions etc rely on             */		
    }

}

static void reorder_list (GtkCList *clist, gint column, gpointer data)
{	
    gtk_clist_sort(GTK_CLIST(clist)); /* when we click on a column header in the cd list, we reorder*/
				      /* in ascending alphabetical order for that column            */		
    gtk_clist_set_sort_column(GTK_CLIST(clist), column);	
}

static gint count_track_entries() /* calculate the total number of tracks for the currently selected cd */
{
    cdt_entry cdt_found;
    gint track_entries_found = 0;
    gint track_no = 1;
    
    do {
	cdt_found = get_cdt_entry(selected_cdc_entry.catalog, track_no);
	if (cdt_found.catalog[0]) {
	    track_entries_found++;
	    track_no++;
	}
    }
    while (cdt_found.catalog[0]);
    return (track_entries_found);
}

static void find_cat_entry(void) /* search for a catalog entry                                         */
{				 /* first we search the database, then we find the entry in our list   */
				 /* because the list may have been reordered since we created the list */
				 /* This way, we don't have to keep track of which row each cd is in   */
    cdc_entry item_found;
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *search_entry;
    GtkWidget *hbox;
    gint i;
    gint first_call = 1;
	
    dialog = gnome_dialog_new("Search",
		 GNOME_STOCK_BUTTON_OK,
		 GNOME_STOCK_BUTTON_CLOSE,
		 NULL);
    hbox = gtk_hbox_new (FALSE, 0);
    label = gtk_label_new ("Search for catalog entry: ");
    search_entry = gtk_entry_new_with_max_length (CAT_CAT_LEN);	 
	
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox), search_entry, FALSE, FALSE, 0);	 
    gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show_all (dialog);
    i = gnome_dialog_run(GNOME_DIALOG (dialog));
 
    if (i==0) {
	item_found = search_cdc_entry(gtk_entry_get_text (GTK_ENTRY (search_entry)), &first_call);
	if (item_found.catalog[0] != '\0') { 
	    gtk_clist_select_row(GTK_CLIST (cd_list), get_row(item_found.catalog),0); /* select the found entry */
	    gtk_clist_moveto(GTK_CLIST (cd_list), get_row(item_found.catalog), 0, 0.5, 0.5); /* move to the entry */
	}
    }
    gnome_dialog_close(GNOME_DIALOG (dialog));   
}
	
static gint get_row(gchar *catalog) /* find which row a particular entry is in, returning the row number */
{
    gint row;
    gchar *row_text;
    
    for (row = 0; row <= 9999; row++) {
	 gtk_clist_get_text (GTK_CLIST (cd_list), row, 0, &row_text);
	 if (strcmp(row_text, catalog)==0) {
	     break;
	 }
    }
    return row;
} 

static void set_insensitive() /* when we unselect a cd, make sure we can't add or delete a track */
			      /* by shading the entire frame */
{
gtk_widget_set_sensitive(track_frame, FALSE);
}

static void create_main_window() /* function that creates the main window */
{
    GtkWidget *hbox;
    GtkWidget *vbox, *vbox2;
    GtkWidget *scrolled_window;
    GtkWidget *scrolled_window2;
    GtkWidget *add_button, *delete_button;
    GtkWidget *app;
    
    gchar     *titles[4] = {"Catalog Entry", "Title", "Type", "Artist"}; /* cd list column titles */
    gchar     *tracks[2] = {"Track no.","Title"}; /* track list column titles */
    
    app = gnome_app_new ("example", "CD Database");
    
    hbox = gtk_hbox_new (FALSE, 0);
    vbox = gtk_vbox_new (FALSE, 0);
    vbox2 = gtk_vbox_new (FALSE, 0);
    cd_list = gtk_clist_new_with_titles (4, titles);
    track_list = gtk_clist_new_with_titles (2, tracks);
		
    gtk_clist_set_column_width(GTK_CLIST(cd_list), 1, 200); /* set sensible column widths */
    gtk_clist_set_column_width(GTK_CLIST(cd_list), 2, 50);
    gtk_clist_set_column_width(GTK_CLIST(cd_list), 3, 100);
	
    gtk_clist_set_column_justification (GTK_CLIST(cd_list), 0, GTK_JUSTIFY_CENTER); /* justify leftmost column */
    gtk_clist_set_column_justification (GTK_CLIST(track_list), 0, GTK_JUSTIFY_CENTER);
	
    gtk_clist_set_sort_type(GTK_CLIST(cd_list), GTK_SORT_ASCENDING); /* sort rows in ascending alphabetical order */
    gtk_clist_set_sort_type(GTK_CLIST(track_list), GTK_SORT_ASCENDING);
    gtk_clist_set_sort_column(GTK_CLIST(cd_list), 0);
    gtk_clist_set_sort_column(GTK_CLIST(track_list), 0);
    gtk_clist_column_titles_passive (GTK_CLIST (track_list)); /* set the track list column titles passive, */
							      /* to show we can't click on them */
	
    scrolled_window = gtk_scrolled_window_new (NULL, NULL); /* create scrolled window widgets to place the clists in */
    scrolled_window2 = gtk_scrolled_window_new (NULL, NULL);
	
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
				    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC); /* select scrollbar policy to show          */
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window2),  /* a vertical scrollbar only when necessary */
				    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
   
    track_frame = gtk_frame_new("Tracks"); /* create the frame to hold the track list and buttons */
    add_button = gtk_button_new_with_label("Add..."); /* create the add track button */
    delete_button = gtk_button_new_with_label("Delete"); /* create the delete track button */
	
    gtk_box_pack_start(GTK_BOX(hbox), add_button, FALSE, FALSE, 0); 
    gtk_box_pack_start(GTK_BOX(hbox), delete_button, FALSE, FALSE, 0);
	
    gtk_container_add(GTK_CONTAINER(scrolled_window), cd_list);			   							   
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
	
	
    gtk_container_add(GTK_CONTAINER(scrolled_window2), track_list);
    gtk_box_pack_start(GTK_BOX(vbox2), scrolled_window2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(track_frame), vbox2);
    gtk_box_pack_start(GTK_BOX(vbox), track_frame, TRUE, TRUE, 0);
    
    gtk_container_border_width(GTK_CONTAINER (vbox), 10);
    gnome_app_create_menus (GNOME_APP (app), menubar);
    gnome_app_create_toolbar (GNOME_APP (app), toolbar);
	
	
    gnome_app_set_contents(GNOME_APP(app), vbox);
    
    /* add our callbacks */
    gtk_signal_connect (GTK_OBJECT (app), "delete_event",
                        GTK_SIGNAL_FUNC (gtk_main_quit),
                        NULL);
			     
    gtk_signal_connect(GTK_OBJECT (cd_list), 
		       "select_row",
		       GTK_SIGNAL_FUNC(select_row_callback),
		       NULL);
				   
    gtk_signal_connect(GTK_OBJECT (track_list),
		       "select_row",
		       GTK_SIGNAL_FUNC(select_track_row_callback),
		       NULL);
				
    gtk_signal_connect(GTK_OBJECT (cd_list), /* when a column title is clicked, reorder the list */
		       "click-column",
		       GTK_SIGNAL_FUNC(reorder_list),
		       NULL);
		       
    gtk_signal_connect(GTK_OBJECT (cd_list), /* when a cd is unselected, shade the track frame */
		       "unselect_row",
		       GTK_SIGNAL_FUNC(set_insensitive),
		       NULL);    
/* add the button functionality */
    	
    gtk_signal_connect (GTK_OBJECT (add_button), 
			"clicked",
			GTK_SIGNAL_FUNC (add_track_dialog),
			NULL);
	
    gtk_signal_connect(GTK_OBJECT (delete_button),
		       "clicked",
		       GTK_SIGNAL_FUNC (delete_track_dialog), 
		       NULL);
				
    
    set_insensitive();
    gtk_widget_show_all (app);
}
    
int main(int argc, char *argv[])
{ 
    GtkWidget *dialog, *error_dialog;
    gint i;
		
    gnome_init ("example", "0.1", argc, argv);
   
    if (!database_initialize(0)) {	
	dialog = gnome_message_box_new ("Sorry, unable to initialize database\nCreate new database?",
					GNOME_MESSAGE_BOX_QUESTION,
					GNOME_STOCK_BUTTON_YES,
					GNOME_STOCK_BUTTON_CANCEL,
					NULL);
	gtk_widget_show_all(dialog);
	i = gnome_dialog_run(GNOME_DIALOG (dialog));
	if (i==0) {
	    if (!database_initialize(1)) {
		error_dialog = gnome_message_box_new ("Failed to initialize database",
						      GNOME_MESSAGE_BOX_ERROR,
						      GNOME_STOCK_BUTTON_OK,
						      NULL);
		gtk_widget_show_all(error_dialog);
		return 0;
	    }
	}
	else return 0;
    }						      	
    
    create_main_window();
    update_clist();
  
    gtk_main ();
    return 0;
}
