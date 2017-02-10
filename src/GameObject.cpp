	#include "GameObject.h"
#include "GameManager.h"

GameObject::GameObject(GameObjectType objType,
	glm::vec3 startPosition,
	glm::vec3 startDirection,
	float startVelocity,
	glm::vec3 initialScale,
	InputComponent* input,
	PhysicsComponent* physics,
	RenderComponent* render,
	ActionComponent* action)
	: direction(glm::normalize(startDirection)),
	velocity(startVelocity),
	type(objType),
	toggleMovement(false),
	orientAngle_(0),
	yRotationAngle_(0),
	render_(render),
	input_(input),
	physics_(physics),
	action_(action) {

	// Set initial position and scale values
	setPosition(startPosition);
	setScale(initialScale);
}

GameObject::~GameObject() {

}

void GameObject::initComponents() {
	if (input_ != NULL) {
	  input_->setGameObjectHolder(shared_from_this());
	}

	glm::vec3 minBoundBoxPt(0.0f, 0.0f, 0.0f);
	glm::vec3 maxBoundBoxPt(0.0f, 0.0f, 0.0f);

	if (render_ != NULL) {
		render_->setGameObjectHolder(shared_from_this());
		minBoundBoxPt = render_->getShape()->getMin();
		maxBoundBoxPt = render_->getShape()->getMax();
	}

	if (physics_ != NULL) {
		physics_->setGameObjectHolder(shared_from_this());
		physics_->initBoundingBox(minBoundBoxPt, maxBoundBoxPt);
		physics_->initObjectPhysics();
	}

	if (action_ != NULL) {
	    action_->setGameObjectHolder(shared_from_this());
	    action_->initActionComponent();
	}
}

glm::vec3& GameObject::getPosition() {
	return position_;
}

glm::vec3& GameObject::getScale() {
	return scale_;
}

void GameObject::setOrientAngle(float orientAngle) {
   orientAngle_ = orientAngle;
}

float GameObject::getOrientAngle() {
   return orientAngle_;
}

float GameObject::getYAxisRotation() {
	return yRotationAngle_;
}

void GameObject::setPosition(glm::vec3& newPosition) {
	position_ = newPosition;
	transform.setTranslation(position_);
}

void GameObject::setScale(glm::vec3& newScale) {
	scale_ = newScale;
	transform.setScale(scale_);
}

void GameObject::setYAxisRotation(float angle) {
	static glm::vec3 yAxis(0.0f, 1.0f, 0.0f);

	yRotationAngle_ = angle;
	transform.setRotate(angle, yAxis);
}


void GameObject::changeMaterial(std::shared_ptr<Material> newMaterial) {
	if (newMaterial != render_->getMaterial()) {
		render_->setMaterial(newMaterial);
	}
}

void GameObject::update(double deltaTime) {
	if (input_ != NULL) {
		input_->pollInput();
	}

	if (physics_ != NULL) {
		physics_->updatePhysics(deltaTime);
	}
}

void GameObject::draw(std::shared_ptr<MatrixStack> P, std::shared_ptr<MatrixStack> M, std::shared_ptr<MatrixStack> V) {
	if (render_ != NULL) {
		render_->draw(P, M, V);
	}
}

void GameObject::performAction(double deltaTime, double totalTime) {
    if (action_ != NULL) {
        action_->checkAndPerformAction(deltaTime, totalTime);
    }
}

void GameObject::changeShader(const std::string& newShaderName) {
	if (render_ != NULL) {
		render_->changeShader(newShaderName);
	}
}

RenderComponent* GameObject::getRenderComponent() {
    return render_;
}

bool GameObject::checkIntersection(std::shared_ptr<GameObject> otherObj) {	
	PhysicsComponent* otherObjPhysics = otherObj->physics_;
	if (physics_ != NULL && otherObjPhysics != NULL) {
		return physics_->getBoundingBox().checkIntersection(otherObjPhysics->getBoundingBox());
	}

	return false;
}

BoundingBox* GameObject::getBoundingBox() {
   if (physics_) {
      return &physics_->getBoundingBox();
   }
   return NULL;
}
