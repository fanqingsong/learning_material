Beginning Linux Programming 2nd Edition - Chapter 17
----------------------------------------------------

GNOME user interface to the CD Database
---------------------------------------

This is the documentation of the GNOME front end to the CD database which is first written with a line based user interface in chapter 7.

The GNOME user interface program is a direct replacement for the file app_ui.c and therefore the database application can be compiled in the same fashion as the original program, with an updated makefile.

As in chapter 7, it should only be necessary to type 'make' to compile the application.

The Requirements
----------------

A graphical user interface to a CD database suggests an approach where
the CDs, and Tracks for each CD are displayed in separate areas to distinguish their relationship. 

(In database language, the CD catalog field is the primary key, and we have a 'one to many' relationship between the CDs and Tracks)

We need buttons to add and delete both CDs and Tracks, as well as a 'Search' button. So that it's clear whether an 'Add' button refers to a CD or a Track, we'll put buttons that refer to CDs on a toolbar and buttons that act on Tracks on the main window.

To summarize, we require:

1) Separate areas for CD and Track display
2) A Toolbar holding CD add and delete buttons
3) Buttons on the main window that add and delete tracks
4) A Search facility to locate CDs by their catalog field
5) An 'About' box

The Implementation
------------------

GTK+ has a widget perfect for this application, called the CList widget. The CList widget is a multi-column list widget, with each column optionally having a title which can be made active, allowing us to bind a fuction to its selection.

Cells can contain a pixmap, text or both, but we stick to using text only here.

We'll use two of these widgets, one to hold the list of CDs and the other a list of Tracks. The CD CList will have four columns for the Catalog, title, type and artist, and the Track Clist will have two columns holding the track number, and track description. 

The CList widget has functions that allow us to add and remove individual rows to the list, and clear the entire list. We can also specify the width and justification of each column in the list.
The CList widget can deal with selections, both of individual and multiple rows.

You can find complete documentation of the CList widget in the GTK tutorial section of both the GTK+ and GNOME web sites, www.gtk.org and www.gnome.org.

When the user clicks on a row of a CList, that row is automatically 'selected' and highlighted, and the 'select_row' signal is generated. 

We fill the CD CList with complete list of CDs, and bind the 'select_row' signal of the CD CList to a function that updates the Track CList with the contents of that CD's tracks contained in the database.

When a CD is deselected, the 'unselect_row' signal is called, which we use to shade the Track CList and the add and delete Track buttons, since it doesn't make sense to be able to browse the tracks, or add or delete a track to a 'non-existent' CD.

How it works
------------

The front end is a very simple implementation of a CD database, and putting a graphical user interface to the 'back end' developed in chapter 7 shows up the limitations of the original design.

The program GNOME_ui.c starts by attempting to initialize the database, and offers to create a new database if it fails. If the user doesn't wish to create a new database, or the creation fails, the program quits. 

We then create the main window, adding menus and a toolbar. The CList widget doesn't have support for scrollbars itself, but by placing them in a ScrollWindow widget, we add a vertical scrollbar to both the CLists. 

The Track CList, and the add and delete track buttons are grouped together in a frame widget for two reasons. Firstly, a frame around the track list and its buttons makes clear the purpose of the buttons, and secondly, we can shade the entire group simply by setting the sensitivity of the frame to False.

We add CDs to the CD CList by recursively calling search_cdc_entry with a NULL search string, which returns every cdc entry. Then we lookup the corresponding catalog, title, type and artist field of that cdc entry and and create the row for that CD, using gtk_clist_append(). We know we've completed the list when search_cdc_entry() returns NULL.

The 'select_row' signal of the CD CList is bound to a function which updates the track CList based on the row selected. We get the cdc entry of the selected row by passing the text of the catalog field of the selected row (got by the function gtk_clist_get_text()) to get_cdc_entry(). 

We can then use that cdc entry in the function update_track_list() to recover all the tracks for that CD and add them to the CList. 

The column titles of the CD CList are by default sensitive, meaning that they emit signals when clicked. We bind this 'click_column' signal to a function that reorders the CD CList in ascending alphabetical order in the column clicked, meaning we can sort by Catalog, Title, Type or Artist by clicking on the appropriate column title.

The is achieved using the functions gtk_clist_set_sort_type() which sets first of all whether we want to sort in ascending or descending order, and then calling gtk_clist_set_sort_column() to sort by a particular column.

A limitation of the database as it stands is that we cannot, say, in a track list of 10 tracks, delete track number five, and leave tracks 6  to 10 alone, since when the CList was next updated, and the update track list searched for track five, the get_cdt_entry() function would return NULL and our search would stop filling only four rows of the track CList. 

To stop this happening, when a track is deleted, higher numbered tracks are automatically deleted as well, so in the above situation, tracks 6 to 10 are deleted as well. 

There are fuctions to create the necessary dialog boxes for when we wish to add or delete CDs or tracks. 

There's hopefully no need for any more explanation, since GTK+ has such sensibly named functions that browsing GTK+ source code is often the best information on how a program works. 


 



