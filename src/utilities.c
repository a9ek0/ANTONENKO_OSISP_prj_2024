#include "../include/utilities.h"

void tree_to_array(filetype *queue, int *front, int *rear, int *index) {
    while (*rear >= *front && *index <= 30) {
        filetype curr_node = queue[*front];
        (*front)++;
        file_array[*index] = curr_node;
        (*index)++;

        if (*index < 6) {
            if (curr_node.valid) {
                int i;
                for (i = 0; i < curr_node.num_children && *rear >= *front; i++) {
                    queue[*rear] = *(curr_node.children[i]);
                    (*rear)++;
                }
                while (i < 5) {
                    filetype waste_node;
                    waste_node.valid = 0;
                    queue[*rear] = waste_node;
                    (*rear)++;
                    i++;
                }
            } else {
                int i = 0;
                while (i < 5) {
                    filetype waste_node;
                    waste_node.valid = 0;
                    queue[*rear] = waste_node;
                    (*rear)++;
                    i++;
                }
            }
        }
    }
}