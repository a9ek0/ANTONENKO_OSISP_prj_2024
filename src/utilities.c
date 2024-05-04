#include "../include/utilities.h"

void tree_to_array(filetype *queue, int *front, int *rear, int *index) {

    if (rear < front)
        return;
    if (*index > 30)
        return;


    filetype curr_node = queue[*front];
    *front += 1;
    file_array[*index] = curr_node;
    *index += 1;

    if (*index < 6) {

        if (curr_node.valid) {
            int n = 0;
            int i;
            for (i = 0; i < curr_node.num_children; i++) {
                if (*rear < *front)
                    *rear = *front;
                queue[*rear] = *(curr_node.children[i]);
                *rear += 1;
            }
            while (i < 5) {
                filetype waste_node;
                waste_node.valid = 0;
                queue[*rear] = waste_node;
                *rear += 1;
                i++;
            }
        } else {
            int i = 0;
            while (i < 5) {
                filetype waste_node;
                waste_node.valid = 0;
                queue[*rear] = waste_node;
                *rear += 1;
                i++;
            }
        }
    }

    tree_to_array(queue, front, rear, index);

}
