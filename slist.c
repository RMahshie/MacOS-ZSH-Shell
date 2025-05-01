/**
 * @file slist.c
 * @author CS3650 staff
 *
 * A simple linked list of strings.
 *
 * This might be useful for directory listings and for manipulating paths.
 */

#include <string.h>
#include <stdlib.h>
#include "slist.h"

// function to create a new list node with the given text and link to the rest of the list
slist_t*
s_cons(const char* text, slist_t* rest)
{
    // allocate memory for a new list node
    slist_t* xs = malloc(sizeof(slist_t));
    // duplicate the input text and assign it to the node's data
    xs->data = strdup(text);
    // set the next pointer to the rest of the list
    xs->next = rest;
    // return the newly created node
    return xs;
}

// function to free the memory allocated for the list
void
s_free(slist_t* xs)
{
    if (xs) {
        // recursively free the next nodes in the list
        s_free(xs->next);
        // free the duplicated text
        free(xs->data);
        // free the current node
        free(xs);
    }
}

// function to split a string into a list based on a delimiter
slist_t*
s_split(const char* text, char delim)
{
    // if the input string is empty, return null
    if (*text == 0) {
        return 0;
    }

    int plen = 0;
    // find the length of the next segment up to the delimiter
    while (text[plen] != 0 && text[plen] != delim) {
        plen += 1;
    }

    int skip = 0;
    // determine if the delimiter was found and should be skipped
    if (text[plen] == delim) {
        skip = 1;
    }

    // recursively split the remaining string after the delimiter
    slist_t* rest = s_split(text + plen + skip, delim);

    // allocate memory for the current segment
    char* part = malloc(plen + 1);
    // copy the segment into the allocated memory
    memcpy(part, text, plen);
    // null-terminate the copied segment
    part[plen] = 0;

    // create a new list node with the current segment and link it to the rest
    return s_cons(part, rest);
}
