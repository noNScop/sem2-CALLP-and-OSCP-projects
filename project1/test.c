#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


typedef struct {
    char *key;
    int integer;
    char *string;
    bool is_str;
} KVPair;

typedef struct {
    char *name;
    KVPair *dict;
    size_t size;
    size_t pos;
} Section;
// if is_str==true/false
// data[idx of section].dict[idx of key].string/intiger == value
//
void fill_line(char line[], char *buffer, size_t i, size_t j);
bool add_section(Section **data, size_t *sz, size_t *didx, char *sect);
bool add_kv(Section **data, size_t didx, char *key, char *value);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <PATH-TO-INI-FILE.ini> <section.key>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error");
        return 1;
    }
    //---
    char input_section[strlen(argv[2])+1];
    char input_key[strlen(argv[2])+1];
    size_t len = strlen(argv[2]);
    int g = 0;
    while (argv[2][g] != '.') {
        input_section[g] = argv[2][g];
        g++;
    }
    input_section[g] = '\0';
    g += 1;
    int g2 = 0;
    while (argv[2][g] != '\0') {
        input_key[g2++] = argv[2][g];
        g++;
    }
    input_key[g2] = '\0';
    //---
    size_t bufsize = 1;
    char *buffer = malloc(bufsize * sizeof(char));
    if (buffer == NULL) {
        perror("Error alocating memory");
        return 1;
    }

    int ch;
    size_t pos = 0;
    // Read the file content into the buffer
    while ((ch = fgetc(file)) != EOF) {
        if (pos == bufsize) {
            bufsize *= 2;
            char *temp = realloc(buffer, bufsize * sizeof(char));
            if (temp == NULL) {
                perror("Error reallocating memory");
                free(buffer);
                return 1;
            }

            buffer = temp;
        }

        buffer[pos++] = ch;
    }

    fclose(file);
    buffer[pos] = '\0';

    size_t data_sz = 1;
    size_t didx = 0;
    Section *data = malloc(data_sz * sizeof(Section));

    // Transform data in buffer to make operations easier
    // j indicates the beginning of a line
    size_t j = 0;
    size_t i = 0;
    bool err = false;
    for (; i < pos; i++, j++) {
        // Parse one line at a time
        if (buffer[i] == '\n') {
            char line[j+1];
            fill_line(line, buffer, i-j, i);
            if (line[0] == '[') {
                int tempk=0;
                int lc=0, rc=0;
                char sect[j-1];
                // Get rid of square brackets in sections names
                for (size_t k = 0, l = 0; l < j+1; l++) {
                    if (line[l] == '['){
                        lc++;
                        if (lc>1){
                            perror("Invalid section name in .ini file");
                            return 1;
                        }
                    }
                    else if (line[l] == ']'){
                        rc++;
                        if (rc>1){
                            perror("Invalid section name in .ini file");
                            return 1;
                        }
                    } else {
                        if(isalnum(line[l]) == 0 && line[l] != '-' && line[l] != '\0'){
                            perror("Invalid section name in .ini file");
                            return 1;
                        } else {
                            sect[k++] = line[l];
                            tempk=k;
                        }
                    }
                }
                if (line[tempk] != ']'){
                    perror("Invalid section name in .ini file");
                    return 1;
                }
                sect[tempk]='\0';

                err = add_section(&data, &data_sz, &didx, sect);
                if (err) {
                    free(buffer);
                    for (int i=0; i < didx; i++) {
                        free(data[i].name);
                        free(data[i].dict);
                    }

                    free(data);
                    return 1;
                } else {
                    data[didx-1].size = 1;
                    data[didx-1].pos = 0;
                    data[didx-1].dict = malloc(data[didx-1].size * sizeof(KVPair));
                }

            } else if (line[0] != ';' && line[0] != '\0') {
                char key[strlen(line)+1];
                char value[strlen(line)+1];
                size_t len = strlen(line);
                int k = 0;
                bool flag2=false;
                while (line[k] != ' ' || line[k+1] != '=') {
                    if (isalnum(line[k]) == 0 && line[k] != '-'){
                        perror("Invalid key name in .ini file");
                        return 1;
                    }
                    key[k] = line[k];
                    k++;
                }

                key[k] = '\0';
                k += 3;
                int v = 0;
                while (line[k] != '\0') {
                    value[v++] = line[k];
                    k++;
                }
                value[v] = '\0';
                // --------------------------------
                err = add_kv(&data, didx, key, value);
                if (err) {
                    free(buffer);
                    for (int i=0; i < didx; i++) {
                        free(data[i].name);
                        for (int j=0; j < data[i].pos; j++) {
                            free(data[i].dict[j].key);
                            free(data[i].dict[j].string);
                        }

                        free(data[i].dict);
                    }

                    free(data);
                    return 1;
                }
            }
            // j will be incremented to 0 after the loop iteration
            j = -1;
        }

    }

    free(buffer);

    // continue...
    bool success = false;
    for (int i=0; i < didx; i++) {
        if (strcmp(data[i].name, input_section) == 0) {
            for (int j=0; j < data[i].pos; j++) {
                if (strcmp(data[i].dict[j].key, input_key) == 0) {
                    if (data[i].dict[j].is_str) {
                        printf("%s\n", data[i].dict[j].string);
                    } else {
                        printf("%d\n", data[i].dict[j].integer);
                    }

                    success = true;
                }
            }
        }
    }

    if (!success) {
        printf("Couldn't find provided section.key in %s\n", argv[1]);
    }

    for (int i=0; i < didx; i++) {
        free(data[i].name);
        for (int j=0; j < data[i].pos; j++) {
            free(data[i].dict[j].key);
            if (data[i].dict[j].is_str) {
                free(data[i].dict[j].string);
            }
        }

        free(data[i].dict);
    }

    free(data);
    return 0;
}

