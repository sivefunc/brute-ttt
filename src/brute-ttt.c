// Libraries needed, here's a quick summary of why i'm using it:
#include <stdio.h>                          // rename, fprintf, sprintf.
#include <stdlib.h>                         // malloc, calloc.
#include <string.h>                         // strcat, strcpy
#include <inttypes.h>                       // uintN_t
#include <sys/stat.h>                       // mkdir

// Enum declaration
enum STATES {PLAYING, WIN, TIE};            // WIN and TIE are the leafs nodes
                                            // of the tree and PLAYING
                                            // state is for internal nodes.

enum PLAYER {EMPTY, PLAYER_1, PLAYER_2};    // Player in each cell and player
                                            // moving in each game.

// Struct declaration
// Game of tic tac toe
typedef struct Game
{
    uint8_t rows;                           // height -> in this case is 3.
    uint8_t columns;                        // width  -> in this case is 3.
    uint8_t ** board;                       // Board each cell is in
                                            // {EMPTY, PLAYER_1 or PLAYER_2}.
    uint8_t player_moving;                  // Current player moving.
} Game;

// General tree
typedef struct treenode
{
    Game * game;                            // check Game struct above.
    uint8_t children_count;                 // N. of possible empty locations
                                            // (considering there's
                                            // not win yet).
    struct treenode **children;             // the actual games with the piece
                                            // placed.
} treenode;

// Prototypes
treenode * create_node(treenode * parent);  // create node then insert it onto
                                            // parent if not NULL.

Game * create_from_copy(Game * src);        // deep copies a game.

uint8_t game_state(Game * game);            // return STATE constant indicating
                                            // {PLAYING, WIN or TIE}.

void make_move(treenode * parent);          // recursively play each available
                                            // game until WIN or TIE state.

void write_html(treenode *node,             // recursively writes html of
                char *path,                 // each game.
                uint8_t depth);

int main(void)
{
    // tic tac toe size (3x3)
    const uint8_t rows = 2, columns = 2;
    
    // Creating initial parent and corresponding game.
    treenode * root = create_node(NULL);
    Game * game = malloc(sizeof(Game));
    game -> rows = rows;
    game -> columns = columns;   
    game -> player_moving = EMPTY;

    uint8_t **board = malloc(sizeof(uint8_t*) * game -> rows);
    for (uint8_t row = 0; row < rows; row++)
    {
        board[row] = malloc(sizeof(uint8_t) * game -> columns);
        for (uint8_t column = 0; column < game -> columns; column++)
        {
            board[row][column] = EMPTY;
        }
    }
    game -> board = board;
    root -> game = game;

    make_move(root); 
    write_html(root, "web/", 0);            
    rename("web/game.html", "web/index.html");
    return 0x0;
}

void write_html(treenode *node, char *path,  uint8_t depth)
{
    char *path_with_name = calloc(strlen(path) + strlen("game.html\0") + 1, 1);
    
    strcpy(path_with_name, path);
    strcat(path_with_name, "game.html\0");
        
    FILE *p = fopen(path_with_name, "w");
    free(path_with_name);
    char *href = calloc(depth * 3 + 1, sizeof(char));
    for (uint8_t i = 0; i < depth; i++) strcat(href, "../");
    
    fprintf(p,
            "<!DOCTYPE html>\n"
            "<html>\n"
            "   <head>\n"
            "       <meta charset=\"utf-8\">\n"
            "       <title>brute-ttt</title>\n"
            "       <link rel=\"stylesheet\" href=\"%scss/styles.css\">\n"
            "   </head>\n"
            "   <body>\n",
            href);

    fprintf(p, "       ");
    if (node -> children_count == 0) // leaf
    {
        fprintf(p, "<h1>%s</h1>\n", game_state(node -> game) == TIE ? "TIE" :
                        node -> game -> player_moving != PLAYER_2 ?
                            "PLAYER 1 - WON" :
                            "PLAYER 2 - WON");
    }

    else
    {
        fprintf(p, "<h1>%s</h1>\n", node -> game -> player_moving != PLAYER_1 ?
                    "PLAYER 1 - Make your move" :
                    "PLAYER 2 - Make your move");
    }

    fprintf(p, "       <div class=\"grid\">\n");

    uint8_t curr_child = 1;
    for (uint8_t row = 0; row < node -> game -> rows; row++)
    {
        fprintf(p, "           <div class=\"row\">\n");
        for (uint8_t column = 0; column < node -> game -> columns; column++)
        {
            fprintf(p, "               ");
            if (node -> game -> board[row][column] == PLAYER_1)
                fprintf(p, "<a class=\"cell X\">X</a>\n");

            else if (node -> game -> board[row][column] == PLAYER_2)
                fprintf(p, "<a class=\"cell O\">O</a>\n");

            else // empty
            {
                if (node -> children_count == 0)
                {
                    fprintf(p, "<a class=\"cell\"> </a>\n");
                }
                
                else
                {
                    fprintf(p,
                            "<a href=\"%d/game.html\" class=\"cell\"> </a>\n",
                            curr_child);
                    curr_child++;
                }
            }
        }
        fprintf(p, "           </div>\n");
    }
    fprintf(p, "       </div>\n");
    fprintf(p, "<a id=\"new-game\" href=\"%sindex.html\">New game</a>", href);
    fprintf(p, "   </body>\n"
                "</html>\n");

    fclose(p);
    free(href);
    for (uint8_t child = 0; child < node -> children_count; child++)
    {
        char *new_path = calloc(strlen(path) + strlen("1/") + 1, sizeof(char));
        strcpy(new_path, path);
        sprintf(new_path + strlen(path), "%hhu", child + 1);
        strcat(new_path, "/");
        mkdir(new_path, 0777);
        write_html(node -> children[child], new_path, depth + 1);
        free(new_path);
    }
}

