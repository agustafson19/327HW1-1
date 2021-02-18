#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string.h>
#include <stdint.h>

#include <endian.h>

#define W_WIDTH 80
#define W_HEIGHT 24

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

void getArgs(int argc, char *argv[], uint8_t *args);

void createrooms(room_t **rooms, uint8_t *numrooms);

void save(room_t *rooms, uint8_t numrooms, entity_t *player);
void load(room_t **rooms, uint8_t *numrooms, entity_t **player);

void maprooms(char map[W_HEIGHT][W_WIDTH], room_t *rooms, uint8_t numrooms);

void initmap(char map[W_HEIGHT][W_WIDTH]);
void sketchmap(char display[W_HEIGHT][W_WIDTH], char map[W_HEIGHT][W_WIDTH]);

void place(char display[W_HEIGHT][W_WIDTH], entity_t *entities, uint8_t count);
void draw(char display[W_HEIGHT][W_WIDTH]);

int main(int argc, char *argv[]) {
    /* Switch Processing */
    uint8_t args = 0x00;
    getArgs(argc, argv, &args);

    /* Seeding */
    srand(time(NULL));

    /* Declarations */
    uint8_t numrooms;
    room_t *rooms;
    entity_t *player;
    char map[W_HEIGHT][W_WIDTH];
    char display[W_HEIGHT][W_WIDTH];

    /* Saving and Loading */
    if (args & LOAD)
        load(&rooms, &numrooms, &player);
    else {
        createrooms(&rooms, &numrooms);
        player = malloc(sizeof(entity_t));
        player->xpos = rooms[0].xpos;
        player->ypos = rooms[0].ypos;
        player->symbol = '@';
    }
    if (args & SAVE)
        save(rooms, numrooms, player);

    /* Sketching Map */
    initmap(map);
    maprooms(map, rooms, numrooms);

    /* Displaying */
    sketchmap(display, map);
    place(display, player, 1);
    draw(display);

    /* Cleaning Memory */
    free(player);
    free(rooms);
}

