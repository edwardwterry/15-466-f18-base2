#include "GameMode.hpp"

#include "MenuMode.hpp"
#include "Load.hpp"
#include "MeshBuffer.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp" //helper for dumpping OpenGL error messages
#include "read_chunk.hpp" //helper for reading a vector of structures from a file
#include "data_path.hpp" //helper to get paths relative to executable
#include "compile_program.hpp" //helper to compile opengl shader programs
#include "draw_text.hpp" //helper to... um.. draw text
#include "vertex_color_program.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>


Load< MeshBuffer > meshes(LoadTagDefault, [](){
	return new MeshBuffer(data_path("grid.pnc"));
});

Load< GLuint > meshes_for_vertex_color_program(LoadTagDefault, [](){
	return new GLuint(meshes->make_vao_for_program(vertex_color_program->program));
});

Scene::Transform *hover_seg_transform = nullptr;
// Scene::Transform *p1_seg_transform = nullptr;
// Scene::Transform *p2_seg_transform = nullptr;

Scene::Camera *camera = nullptr;

Load< Scene > scene(LoadTagDefault, [](){
	Scene *ret = new Scene;
	//load transform hierarchy:
	ret->load(data_path("grid.scene"), [](Scene &s, Scene::Transform *t, std::string const &m){
		Scene::Object *obj = s.new_object(t);

		obj->program = vertex_color_program->program;
		obj->program_mvp_mat4  = vertex_color_program->object_to_clip_mat4;
		obj->program_mv_mat4x3 = vertex_color_program->object_to_light_mat4x3;
		obj->program_itmv_mat3 = vertex_color_program->normal_to_light_mat3;

		MeshBuffer::Mesh const &mesh = meshes->lookup(m);
		obj->vao = *meshes_for_vertex_color_program;
		obj->start = mesh.start;
		obj->count = mesh.count;
	});

	//look up paddle and ball transforms:
	for (Scene::Transform *t = ret->first_transform; t != nullptr; t = t->alloc_next) {
		if (t->name == "Hover_Seg") {
			if (hover_seg_transform) throw std::runtime_error("Multiple 'Paddle' transforms in scene.");
			hover_seg_transform = t;
		}
		// if (t->name == "P1_Seg") {
		// 	if (p1_seg_transform) throw std::runtime_error("Multiple 'Paddle' transforms in scene.");
		// 	p1_seg_transform = t;
		// }
		// if (t->name == "P2_Seg") {
		// 	if (p2_seg_transform) throw std::runtime_error("Multiple 'Paddle' transforms in scene.");
		// 	p2_seg_transform = t;
		// }
	}
	if (!hover_seg_transform) throw std::runtime_error("No 'Hover_Seg' transform in scene.");
	// if (!ball_transform) throw std::runtime_error("No 'Ball' transform in scene.");
	// if (!grid_transform) throw std::runtime_error("No 'Ball' transform in scene.");

	//look up the camera:
	for (Scene::Camera *c = ret->first_camera; c != nullptr; c = c->alloc_next) {
		if (c->transform->name == "Camera") {
			if (camera) throw std::runtime_error("Multiple 'Camera' objects in scene.");
			camera = c;
		}
	}
	if (!camera) throw std::runtime_error("No 'Camera' camera in scene.");
	return ret;
});

GameMode::GameMode(Client &client_) : client(client_) {
	client.connection.send_raw("h", 1); //send a 'hello' to the server
}

GameMode::~GameMode() {
}

bool GameMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}

	if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
			controls.up = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
			controls.down = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
			controls.left = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
			controls.right = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_SPACE) {
			controls.lock = (evt.type == SDL_KEYDOWN);
			return true;
		}
	}
	// if (client.connection) {
	// 	//send game state to server:
	// 	client.connection.send_raw("c", 1);
	// 	client.connection.send_raw(&controls, sizeof(Controls));
	// }	
	return false;
}

void GameMode::update(float elapsed) {
	state.controls.up = controls.up;
	state.controls.down = controls.down;
	state.controls.left = controls.left;
	state.controls.right = controls.right;
	state.controls.lock = controls.lock;
	state.update();

	if (client.connection) {
		//send game state to server:
		client.connection.send_raw("c", 1);
		client.connection.send_raw(&controls, sizeof(Controls));
	}


	// int count = 0;
	client.poll([&](Connection *c, Connection::Event event){
		if (event == Connection::OnOpen) {
			//probably won't get this.
		} else if (event == Connection::OnClose) {
			std::cerr << "Lost connection to server." << std::endl;
		} else { assert(event == Connection::OnRecv);
			if (c->recv_buffer[0] == 's') {
				std::cout<<"c->recv_buffer.size(): "<<c->recv_buffer.size()<<std::endl;
				if (c->recv_buffer.size() < (1 + sizeof(uint32_t) * state.segment_status.size())) {
				// if (c->recv_buffer.size() < 1 + sizeof(Game::grid_size)) {
					// std::cout<<"count: "<<count<<std::endl;
					// count++;
					return; //wait for more data
				} else {
					// memcpy(&state.segment_status, c->recv_buffer.data() + 1, sizeof(Game::segment_status));
					// c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1 + sizeof(Game::segment_status));
					// memcpy(&state.grid_size, c->recv_buffer.data() + 1, sizeof(Game::grid_size));
					// c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1 + sizeof(Game::grid_size));
				}
			}		
			// std::cerr << "Ignoring " << c->recv_buffer.size() << " bytes from server." << std::endl;
			// c->recv_buffer.clear();
		}
	}, 0.01);

	//copy game state to scene positions:
	// std::cout<<state.grid_size<<std::endl;//->second;
	// auto it = state.grid_size.begin();
	// auto it = state.segment_status.begin();
	
	// // // Iterate over the map using iterator
	// while(it != state.segment_status.end())
	// {
	// 	std::cout<<*it<<std::endl;
	// 	it++;
	// }
	// std::cout<<"\n";

	// move player cursor
	hover_seg_transform->position = state.segment_id_to_coord(state.active_segment);
	if (state.grid.find(state.active_segment)->second.x % 2 != 0){ // if on a vertical
		hover_seg_transform->rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	} else {
		hover_seg_transform->rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
}

void GameMode::draw(glm::uvec2 const &drawable_size) {
	camera->aspect = drawable_size.x / float(drawable_size.y);

	glClearColor(0.25f, 0.0f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set up basic OpenGL state:
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//set up light positions:
	glUseProgram(vertex_color_program->program);

	glUniform3fv(vertex_color_program->sun_color_vec3, 1, glm::value_ptr(glm::vec3(0.81f, 0.81f, 0.76f)));
	glUniform3fv(vertex_color_program->sun_direction_vec3, 1, glm::value_ptr(glm::normalize(glm::vec3(-0.2f, 0.2f, 1.0f))));
	glUniform3fv(vertex_color_program->sky_color_vec3, 1, glm::value_ptr(glm::vec3(0.2f, 0.2f, 0.3f)));
	glUniform3fv(vertex_color_program->sky_direction_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 1.0f, 0.0f)));

	scene->draw(camera);

	GL_ERRORS();
}
