#ifndef ANTONENKO_OSISP_PRJ_2024_UTILITIES_H
#define ANTONENKO_OSISP_PRJ_2024_UTILITIES_H

#include "../include/filetype.h"

typedef struct filetype filetype;

void serialize_array(filetype *queue, int *front, int *rear, int *index);

#endif //ANTONENKO_OSISP_PRJ_2024_UTILITIES_H
