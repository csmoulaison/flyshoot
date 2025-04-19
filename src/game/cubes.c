void reposition_cube(struct v3* position, mat4 orientation, struct v3 camera_forward, struct v3 camera_right)
{
	struct v3 camera_up = {{{0, 1, 0}}};

	*position = v3_zero();
	*position = v3_add(*position, v3_scale(camera_right, rand_t() * (CUBE_POS_MAX_XY * 2) - CUBE_POS_MAX_XY));
	*position = v3_add(*position, v3_scale(camera_up, rand_t() * (CUBE_POS_MAX_XY * 2) - CUBE_POS_MAX_XY));
	*position = v3_add(*position, v3_scale(camera_forward, MAX_DRAW_DISTANCE_Z + rand_t() * CUBE_POS_MAX_Z));

	struct v3 rot = 
	{{{
		rand_t(),
		rand_t(),
		rand_t()
	}}};
	glm_mat4_identity(orientation);
   	glm_rotate(orientation, radians(rand_t() * 180), rot.data);
}
