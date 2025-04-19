#define CUBES_LEN 128
#define MAX_DRAW_DISTANCE_Z 25
#define CUBE_POS_MAX_XY  15
// TODO - increasing difficulty will probably be partly a matter of reducing
// this maximum over time, because the cubes are repositioned once they pass
// the position of the camera.
#define CUBE_POS_MAX_Z 50

#include "render_group.c"
#include "input.c"
#include "game_memory.c"
#include "cubes.c"
#include "camera.c"
#include "game_init.c"
#include "game_loop.c"
