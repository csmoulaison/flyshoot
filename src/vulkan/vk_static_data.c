// TODO - We actually don't need this many vertices. Maybe not worth fixing
// before moving on, but figured it's worth mentioning.
#define CUBE_VERTICES_LEN 24
struct vk_cube_vertex cube_vertices[CUBE_VERTICES_LEN] = {
    { {{{-0.5f,  0.5f, -0.5f}}}, {{{ 1.0f, 0.0f, 0.0f}}} },
    { {{{ 0.5f,  0.5f, -0.5f}}}, {{{ 0.0f, 1.0f, 0.0f}}} },
    { {{{ 0.5f, -0.5f, -0.5f}}}, {{{ 0.0f, 0.0f, 1.0f}}} },
    { {{{-0.5f, -0.5f, -0.5f}}}, {{{ 0.0f, 0.0f, 0.0f}}} }, 

    { {{{-0.5f, -0.5f,  0.5f}}}, {{{ 1.0f, 0.0f, 0.0f}}} },
    { {{{ 0.5f, -0.5f,  0.5f}}}, {{{ 0.0f, 1.0f, 0.0f}}} },
    { {{{ 0.5f,  0.5f,  0.5f}}}, {{{ 0.0f, 0.0f, 1.0f}}} },
    { {{{-0.5f,  0.5f,  0.5f}}}, {{{ 0.0f, 0.0f, 0.0f}}} },

    { {{{-0.5f,  0.5f,  0.5f}}}, {{{ 1.0f, 0.0f, 0.0f}}} },
    { {{{-0.5f,  0.5f, -0.5f}}}, {{{ 0.0f, 1.0f, 0.0f}}} },
    { {{{-0.5f, -0.5f, -0.5f}}}, {{{ 0.0f, 0.0f, 1.0f}}} },
    { {{{-0.5f, -0.5f,  0.5f}}}, {{{ 0.0f, 0.0f, 0.0f}}} },

    { {{{ 0.5f, -0.5f,  0.5f}}}, {{{ 1.0f, 0.0f, 0.0f}}} },
    { {{{ 0.5f, -0.5f, -0.5f}}}, {{{ 0.0f, 1.0f, 0.0f}}} },
    { {{{ 0.5f,  0.5f, -0.5f}}}, {{{ 0.0f, 0.0f, 1.0f}}} },
    { {{{ 0.5f,  0.5f,  0.5f}}}, {{{ 0.0f, 0.0f, 0.0f}}} },

    { {{{-0.5f, -0.5f, -0.5f}}}, {{{ 1.0f, 0.0f, 0.0f}}} },
    { {{{ 0.5f, -0.5f, -0.5f}}}, {{{ 0.0f, 1.0f, 0.0f}}} },
    { {{{ 0.5f, -0.5f,  0.5f}}}, {{{ 0.0f, 0.0f, 1.0f}}} },
    { {{{-0.5f, -0.5f,  0.5f}}}, {{{ 0.0f, 0.0f, 0.0f}}} },

    { {{{-0.5f,  0.5f,  0.5f}}}, {{{ 1.0f, 0.0f, 0.0f}}} },
    { {{{ 0.5f,  0.5f,  0.5f}}}, {{{ 0.0f, 1.0f, 0.0f}}} },
    { {{{ 0.5f,  0.5f, -0.5f}}}, {{{ 0.0f, 0.0f, 1.0f}}} },
    { {{{-0.5f,  0.5f, -0.5f}}}, {{{ 0.0f, 0.0f, 0.0f}}} }
};

#define CUBE_INDICES_LEN 36
uint16_t cube_indices[CUBE_INDICES_LEN] = 
{
	0, 1, 2, 
	2, 3, 0,

	4, 5, 6, 
	6, 7, 4,

	8,  9,  10, 
	10, 11, 8,

	12, 13, 14, 
	14, 15, 12,

	16, 17, 18, 
	18, 19, 16,

	20, 21, 22, 
	22, 23, 20,
};

#define RETICLE_VERT_SHORT 0.002
#define RETICLE_VERT_LONG 0.01
#define RETICLE_VERT_OFF 0.02

#define RETICLE_VERTICES_LEN 16
struct vk_reticle_vertex reticle_vertices[RETICLE_VERTICES_LEN] =
{
	{ {{{-RETICLE_VERT_LONG + RETICLE_VERT_OFF, -RETICLE_VERT_SHORT}}} },
	{ {{{ RETICLE_VERT_LONG + RETICLE_VERT_OFF, -RETICLE_VERT_SHORT}}} },
	{ {{{ RETICLE_VERT_LONG + RETICLE_VERT_OFF,  RETICLE_VERT_SHORT}}} },
	{ {{{-RETICLE_VERT_LONG + RETICLE_VERT_OFF,  RETICLE_VERT_SHORT}}} },

	{ {{{-RETICLE_VERT_LONG - RETICLE_VERT_OFF, -RETICLE_VERT_SHORT}}} },
	{ {{{ RETICLE_VERT_LONG - RETICLE_VERT_OFF, -RETICLE_VERT_SHORT}}} },
	{ {{{ RETICLE_VERT_LONG - RETICLE_VERT_OFF,  RETICLE_VERT_SHORT}}} },
	{ {{{-RETICLE_VERT_LONG - RETICLE_VERT_OFF,  RETICLE_VERT_SHORT}}} },

	{ {{{-RETICLE_VERT_SHORT, -RETICLE_VERT_LONG + RETICLE_VERT_OFF}}} },
	{ {{{ RETICLE_VERT_SHORT, -RETICLE_VERT_LONG + RETICLE_VERT_OFF}}} },
	{ {{{ RETICLE_VERT_SHORT,  RETICLE_VERT_LONG + RETICLE_VERT_OFF}}} },
	{ {{{-RETICLE_VERT_SHORT,  RETICLE_VERT_LONG + RETICLE_VERT_OFF}}} },

	{ {{{-RETICLE_VERT_SHORT, -RETICLE_VERT_LONG - RETICLE_VERT_OFF}}} },
	{ {{{ RETICLE_VERT_SHORT, -RETICLE_VERT_LONG - RETICLE_VERT_OFF}}} },
	{ {{{ RETICLE_VERT_SHORT,  RETICLE_VERT_LONG - RETICLE_VERT_OFF}}} },
	{ {{{-RETICLE_VERT_SHORT,  RETICLE_VERT_LONG - RETICLE_VERT_OFF}}} }
};

#define RETICLE_INDICES_LEN 24
uint16_t reticle_indices[RETICLE_INDICES_LEN] =
{
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
	8, 9, 10, 10, 11, 8,
	12, 13, 14, 14, 15, 12
};
