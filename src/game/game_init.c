#define SPAWN_RANGE 15

void game_init(void* mem, uint32_t mem_bytes)
{
	srand(time(NULL));

    struct game_memory* game = (struct game_memory*)mem;

    game->t = 0;
    game->camera_position = v3_new(0.0f, 0.0f, 0.0f);
    game->camera_yaw = -90.0f;
    game->camera_pitch = 0.0f;
    game->camera_yaw_target = -90.0f;
    game->camera_pitch_target = 0.0f;
    sync_camera_directions(
	    &game->camera_forward,
	    &game->camera_right,
	    game->camera_yaw,
	    game->camera_pitch);

    for(int32_t i = 0; i < CUBES_LEN; i++)
    {
	    reposition_cube(&game->cube_positions[i], game->cube_orientations[i], game->camera_forward, game->camera_right);

	    game->cube_rotations_per_frame[i] = 
	    (struct v3){{{
		    rand_t(),
		    rand_t(),
		    rand_t()
	    }}};
    }
}
