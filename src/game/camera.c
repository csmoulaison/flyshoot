void sync_camera_directions(struct v3* forward, struct v3* right, float yaw, float pitch)
{
	*forward = v3_new(
    	cos(radians(yaw)) * cos(radians(pitch)),
    	sin(radians(pitch)),
    	sin(radians(yaw)) * cos(radians(pitch)));
	*forward = v3_normalize(*forward);

	struct v3 cam_up = {{{0, 1, 0}}};
	*right = v3_normalize(v3_cross(*forward, cam_up));
}
