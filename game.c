#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>
#include <stdint.h>

#include <endian.h>

#define W_WIDTH 80
#define W_HEIGHT 21

#define SAVE 0x01
#define LOAD 0x02

typedef struct room {
    uint8_t xpos;
    uint8_t ypos;
    uint8_t xsize;
    uint8_t ysize;
} room_t;

typedef struct entity {
    uint8_t xpos;
    uint8_t ypos;
    char symbol;
} entity_t;

void get_args(int argc, char *argv[], uint8_t *args);

void generate(entity_t **player, uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t *num_rooms, room_t **rooms, uint16_t *num_u_stairs, entity_t **u_stairs, uint16_t *num_d_stairs, entity_t **d_stairs);

void save(entity_t *player, uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t num_rooms, room_t *rooms, uint16_t num_u_stairs, entity_t *u_stairs, uint16_t num_d_stairs, entity_t *d_stairs);
void load(entity_t **player, uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t *num_rooms, room_t **rooms, uint16_t *num_u_stairs, entity_t **u_stairs, uint16_t *num_d_stairs, entity_t **d_stairs);

void map_rooms(char map[W_HEIGHT][W_WIDTH], uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t num_rooms, room_t *rooms);

void init_map(char map[W_HEIGHT][W_WIDTH]);
void sketch_map(char display[W_HEIGHT][W_WIDTH], char map[W_HEIGHT][W_WIDTH]);

void place(char display[W_HEIGHT][W_WIDTH], entity_t *entities, uint16_t count);
void draw(char display[W_HEIGHT][W_WIDTH]);

int main(int argc, char *argv[])
{
    /* Switch Processing */
    uint8_t args = 0x00;
    get_args(argc, argv, &args);

    /* Seeding */
    srand(time(NULL));

    /* Declarations */
    entity_t *player;
    uint8_t hardness[W_HEIGHT][W_WIDTH];
    uint16_t num_rooms;
    room_t *rooms;
    uint16_t num_u_stairs;
    entity_t *u_stairs;
    uint16_t num_d_stairs;
    entity_t *d_stairs;
    char map[W_HEIGHT][W_WIDTH];
    char display[W_HEIGHT][W_WIDTH];

    /* Saving and Loading */
    if (args & LOAD)
        load(&player, hardness, &num_rooms, &rooms, &num_u_stairs, &u_stairs, &num_d_stairs, &d_stairs);
    else
        generate(&player, hardness, &num_rooms, &rooms, &num_u_stairs, &u_stairs, &num_d_stairs, &d_stairs);
    if (args & SAVE)
        save(player, hardness, num_rooms, rooms, num_u_stairs, u_stairs, num_d_stairs, d_stairs);

    /* Sketching Map */
    init_map(map);
    map_rooms(map, hardness, num_rooms, rooms);

    /* Displaying */
    sketch_map(display, map);
    place(display, player, 1);
    place(display, u_stairs, num_u_stairs);
    place(display, d_stairs, num_d_stairs);
    draw(display);

    /* Cleaning Memory */
    free(player);
    free(rooms);
    free(u_stairs);
    free(d_stairs);
}

void get_args(int argc, char *argv[], uint8_t *args)
{
    uint8_t i;
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--save")) {
            *args |= SAVE;
        }
        else if (!strcmp(argv[i], "--load")) {
            *args |= LOAD;
        }
    }
}

