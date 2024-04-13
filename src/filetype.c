#include "../include/filetype.h"

filetype *filetype_from_path(const char *path) {
    char curr_folder[100];
    char *path_name = malloc(strlen(path) + 2);
    strcpy(path_name, path);

    filetype *curr_node = root;

    if (strcmp(path_name, "/") == 0)
        return curr_node;

    if (path_name[0] != '/') {
        printf("INCORRECT PATH\n");
        exit(1);
    } else {
        path_name++;
    }

    if (path_name[strlen(path_name) - 1] == '/') {
        path_name[strlen(path_name) - 1] = '\0';
    }

    char *index;
    int flag;

    while (strlen(path_name) != 0) {
        index = strchr(path_name, '/');

        if (index != NULL) {
            strncpy(curr_folder, path_name, index - path_name);
            curr_folder[index - path_name] = '\0';

            flag = 0;
            for (int i = 0; i < curr_node->num_children; i++) {
                if (strcmp(curr_node->children[i]->name, curr_folder) == 0) {
                    curr_node = curr_node->children[i];
                    flag = 1;
                    break;
                }
            }
            if (flag == 0)
                return NULL;
        } else {
            strcpy(curr_folder, path_name);
            for (int i = 0; i < curr_node->num_children; i++) {
                if (strcmp(curr_node->children[i]->name, curr_folder) == 0) {
                    curr_node = curr_node->children[i];
                    return curr_node;
                }
            }
            return NULL;
        }
        path_name = index + 1;
    }

    return curr_node;
}