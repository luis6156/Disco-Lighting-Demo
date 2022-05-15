#include "lab_m1/Tema3/tema3.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema3::Tema3()
{
	typeLight = 1;
	cutOff = RADIANS(30);
}


Tema3::~Tema3()
{
}

Mesh* CreateCircle(
	const std::string& name,
	float radius,
	int nr_triangles) {

	float tmp = 2 * 3.141592f / nr_triangles;

	std::vector<VertexFormat> vertices =
	{
		VertexFormat(glm::vec3(0.f))
	};

	std::vector<unsigned int> indices = { 0 };

	for (int i = nr_triangles - 1; i >= 0; --i) {
		vertices.emplace_back(
			VertexFormat(
				glm::vec3(
					radius * cos(i * tmp), // + x = 0
					-1.f, // + y = 0
					radius * sin(i * tmp))
			)
		);
	}

	for (int i = 1; i <= nr_triangles; ++i) {
		indices.push_back(i);
	}
	indices.push_back(1);

	Mesh* cone = new Mesh(name);

	cone->InitFromData(vertices, indices);
	cone->SetDrawMode(GL_TRIANGLE_FAN);

	return cone;
}

void Tema3::Init()
{
	bgSound = createIrrKlangDevice();
	string pathString = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema3", "sounds", "music.mp3");
	char* path = &pathString[0];
	bgSound->play2D(path, true);
	bgSound->setSoundVolume(0.8f);

	{
		Mesh* mesh = new Mesh("box");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("sphere");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("plane");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = new Mesh("player");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "characters"), "player.fbx");
		meshes[mesh->GetMeshID()] = mesh;
	}

	{
		Mesh* mesh = CreateCircle("cone", 1.f, 100);
		AddMeshToList(mesh);
	}

	{
		curr_texture = CreateRandomTexture(16, 16);
		prev_texture = curr_texture;
	}

	// Create a shader program for drawing face polygon with the color of the normal
	{
		Shader* shader = new Shader("ColorShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema3", "shaders", "VertexShaderColor.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema3", "shaders", "FragmentShaderColor.glsl"), GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("LightShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema3", "shaders", "VertexShaderLight.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema3", "shaders", "FragmentShaderLight.glsl"), GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	{
		Shader* shader = new Shader("DiscoBallShader");
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema3", "shaders", "VertexShaderLight.glsl"), GL_VERTEX_SHADER);
		shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema3", "shaders", "FragmentShaderDiscoBall.glsl"), GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	// Light & material properties
	{
		lightPosition = glm::vec3(0, 1, 1);
		lightDirection = glm::vec3(0, -1, 0);
		materialShininess = 30;
		materialKd = 1.;
		materialKs = 1.;
	}

	// Generate random colors
	GenerateNewFloorColors();
	prev_floor_colors = curr_floor_colors;

	// Generate random dancers positions
	//dancers_pos.push_back(glm::vec3(-2.f + 0.25f, 0.25f + 0.05f, -0.25f));
	for (int i = 0; i < num_dancers; ++i) {
		glm::vec3 pos = glm::vec3(rand() % 8, 0.05f, rand() % 8);
		pos.x = pos.x * 0.5f - 2.f + 0.25f;
		pos.z = -pos.z * 0.5f - 0.25f;
		while (find(dancers_pos.begin(), dancers_pos.end(), pos) != dancers_pos.end() || 
			find(dancers_pos.begin(), dancers_pos.end(), glm::vec3(pos.x + 1, pos.y, pos.z)) != dancers_pos.end() ||
			find(dancers_pos.begin(), dancers_pos.end(), glm::vec3(pos.x - 1, pos.y, pos.z)) != dancers_pos.end() ||
			find(dancers_pos.begin(), dancers_pos.end(), glm::vec3(pos.x, pos.y, pos.z + 1)) != dancers_pos.end() ||
			find(dancers_pos.begin(), dancers_pos.end(), glm::vec3(pos.x, pos.y, pos.z - 1)) != dancers_pos.end()) {
			pos.x = rand() % 8;
			pos.z = rand() % 8;
			pos.x = pos.x * 0.5f - 2.f + 0.25f;
			pos.z = -pos.z * 0.5f - 0.25f;
		}
		dancers_pos.push_back(pos);
		dancers_time.push_back(Engine::GetElapsedTime());
		dancers_dir.push_back(rand() % 4);
		float speed = 0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.2f - 0.5f)));
		dancers_speed.push_back(speed);
	}

	// Generate positions for ground cells
	{
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 8; ++j) {
				ground_cells.emplace_back(glm::vec3(j * 0.5f + 0.25f - 2.f, 0.025f, -i * 0.5f - 0.25f));
			}
		}
	}

	{
		spot_positions.emplace_back(glm::vec3(-1.2f, 2.f, -1.2f));
		spot_positions.emplace_back(glm::vec3(1.2f, 2.f, -1.2f));
		spot_positions.emplace_back(glm::vec3(-1.2f, 2.f, -3.2f));
		spot_positions.emplace_back(glm::vec3(1.2f, 2.f, -3.2f));

		GenerateNewSpotLightColors();
		prev_spot_colors = curr_spot_colors;
		for (int i = 0; i < 4; ++i) {
			spot_directions.emplace_back(glm::vec3(0.f, -1.f, 0.f));
			spot_rotation.emplace_back(glm::vec3(0.f));
			if (rand() % 2 > 0) {
				spot_rotation[i].x = 1.f;
			}
			if (rand() % 2) {
				spot_rotation[i].z = 1.f;
			}
			if (spot_rotation[i] == glm::vec3(0.f)) {
				if (rand() % 2) {
					spot_rotation[i].x = 1.f;
				}
				else {
					spot_rotation[i].z = 1.f;
				}
			}
			float speed = -0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.5f + 0.5f)));
			while (speed < 0.2f && speed > -0.2f) {
				speed = -0.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.5f + 0.5f)));
			}
			spot_speeds.push_back(speed);
			spot_angles.push_back(0.f);
			spot_times.push_back(Engine::GetElapsedTime());
		}
	}

	time_texture = std::chrono::system_clock::now();
	last_jump_time = Engine::GetElapsedTime();
}

