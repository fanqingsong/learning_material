/* As usual, we start out with the included files. We'll need functions from the time
 and math libraries for determining the coordinates of the clock hands. */

#include <gnome.h>
#include <time.h>
#include <math.h>

/* Now we #define some constants for our clock. */

#define CANVAS_SIZE 100.0
#define MIDDLE (CANVAS_SIZE/2.0)  
#define SECOND_HAND_LENGTH 40.0
#define MINUTE_HAND_LENGTH 45.0
#define HOUR_HAND_LENGTH 20.0
#define DOT_RADIUS 45.0    /* distance from center of the clock to dots */

/* Next we initialize our global variables. */

GtkWidget *canvas = NULL;
GnomeCanvasItem *second_hand = NULL;
GnomeCanvasItem *hand = NULL;
gboolean secondhand_is_visible = TRUE;
GnomeCanvasPoints *points, *second_hand_points;  /* the arrays that hold the hand coordinates */
GtkWidget *clock_app;

/* Now the prototypes for our functions. */

static void create_dots (int dots, GtkWidget *canvas);
static void change_canvas_scale (GtkAdjustment *adj, gfloat *value);
static void show_preference_dlg (void);
static void show_about_dlg (void);
static void apply_preferences (GnomePropertyBox *property_box, gint page_num,
                               GtkWidget *sh_checkbox);
static gint redraw (gpointer data);

/* Here we define three top level menus, file, settings and help, which hold an 'exit',
 'preferences' and 'about' option, respectively. We link the 'preferences' and 'about'
 items to functions in our program to bring up the appropriate dialog box. */

GnomeUIInfo file_menu[] = {
  GNOMEUIINFO_MENU_EXIT_ITEM(gtk_main_quit, NULL),
  GNOMEUIINFO_END
};

GnomeUIInfo help_menu[] = {
  GNOMEUIINFO_MENU_ABOUT_ITEM(show_about_dlg, NULL),  
  GNOMEUIINFO_END
};

GnomeUIInfo settings_menu[] = {
  GNOMEUIINFO_MENU_PREFERENCES_ITEM(show_preference_dlg, NULL),
  GNOMEUIINFO_END
};

GnomeUIInfo menubar[] = {
  GNOMEUIINFO_MENU_FILE_TREE(file_menu),
  GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
  GNOMEUIINFO_MENU_HELP_TREE(help_menu),
  GNOMEUIINFO_END
};

/* Next we define the toolbar, which holds a single item, an Exit button. */

GnomeUIInfo toolbar[] = {
  GNOMEUIINFO_ITEM_STOCK("Exit", "Exit the application", gtk_main_quit,
                         GNOME_STOCK_PIXMAP_EXIT),
  GNOMEUIINFO_END
};

/* Now we define a function that creates the dots that go round the outside of the clock.
 We'll call it sixty times, one for each minute, and this function will calculate that
 dot's position around the clock and create the CanvasItem. */

static void create_dot( int dots, GtkWidget* canvas)
{
  double angle = dots * M_PI / (360.0/12);
  double x1, y1, x2, y2;
  double size;
  GnomeCanvasItem *dot;

/* Here we select the size of the dot based on where it appears in the clock.
 We make the dots that appear at the 12, 3, 6 and 9 o'clock positions the largest,
 followed by the multiples of five minutes. */

  if ((dots % 15) == 0) {
    size = 2.0; 
  }
  else if ((dots % 5) == 0) {
    size = 1.0;
  }
  else size = 0.5;

  x1 = MIDDLE - size + (DOT_RADIUS * sin (angle));
  y1 = MIDDLE - size + (DOT_RADIUS * cos (angle));
  x2 = MIDDLE + size + (DOT_RADIUS * sin (angle));
  y2 = MIDDLE + size + (DOT_RADIUS * cos (angle));

  dot = gnome_canvas_item_new(gnome_canvas_root (GNOME_CANVAS(canvas)),
                             GNOME_TYPE_CANVAS_ELLIPSE, "x1", x1, "y1", y1, "x2", x2,
                             "y2", y2, "fill_color", "red", NULL);
}

/* Next is the heart of the application, the function that redraws, or more accurately,
 redefines the coordinates of the clock hands based on the current local time.
 For the minute and hour hand we actually use a single line object, defined with
 three points, so that we only need to redraw two canvas items every second,
 the second hand and the hour/minute hand. */

