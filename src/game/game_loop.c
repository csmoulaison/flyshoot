#define MAX_NETWORK_LINES 4
#define CUBES_MOVE_SPEED 3
#define CAM_LOOK_SPEED 12
#define CAM_LOOK_LERP_SPEED 1

// TODO - majorus
// * Dynamic up vector. Always up based on current forward? Think about what
//   that means.
// * Raycast from camera to hitting cube.
// * Cube knockback - change rotation based on hit normal and current rotation,
//   and knockback in reverse direction of normal offset by our current speed
//   of course.
// * Maybe cubes for amount of bullets available?
// * Visual effect for hitting cube. Maybe flashes white momentarily. That
//   effect could even be the same as the bullet disappearing when fired at the UI
//   level.
// * Lerp camera point to target and implement visual cursor which moves freely,
//   with the camera following. At the same time clamp the range of the camera
//   forward direction. Not sure what kind of curves needed for this clamped
//   movement yet.
// * Restart game on hit by cube - collision detection.
// * Dude so like what if these are actually authored levels from start to
//   finish. That way we need a REAL level editor???

void game_loop(
    void*                mem,
    size_t               mem_bytes,
    float                dt,
    uint32_t             window_w,
    uint32_t             window_h,
    struct input_state*  input,
    struct render_group* render_group)
{
    struct game_memory* game = (struct game_memory*)mem;

	// TODO - wrap yaw - hard with lerp
	game->camera_yaw_target   += (float)input->mouse_delta_x * CAM_LOOK_SPEED * dt;
	game->camera_pitch_target -= (float)input->mouse_delta_y * CAM_LOOK_SPEED * dt;
	if(game->camera_pitch > 90) 
	{
		game->camera_pitch = 90;
	}
	else if(game->camera_pitch < -90)
	{
		game->camera_pitch = -90;
	}
	game->camera_yaw = lerp(game->camera_yaw, game->camera_yaw_target, dt * CAM_LOOK_LERP_SPEED);
	game->camera_pitch = lerp(game->camera_pitch, game->camera_pitch_target, dt * CAM_LOOK_LERP_SPEED);

	sync_camera_directions(
		&game->camera_forward,
		&game->camera_right,
		game->camera_yaw,
		game->camera_pitch);

	mat4 cube_transforms[CUBES_LEN];
	for(uint32_t i = 0; i < CUBES_LEN; i++)
	{
		game->cube_positions[i] = v3_add(
			game->cube_positions[i], 
			v3_scale(game->camera_forward, -CUBES_MOVE_SPEED * dt));
		glm_rotate(
			game->cube_orientations[i], 
			dt * radians(180), 
			game->cube_rotations_per_frame[i].data);

		if(v3_dot(game->camera_forward, game->cube_positions[i]) < 0)
		{
			reposition_cube(
				&game->cube_positions[i], 
				game->cube_orientations[i], 
				game->camera_forward, 
				game->camera_right);
		}

		glm_mat4_identity(cube_transforms[i]);
		glm_translate(cube_transforms[i], game->cube_positions[i].data);
		glm_mat4_mul(cube_transforms[i], game->cube_orientations[i], cube_transforms[i]);
	}	

	render_group->clear_color = v3_new(.0, .0, .0);
	render_group->max_draw_distance_z = MAX_DRAW_DISTANCE_Z;

	render_group->camera_position = game->camera_position;
	render_group->camera_target = v3_add(game->camera_position, game->camera_forward);

#define CAMERA_RETICLE_OFFSET_MOD 0.035
	render_group->reticle_offset = (struct v2)
	{{{
		(game->camera_yaw   - game->camera_yaw_target)   / ((float)window_w * -CAMERA_RETICLE_OFFSET_MOD),
		(game->camera_pitch - game->camera_pitch_target) / ((float)window_h *  CAMERA_RETICLE_OFFSET_MOD)
	}}};

	memcpy(render_group->cube_transforms, cube_transforms, sizeof(struct m4) * CUBES_LEN);
}