void Tema3::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// Sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void Tema3::GenerateNewFloorColors() {
	prev_floor_colors = curr_floor_colors;
	curr_floor_colors.clear();
	for (int i = 0; i < 64; ++i) {
		glm::vec3 color = glm::vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
		while (find(curr_floor_colors.begin(), curr_floor_colors.end(), color) != curr_floor_colors.end()) {
			color.r = (float)rand() / RAND_MAX;
			color.g = (float)rand() / RAND_MAX;
			color.b = (float)rand() / RAND_MAX;
		}
		curr_floor_colors.push_back(color);
	}
}

void Tema3::GenerateNewSpotLightColors() {
	prev_spot_colors = curr_spot_colors;
	curr_spot_colors.clear();
	for (int i = 0; i < 4; ++i) {
		curr_spot_colors.emplace_back(glm::vec3((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX));
	}
}

void Tema3::UpdateMix() {
	if (!isPaused) {
		std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsed_time = now - time_texture;
		if (elapsed_time.count() > 3.f) {
			time_texture = now;
			mix_factor = 0.f;
			prev_texture = curr_texture;
			curr_texture = CreateRandomTexture(16, 16);
			GenerateNewFloorColors();
			GenerateNewSpotLightColors();
		}
		else {
			mix_factor = elapsed_time.count() / 3.f;
		}
	}
}

void Tema3::MixColors() {
	for (int i = 0; i < curr_floor_colors.size(); ++i) {
		floor_colors[i] = glm::mix(prev_floor_colors[i], curr_floor_colors[i], mix_factor);
	}
	for (int i = 0; i < curr_spot_colors.size(); ++i) {
		spot_colors[i] = glm::mix(prev_spot_colors[i], curr_spot_colors[i], mix_factor);
	}
}

void Tema3::RenderDiscoBall() {
	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, disco_ball_position);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
	if (isDiscoBallOn) {
		RenderMeshTexture(meshes["sphere"], shaders["DiscoBallShader"], modelMatrix);
	}
}

void Tema3::RenderGround() {
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			// Ground cells
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, ground_cells[i * 8 + j]);
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.05f, 0.5f));
			if (isPointlightOn) {
				RenderMeshLight(meshes["box"], shaders["LightShader"], modelMatrix, floor_colors[i * 8 + j]);
			}
			else {
				RenderMeshLight(meshes["box"], shaders["LightShader"], modelMatrix, glm::vec3(0.f));
			}
		}
	}
}