static gint redraw (gpointer data)
{
  struct tm *tm_ptr;
  time_t the_time;
  float second_angle;
  float minute_angle;
  float hour_angle;

  time(&the_time);
  tm_ptr = localtime (&the_time);
  /* See Chapter 4 for an explanation of the time function. */
  second_angle = tm_ptr->tm_sec * M_PI / 30.0;
  /* the angle the second hand makes with the vertical */

/* We set up the second hand coordinate array to draw a line from the middle
 to the point on the clock. */

  second_hand_points->coords[0] = MIDDLE;
  second_hand_points->coords[1] = MIDDLE;
  second_hand_points->coords[2] = MIDDLE + (SECOND_HAND_LENGTH*sin(second_angle));
  second_hand_points->coords[3] = MIDDLE -  (SECOND_HAND_LENGTH * cos (second_angle));

/* Next we calculate the appropriate angles for the hour and minute hands and use
 these to fill the coordinate array. */

  minute_angle = tm_ptr->tm_min * M_PI / 30.0; 
  hour_angle = (tm_ptr->tm_hour % 12) * M_PI / 6.0 +  (M_PI * tm_ptr->tm_min / 360.0);  

  points->coords[0] = MIDDLE + (HOUR_HAND_LENGTH * sin (hour_angle));
  points->coords[1] = MIDDLE - (HOUR_HAND_LENGTH * cos (hour_angle));
  points->coords[2] = MIDDLE;
  points->coords[3] = MIDDLE;
  points->coords[4] = MIDDLE + (MINUTE_HAND_LENGTH * sin (minute_angle));
  points->coords[5] = MIDDLE - (MINUTE_HAND_LENGTH * cos (minute_angle));

/* We check if the hands have been created, if not, we create them, otherwise we give them
 their new set of coordinates. */
  
  if (hand == NULL) {
    hand = (gnome_canvas_item_new(gnome_canvas_root (GNOME_CANVAS (canvas)),
            GNOME_TYPE_CANVAS_LINE, "points",points, "fill_color", "blue",
            "width_units", 3.5, "cap_style", GDK_CAP_ROUND,
            "join_style", GDK_JOIN_ROUND, NULL));

    second_hand = (gnome_canvas_item_new(gnome_canvas_root (GNOME_CANVAS (canvas)),
                   GNOME_TYPE_CANVAS_LINE, "points",second_hand_points,
                   "fill_color", "white", "width_pixels", 2, NULL));
  }
  else
    gnome_canvas_item_set (hand, "points", points, NULL);
  gnome_canvas_item_set (second_hand, "points", second_hand_points, NULL);
}

/* This function creates an about dialog box which appears when we click on 'about' in
 the help menu. */

static void show_about_dlg(void) 
{
  GtkWidget *about;
  const gchar *authors[] = { "Andrew Froggatt", NULL };
  about = gnome_about_new("Gnome Clock", "0.1",
                          "Released under the GNU Public License", authors,
                          ("A simple graphical clock for GNOME"), NULL);
  gtk_widget_show (about);
}

/* Here we create our preference box in which we place a check box and slider widget.
 The GnomePropertyBox widget is derived from the GnomeDialog widget, and has the extra
 functionality of an embedded notebook widget that provides separate 'pages' each with
 a tab, that we can use to separate groups of widgets. GnomePropertyBox also defines
 two new signals: apply and help. apply is sent whenever the user clicks on ok or apply,
 and help is sent when the help button is clicked. We use the signals to provide a
 non-modal box. */

static void show_preference_dlg(void) 
{
  GtkWidget *preferencebox;
  GtkWidget *vbox, *hbox;
  GtkWidget *sh_checkbox;
  GtkObject *adj;
  GtkWidget *hscale;
  GtkWidget *label;

  preferencebox = gnome_property_box_new();
  vbox = gtk_vbox_new(FALSE, 2);
  hbox = gtk_hbox_new(FALSE, 0);
  sh_checkbox = gtk_check_button_new_with_label( "Second hand visible");
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON (sh_checkbox), secondhand_is_visible);
  gtk_box_pack_start (GTK_BOX (vbox), sh_checkbox, FALSE, FALSE, 0);

/* We set up a signal handlers for the check box so that the ok and apply buttons
 become 'sensitive' (unshaded) when the check box is toggled. */

  gtk_signal_connect_object (GTK_OBJECT(sh_checkbox), "toggled",
                             GTK_SIGNAL_FUNC(gnome_property_box_changed),
                             GTK_OBJECT(preferencebox));

  label = gtk_label_new("Clock zoom factor: ");
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

