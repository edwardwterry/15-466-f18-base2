#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> //allows the use of 'uvec2' as an unordered_map key
#include <glm/gtx/string_cast.hpp> 

struct Game {
	glm::vec2 paddle = glm::vec2(0.0f,-3.0f);
	glm::vec2 ball = glm::vec2(0.0f, 0.0f);
	glm::vec2 ball_velocity = glm::vec2(0.0f,-2.0f);

	Game();

	void update(float time);

	static constexpr const float FrameWidth = 10.0f;
	static constexpr const float FrameHeight = 8.0f;
	static constexpr const float PaddleWidth = 2.0f;
	static constexpr const float PaddleHeight = 0.4f;
	static constexpr const float BallRadius = 0.5f;

	uint32_t grid_size = 3;
	std::unordered_map< uint32_t, glm::uvec2 > grid;
	
	// populate the hash table

	uint32_t edge_index = 0;
};