void Tema3::RenderWalls() {
	// Walls
	for (int j = 0; j < 8; ++j) {
		// Left
		{
			vector<glm::vec3> positions;
			vector<glm::vec3> colors;
			if (j == 0) {
				positions.push_back(ground_cells[0]);
				positions.push_back(ground_cells[8]);
				colors.push_back(floor_colors[0]);
				colors.push_back(floor_colors[8]);
			}
			else if (j == 7) {
				positions.push_back(ground_cells[56]);
				positions.push_back(ground_cells[48]);
				colors.push_back(floor_colors[56]);
				colors.push_back(floor_colors[48]);
			}
			else {
				positions.push_back(ground_cells[j * 8]);
				positions.push_back(ground_cells[(j - 1) * 8]);
				positions.push_back(ground_cells[(j + 1) * 8]);
				colors.push_back(floor_colors[j * 8]);
				colors.push_back(floor_colors[(j - 1) * 8]);
				colors.push_back(floor_colors[(j + 1) * 8]);
			}

			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.f - 0.05f, 1.f, -j * 0.5f - 0.25f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 2.f, 0.5f));
			RenderMeshLight(meshes["box"], shaders["LightShader"], modelMatrix, glm::vec3(0.f), positions, colors);
		}

		// Far
		{
			vector<glm::vec3> positions;
			vector<glm::vec3> colors;
			if (j == 0) {
				positions.push_back(ground_cells[56]);
				positions.push_back(ground_cells[57]);
				colors.push_back(floor_colors[56]);
				colors.push_back(floor_colors[57]);
			}
			else if (j == 7) {
				positions.push_back(ground_cells[63]);
				positions.push_back(ground_cells[62]);
				colors.push_back(floor_colors[63]);
				colors.push_back(floor_colors[62]);
			}
			else {
				positions.push_back(ground_cells[56 + j]);
				positions.push_back(ground_cells[55 + j]);
				positions.push_back(ground_cells[57 + j]);
				colors.push_back(floor_colors[56 + j]);
				colors.push_back(floor_colors[55 + j]);
				colors.push_back(floor_colors[57 + j]);
			}

			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(j * 0.5f - 2.f + 0.25f, 1.f, -4.f - 0.05f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 2.f, 0.1f));
			RenderMeshLight(meshes["box"], shaders["LightShader"], modelMatrix, glm::vec3(0.f), positions, colors);
		}

		// Right
		{
			vector<glm::vec3> positions;
			vector<glm::vec3> colors;
			if (j == 0) {
				positions.push_back(ground_cells[7]);
				positions.push_back(ground_cells[15]);
				colors.push_back(floor_colors[7]);
				colors.push_back(floor_colors[15]);
			}
			else if (j == 7) {
				positions.push_back(ground_cells[63]);
				positions.push_back(ground_cells[55]);
				colors.push_back(floor_colors[63]);
				colors.push_back(floor_colors[55]);
			}
			else {
				positions.push_back(ground_cells[8 * (j + 1) - 1]);
				positions.push_back(ground_cells[8 * (j + 2) - 1]);
				positions.push_back(ground_cells[8 * j - 1]);
				colors.push_back(floor_colors[8 * (j + 1) - 1]);
				colors.push_back(floor_colors[8 * (j + 2) - 1]);
				colors.push_back(floor_colors[8 * j - 1]);
			}

			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(2.f + 0.05f, 1.f, -j * 0.5f - 0.25f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 2.f, 0.5f));
			RenderMeshLight(meshes["box"], shaders["LightShader"], modelMatrix, glm::vec3(0.f), positions, colors);
		}
	}
}

void Tema3::RenderCeiling() {
	// Ceiling
	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, 2.f + 0.05f, -2.f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(4.f, 0.1f, 4.f));
	RenderMeshLight(meshes["box"], shaders["LightShader"], modelMatrix, glm::vec3(0.f));
}

