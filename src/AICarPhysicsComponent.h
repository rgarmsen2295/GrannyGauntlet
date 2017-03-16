#ifndef AI_CAR_PHYSICS_COMPONENT
#define AI_CAR_PHYSICS_COMPONENT

#include "PhysicsComponent.h"

class AICarPhysicsComponent : public PhysicsComponent {
public:
	AICarPhysicsComponent();

	~AICarPhysicsComponent();

	void initObjectPhysics();

	void updatePhysics(float deltaTime);

	void updateAnimation(float deltaTime);

	void startDeliveryAnimation() {};

private:
	float gravity;
	float yVelocity;
	bool untouched = true;

	/* Animation data */
	bool animated;
	glm::vec3 animRotAxis;
	const float animSpeed = 5.0;

	glm::vec3 moveAlongPath(float deltaTime);
};

#endif
