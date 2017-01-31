#include "CookieThrower.h"
#include "GameManager.h"
#include "ShaderManager.h"
#include <glm/gtx/rotate_vector.hpp>

/*
 * Poll input from the given input handler, and throws a throwable
 * dependent on the input and according constraints.
 */
CookieThrower::CookieThrower() {
    aimInputComponent = new AimInputComponent();
    cookieShape = std::make_shared<Shape>();
    cookieShape->loadMesh(RESOURCE_DIR + "sphere.obj");
    cookieShape->resize();
    cookieShape->init();
}

CookieThrower::~CookieThrower() {}

void CookieThrower::pollAndThrow(double deltaTime, double totalTime) {

    GameManager& gameManager = GameManager::instance();
    Camera& camera = gameManager.getCamera();
    GameObject& player = gameManager.getPlayer();
    ShaderManager& shaderManager = ShaderManager::instance();
    glm::vec3 initialScale(0.5f, 0.1f, 0.5f);

    aimInputComponent->pollInput();

    if(aimInputComponent->toggleXRotation){
        xRot += aimInputComponent->velocity * deltaTime;
    }
    if(aimInputComponent->toggleYRotation){
        yRot += aimInputComponent->velocity * deltaTime;
    }

    if (aimInputComponent->toggleThrow) {
        if (totalTime >= previousCookieTime + 0.5) {

            CookiePhysicsComponent *cookiePhysicsComp = new CookiePhysicsComponent();
            BunnyRenderComponent *renderComp = new BunnyRenderComponent(cookieShape,
               "Phong", obsidian);

            glm::vec3 upDownRotAxis = glm::cross(player.direction, glm::vec3(0.0, 1.0, 0.0));
            glm::vec3 throwDirection = glm::rotate(player.direction, xRot, upDownRotAxis);
            throwDirection = glm::rotateY(throwDirection, yRot);

            GameObject *cookieObj = new GameObject(
                    GameObjectType::DYNAMIC_OBJECT,
                    player.getPosition(),
                    throwDirection,
                    startVelocity,
                    initialScale,
                    NULL,
                    cookiePhysicsComp,
                    renderComp);

            gameManager.getGameWorld().addDynamicGameObject(cookieObj);

            previousCookieTime = totalTime;
        }
    }
}

void CookieThrower::draw() {
    GLuint VAID;
    glGenVertexArrays(1, &VAID);
    glBindVertexArray(VAID);

    GLuint pid = ShaderManager::instance().getShaderProgram("SuperSimple")->getPid();

    GameManager& gameManager = GameManager::instance();
    GameObject& player = gameManager.getPlayer();

    glm::vec3 upDownRotAxis = glm::cross(player.direction, glm::vec3(0.0, 1.0, 0.0));
    glm::vec3 throwDirection = glm::rotate(player.direction, xRot, upDownRotAxis);
    throwDirection = glm::rotateY(throwDirection, yRot);

    static glm::vec3 initialThrowDir = throwDirection;

    GLfloat lineVertices[6] = {
        0, 0, 0,
        -throwDirection.z + (initialThrowDir.z), throwDirection.y, throwDirection.x
    };

    GLuint VBOs;
    glGenBuffers(1, &VBOs);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_STATIC_DRAW);

    glUseProgram(pid);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*) 0
    );

    glDrawArrays(GL_LINES, 0, 2);

    glDisableVertexAttribArray(0);

    glUseProgram(0);
}
