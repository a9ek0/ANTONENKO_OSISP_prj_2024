#include "../include/utilities.h"

void serialize_array(filetype *fs_root, int *index) {
    if (!fs_root || *index >= MAX_FILES)
        return;

    file_array[*index] = *fs_root;
    (*index)++;

    if (fs_root->valid != 0) {
        for (int i = 0; i < fs_root->num_children; i++) {
            if (fs_root->children[i] != NULL) {
                serialize_array(fs_root->children[i], index);
            }
        }
    }
}

void deserialize_array(int *index, filetype *fs_root) {
    if (*index >= MAX_FILES)
        return;

    *fs_root = file_array[*index];
    (*index)++;
    fs_root->children = malloc(sizeof(filetype *) * fs_root->num_children);

    if (fs_root->valid != 0) {
        for (int i = 0; i < fs_root->num_children; i++) {
            filetype *child = malloc(sizeof(filetype));
            if (child == NULL) {
                return;
            }

            deserialize_array(index, child);
            fs_root->children[i] = child;
        }
    }
}

void serialize_inode_to_file(inode *i, FILE *fp) {
    printf("serialize_inode_to_file\n");
    fwrite(i->datablocks, sizeof(int), 16, fp);
    fwrite(&i->number, sizeof(int), 1, fp);
    fwrite(&i->blocks, sizeof(int), 1, fp);
    fwrite(&i->size, sizeof(int), 1, fp);
    fwrite(&i->permissions, sizeof(mode_t), 1, fp);
    fwrite(&i->user_id, sizeof(uid_t), 1, fp);
    fwrite(&i->group_id, sizeof(gid_t), 1, fp);
    fwrite(&i->a_time, sizeof(time_t), 1, fp);
    fwrite(&i->m_time, sizeof(time_t), 1, fp);
    fwrite(&i->c_time, sizeof(time_t), 1, fp);
    fwrite(&i->b_time, sizeof(time_t), 1, fp);
}

void deserialize_inode_from_file(inode *i, FILE *fp) {
    fread(i->datablocks, sizeof(int), 16, fp);
    fread(&i->number, sizeof(int), 1, fp);
    fread(&i->blocks, sizeof(int), 1, fp);
    fread(&i->size, sizeof(int), 1, fp);
    fread(&i->permissions, sizeof(mode_t), 1, fp);
    fread(&i->user_id, sizeof(uid_t), 1, fp);
    fread(&i->group_id, sizeof(gid_t), 1, fp);
    fread(&i->a_time, sizeof(time_t), 1, fp);
    fread(&i->m_time, sizeof(time_t), 1, fp);
    fread(&i->c_time, sizeof(time_t), 1, fp);
    fread(&i->b_time, sizeof(time_t), 1, fp);
}

void serialize_filetype_to_file(filetype *f, FILE *fp) {
    printf("s: %s\n", f->name);
    fwrite(&f->valid, sizeof(int), 1, fp);
    fwrite(f->path, sizeof(char), 100, fp);
    fwrite(f->name, sizeof(char), 100, fp);
    if (f->inum != NULL) {
        int null_flag = 1;
        fwrite(&null_flag, sizeof(int), 1, fp);

        serialize_inode_to_file(f->inum, fp);
    } else {
        int null_flag = 0;
        fwrite(&null_flag, sizeof(int), 1, fp);
    }
    fwrite(&f->num_children, sizeof(int), 1, fp);
    fwrite(&f->num_links, sizeof(int), 1, fp);
    fwrite(f->type, sizeof(char), 20, fp);
    if (f->num_children > 0) {
        for (int i = 0; i < f->num_children; i++) {
            serialize_filetype_to_file(f->children[i], fp);
        }
    }
}

void deserialize_filetype_from_file(filetype *f, FILE *fp) {
    printf("%s\n", f->name);
    fread(&f->valid, sizeof(int), 1, fp);
    fread(f->path, sizeof(char), 100, fp);
    if (feof(fp)) {
        return;
    }
    fread(f->name, sizeof(char), 100, fp);
    int null_flag;
    fread(&null_flag, sizeof(int), 1, fp);
    if (null_flag == 1) {
        inode *inum = malloc(sizeof(inode));
        deserialize_inode_from_file(inum, fp);
        f->inum = inum;
    } else {
        f->inum = NULL;
    }
    fread(&f->num_children, sizeof(int), 1, fp);
    fread(&f->num_links, sizeof(int), 1, fp);
    fread(f->type, sizeof(char), 20, fp);
    if (f->num_children > 0) {
        f->children = malloc(f->num_children * sizeof(filetype*));
        for (int i = 0; i < f->num_children; i++) {
            filetype *child = malloc(sizeof(filetype));
            deserialize_filetype_from_file(child, fp);
            f->children[i] = child;
        }
    }
}