void getArgs(int argc, char *argv[], uint8_t *args) {
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

void createrooms(room_t **rooms, uint8_t *numrooms) {
    /* printf("CREATING:\n"); */
    uint8_t i, j, v;
    *numrooms = 6 + rand() % 2;
    *rooms = malloc(sizeof(room_t) * *numrooms);
    do {
        for (i = 0; i < *numrooms; i++) {
            (*rooms)[i].xsize = 4 + rand() % 10;
            (*rooms)[i].ysize = 3 + rand() % 10;
            (*rooms)[i].xpos = rand() % (W_WIDTH - (*rooms)[i].xsize);
            (*rooms)[i].ypos = rand() % (W_HEIGHT - (*rooms)[i].ysize);
        }
        v = 1;
        for (i = 1; i < *numrooms && v; i++) {
            for (j = 0; j < i && v; j++) {
                if (!((*rooms)[i].xpos + (*rooms)[i].xsize < (*rooms)[j].xpos || (*rooms)[j].xpos + (*rooms)[j].xsize < (*rooms)[i].xpos)
                        && !((*rooms)[i].ypos + (*rooms)[i].ysize < (*rooms)[j].ypos || (*rooms)[j].ypos + (*rooms)[j].ysize < (*rooms)[i].ypos)) {
                    v = 0;
                }
            }
        }
    } while(!v);
    /*
    for (i = 0; i < *numrooms; i++) {
        printf("%d : %d : %d : %d\n", (*rooms)[i].xpos, (*rooms)[i].ypos, (*rooms)[i].xsize, (*rooms)[i].ysize);
    }
    */
}

void save(room_t *rooms, uint8_t numrooms, entity_t *player) {
    /* printf("SAVING:\n"); */
    uint8_t i;
    uint16_t ws;
    uint32_t w;
    char *home = getenv("HOME");
    char *gamedir = ".rlg327";
    char *save = "dungeon";
    char *path = malloc(strlen(home) + strlen(gamedir) + strlen(save) + 3);
    sprintf(path, "%s/%s/%s", home, gamedir, save);
    FILE *f = fopen(path, "w");
    fwrite(&numrooms, sizeof(uint8_t), 1, f);
    for (i = 0; i < numrooms; i++) {
        w = (rooms[i].xpos << 24) + (rooms[i].ypos << 16) + (rooms[i].xsize << 8) + rooms[i].ysize;
        /* printf("%x\n", w); */
        w = htobe32(w);
        fwrite(&w, sizeof(uint32_t), 1, f);
        /* printf("%d : %d : %d : %d\n", rooms[i].xpos, rooms[i].ypos, rooms[i].xsize, rooms[i].ysize); */
    }
    ws = (player->xpos << 8) + player->ypos;
    /* printf("%x\n", ws); */
    ws = htobe16(ws);
    fwrite(&ws, sizeof(uint16_t), 1, f);
    fclose(f);
    free(path);
}

void load(room_t **rooms, uint8_t *numrooms, entity_t **player) {
    /* printf("LOADING:\n"); */
    uint8_t i;
    uint16_t rs;
    uint32_t r;
    char *home = getenv("HOME");
    char *gamedir = ".rlg327";
    char *save = "dungeon";
    char *path = malloc(strlen(home) + strlen(gamedir) + strlen(save) + 3);
    sprintf(path, "%s/%s/%s", home, gamedir, save);
    FILE *f = fopen(path, "r");
    fread(numrooms, sizeof(uint8_t), 1, f);
    *rooms = malloc(sizeof(room_t) * *numrooms);
    for (i = 0; i < *numrooms; i++) {
        fread(&r, sizeof(uint32_t), 1, f);
        r = be32toh(r);
        (*rooms)[i].ysize = r % 0x100;
        (*rooms)[i].xsize = (r >> 8) % 0x100;
        (*rooms)[i].ypos = (r >> 16) % 0x100;
        (*rooms)[i].xpos = (r >> 24) % 0x100;
        /*
        printf("%x\n", r);
        printf("%d : %d : %d : %d\n", (*rooms)[i].xpos, (*rooms)[i].ypos, (*rooms)[i].xsize, (*rooms)[i].ysize);
        */
    }
    *player = malloc(sizeof(entity_t));
    fread(&rs, sizeof(uint16_t), 1, f);
    rs = be16toh(rs);
    /* printf("%x\n", rs); */
    (*player)->ypos = rs % 0x100;
    (*player)->xpos = (rs >> 8) % 0x100;
    (*player)->symbol = '@';
    fclose(f);
    free(path);
}

void maprooms(char map[W_HEIGHT][W_WIDTH], room_t *rooms, uint8_t numrooms) {
    uint8_t i, j, k;
    for (i = 0; i < numrooms; i++) {
        for (j = 0; j < rooms[i].ysize; j++) {
            for (k = 0; k < rooms[i].xsize; k++) {
                map[rooms[i].ypos + j][rooms[i].xpos + k] = '.';
            }
        }
        /* display[rooms[i].ypos][rooms[i].xpos] = '0' + i; */
    }
    for (i = 1; i < numrooms; i++) {
        for (j = rooms[i-1].ypos + rooms[i-1].ysize / 2; j <= rooms[i].ypos + rooms[i].ysize / 2; j++) {
            if (map[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] == ' ') {
                map[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] = '#';
            }
        }
        for (j = rooms[i-1].ypos + rooms[i-1].ysize / 2; j >= rooms[i].ypos + rooms[i].ysize / 2; j--) {
            if (map[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] == ' ') {
                map[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] = '#';
            }
        }
        for (j = rooms[i-1].xpos + rooms[i-1].xsize / 2; j <= rooms[i].xpos + rooms[i].xsize / 2; j++) {
            if (map[rooms[i].ypos + rooms[i].ysize / 2][j] == ' ') {
                map[rooms[i].ypos + rooms[i].ysize / 2][j] = '#';
            }
        }
        for (j = rooms[i-1].xpos + rooms[i-1].xsize / 2; j >= rooms[i].xpos + rooms[i].xsize / 2; j--) {
            if (map[rooms[i].ypos + rooms[i].ysize / 2][j] == ' ') {
                map[rooms[i].ypos + rooms[i].ysize / 2][j] = '#';
            }
        }
    }
    map[rooms[1].ypos + 1][rooms[1].xpos + 1] = '>';
    map[rooms[numrooms-1].ypos + 1][rooms[numrooms-1].xpos + 1] = '<';
}

void initmap(char map[W_HEIGHT][W_WIDTH]) {
    uint8_t i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            map[i][j] = ' ';
        }
    }
}

void sketchmap(char display[W_HEIGHT][W_WIDTH], char map[W_HEIGHT][W_WIDTH]) {
    uint8_t i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            display[i][j] = map[i][j];
        }
    }
}

void place(char display[W_HEIGHT][W_WIDTH], entity_t *entities, uint8_t count) {
    uint8_t i;
    for (i = 0; i < count; i++) {
        display[entities[i].ypos][entities[i].xpos] = entities[i].symbol;
    }
}

void draw(char display[W_HEIGHT][W_WIDTH]) {
    uint8_t i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            printf("%c", display[i][j]);
        }
        printf("\n");
    }
}