void generate(entity_t **player, uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t *num_rooms, room_t **rooms, uint16_t *num_u_stairs, entity_t **u_stairs, uint16_t *num_d_stairs, entity_t **d_stairs)
{
    /* printf("CREATING:\n"); */
    uint8_t i, j, v;
    *num_rooms = 6 + rand() % 2;
    *rooms = malloc(sizeof(room_t) * *num_rooms);
    do {
        for (i = 0; i < *num_rooms; i++) {
            (*rooms)[i].xsize = 4 + rand() % 10;
            (*rooms)[i].ysize = 3 + rand() % 10;
            (*rooms)[i].xpos = rand() % (W_WIDTH - (*rooms)[i].xsize);
            (*rooms)[i].ypos = rand() % (W_HEIGHT - (*rooms)[i].ysize);
        }
        v = 1;
        for (i = 1; i < *num_rooms && v; i++) {
            for (j = 0; j < i && v; j++) {
                if (!((*rooms)[i].xpos + (*rooms)[i].xsize < (*rooms)[j].xpos || (*rooms)[j].xpos + (*rooms)[j].xsize < (*rooms)[i].xpos)
                        && !((*rooms)[i].ypos + (*rooms)[i].ysize < (*rooms)[j].ypos || (*rooms)[j].ypos + (*rooms)[j].ysize < (*rooms)[i].ypos)) {
                    v = 0;
                }
            }
        }
    } while(!v);
    /*
    for (i = 0; i < *num_rooms; i++) {
        printf("%d : %d : %d : %d\n", (*rooms)[i].xpos, (*rooms)[i].ypos, (*rooms)[i].xsize, (*rooms)[i].ysize);
    }
    */
    *player = malloc(sizeof(entity_t));
    (*player)->xpos = (*rooms)[0].xpos;
    (*player)->ypos = (*rooms)[0].ypos;
    (*player)->symbol = '@';
    *num_u_stairs = 1;
    *u_stairs = malloc(sizeof(entity_t));
    (*u_stairs)->xpos = (*rooms)[2].xpos + 1;
    (*u_stairs)->ypos = (*rooms)[2].ypos + 1;
    (*u_stairs)->symbol = '<';
    *num_d_stairs = 1;
    *d_stairs = malloc(sizeof(entity_t));
    (*d_stairs)->xpos = (*rooms)[*num_rooms - 1].xpos + 1;
    (*d_stairs)->ypos = (*rooms)[*num_rooms - 1].ypos + 1;
    (*d_stairs)->symbol = '>';
    //TODO HARDNESS
}

void save(entity_t *player, uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t num_rooms, room_t *rooms, uint16_t num_u_stairs, entity_t *u_stairs, uint16_t num_d_stairs, entity_t *d_stairs)
{
    /* printf("SAVING:\n"); */
    uint8_t i, j;
    uint8_t w_byte;
    uint16_t w_short;
    uint32_t w;
    char version[13] = "RLG327-@2021";
    char *home = getenv("HOME");
    char *gamedir = ".rlg327";
    char *save = "dungeon";
    char *path = malloc(strlen(home) + strlen(gamedir) + strlen(save) + 3);
    sprintf(path, "%s/%s/%s", home, gamedir, save);
    FILE *f = fopen(path, "w");
    for (i = 0; i < 12; i++) {
        fwrite(&version[i], sizeof(char), 1, f);
    }
    w = htobe32(0);
    fwrite(&w, sizeof(uint32_t), 1, f);
    w = htobe32(1708 + num_rooms * 4 + num_u_stairs * 2 + num_d_stairs * 2);
    fwrite(&w, sizeof(uint32_t), 1, f);
    w_byte = player->xpos;
    fwrite(&w_byte, sizeof(uint8_t), 1, f);
    w_byte = player->ypos;
    fwrite(&w_byte, sizeof(uint8_t), 1, f);
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            w_byte = hardness[i][j];
            fwrite(&w_byte, sizeof(uint8_t), 1, f);
        }
    }
    w_short = htobe16(num_rooms);
    fwrite(&w_short, sizeof(uint16_t), 1, f);
    for (i = 0; i < num_rooms; i++) {
        w_byte = rooms[i].xpos;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
        w_byte = rooms[i].ypos;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
        w_byte = rooms[i].xsize;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
        w_byte = rooms[i].ysize;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
    }
    w_short = htobe16(num_u_stairs);
    fwrite(&w_short, sizeof(uint16_t), 1, f);
    for (i = 0; i < num_u_stairs; i++) {
        w_byte = u_stairs[i].xpos;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
        w_byte = u_stairs[i].ypos;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
    }
    w_short = htobe16(num_d_stairs);
    fwrite(&w_short, sizeof(uint16_t), 1, f);
    for (i = 0; i < num_d_stairs; i++) {
        w_byte = d_stairs[i].xpos;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
        w_byte = d_stairs[i].ypos;
        fwrite(&w_byte, sizeof(uint8_t), 1, f);
    }
    fclose(f);
    free(path);
}

