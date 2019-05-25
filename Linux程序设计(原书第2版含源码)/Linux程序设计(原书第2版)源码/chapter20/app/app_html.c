#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cd_data.h"
#include "html.h"

const char *title = "HTML CD Database";
const char *req_one_entry = "CAT";

void process_no_entry(void);
void process_cat(const char *option, const char *title);
void space_to_plus(char *str);

int main(int argc, char *argv[]) {
    
    if (!database_initialise(0)) {
        html_content();
        html_start(title);
        html_text("Sorry, database could not initialize");
        html_text("<P>");
        html_text("Please mail <A HREF=\"mailto:webmaster@anyhost.com\">webmaster</A> for assistance");
        html_end();
        exit(EXIT_SUCCESS);
    }
    
    if (!get_input()) {
        database_close();
        html_content();
        html_start(title);
        html_text("Sorry, METHOD not POST or GET");
        html_text("Please mail <A HREF=\"mailto:webmaster@anyhost.com\">webmaster</A> for assistance");
        html_end();
        exit(EXIT_SUCCESS);
    }


    html_content();
    html_start(title);
    if (strcmp(name_val_pairs[0].name, req_one_entry) == 0) {
            /* the value part is the current catalogue entry */
        process_cat(name_val_pairs[0].name, name_val_pairs[0].value);

    }
    else {
            /* No catalogue entry selected */
        process_no_entry();        
    }
    html_end();
    database_close();    
    exit(EXIT_SUCCESS);
}


/* We got here because no entry is selected.
   Show a standard screen
   */
void process_no_entry(void) {
    char tmp_buffer[120];
    char tmp2_buffer[120];    
    cdc_entry item_found;
    int first_call = 1;
    int items_found = 1;

    html_header(1, "CD database listing");
    html_text("Select a title to show the tracks");
    html_text("<BR><HR><BR>");    


    while(items_found) {
        item_found = search_cdc_entry("", &first_call);
        if (item_found.catalogue[0] == '\0') {
            items_found = 0;
        }
        else {
            sprintf(tmp_buffer, "Catalogue: %s", item_found.catalogue);
            html_text(tmp_buffer);
            html_text("<BR><BR>");
            strcpy(tmp2_buffer, item_found.catalogue);
            space_to_plus(tmp2_buffer);
            sprintf(tmp_buffer, "Title: <A HREF=\"/cgi-bin/cddb/cdhtml?CAT=%s\">%s</A>", tmp2_buffer, item_found.title);
            html_text(tmp_buffer);
            html_text("<BR><BR>");
            sprintf(tmp_buffer, "Type: %s", item_found.type);
            html_text(tmp_buffer);                
            html_text("<BR><BR>");
            sprintf(tmp_buffer, "Artist: %s", item_found.artist);
            html_text(tmp_buffer);                
            html_text("<BR><HR><BR>");
        }
    } /* while */
}


/*
  we get here with addition parameters now parsed */
void process_cat(const char *name_type, const char *cat_title) {
    char tmp_buffer[120];    
    cdc_entry cdc_item_found;
    cdt_entry cdt_item_found;    
    int first = 1;
    int track_no = 1;
    
    if (strcmp(name_type, req_one_entry) == 0) {
        /* Screen with one entry, defined by cat_title to be shown */
        html_header(1, "CD catalogue entry");
        html_text("<BR><BR>");
        html_text("Return to: <A HREF=\"/cgi-bin/cddb/cdhtml\">list</A>");
        html_text("<BR><BR>");        
        html_text("<HR>");
        
        cdc_item_found = search_cdc_entry(cat_title, &first);
        if (cdc_item_found.catalogue[0] == '\0') {
            html_text("Sorry, couldn't find item ");
            html_text(cat_title);
        }
        else {
        
            sprintf(tmp_buffer, "Catalogue: %s", cdc_item_found.catalogue);
            html_text(tmp_buffer);
            html_text("<BR>");                
            sprintf(tmp_buffer, "Title: %s", cdc_item_found.title);
            html_text(tmp_buffer);
            html_text("<BR>");
            sprintf(tmp_buffer, "Type: %s", cdc_item_found.type);
            html_text(tmp_buffer);                
            html_text("<BR>");
            sprintf(tmp_buffer, "Artist: %s", cdc_item_found.artist);
            html_text(tmp_buffer);
            html_text("<BR>");

            html_text("<OL>");
            cdt_item_found = get_cdt_entry(cdc_item_found.catalogue, track_no);
            while(cdt_item_found.catalogue[0] != '\0') {
                sprintf(tmp_buffer, "<LI> %s", cdt_item_found.track_txt);
                html_text(tmp_buffer);
                track_no++;
                cdt_item_found = get_cdt_entry(cdc_item_found.catalogue, track_no);                
            }
            html_text("</OL>");
            html_text("<HR>");
        }
    }
}


void space_to_plus(char *str) {
    while (*str) {
        if (*str == ' ') *str = '+';
        str++;
    }
}



