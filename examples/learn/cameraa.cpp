#include "cameraa.hpp"


namespace volrend {


Camera::Camera(float width, float height, float fx, float fy)
    : width(width),
      height(height),
      fx(fx < 0.f ? CAMERA_DEFAULT_FOCAL_LENGTH : fx),
      fy(fy < 0.f ? this->fx : fy){
	center = { -3.55f, 0.0, 3.55f };
	v_back = { -0.7071068f, 0.0f, 0.7071068f };
	v_world_up = { 0.0f, 0.0f, 1.0f };
	origin = { 0.0f, 0.0f, 0.0f };
    _update();
}

Camera::~Camera() {

}

void Camera::_update(bool transform_from_vecs) {
	if (transform_from_vecs) {
		v_back = bx::normalize(v_back);
		v_right = bx::normalize(bx::cross(v_world_up, v_back));
		v_up = bx::cross(v_back, v_right);
		for(int i=0;i<3;i++)
		{
			transform[0+i] = v_right.raw[i];
			transform[3+i] = v_up.raw[i];
			transform[6+i] = v_back.raw[i];
		}
		cen = center;
	}

}

}  // namespace volrend
