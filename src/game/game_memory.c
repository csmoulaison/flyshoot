struct game_memory
{
    float t;
	struct v3 camera_position;

	float camera_yaw;
	float camera_pitch;
	float camera_yaw_target;
	float camera_pitch_target;
	// Cached values that must be upated based on yaw and pitch
	struct v3 camera_forward;
	struct v3 camera_right;

	struct v3 cube_positions[CUBES_LEN];
	mat4 cube_orientations[CUBES_LEN];
	struct v3 cube_rotations_per_frame[CUBES_LEN];
};