void Tema3::UpdateDancers(float deltaTimeSeconds) {
	if (!isPaused) {
		for (int i = 0; i < num_dancers; ++i) {
			bool collision = false;
			if (dancers_dir[i] == 0) {
				if (DancerToWallsCollision(glm::vec3(dancers_pos[i].x - deltaTimeSeconds * dancers_speed[i], dancers_pos[i].y, dancers_pos[i].z))) {
					collision = true;
				}
			}
			else if (dancers_dir[i] == 1) {
				if (DancerToWallsCollision(glm::vec3(dancers_pos[i].x + deltaTimeSeconds * dancers_speed[i], dancers_pos[i].y, dancers_pos[i].z))) {
					collision = true;
				}
			}
			else if (dancers_dir[i] == 2) {
				if (DancerToWallsCollision(glm::vec3(dancers_pos[i].x, dancers_pos[i].y, dancers_pos[i].z - deltaTimeSeconds * dancers_speed[i]))) {
					collision = true;
				}
			}
			else {
				if (DancerToWallsCollision(glm::vec3(dancers_pos[i].x, dancers_pos[i].y, dancers_pos[i].z + deltaTimeSeconds * dancers_speed[i]))) {
					collision = true;
				}
			}

			if (Engine::GetElapsedTime() - dancers_time[i] > dancers_max_time || collision) {
				dancers_time[i] = Engine::GetElapsedTime();
				int choice = rand() % 100;
				if (choice <= 25) {
					dancers_dir[i] = 0;
				}
				else if (choice <= 50) {
					dancers_dir[i] = 1;
				}
				else if (choice <= 75) {
					dancers_dir[i] = 2;
				}
				else {
					dancers_dir[i] = 3;
				}
			}
			else {
				if (dancers_dir[i] == 0) {
					dancers_pos[i].x -= deltaTimeSeconds * dancers_speed[i];
				}
				else if (dancers_dir[i] == 1) {
					dancers_pos[i].x += deltaTimeSeconds * dancers_speed[i];
				}
				else if (dancers_dir[i] == 2) {
					dancers_pos[i].z -= deltaTimeSeconds * dancers_speed[i];
				}
				else {
					dancers_pos[i].z += deltaTimeSeconds * dancers_speed[i];
				}
			}
		}
		if (Engine::GetElapsedTime() - last_jump_time > 2 && !isJumping) {
			jumper_idx = rand() % dancers_pos.size();
			isJumping = true;
		}
	}
}

