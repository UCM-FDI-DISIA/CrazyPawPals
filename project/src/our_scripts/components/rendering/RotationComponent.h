#pragma once
#include "../../../ecs/Component.h"

class Transform;
struct dyn_image;

class RotationComponent : public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::ROTATIONCOMPONENT);

	enum class Mode
	{
		NONE = 0,		   // No gira
		CONTINUOUS = 1,	   // Gira indefinidamente
		LIMITED_ANGLE = 2, // Gira hasta cierto ángulo y se detiene
		TOGGLE = 3,		   // Cambia sentido cada cierto tiempo
		CYCLE = 4		   // Ciclos de girar y pausar
	};

	RotationComponent(Mode mode,
					  float speed,
					  float max_rotation,
					  uint32_t flip_time = 0,
					  uint32_t toggle_interval = 0,
					  uint32_t active_time = 0,
					  uint32_t pause_time = 0,
					  bool can_flip = false);

	virtual ~RotationComponent() {};
	void initComponent() override;
	void update(uint32_t delta_time) override;
	void setInitialRotation(float angle = 0.0f);

protected:
	// Configuración
	Mode _mode;
	float _speed;
	float _max_rotation;
	uint32_t _toggle_interval;
	uint32_t _active_time;
	uint32_t _pause_time;
	uint32_t _flip_time;
	bool _can_flip;

	// Estado
	bool _rotating;
	bool _reverse;
	float _accumulated_rotation;
	uint32_t _last_toggle_time;
	uint32_t _last_cycle_time;
	uint32_t _last_flip_time;

	Transform *_tr;
	dyn_image *_dy;
};