void fill_line(char line[], char *buffer, size_t i, size_t j) {
    int idx = 0;
    for (; i < j; i++, idx++) {
        line[idx] = buffer[i];
    }

    line[idx] = '\0';
}

bool add_section(Section **data, size_t *sz, size_t *didx, char *sect) {
    if (*didx == *sz) {
        *sz *= 2;
        Section *temp = realloc(*data, (*sz) * sizeof(Section));
        if (temp == NULL) {
            perror("Error reallocating memory");
            return true;
        }
        *data = temp;
    }

    (*data)[*didx].name = malloc((strlen(sect) + 1) * sizeof(char));
    if ((*data)[*didx].name == NULL) {
        perror("Error allocating memory");
        return true;
    }

    strcpy((*data)[*didx].name, sect);
    (*didx)++;
    return false;
}

bool add_kv(Section **data, size_t didx, char *key, char *value) {
    size_t key_len = strlen(key);
    size_t value_len = strlen(value);
    size_t idx = (*data)[didx-1].pos;
    if (idx == (*data)[didx-1].size) {
        (*data)[didx-1].size *= 2;
        KVPair *temp = realloc((*data)[didx-1].dict, ((*data)[didx-1].size) * sizeof(KVPair));
        if (temp == NULL) {
            perror("Error reallocating memory");
            return true;
        }

        (*data)[didx-1].dict = temp;
    }

    KVPair kvp;
    char *temp = malloc((key_len + 1) * sizeof(char));
    if (temp == NULL) {
        perror("Error reallocating memory");
        return true;
    }

    kvp.key = temp;
    strcpy(kvp.key, key);
    for (int i=0; i < value_len; i++) {
        if (isdigit(value[i])) {
            kvp.is_str = false;
        } else {
            kvp.is_str = true;
            break;
        }
    }

    if (kvp.is_str) {
        char *temp = malloc((value_len + 1) * sizeof(char));
        if (temp == NULL) {
            perror("Error reallocating memory");
            return true;
        }

        kvp.string = temp;
        strcpy(kvp.string, value);
    } else {
        kvp.integer = atoi(value);
    }

    (*data)[didx-1].dict[idx] = kvp;
    (*data)[didx-1].pos += 1;
    return false;
}