#ifndef COOKIE_ACTION_COMPONENT_H
#define COOKIE_ACTION_COMPONENT_H

#include <memory>
#include <stdio.h>
#include <stdlib.h>

#include "ActionComponent.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "ShaderHelper.h"
#include "ShaderManager.h"
#include "AimInputComponent.h"

class Shape;

class CookieActionComponent : public ActionComponent {
public:

    // Constructs a new CookieActionComponent
    CookieActionComponent();

    ~CookieActionComponent();

    void initActionComponent();

    // checks for user input and throws a Cookie
    void checkAndPerformAction(double deltaTime, double totalTime);

private:
    GameObject* gameObj;
    GameObject* gameObj1;
    GameObject* gameObj2;
    AimInputComponent* aimInputComponent;
    std::shared_ptr<Shape> cookieShape;
    float xRot = 0.0;
    float yRot = 0.0;
    float startVelocity = 20.0f;
    double previousCookieTime = 0.0;

};

#endif