void Tema3::RenderDancers() {
	for (int i = 0; i < num_dancers; ++i) {
		glm::mat4 modelMatrix = glm::mat4(1);
		//modelMatrix = glm::translate(modelMatrix, glm::vec3(dancers_pos[i].x * 0.5f - 2.f + 0.25f, 0.25f + 0.05f, -dancers_pos[i].z * 0.5f - 0.25f));
		vector<glm::vec3> positions;
		vector<glm::vec3> colors;
		for (int j = 0; j < ground_cells.size(); ++j) {
			if (PointToRectangleCollision(ground_cells[j].x - 0.25f, ground_cells[j].z - 0.25f, ground_cells[j].x + 0.25f, ground_cells[j].z + 0.25f, dancers_pos[i].x, dancers_pos[i].z)) {
				positions.push_back(ground_cells[j]);
				colors.push_back(floor_colors[j]);
				if (j % 8 == 0) {
					if (j == 0) {
						positions.push_back(ground_cells[1]);
						positions.push_back(ground_cells[8]);
						positions.push_back(ground_cells[9]);
						colors.push_back(floor_colors[1]);
						colors.push_back(floor_colors[8]);
						colors.push_back(floor_colors[9]);
					}
					else if (j == 56) {
						positions.push_back(ground_cells[57]);
						positions.push_back(ground_cells[48]);
						positions.push_back(ground_cells[49]);
						colors.push_back(floor_colors[57]);
						colors.push_back(floor_colors[48]);
						colors.push_back(floor_colors[49]);
					}
					else {
						positions.push_back(ground_cells[j + 1]);
						positions.push_back(ground_cells[j + 8]);
						positions.push_back(ground_cells[j - 8]);
						positions.push_back(ground_cells[j + 9]);
						positions.push_back(ground_cells[j - 7]);
						colors.push_back(floor_colors[j + 1]);
						colors.push_back(floor_colors[j + 8]);
						colors.push_back(floor_colors[j - 8]);
						colors.push_back(floor_colors[j + 9]);
						colors.push_back(floor_colors[j - 7]);
					}
				}
				else if (j % 8 == 7) {
					if (j == 7) {
						positions.push_back(ground_cells[6]);
						positions.push_back(ground_cells[15]);
						positions.push_back(ground_cells[14]);
						colors.push_back(floor_colors[6]);
						colors.push_back(floor_colors[15]);
						colors.push_back(floor_colors[14]);
					}
					else if (j == 63) {
						positions.push_back(ground_cells[55]);
						positions.push_back(ground_cells[62]);
						positions.push_back(ground_cells[54]);
						colors.push_back(floor_colors[55]);
						colors.push_back(floor_colors[62]);
						colors.push_back(floor_colors[54]);
					}
					else {
						positions.push_back(ground_cells[j - 1]);
						positions.push_back(ground_cells[j + 8]);
						positions.push_back(ground_cells[j - 8]);
						positions.push_back(ground_cells[j - 9]);
						positions.push_back(ground_cells[j + 7]);
						colors.push_back(floor_colors[j - 1]);
						colors.push_back(floor_colors[j + 8]);
						colors.push_back(floor_colors[j - 8]);
						colors.push_back(floor_colors[j - 9]);
						colors.push_back(floor_colors[j + 7]);
					}
				}
				else if (j < 8) {
					positions.push_back(ground_cells[j + 8]);
					positions.push_back(ground_cells[j + 9]);
					positions.push_back(ground_cells[j + 7]);
					positions.push_back(ground_cells[j + 1]);
					positions.push_back(ground_cells[j - 1]);
					colors.push_back(floor_colors[j + 8]);
					colors.push_back(floor_colors[j + 9]);
					colors.push_back(floor_colors[j + 7]);
					colors.push_back(floor_colors[j + 1]);
					colors.push_back(floor_colors[j - 1]);
				}
				else if (j > 55) {
					positions.push_back(ground_cells[j - 1]);
					positions.push_back(ground_cells[j + 1]);
					positions.push_back(ground_cells[j - 8]);
					positions.push_back(ground_cells[j - 9]);
					positions.push_back(ground_cells[j - 7]);
					colors.push_back(floor_colors[j - 1]);
					colors.push_back(floor_colors[j + 1]);
					colors.push_back(floor_colors[j - 8]);
					colors.push_back(floor_colors[j - 9]);
					colors.push_back(floor_colors[j - 7]);
				}
				else {
					positions.push_back(ground_cells[j + 8]);
					positions.push_back(ground_cells[j - 8]);
					positions.push_back(ground_cells[j + 9]);
					positions.push_back(ground_cells[j + 7]);
					positions.push_back(ground_cells[j - 9]);
					positions.push_back(ground_cells[j - 7]);
					positions.push_back(ground_cells[j + 1]);
					positions.push_back(ground_cells[j - 1]);
					colors.push_back(floor_colors[j + 8]);
					colors.push_back(floor_colors[j - 8]);
					colors.push_back(floor_colors[j + 9]);
					colors.push_back(floor_colors[j + 7]);
					colors.push_back(floor_colors[j - 9]);
					colors.push_back(floor_colors[j - 7]);
					colors.push_back(floor_colors[j + 1]);
					colors.push_back(floor_colors[j - 1]);
				}
				break;
			}
		}

		if (!isPaused) {
			if (isJumping && dancers_pos[jumper_idx].y < 0.05f) {
				isJumping = false;
				dancers_pos[jumper_idx].y = 0.05f;
			}
			else if (isJumping) {
				dancers_pos[jumper_idx].y = glm::abs(cos(Engine::GetElapsedTime()));
			}
		}

		modelMatrix = glm::translate(modelMatrix, dancers_pos[i]);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
		RenderMeshLight(meshes["player"], shaders["LightShader"], modelMatrix, glm::vec3(0.f), positions, colors);
	}
}

