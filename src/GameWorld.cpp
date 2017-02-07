#include "GameWorld.h"
#include "GameManager.h"
#include "ShaderManager.h"
#include "CookieThrower.h"
#include <glm/gtx/rotate_vector.hpp>

GameWorld::GameWorld()
	: updateCount(0),
	renderCount(0),
	numBunniesHit(0) {

	// Seed the PRNG with the current time for any random elements in the world
	std::srand(std::time(NULL));

	// TODO(rgarmsen2295): Make this look nicer
	addLight({ -10.0f, 10.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1 });
}

GameWorld::~GameWorld() {}

void GameWorld::addDynamicGameObject(GameObject* obj) {
	this->dynamicGameObjects_.push_back(obj);
}

void GameWorld::addStaticGameObject(GameObject* obj) {
	this->staticGameObjects_.push_back(obj);
}

void GameWorld::addLight(const Light& newLight) {
	lights.push_back(newLight);
}

int GameWorld::getNumDynamicGameObjects() {
	return this->dynamicGameObjects_.size();
}

int GameWorld::getNumStaticGameObjects() {
	return this->staticGameObjects_.size();
}

const std::vector<Light>& GameWorld::getLights() {
	return lights;
}

void GameWorld::clearGameObjects() {
	this->dynamicGameObjects_.clear();
}

void GameWorld::resetWorld() {
	this->clearGameObjects();
	this->numBunniesHit = 0;
}

void GameWorld::updateGameObjects(double deltaTime, double totalTime) {
	
#ifdef DEBUG
	// Keep track of the last spawn time internally to know when to spawn next
	static double previousSpawnTime = 0.0;

	// Spawn a new bunny every ~3 seconds, and max out at 30 bunnies
	if (totalTime >= previousSpawnTime + 3.0 && getNumDynamicGameObjects() < 30) {
		addBunnyToGameWorld();
		previousSpawnTime = totalTime;
	}
#endif

//


    static CookieThrower* cookieThrower = new CookieThrower();
    cookieThrower->pollAndThrow(deltaTime, totalTime);

    // create a GamoObj
    // give it to Cookie Thrower
    // also add to the list
    // modify it in Cookie Thrower (should be automatically rendered)

	for (GameObject* obj : this->dynamicGameObjects_) {
		obj->update(deltaTime);
	}

	for (GameObject* obj : this->staticGameObjects_) {
		obj->update(deltaTime);
	}

	updateCount++;
}

std::shared_ptr<std::array<glm::vec4, 6>> createFrustrum(const glm::mat4& comp) {
	std::shared_ptr<std::array<glm::vec4, 6>> frustrumPlanes = std::make_shared<std::array<glm::vec4, 6>>();

	glm::vec3 n; //use to pull out normal
	float l; //length of normal for plane normalization

	glm::vec4 Left;
	Left.x = comp[0][3] + comp[0][0]; // see handout to fill in with values from comp
	Left.y = comp[1][3] + comp[1][0]; // see handout to fill in with values from comp
	Left.z = comp[2][3] + comp[2][0]; // see handout to fill in with values from comp
	Left.w = comp[3][3] + comp[3][0]; // see handout to fill in with values from comp
	n = glm::vec3(Left.x, Left.y, Left.z); // Normal of plane
	l = glm::length(n); // Length of normal
	(*frustrumPlanes)[0] = Left = Left / l;
	
	glm::vec4 Right;
	Right.x = comp[0][3] - comp[0][0]; // see handout to fill in with values from comp
	Right.y = comp[1][3] - comp[1][0]; // see handout to fill in with values from comp
	Right.z = comp[2][3] - comp[2][0]; // see handout to fill in with values from comp
	Right.w = comp[3][3] - comp[3][0]; // see handout to fill in with values from comp
	n = glm::vec3(Right.x, Right.y, Right.z);
	l = glm::length(n);
	(*frustrumPlanes)[1] = Right = Right / l;

	glm::vec4 Bottom;
	Bottom.x = comp[0][3] + comp[0][1]; // see handout to fill in with values from comp
	Bottom.y = comp[1][3] + comp[1][1]; // see handout to fill in with values from comp
	Bottom.z = comp[2][3] + comp[2][1]; // see handout to fill in with values from comp
	Bottom.w = comp[3][3] + comp[3][1]; // see handout to fill in with values from comp
	n = glm::vec3(Bottom.x, Bottom.y, Bottom.z);
	l = glm::length(n);
	(*frustrumPlanes)[2] = Bottom = Bottom / l;
	
	glm::vec4 Top;
	Top.x = comp[0][3] - comp[0][1]; // see handout to fill in with values from comp
	Top.y = comp[1][3] - comp[1][1]; // see handout to fill in with values from comp
	Top.z = comp[2][3] - comp[2][1]; // see handout to fill in with values from comp
	Top.w = comp[3][3] - comp[3][1]; // see handout to fill in with values from comp
	n = glm::vec3(Top.x, Top.y, Top.z);
	l = glm::length(n);
	(*frustrumPlanes)[3] = Top = Top / l;

	glm::vec4 Near;
	Near.x = comp[0][3] + comp[0][2]; // see handout to fill in with values from comp
	Near.y = comp[1][3] + comp[1][2]; // see handout to fill in with values from comp
	Near.z = comp[2][3] + comp[2][2]; // see handout to fill in with values from comp
	Near.w = comp[3][3] + comp[3][2]; // see handout to fill in with values from comp
	n = glm::vec3(Near.x, Near.y, Near.z);
	l = glm::length(n);
	(*frustrumPlanes)[4] = Near = Near / l;

	glm::vec4 Far;
	Far.x = comp[0][3] - comp[0][2]; // see handout to fill in with values from comp
	Far.y = comp[1][3] - comp[1][2]; // see handout to fill in with values from comp
	Far.z = comp[2][3] - comp[2][2]; // see handout to fill in with values from comp
	Far.w = comp[3][3] - comp[3][2]; // see handout to fill in with values from comp
	(*frustrumPlanes)[5] = Far = Far / l;

	return frustrumPlanes;
}

