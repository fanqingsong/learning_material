#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FIELD_LEN 250 /* how long each name or value can be */
#define NV_PAIRS 200  /* how many name=value pairs we can process */

typedef struct name_value_st {
    char name[FIELD_LEN + 1];
    char value[FIELD_LEN + 1];
} name_value;

name_value name_val_pairs[NV_PAIRS];

static int get_input(void);
static void send_error(char *error_text);
static void load_nv_pair(char *tmp_buffer, int nv_entry_number_to_load);
static char x2c(char *what);
static void unescape_url(char *url);


int main(int argc, char *argv[])
{
    int nv_entry_number = 0;

    if (!get_input()) {
        exit(EXIT_FAILURE);
    }

    printf("Content-type: text/plain\r\n");
    printf("\r\n");

    printf("Information decoded was:-\r\n\r\n");
    while(name_val_pairs[nv_entry_number].name[0] != '\0') {
        printf("Name=%s, Value=%s\r\n",
               name_val_pairs[nv_entry_number].name,
               name_val_pairs[nv_entry_number].value);
        nv_entry_number++;
    }
    printf("\r\n");
    exit(EXIT_SUCCESS);
}


static int get_input(void)
{
    int nv_entry_number = 0;
    int got_data = 0;
    char *ip_data = 0;
    int ip_length = 0;
    char tmp_buffer[(FIELD_LEN * 2) + 2];
    int tmp_offset = 0;
    char *tmp_char_ptr;
    int chars_processed = 0;

    tmp_char_ptr = getenv("REQUEST_METHOD");
    if (tmp_char_ptr) {
        if (strcmp(tmp_char_ptr, "POST") == 0) {
            tmp_char_ptr = getenv("CONTENT_LENGTH");
            if (tmp_char_ptr) {
                ip_length = atoi(tmp_char_ptr);
                ip_data = malloc(ip_length + 1); /* allow for NULL character */
                if (fread(ip_data, 1, ip_length, stdin) != ip_length) {
                    send_error("Bad read from stdin");
                    return(0);
                }
                ip_data[ip_length] = '\0';
                got_data = 1;        
            }
        }
    }

    tmp_char_ptr = getenv("REQUEST_METHOD");
    if (tmp_char_ptr) {
        if (strcmp(getenv("REQUEST_METHOD"), "GET") == 0) {
            tmp_char_ptr = getenv("QUERY_STRING");
            if (tmp_char_ptr) {
                ip_length = strlen(tmp_char_ptr);
                ip_data = malloc(ip_length + 1); /* allow for NULL character */
                strcpy(ip_data, getenv("QUERY_STRING"));
                ip_data[ip_length] = '\0';
                got_data = 1;
            }
        }
    }

    if (!got_data) {
        send_error("No data received");
        return(0);
    }

    if (ip_length <= 0) {
        send_error("Input length not > 0");
        return(0);
    }


    memset(name_val_pairs, '\0', sizeof(name_val_pairs));
    tmp_char_ptr = ip_data;
    while (chars_processed <= ip_length && nv_entry_number < NV_PAIRS) {
        /* copy a single name=value pair to a tmp buffer */
        tmp_offset = 0;
        while (*tmp_char_ptr && *tmp_char_ptr != '&' &&
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
    }
    return(1);
}


static void send_error(char *error_text)
{
    printf("Content-type: text/plain\r\n");
    printf("\r\n");
    printf("Woops:- %s\r\n", error_text);
}


/* Assumes name_val_pairs array is currently full of NULL characters */
static void load_nv_pair(char *tmp_buffer, int nv_entry)
{
    int chars_processed = 0;
    char *src_char_ptr;
    char *dest_char_ptr;

    /* get the part before the '=' sign */
    src_char_ptr = tmp_buffer;
    dest_char_ptr = name_val_pairs[nv_entry].name;
    while(*src_char_ptr && *src_char_ptr != '=' &&
          chars_processed < FIELD_LEN) {
        /* Change a '+' to a ' ' */
        if (*src_char_ptr == '+')
            *dest_char_ptr = ' ';
        else
            *dest_char_ptr = *src_char_ptr;
        dest_char_ptr++;
        src_char_ptr++;
        chars_processed++;
    }

    /* skip the '=' character */
    if (*src_char_ptr == '=') {
        /* get the part after the '=' sign */
        src_char_ptr++;
        dest_char_ptr = name_val_pairs[nv_entry].value;
        chars_processed = 0;
        while(*src_char_ptr && *src_char_ptr != '=' &&
               chars_processed < FIELD_LEN) {
            /* Change a '+' to a ' ' */
            if (*src_char_ptr == '+')
                *dest_char_ptr = ' ';
            else
                *dest_char_ptr = *src_char_ptr;
            dest_char_ptr++;
            src_char_ptr++;
            chars_processed++;
        }
    }

    /* Now need to decode %XX characters from the two fields */
    unescape_url(name_val_pairs[nv_entry].name);
    unescape_url(name_val_pairs[nv_entry].value);
}


/* this routine borrowed from the examples that come with the NCSA server */
static void unescape_url(char *url)
{
    int x,y;

    for (x=0,y=0; url[y]; ++x,++y) {
        if ((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y += 2;
        }
    }
    url[x] = '\0';
}

/* this routine borrowed from the examples that come with the NCSA server */
static char x2c(char *what)
{
    register char digit;

    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return(digit);
}

