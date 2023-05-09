
#include "autocomplete.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compare terms by text, ascending
int compare_terms_by_text(const void* leftptr, const void* rightptr) {
    struct term left = *(struct term*)leftptr;
    struct term right = *(struct term*)rightptr;

    return strcmp(left.term, right.term);
}

// compare terms by weight, descending
int compare_terms_by_weight(const void* leftptr, const void* rightptr) {
    struct term left = *(struct term*)leftptr;
    struct term right = *(struct term*)rightptr;

    if (left.weight > right.weight) {
        return -1;
    } else if (left.weight < right.weight) {
        return 1;
    }
    return 0;
}

void read_in_terms(struct term **pterms, int *pnterms, char *filename) {
    FILE* fd;
    fd = fopen(filename, "r");

    if (fd == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    // The first line in the file is the number of terms we need to read
    fscanf(fd, "%d", pnterms);

    // Allocate memory for the terms
    // struct term* 
    term *terms = (struct term*)malloc(*pnterms * sizeof(struct term));
    *pterms = terms;

    for (int i = 0; i < *pnterms; i++) {
        struct term *t = &terms[i];

        // read the weight
        fscanf(fd, "%lf ", &t->weight);

        // read the term
        if (fgets(t->term, 200, fd) == NULL) {
            printf("Error reading term\n");
            exit(1);
        }

        // Remove the newline character
        t->term[strlen(t->term) - 1] = '\0';
    }

    // Sort lexicographically
    qsort(terms, *pnterms, sizeof(struct term), compare_terms_by_text);
}

int lowest_match(struct term *terms, int nterms, char *substr) {
    int startIdx = 0;
    int endIdx = nterms - 1;
    int midIdx = (startIdx + endIdx) / 2;

    // printf("substr: '%s' (size: %d)\n", substr, (int)(strlen(substr)));
    // binary search
    while (startIdx < endIdx) {
        midIdx = (startIdx + endIdx) / 2;

        int cmp = strncmp(substr, terms[midIdx].term, strlen(substr));
        // printf(
        //     "checking [%d,%d] of %d => mid is %d => %s => %d\n",
        //     startIdx,
        //     endIdx,
        //     nterms,
        //     midIdx,
        //     terms[midIdx].term,
        //     cmp
        // );

        if (cmp <= 0) {
            // the term is less than the search term, so we need to search the lower half
            // we need to include the midIdx in the search, because it might be the lowest match
            endIdx = midIdx;
        } else {
            // the term is greater than the search term, so we need to search the upper half
            startIdx = midIdx + 1;
        }
    }
    // printf("lowest exit [%d,%d] => mid is %d => %s\n", startIdx, endIdx, midIdx, terms[midIdx].term);

    // If the term doesn't match, return -1 to signal that there are no matches
    if (startIdx < nterms && strncmp(substr, terms[startIdx].term, strlen(substr)) != 0) {
        return -1;
    }

    return startIdx;
}

int highest_match(struct term *terms, int nterms, char *substr) {
    int startIdx = 0;
    int endIdx = nterms - 1;
    int midIdx;

    // printf("substr: '%s' (size: %d)\n", substr, (int)(strlen(substr)));
    // binary search
    while (startIdx < endIdx) {
        midIdx = (startIdx + endIdx) / 2;

        int cmp = strncmp(substr, terms[midIdx].term, strlen(substr));
        // printf(
        //     "checking [%d,%d] of %d => mid is %d => %s => %d\n",
        //     startIdx,
        //     endIdx,
        //     nterms,
        //     midIdx,
        //     terms[midIdx].term,
        //     cmp
        // );

        if (cmp < 0) {
            // the term is less than the search term, so we need to search the lower half
            endIdx = midIdx - 1;
        } else if (startIdx == midIdx) {
            // If we are stuck, the search term doesn't match anything, and we can exit
            return -1;
        } else {
            // the term is greater than the search term, so we need to search the upper half
            // we need to include the midIdx in the search, because it might be the highest match
            startIdx = midIdx;
        }
    }
    // printf("highest exit [%d,%d] => mid is %d => %s\n", startIdx, endIdx, midIdx, terms[midIdx].term);

    // If the term doesn't match, return -1 to signal that there are no matches
    if (endIdx > -1 && strncmp(substr, terms[endIdx].term, strlen(substr)) != 0) {
        return -1;
    }

    return endIdx;
}

void autocomplete(struct term **answer, int *n_answer, struct term *terms, int nterms, char *substr) {
    int lowestIdx = lowest_match(terms, nterms, substr);
    int highestIdx = highest_match(terms, nterms, substr);

    if (lowestIdx == -1 || highestIdx == -1) {
        *n_answer = 0;
        return;
    }

    *n_answer = highestIdx - lowestIdx + 1;
    *answer = (struct term*)malloc(*n_answer * sizeof(struct term));

    for (int i = 0; i < *n_answer; i++) {
        (*answer)[i] = terms[lowestIdx + i];
    }

    qsort(*answer, *n_answer, sizeof(struct term), compare_terms_by_weight);
}
