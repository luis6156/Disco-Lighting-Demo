#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include <chrono>
#include "lab_m1/Tema3/irrKlang/irrKlang.h"

using namespace std;
using namespace irrklang;

namespace m1
{
    class Tema3 : public gfxc::SimpleScene
    {
     public:
        Tema3();
        ~Tema3();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMeshColor(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, const float transparency);
        void RenderMeshLight(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& object_color, const vector<glm::vec3>& lights_position = vector<glm::vec3>(), const vector<glm::vec3>& lights_color = vector<glm::vec3>());
        bool PointToRectangleCollision(float x1, float y1, float x2, float y2, float point_x, float point_y);
        bool DancerToWallsCollision(const glm::vec3& dancer_pos);
        Texture2D* CreateRandomTexture(unsigned int width, unsigned int height);
        void RenderMeshTexture(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix);
        void GenerateNewFloorColors();
        void GenerateNewSpotLightColors();
        void RenderSpots();
        void UpdateSpots(float deltaTimeSeconds);
        void RenderDancers();
        void UpdateDancers(float deltaTimeSeconds);
        void RenderCeiling();
        void RenderWalls();
        void RenderGround();
        void RenderDiscoBall();
        void MixColors();
        void UpdateMix();

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        glm::vec3 lightPosition;
        glm::vec3 lightDirection;
        unsigned int materialShininess;
        float materialKd;
        float materialKs;

        // TODO(student): If you need any other class variables, define them here.
        ISoundEngine* bgSound;
        int typeLight;
        float cutOff;
        float MIN_CUTOFF = 10;
        float MAX_CUTOFF = 160;
        glm::vec3 floor_colors[64];
        vector<glm::vec3> curr_floor_colors;
        vector<glm::vec3> prev_floor_colors;
        int num_dancers = 5;
        vector<glm::vec3> dancers_pos;
        vector<glm::vec3> ground_cells;
        vector<int> dancers_time;
        int dancers_max_time = 1;
        vector<int> dancers_dir;
        vector<float> dancers_speed;
        bool isPaused = false;
        vector<glm::vec3> spot_positions;
        glm::vec3 spot_colors[4];
        vector<glm::vec3> curr_spot_colors;
        vector<glm::vec3> prev_spot_colors;
        vector<glm::vec3> spot_directions;
        vector<glm::vec3> spot_rotation;
        vector<float> spot_speeds;
        vector<float> spot_angles;
        vector<int> spot_times;
        Texture2D* curr_texture;
        Texture2D* prev_texture;
        float mix_factor = 0;
        std::chrono::time_point<std::chrono::system_clock> time_texture;
        glm::vec3 disco_ball_position = glm::vec3(0.f, 1.75f, -2.f);
        bool isMusicOff = false;
        bool isPointlightOn = true;
        bool isSpotlightOn = true;
        bool isDiscoBallOn = true;
        int last_jump_time;
        bool isJumping = false;
        int jumper_idx = 0;
    };
}   // namespace m1