void load(entity_t **player, uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t *num_rooms, room_t **rooms, uint16_t *num_u_stairs, entity_t **u_stairs, uint16_t *num_d_stairs, entity_t **d_stairs)
{
    uint8_t i, j;
    uint16_t count = 0;
    uint8_t r_byte;
    uint16_t r_short;
    uint32_t r;
    char version[13];
    version[12] = '\0';
    char *home = getenv("HOME");
    char *gamedir = ".rlg327";
    char *save = "dungeon";
    char *path = malloc(strlen(home) + strlen(gamedir) + strlen(save) + 3);
    sprintf(path, "%s/%s/%s", home, gamedir, save);
    FILE *f = fopen(path, "r");
    fread(version, sizeof(char), 12, f);
    fread(&r, sizeof(uint32_t), 1, f);
    fread(&r, sizeof(uint32_t), 1, f);
    *player = malloc(sizeof(entity_t));
    fread(&r_byte, sizeof(uint8_t), 1, f);
    (*player)->xpos = r_byte;
    fread(&r_byte, sizeof(uint8_t), 1, f);
    (*player)->ypos = r_byte;
    (*player)->symbol = '@';
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            fread(&r_byte, sizeof(uint8_t), 1, f);
            hardness[i][j] = r_byte;
            count++;
        }
    }
    fread(&r_short, sizeof(uint16_t), 1, f);
    *num_rooms = be16toh(r_short);
    *rooms = malloc(sizeof(room_t) * *num_rooms);
    for (i = 0; i < *num_rooms; i++) {
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*rooms)[i].xpos = r_byte;
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*rooms)[i].ypos = r_byte;
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*rooms)[i].xsize = r_byte;
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*rooms)[i].ysize = r_byte;
    }
    fread(&r_short, sizeof(uint16_t), 1, f);
    *num_u_stairs = be16toh(r_short);
    *u_stairs = malloc(sizeof(entity_t) * *num_u_stairs);
    for (i = 0; i < *num_u_stairs; i++) {
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*u_stairs)[i].xpos = r_byte;
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*u_stairs)[i].ypos = r_byte;
        (*u_stairs)[i].symbol = '<';
    }
    fread(&r_short, sizeof(uint16_t), 1, f);
    *num_d_stairs = be16toh(r_short);
    *d_stairs = malloc(sizeof(entity_t) * *num_d_stairs);
    for (i = 0; i < *num_d_stairs; i++) {
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*d_stairs)[i].xpos = r_byte;
        fread(&r_byte, sizeof(uint8_t), 1, f);
        (*d_stairs)[i].ypos = r_byte;
        (*d_stairs)[i].symbol = '>';
    }
    fclose(f);
    free(path);
}

void map_rooms(char map[W_HEIGHT][W_WIDTH], uint8_t hardness[W_HEIGHT][W_WIDTH], uint16_t num_rooms, room_t *rooms)
{
    uint8_t i, j, k;
    for (i = 0; i < num_rooms; i++) {
        for (j = 0; j < rooms[i].ysize; j++) {
            for (k = 0; k < rooms[i].xsize; k++) {
                map[rooms[i].ypos + j][rooms[i].xpos + k] = '.';
            }
        }
        /* display[rooms[i].ypos][rooms[i].xpos] = '0' + i; */
    }
    //TODO MAP CORRODORS
}

void init_map(char map[W_HEIGHT][W_WIDTH])
{
    uint8_t i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            map[i][j] = ' ';
        }
    }
}

void sketch_map(char display[W_HEIGHT][W_WIDTH], char map[W_HEIGHT][W_WIDTH])
{
    uint8_t i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            display[i][j] = map[i][j];
        }
    }
}

void place(char display[W_HEIGHT][W_WIDTH], entity_t *entities, uint16_t count)
{
    uint8_t i;
    for (i = 0; i < count; i++) {
        display[entities[i].ypos][entities[i].xpos] = entities[i].symbol;
    }
}

void draw(char display[W_HEIGHT][W_WIDTH])
{
    uint8_t i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            printf("%c", display[i][j]);
        }
        printf("\n");
    }
}
