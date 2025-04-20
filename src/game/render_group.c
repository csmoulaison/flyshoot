struct render_group 
{
	float t;
	
	struct v3 clear_color;
	float max_draw_distance_z;

	// TODO - better as a transform? see how usage emerges
	struct v3 camera_position;
	struct v3 camera_target;

	struct v2 reticle_offset;

	struct m4 cube_transforms[CUBES_LEN];
};
