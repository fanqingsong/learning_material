#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "html.h"

name_value name_val_pairs[NV_PAIRS];



int get_input(void) {
    int nv_entry_number = 0;
    int got_data = 0;
    char *ip_data = 0;
    int ip_length = 0;
    char tmp_buffer[(FIELD_LEN * 2) + 2];
    int tmp_offset = 0;
    char *tmp_char_ptr;
    int chars_processed = 0;
    

    if (getenv("REQUEST_METHOD") == NULL) {
        send_error("No REQUEST_METHOD environment - use as CGI program!");
        exit(EXIT_FAILURE);
    }
    
        /* discover what type of request we are processing */
    if (strcmp(getenv("REQUEST_METHOD"), "POST") == 0) {
            /* it's a POST type request */
        ip_length = atoi(getenv("CONTENT_LENGTH"));
        ip_data = malloc(ip_length + 1); /* allow for NULL character */
        if (fread(ip_data, 1, ip_length, stdin) != ip_length) {
            send_error("Bad read from stdin");
            return(0);
        }
        ip_data[ip_length] = '\0';
        got_data = 1;        
    }
    
    if (strcmp(getenv("REQUEST_METHOD"), "GET") == 0) {
        ip_length = strlen(getenv("QUERY_STRING"));
        ip_data = malloc(ip_length + 1); /* allow for NULL character */
        strcpy(ip_data, getenv("QUERY_STRING"));
        ip_data[ip_length] = '\0';
        got_data = 1;
    }

    if (!got_data) {
        send_error("No data received");
        return(0);
    }

        /* If we get here we now have the encoded data stored in ip_data */
    
        /* We now need to separate each name=vale part of the string, then
           individually decode each name and value. */

    memset(name_val_pairs, '\0', sizeof(name_val_pairs));
    nv_entry_number = 0;
    tmp_char_ptr = ip_data;    
    while (chars_processed <= ip_length && nv_entry_number < NV_PAIRS) {

            /* copy a single name=value pair to a tmp buffer */
        tmp_offset = 0;
        while (*tmp_char_ptr &&
               *tmp_char_ptr != '&' &&
               tmp_offset < FIELD_LEN) {
            tmp_buffer[tmp_offset] = *tmp_char_ptr;
            tmp_offset++;
            tmp_char_ptr++;
            chars_processed++;
        }
        tmp_buffer[tmp_offset] = '\0';

            /* decode and load the pair */
        load_nv_pair(tmp_buffer, nv_entry_number);

            /* move on to the next name=value pair */
        tmp_char_ptr++;
        nv_entry_number++;
    } /* while chars to process */

    return(1); /* if we get we have loaded some input data */

} /* get_input */



/* send a error string back to the client */
void send_error(char *error_text) {
    html_content();
    html_start("Woops");
    html_text(error_text);
    html_end();
}


/* Decode and load a name=vale string into a name_value pair entry */
/* Assumes name_value array is currently full of NULL characters */
void load_nv_pair(char *tmp_buffer, int nv_entry) {
    int chars_processed = 0;
    char *src_char_ptr;
    char *dest_char_ptr;

    src_char_ptr = tmp_buffer;
    dest_char_ptr = name_val_pairs[nv_entry].name;
    while(*src_char_ptr &&
          *src_char_ptr != '=' &&
          chars_processed < FIELD_LEN) {
        if (*src_char_ptr == '+') *dest_char_ptr = ' ';
        else *dest_char_ptr = *src_char_ptr;
        dest_char_ptr++;
        src_char_ptr++;        
        chars_processed++;
    }
    
    if (*src_char_ptr == '=') {
        src_char_ptr++; /* skip the '=' character */
        dest_char_ptr = name_val_pairs[nv_entry].value;
        chars_processed = 0;
        while(*src_char_ptr &&
              *src_char_ptr != '=' &&
              chars_processed < FIELD_LEN) {
            if (*src_char_ptr == '+') *dest_char_ptr = ' ';
            else *dest_char_ptr = *src_char_ptr;
            dest_char_ptr++;
            src_char_ptr++;        
            chars_processed++;
        }
    }

        /* Now need to decode %XX characters from the two fields */
    unescape_url(name_val_pairs[nv_entry].name);    
    unescape_url(name_val_pairs[nv_entry].value);
        
} /* load_nv_pair */

/* this routine borrowed from the examples that come with the NCSA server */
char x2c(char *what) {
    register char digit;
    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
                                                             
}


/* this routine borrowed from the examples that come with the NCSA server */
void unescape_url(char *url) {
    register int x,y;

    for (x=0,y=0; url[y]; ++x,++y) {
        if ((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y += 2;
        }
    }
    url[x] = '\0';
}


void html_content(void) {
    printf("Content-type: text/html\r\n\r\n");
}

void html_start(const char *title) {
    printf("<HTML>\r\n");
    printf("<HEAD>\r\n");
    printf("<TITLE>%s</TITLE>\r\n", title);
    printf("</HEAD>\r\n");
    printf("<BODY>\r\n");    
    
}

void html_end(void) {
    printf("</BODY>\r\n");
    printf("</HTML>\r\n");        
}

void html_header(int level, const char *header_text) {
    if (level < 1 || level > 6) return;
    if (!header_text) return;
    
    printf("<H%d>%s</H%d>\r\n", level, header_text, level);
}

void html_text(const char *text) {
    printf("%s\r\n", text);
}
        