float distToPlane(float A, float B, float C, float D, vec3 point) {
	return (A * point.x + B * point.y + C * point.z + D);
}

int viewFrustCull(vec3 center, float radius) {
	for (int i=0; i < 6; i++) {
		float dist = distToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center);

		if (dist < radius) {
			return 1;
		}
	}

	return 0;
}

void GameWorld::drawGameObjects() {
	GameManager& gameManager = GameManager::instance();
	Camera& camera = gameManager.getCamera();
	
	// Create the matrix stacks
	std::shared_ptr<MatrixStack> P = std::make_shared<MatrixStack>();
	std::shared_ptr<MatrixStack> M = std::make_shared<MatrixStack>();
	std::shared_ptr<MatrixStack> V = std::make_shared<MatrixStack>();

	// Apply perspective projection
	P->pushMatrix();
	P->perspective(45.0f, aspect, 0.01f, 100.0f);

	// Set up view Matrix
	V->pushMatrix();
	V->loadIdentity();
	V->lookAt(camera.getEye(), camera.getTarget(), camera.getUp());

	// Create view-perspective matrix
	glm::mat4 compViewPerspective = P->topMatrix() * V->topMatrix();

	auto frustrumPlanes = createFrustrum(compViewPerspective);

	// Draw non-static objects
	for (GameObject* obj : this->dynamicGameObjects_) {
		if (viewFrustCull(obj) != 1) {
			obj->draw(P, M, V);
		}
	}

	// Draw static objects
	for (GameObject *obj : this->staticGameObjects_) {
		if (viewFrustCull() != 1) {
			obj->draw(P, M, V);
		}
	}
	renderCount++;
}

GameObjectType GameWorld::checkCollision(GameObject* objToCheck) {
	GameManager& gameManager = GameManager::instance();
   GameObject& player = gameManager.getPlayer();

	// Check the player against the object
	if (&player != objToCheck && objToCheck->boundBox.checkIntersection(player.boundBox)) {
		return GameObjectType::PLAYER;
	}

	// Check against dynamic objects
	for (GameObject* obj : dynamicGameObjects_) {
		if (obj != objToCheck && objToCheck->boundBox.checkIntersection(obj->boundBox)) {
			return obj->type;
		}
	}

	// Check against static objects
	for (GameObject* obj : staticGameObjects_) {
		if (obj != objToCheck && objToCheck->boundBox.checkIntersection(obj->boundBox)) {
			return obj->type;
		}
	}

	return GameObjectType::NO_OBJECT;
}

unsigned long GameWorld::getRenderCount() {
	return renderCount;
}

// TODO(rgarmsen2295): Abstract into "bunny world" specific sub-class
int GameWorld::getNumBunniesHit() {
	return numBunniesHit;
}

// TODO(rgarmsen2295): Abstract into "bunny world" specific sub-class
void GameWorld::registerBunnyHit() {
	numBunniesHit++;
}

// TODO(rgarmsen2295): Abstract into "bunny world" specific sub-class
void GameWorld::addBunnyToGameWorld() {
	ShaderManager& shaderManager = ShaderManager::instance();
	std::shared_ptr<Program> progPhong = shaderManager.getShaderProgram("Phong");

	static std::shared_ptr<Shape> bunnyShape = std::make_shared<Shape>();
	static bool hasBunnyLoaded = false;

	// TODO(rgarmsen2295): Refactor into some sort of manager class to simply and remove complexity from GameWorld
	// Only perform bunny shape init stuff once
	if (!hasBunnyLoaded) {
		bunnyShape->loadMesh(RESOURCE_DIR + "bunny.obj");
		bunnyShape->resize();
		bunnyShape->init();

		hasBunnyLoaded = true;
	}

	// Get a random start location between (-10, 0, -10) and (10, 0, 10)
	float randomStartX = (std::rand() % 20) - 10.0f;
	float randomStartZ = (std::rand() % 20) - 10.0f;

	glm::vec3 startPosition(randomStartX, 1.0f, randomStartZ);

	glm::vec3 startDirection(randomStartX, 0.0f, randomStartZ);
	glm::normalize(startDirection);

	float startVelocity = 5.0f;

	glm::vec3 initialScale(1.0f, 1.0f, 1.0f);

	BunnyPhysicsComponent* bunnyPhysicsComp = new BunnyPhysicsComponent();
	BunnyRenderComponent* bunnyRenderComp = new BunnyRenderComponent(bunnyShape, "Phong", brass);

	GameObject* bunnyObj = new GameObject(
		GameObjectType::DYNAMIC_OBJECT, 
		startPosition, 
		startDirection, 
		startVelocity, 
		initialScale,
		NULL, 
		bunnyPhysicsComp,
		bunnyRenderComp);

	this->addDynamicGameObject(bunnyObj);
}