/* Next we create a scale widget that will zoom our canvas, and link two signal handlers
 to the value_changed signal, called when a user has moved the scale widget. */

  adj = gtk_adjustment_new (2.0, 0.1, 7.0, 1.0, 1.0, 1.0);
  /* (default, min, max, step, page, page size) */
  hscale = gtk_hscale_new (GTK_ADJUSTMENT (adj));

  gtk_signal_connect_object (GTK_OBJECT (adj), "value_changed",
                             GTK_SIGNAL_FUNC(gnome_property_box_changed),
                             GTK_OBJECT(preferencebox));

  gtk_signal_connect_object (GTK_OBJECT (adj), "value_changed",
                             GTK_SIGNAL_FUNC (change_canvas_scale),
                             &(GTK_ADJUSTMENT (adj)->value));

  gtk_signal_connect (GTK_OBJECT (preferencebox), "apply",
                      GTK_SIGNAL_FUNC(apply_preferences), sh_checkbox);

  gtk_box_pack_start (GTK_BOX (hbox), hscale, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gnome_dialog_set_parent(GNOME_DIALOG(preferencebox), GTK_WINDOW(clock_app));

  gnome_property_box_append_page(GNOME_PROPERTY_BOX(preferencebox),
                                 vbox, gtk_label_new("General"));

  gtk_widget_show_all(preferencebox);
}

/* Next is the function that gets called whenever the ok or apply buttons in
 the preference window are clicked. page_num refers to the current notebook page,
 to provide future support for a per-page apply button. The idea is that the apply
 signal is sent once for each notebook page, and once with a -1 page_num. We just
 reject any page other than -1. When we receive the -1 page, we make the second
 hand visible or invisible according to the status of the check box in the
 preferences dialog box. We update the global variable secondhand_is_visible so
 that the check box displays the correct state if the property box is closed and
 re-created. Note that the GnomePropertyBox widget takes care of closing the property
 window for us, when ok or close are clicked. */

static void apply_preferences (GnomePropertyBox *property_box, gint page_num, GtkWidget *sh_checkbox)
{
  if (page_num != -1)
    return;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (sh_checkbox) ) == FALSE) {
    gnome_canvas_item_hide(second_hand);
    secondhand_is_visible = FALSE;
  }
  else {
    gnome_canvas_item_show(second_hand);
    secondhand_is_visible = TRUE;
  }
}

/* When the scale widget is moved, we update the canvas scale factor, and adjust
 the size of the window so that the clock fits. */

static void change_canvas_scale (GtkAdjustment *adj, gfloat *value) 
{
   gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (canvas), (double) *value);
   gtk_widget_set_usize(GTK_WIDGET (clock_app),
                    110 * (double) *value, 125 * (double) *value);
}

/* Finally we get to main() where we do the usual initialization and creation of our widgets. */

int main (int argc, char *argv[])
{
  GnomeCanvasItem *clock_outline;
  gint dots;

  gnome_init ("clock", "0.1", argc, argv);
  clock_app = gnome_app_new ("clock", "Gnome Clock");
  gtk_widget_set_usize (clock_app, CANVAS_SIZE, CANVAS_SIZE);
  gtk_widget_push_visual(gdk_imlib_get_visual());
  gtk_widget_push_colormap(gdk_imlib_get_colormap());
  canvas = gnome_canvas_new();
  gtk_widget_pop_visual();
  gtk_widget_pop_colormap();

  gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (canvas), 2);
  second_hand_points = gnome_canvas_points_new(2);
  points = gnome_canvas_points_new(3);

  gtk_signal_connect (GTK_OBJECT (clock_app), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

  gtk_widget_set_usize(clock_app, 220, 300);

  gnome_app_set_contents (GNOME_APP (clock_app), canvas);
  gnome_app_create_menus (GNOME_APP (clock_app), menubar);
  gnome_app_create_toolbar (GNOME_APP (clock_app), toolbar);

  clock_outline = gnome_canvas_item_new (gnome_canvas_root(GNOME_CANVAS (canvas)),
                                         GNOME_TYPE_CANVAS_ELLIPSE,
                                         "x1", 0.0, "y1", 0.0, "x2", CANVAS_SIZE,
                                         "y2", CANVAS_SIZE, "outline_color",
                                         "yellow", "width_units", 4.0, NULL);

/* Here we create the sixty dots to mark out the minutes on our clock. */

  for (dots = 0; dots < 60; dots++) {
    create_dot ( dots, canvas);
  }

/* This is the crucial function that calls redraw every 1000 milliseconds
 to update our clock. */

  gtk_timeout_add(1000, redraw, canvas);
  redraw(canvas);
  /* make sure our hands are created before we display the clock */
  gtk_widget_show_all(clock_app);
  gtk_main();
  return 0;
}