void Tema3::UpdateSpots(float deltaTimeSeconds) {
	if (!isPaused) {
		for (int i = 0; i < spot_angles.size(); ++i) {
			if (Engine::GetElapsedTime() - spot_times[i] > 2.f && glm::abs(spot_angles[i]) < 0.01f) {
				spot_angles[i] = 0.f;
				spot_directions[i].x = 0.f;
				spot_directions[i].y = -1.f;
				spot_directions[i].z = 0.f;
				spot_times[i] = Engine::GetElapsedTime();
				if (rand() % 100 > 50) {
					if (rand() % 2 > 0) {
						if (spot_rotation[i].x == 0.f) {
							spot_rotation[i].x = 1.f;
						}
						else {
							spot_rotation[i].x = 0.f;
						}
					}
					if (rand() % 2) {
						if (spot_rotation[i].z == 0.f) {
							spot_rotation[i].z = 1.f;
						}
						else {
							spot_rotation[i].z = 0.f;
						}
					}
					if (spot_rotation[i] == glm::vec3(0.f)) {
						if (rand() % 2) {
							spot_rotation[i].x = 1.f;
						}
						else {
							spot_rotation[i].z = 1.f;
						}
					}
				}
			}
		}

		for (int i = 0; i < spot_directions.size(); ++i) {
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::rotate(modelMatrix, spot_speeds[i] * deltaTimeSeconds, spot_rotation[i]);
			spot_directions[i] = modelMatrix * glm::vec4(spot_directions[i], 1);
			float angle = glm::acos(glm::dot(glm::normalize(spot_directions[i]), glm::normalize(glm::vec3(0, -1.f, 0))));
			if (glm::abs(angle) > RADIANS(10.f)) {
				spot_speeds[i] = -spot_speeds[i];
				modelMatrix = glm::mat4(1);
				modelMatrix = glm::rotate(modelMatrix, spot_speeds[i] * deltaTimeSeconds, spot_rotation[i]);
				spot_directions[i] = modelMatrix * glm::vec4(spot_directions[i], 1);
			}
			else {
				spot_angles[i] += spot_speeds[i] * deltaTimeSeconds;
			}
		}
	}
}

void Tema3::RenderSpots() {
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	{
		for (int i = 0; i < spot_positions.size(); ++i) {
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, glm::vec3(spot_positions[i].x, 2.f, spot_positions[i].z));
			modelMatrix = glm::rotate(modelMatrix, spot_angles[i], spot_rotation[i]);
			modelMatrix = glm::scale(modelMatrix, glm::vec3(tan(RADIANS(20.f)), 1, tan(RADIANS(20.f))) * 2.1f);
			if (isSpotlightOn) {
				RenderMeshColor(meshes["cone"], shaders["ColorShader"], modelMatrix, spot_colors[i], 0.5f);
			}
		}
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

void Tema3::Update(float deltaTimeSeconds)
{
	UpdateMix();

	MixColors();

	RenderDiscoBall();

	RenderGround();

	RenderWalls();

	RenderCeiling();

	UpdateDancers(deltaTimeSeconds);

	RenderDancers();

	UpdateSpots(deltaTimeSeconds);

	RenderSpots();
}


void Tema3::FrameEnd()
{
	//DrawCoordinateSystem();
}

Texture2D* Tema3::CreateRandomTexture(unsigned int width, unsigned int height)
{
	GLuint textureID = 0;
	unsigned int channels = 3;
	unsigned int size = width * height * channels;
	unsigned char* data = new unsigned char[size];

	// TODO(student): Generate random texture data
	srand(time(NULL));
	for (int i = 0; i < size; ++i) {
		data[i] = rand();
	}

	// TODO(student): Generate and bind the new texture ID
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	if (GLEW_EXT_texture_filter_anisotropic) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
	}
	// TODO(student): Set the texture parameters (MIN_FILTER, MAG_FILTER and WRAPPING MODE) using glTexParameteri
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	CheckOpenGLError();

	// Use glTexImage2D to set the texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	// TODO(student): Generate texture mip-maps
	glGenerateMipmap(GL_TEXTURE_2D);

	CheckOpenGLError();

	// Save the texture into a wrapper Texture2D class for using easier later during rendering phase
	Texture2D* texture = new Texture2D();
	texture->Init(textureID, width, height, channels);

	SAFE_FREE_ARRAY(data);
	return texture;
}

bool Tema3::DancerToWallsCollision(const glm::vec3& dancer_pos) {
	if (dancer_pos.x - 0.1f < -2.f || dancer_pos.x + 0.1f > 2.f || dancer_pos.z - 0.1f < -4.f || dancer_pos.z + 0.1f > 0.f) return true;
	return false;
}

bool Tema3::PointToRectangleCollision(float x1, float y1, float x2, float y2, float point_x, float point_y) {
	if (x1 <= point_x && point_x <= x2 && y1 <= point_y && point_y <= y2) return true;
	return false;
}

