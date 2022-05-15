#include "lab_m1/lab4/lab4.h"

#include <vector>
#include <string>
#include <iostream>

#include "lab_m1/lab4/transform3D.h"

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab4::Lab4()
{
}


Lab4::~Lab4()
{
}


void Lab4::Init()
{
    polygonMode = GL_FILL;

    Mesh* mesh = new Mesh("box");
    mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
    meshes[mesh->GetMeshID()] = mesh;

    // Initialize tx, ty and tz (the translation steps)
    translateX = 0;
    translateY = 0;
    translateZ = 0;

    // Initialize sx, sy and sz (the scale factors)
    scaleX = 1;
    scaleY = 1;
    scaleZ = 1;

    // Initialize angular steps
    angularStepOX = 0;
    angularStepOY = 0;
    angularStepOZ = 0;

    angularSunOY = 0;
    angularEarthOY = 0;
    angularMoonOY = 0;

    translateSunX = 0;
}


void Lab4::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Sets the screen area where to draw
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab4::Update(float deltaTimeSeconds)
{
    glm::mat4 sunMatrix, earthMatrix, moonMatrix;
    glLineWidth(3);
    glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(-2.5f, 0.5f, -1.5f);
    modelMatrix *= transform3D::Translate(translateX, translateY, translateZ);
    RenderMesh(meshes["box"], shaders["VertexNormal"], modelMatrix);

    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(0.0f, 0.5f, -1.5f);
    modelMatrix *= transform3D::Scale(scaleX, scaleY, scaleZ);
    RenderMesh(meshes["box"], shaders["Simple"], modelMatrix);

    modelMatrix = glm::mat4(1);
    modelMatrix *= transform3D::Translate(2.5f, 0.5f, -1.5f);
    modelMatrix *= transform3D::RotateOX(angularStepOX);
    modelMatrix *= transform3D::RotateOY(angularStepOY);
    modelMatrix *= transform3D::RotateOZ(angularStepOZ);
    RenderMesh(meshes["box"], shaders["VertexNormal"], modelMatrix);

    angularSunOY += 0.25f * deltaTimeSeconds;
    angularEarthOY += 1 * deltaTimeSeconds;
    angularMoonOY += 1.5f * deltaTimeSeconds;
    translateSunX += 0.1f * deltaTimeSeconds;

    sunMatrix = glm::mat4(1);
    sunMatrix *= transform3D::Translate(translateSunX, 2, 0);
    sunMatrix *= transform3D::Scale(0.7f, 0.7f, 0.7f);
    sunMatrix *= transform3D::RotateOY(angularSunOY);
    RenderMesh(meshes["box"], shaders["Simple"], sunMatrix);

    earthMatrix = sunMatrix * transform3D::RotateOY(angularEarthOY);
    earthMatrix *= transform3D::Translate(2, 0, 0);
    earthMatrix *= transform3D::Scale(0.3f, 0.3f, 0.3f);
    earthMatrix *= transform3D::RotateOY(angularEarthOY);
    RenderMesh(meshes["box"], shaders["Simple"], earthMatrix);

    moonMatrix = earthMatrix * transform3D::RotateOY(angularMoonOY);
    moonMatrix *= transform3D::Translate(2.5f, 0, 0);
    moonMatrix *= transform3D::Scale(0.2f, 0.2f, 0.2f);
    moonMatrix *= transform3D::RotateOY(angularMoonOY);
    RenderMesh(meshes["box"], shaders["Simple"], moonMatrix);
}


void Lab4::FrameEnd()
{
    DrawCoordinateSystem();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab4::OnInputUpdate(float deltaTime, int mods)
{
    // TODO(student): Add transformation logic
    int speed = 5;
    int scale = 2;
    float angle = 1.2f;

    if (window->KeyHold(GLFW_KEY_W)) {
        translateZ -= (speed * deltaTime);
    }
    else if (window->KeyHold(GLFW_KEY_S)) {
        translateZ += (speed * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_A)) {
        translateX -= (speed * deltaTime);
    }
    else if (window->KeyHold(GLFW_KEY_D)) {
        translateX += (speed * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_R)) {
        translateY += (speed * deltaTime);
    }
    else if (window->KeyHold(GLFW_KEY_F)) {
        translateY -= (speed * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_1)) {
        scaleX += (scale * deltaTime);
        scaleY += (scale * deltaTime);
        scaleZ += (scale * deltaTime);
    }
    else if (window->KeyHold(GLFW_KEY_2)) {
        scaleX -= (scale * deltaTime);
        scaleY -= (scale * deltaTime);
        scaleZ -= (scale * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_3)) {
        angularStepOX += (angle * deltaTime);
    }
    else if (window->KeyHold(GLFW_KEY_4)) {
        angularStepOX -= (angle * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_5)) {
        angularStepOY += (angle * deltaTime);
    }
    else if (window->KeyHold(GLFW_KEY_6)) {
        angularStepOY -= (angle * deltaTime);
    }

    if (window->KeyHold(GLFW_KEY_7)) {
        angularStepOZ += (angle * deltaTime);
    }
    else if (window->KeyHold(GLFW_KEY_8)) {
        angularStepOZ -= (angle * deltaTime);
    }
}


void Lab4::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_SPACE)
    {
        switch (polygonMode)
        {
        case GL_POINT:
            polygonMode = GL_FILL;
            break;
        case GL_LINE:
            polygonMode = GL_POINT;
            break;
        default:
            polygonMode = GL_LINE;
            break;
        }
    }
}


void Lab4::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab4::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab4::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab4::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab4::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab4::OnWindowResize(int width, int height)
{
}
