/*
  This is the data stucture definition for the CD database.

  It defines structures and sizes for the two tables that comprise
  the database.

  */

/* The catalogue table */
#define CAT_CAT_LEN       30
#define CAT_TITLE_LEN     70
#define CAT_TYPE_LEN      30
#define CAT_ARTIST_LEN    70

typedef struct {
    char catalogue[CAT_CAT_LEN + 1];
    char title[CAT_TITLE_LEN + 1];
    char type[CAT_TYPE_LEN + 1];
    char artist[CAT_ARTIST_LEN + 1];
} cdc_entry;


/* The tracks table, one entry per track */
#define TRACK_CAT_LEN     CAT_CAT_LEN
#define TRACK_TTEXT_LEN   70

typedef struct {
    char catalogue[TRACK_CAT_LEN + 1];
    int  track_no;
    char track_txt[TRACK_TTEXT_LEN + 1];
} cdt_entry;



/* For convenience we also put the prototype for the functions
   to access the data here */

/* Initialision and termination functions */
int database_initialise(const int new_database);
void database_close(void);

/* two for simple data retrival */
cdc_entry get_cdc_entry(const char *cd_catalogue_ptr);
cdt_entry get_cdt_entry(const char *cd_catalogue_ptr, const int track_no);

/* two for data addition */
int add_cdc_entry(const cdc_entry entry_to_add);
int add_cdt_entry(const cdt_entry entry_to_add);

/* two for data deletion */
int del_cdc_entry(const char *cd_catalogue_ptr);
int del_cdt_entry(const char *cd_catalogue_ptr, const int track_no);

/* one search function */
cdc_entry search_cdc_entry(const char *cd_catalogue_ptr, int *first_call_ptr);