void Tema3::RenderMeshLight(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& object_color, const vector<glm::vec3>& lights_position, const vector<glm::vec3>& lights_color)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// Render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Set shader uniforms for light properties
	if (!lights_position.empty()) {
		int light_position_shader = glGetUniformLocation(shader->program, "point_lights_position");
		glUniform3fv(light_position_shader, lights_position.size(), glm::value_ptr(lights_position[0]));

		int light_color_shader = glGetUniformLocation(shader->program, "point_lights_color");
		glUniform3fv(light_color_shader, lights_color.size(), glm::value_ptr(lights_color[0]));
	}

	int spot_light_position_shader = glGetUniformLocation(shader->program, "spot_lights_position");
	glUniform3fv(spot_light_position_shader, spot_positions.size(), glm::value_ptr(spot_positions[0]));

	int spot_light_color_shader = glGetUniformLocation(shader->program, "spot_lights_color");
	glUniform3fv(spot_light_color_shader, spot_positions.size(), glm::value_ptr(spot_colors[0]));

	int light_direction = glGetUniformLocation(shader->program, "spot_lights_direction");
	glUniform3fv(light_direction, spot_directions.size(), glm::value_ptr(spot_directions[0]));

	// Set eye position (camera position) uniform
	glm::vec3 eyePosition = GetSceneCamera()->m_transform->GetWorldPosition();
	int eye_position = glGetUniformLocation(shader->program, "eye_position");
	glUniform3f(eye_position, eyePosition.x, eyePosition.y, eyePosition.z);

	// Set material property uniforms (shininess, kd, ks, object color) 
	int material_shininess = glGetUniformLocation(shader->program, "material_shininess");
	glUniform1i(material_shininess, materialShininess);

	int material_kd = glGetUniformLocation(shader->program, "material_kd");
	glUniform1f(material_kd, materialKd);

	int material_ks = glGetUniformLocation(shader->program, "material_ks");
	glUniform1f(material_ks, materialKs);

	int object_color_shader = glGetUniformLocation(shader->program, "object_color");
	glUniform3f(object_color_shader, object_color.r, object_color.g, object_color.b);

	int num_lights = glGetUniformLocation(shader->program, "num_point_lights");
	glUniform1i(num_lights, lights_position.size());

	int spot_num_lights = glGetUniformLocation(shader->program, "num_spot_lights");
	glUniform1i(spot_num_lights, spot_positions.size());

	int time = glGetUniformLocation(shader->program, "time");
	glUniform1f(time, Engine::GetElapsedTime());

	int isDiscoBallOn_shader = glGetUniformLocation(shader->program, "isDiscoBallOn");
	glUniform1i(isDiscoBallOn_shader, isDiscoBallOn);

	int isPaused_shader = glGetUniformLocation(shader->program, "isPaused");
	glUniform1i(isPaused_shader, isPaused);

	int isSpotlightOn_shader = glGetUniformLocation(shader->program, "isSpotLightOn");
	glUniform1i(isSpotlightOn_shader, isSpotlightOn);

	int isPointlightOn_shader = glGetUniformLocation(shader->program, "isPointLightOn");
	glUniform1i(isPointlightOn_shader, isPointlightOn);

	int mix_factor_shader = glGetUniformLocation(shader->program, "mix_factor");
	glUniform1f(mix_factor_shader, mix_factor);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, curr_texture->GetTextureID());
	glUniform1i(glGetUniformLocation(shader->program, "texture"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, prev_texture->GetTextureID());
	glUniform1i(glGetUniformLocation(shader->program, "last_texture"), 0);

	int disco_ball_position_shader = glGetUniformLocation(shader->program, "disco_ball_position");
	glUniform3f(disco_ball_position_shader, disco_ball_position.x, disco_ball_position.y, disco_ball_position.z);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->m_VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Tema3::RenderMeshColor(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, const float transparency)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// Render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Set material property uniforms (shininess, kd, ks, object color) 
	int prev_object_color = glGetUniformLocation(shader->program, "object_color");
	glUniform3f(prev_object_color, color.r, color.g, color.b);

	int transparency_shader = glGetUniformLocation(shader->program, "transparency");
	glUniform1f(transparency_shader, transparency);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->m_VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Tema3::RenderMeshTexture(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
	if (!mesh || !shader || !shader->GetProgramID())
		return;

	// Render an object using the specified shader and the specified position
	glUseProgram(shader->program);

	// Set material property uniforms (shininess, kd, ks, object color) 
	int time = glGetUniformLocation(shader->program, "time");
	glUniform1f(time, Engine::GetElapsedTime());

	int mix_factor_shader = glGetUniformLocation(shader->program, "mix_factor");
	glUniform1f(mix_factor_shader, mix_factor);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, curr_texture->GetTextureID());
	glUniform1i(glGetUniformLocation(shader->program, "texture"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, prev_texture->GetTextureID());
	glUniform1i(glGetUniformLocation(shader->program, "last_texture"), 0);

	int disco_ball_position_shader = glGetUniformLocation(shader->program, "disco_ball_position");
	glUniform3f(disco_ball_position_shader, disco_ball_position.x, disco_ball_position.y, disco_ball_position.z);

	int isPaused_shader = glGetUniformLocation(shader->program, "isPaused");
	glUniform1i(isPaused_shader, isPaused);

	// Bind model matrix
	GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
	glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	// Bind view matrix
	glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
	int loc_view_matrix = glGetUniformLocation(shader->program, "View");
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

	// Bind projection matrix
	glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
	int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	// Draw the object
	glBindVertexArray(mesh->GetBuffers()->m_VAO);
	glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema3::OnInputUpdate(float deltaTime, int mods)
{
	float speed = 2;

	if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		glm::vec3 up = glm::vec3(0, 1, 0);
		glm::vec3 right = GetSceneCamera()->m_transform->GetLocalOXVector();
		glm::vec3 forward = GetSceneCamera()->m_transform->GetLocalOZVector();
		forward = glm::normalize(glm::vec3(forward.x, 0, forward.z));

		// Control light position using on W, A, S, D, E, Q
		if (window->KeyHold(GLFW_KEY_W)) lightPosition -= forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_A)) lightPosition -= right * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_S)) lightPosition += forward * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_D)) lightPosition += right * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_E)) lightPosition += up * deltaTime * speed;
		if (window->KeyHold(GLFW_KEY_Q)) lightPosition -= up * deltaTime * speed;

		// TODO(student): Set any other keys that you might need
		if (window->KeyHold(GLFW_KEY_1)) {
			if (cutOff < RADIANS(MAX_CUTOFF)) {
				cutOff += deltaTime;
			}
		}
		else if (window->KeyHold(GLFW_KEY_2)) {
			if (cutOff > RADIANS(MIN_CUTOFF)) {
				cutOff -= deltaTime;
			}
		}

		if (typeLight == 0) {
			if (window->KeyHold(GLFW_KEY_Z)) {
				glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1), deltaTime, glm::vec3(0, 0, 1));
				lightDirection = rotation_matrix * glm::vec4(lightDirection, 1);
			}
			else if (window->KeyHold(GLFW_KEY_X)) {
				glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1), -deltaTime, glm::vec3(0, 0, 1));
				lightDirection = rotation_matrix * glm::vec4(lightDirection, 1);
			}

			if (window->KeyHold(GLFW_KEY_N)) {
				glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1), deltaTime, glm::vec3(0, 1, 0));
				lightDirection = rotation_matrix * glm::vec4(lightDirection, 1);
			}
			else if (window->KeyHold(GLFW_KEY_M)) {
				glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1), -deltaTime, glm::vec3(0, 1, 0));
				lightDirection = rotation_matrix * glm::vec4(lightDirection, 1);
			}
		}
	}
}


void Tema3::OnKeyPress(int key, int mods)
{
	// Add key press event

	// TODO(student): Set keys that you might need
	if (key == GLFW_KEY_SPACE) {
		isPaused = !isPaused;
	}
	if (key == GLFW_KEY_M) {
		isMusicOff = !isMusicOff;
		bgSound->setAllSoundsPaused(isMusicOff);
	}
	if (key == GLFW_KEY_1) {
		isPointlightOn = !isPointlightOn;
	}
	if (key == GLFW_KEY_2) {
		isSpotlightOn = !isSpotlightOn;
	}
	if (key == GLFW_KEY_3) {
		isDiscoBallOn = !isDiscoBallOn;
	}

}


void Tema3::OnKeyRelease(int key, int mods)
{
	// Add key release event
}


void Tema3::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// Add mouse move event
}


void Tema3::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button press event
}


void Tema3::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button release event
}


void Tema3::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema3::OnWindowResize(int width, int height)
{
}
