#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define W_WIDTH 80
#define W_HEIGHT 24

typedef struct room_struct {
    int xpos;
    int ypos;
    int xsize;
    int ysize;
} room;

void createrooms(room *rooms, int numrooms);
int validrooms(room *rooms, int numrooms);
void maprooms(char display[W_HEIGHT][W_WIDTH], room *rooms, int numrooms);
void initDisplay(char display[W_HEIGHT][W_WIDTH]);
void draw(char display[W_HEIGHT][W_WIDTH]);

int main() {
    /* Seeding */
    srand(time(NULL));

    /* Declarations */
    int numrooms;
    room *rooms;
    char display[W_HEIGHT][W_WIDTH];

    /* Initialization */
    numrooms = 6 + rand() % 2;
    rooms = malloc(sizeof(room) * numrooms);
    createrooms(rooms, numrooms);
    initDisplay(display);
    maprooms(display, rooms, numrooms);

    /* Displaying Room */
    draw(display);
    
    /* Cleaning Memory */
    free(rooms);
}

void createrooms(room *rooms, int numrooms) {
    int i;
    do {
        for (i = 0; i < numrooms; i++) {
            rooms[i].xsize = 4 + rand() % 10;
            rooms[i].ysize = 3 + rand() % 10;
            rooms[i].xpos = rand() % (W_WIDTH - rooms[i].xsize);
            rooms[i].ypos = rand() % (W_HEIGHT - rooms[i].ysize);
        }
    } while(!validrooms(rooms, numrooms));
}

int validrooms(room *rooms, int numrooms) {
    int i, j;
    for (i = 1; i < numrooms; i++) {
        for (j = 0; j < i; j++) {
            if (!(rooms[i].xpos + rooms[i].xsize < rooms[j].xpos || rooms[j].xpos + rooms[j].xsize < rooms[i].xpos)
                    && !(rooms[i].ypos + rooms[i].ysize < rooms[j].ypos || rooms[j].ypos + rooms[j].ysize < rooms[i].ypos)) {
                return 0;
            }
        }
    }
    return 1;
}

void maprooms(char display[W_HEIGHT][W_WIDTH], room *rooms, int numrooms) {
    int i, j, k;

    for (i = 0; i < numrooms; i++) {
        for (j = 0; j < rooms[i].ysize; j++) {
            for (k = 0; k < rooms[i].xsize; k++) {
                display[rooms[i].ypos + j][rooms[i].xpos + k] = '.';
            }
        }
        /* display[rooms[i].ypos][rooms[i].xpos] = '0' + i; */
    }

    for (i = 1; i < numrooms; i++) {
        for (j = rooms[i-1].ypos + rooms[i-1].ysize / 2; j <= rooms[i].ypos + rooms[i].ysize / 2; j++) {
            if (display[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] == ' ') {
                display[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] = '#';
            }
        }
        for (j = rooms[i-1].ypos + rooms[i-1].ysize / 2; j >= rooms[i].ypos + rooms[i].ysize / 2; j--) {
            if (display[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] == ' ') {
                display[j][rooms[i-1].xpos + rooms[i-1].xsize / 2] = '#';
            }
        }
        for (j = rooms[i-1].xpos + rooms[i-1].xsize / 2; j <= rooms[i].xpos + rooms[i].xsize / 2; j++) {
            if (display[rooms[i].ypos + rooms[i].ysize / 2][j] == ' ') {
                display[rooms[i].ypos + rooms[i].ysize / 2][j] = '#';
            }
        }
        for (j = rooms[i-1].xpos + rooms[i-1].xsize / 2; j >= rooms[i].xpos + rooms[i].xsize / 2; j--) {
            if (display[rooms[i].ypos + rooms[i].ysize / 2][j] == ' ') {
                display[rooms[i].ypos + rooms[i].ysize / 2][j] = '#';
            }
        }
    }

    display[rooms[0].ypos + 1][rooms[0].xpos + 1] = '>';
    display[rooms[numrooms-1].ypos + 1][rooms[numrooms-1].xpos + 1] = '<';
}

void initDisplay(char display[W_HEIGHT][W_WIDTH]) {
    int i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            display[i][j] = ' ';
        }
    }
}

void draw(char display[W_HEIGHT][W_WIDTH]) {
    int i, j;
    for (i = 0; i < W_HEIGHT; i++) {
        for (j = 0; j < W_WIDTH; j++) {
            printf("%c", display[i][j]);
        }
        printf("\n");
    }
}