treenode * create_node(treenode * parent)
{
    treenode * node = malloc(sizeof(treenode));
    if (node == NULL)
        return NULL;

    node -> children_count = 0;
    node -> children = NULL;
    node -> game = NULL;

    if (parent != NULL)
    {
        parent -> children_count++;
        parent -> children = realloc(parent -> children,
                sizeof(treenode *) * parent -> children_count);

        node -> game = create_from_copy(parent -> game);
        node -> game -> player_moving = parent ->
                game -> player_moving != PLAYER_1 ? PLAYER_1 : PLAYER_2;


        parent -> children[parent -> children_count - 1] = node;
    }

    return node;
}

Game * create_from_copy(Game * src)
{
    Game * dst = malloc(sizeof(Game));
    dst -> rows = src -> rows;
    dst -> columns = src -> columns;
    dst -> board = (uint8_t **) malloc(sizeof(uint8_t *) * dst -> rows);

    for (uint8_t i = 0; i < dst -> rows; i++)
    {
        dst -> board[i] = (uint8_t *) malloc(sizeof(uint8_t) * dst -> columns);
        for (uint8_t j = 0; j < dst -> columns; j++)
            dst -> board[i][j] = src -> board[i][j];
    }
    return dst;
}

uint8_t game_state(Game * game)
{
    uint8_t state = PLAYING;
    
    // row check 
    for (uint8_t row = 0; row < game -> rows; row++)
    {
        for (uint8_t column = 0; column < game -> columns - 1; column++)
        {
            state = game -> board[row][column];
            if (state != game -> board[row][column+1])
            {
                state = PLAYING;
                break;
            }
        }
        if (state != EMPTY && state != PLAYING)
            return WIN;

    }

    // column check
    for (uint8_t column = 0; column < game -> columns; column++)
    {
        for (uint8_t row = 0; row < game -> rows - 1; row++)
        {
            state = game -> board[row][column];
            if (state != game -> board[row+1][column])
            {
                state = PLAYING;
                break;
            }
        }

        if (state != EMPTY && state != PLAYING)
            return WIN;
    }

    // diagonal check (we are assuming it's square)
    // \.
    //  \.
    //   \.
    //
    for (uint8_t row = 0; row < game -> rows - 1; row++)
    {
        state = game -> board[row][row];
        if (state != game -> board[row+1][row+1])
        {
            state = PLAYING;
            break;
        }
    }

    if (state != EMPTY && state != PLAYING)
        return WIN;   

    //    /
    //   /
    //  /
    // /

    uint8_t column = 0;
    for (uint8_t row = game -> rows - 1; row > 0; row--, column++)
    {
        state = game -> board[row][column];
        if (state != game -> board[row-1][column+1])
        {
            state = PLAYING;
            break;
        }
    }
    
    if (state != EMPTY && state != PLAYING)
        return WIN;   


    for (uint8_t row = 0; row < game -> rows; row++)
        for (uint8_t column = 0; column < game -> columns; column++)
            if (game -> board[row][column] == EMPTY)
                return PLAYING;
    
    return TIE;
}

void make_move(treenode * parent)
{
    Game *game = parent -> game;
    uint8_t state = game_state(game);
    if (state != PLAYING)
    {
        return;
    }

    for (uint8_t row = 0; row < game -> rows; row++)
    {
        for (uint8_t column = 0; column < game -> columns; column++)
        {
            if (game -> board[row][column] == EMPTY)
            {
                treenode * child = create_node(parent);

                child -> game -> board[row][column] =
                        child -> game -> player_moving;

                make_move(child);
            }
        }
    }
}
