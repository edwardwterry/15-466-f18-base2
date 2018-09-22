#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <map>
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp> //allows the use of 'uvec2' as an unordered_map key
#include <glm/gtx/string_cast.hpp> 

struct Game {
	glm::vec2 paddle = glm::vec2(0.0f,-3.0f);
	glm::vec2 ball = glm::vec2(0.0f, 0.0f);
	glm::vec2 ball_velocity = glm::vec2(0.0f,-2.0f);

	Game();

	void update(float time);
	// void update(const Game::Controls &c);

	void update_active_segment();

	static constexpr const float FrameWidth = 10.0f;
	static constexpr const float FrameHeight = 8.0f;
	static constexpr const float PaddleWidth = 2.0f;
	static constexpr const float PaddleHeight = 0.4f;
	static constexpr const float BallRadius = 0.5f;

	uint32_t grid_size = 3;
	uint32_t mesh_size = grid_size * 2; 
	std::unordered_map< uint32_t, glm::uvec2 > grid;
	std::unordered_map< glm::uvec2, uint32_t > inv_grid;
	uint32_t edge_index = 0;
	std::unordered_map< uint32_t, uint32_t > segment_status;
	uint32_t active_segment = 0;
	uint32_t num_digit_segments = 7;
	
	enum SegmentOptions{
		INACTIVE,
		HOVER,
		LOCKED,
		SCORED
	};

	struct Controls {
		bool up = 0;
		bool down = 0;
		bool left = 0;
		bool right = 0;
		bool lock = 0;
	} controls;

	std::map< std::vector< bool >, uint32_t > number_templates;
	std::vector< int32_t > delta;
	
	uint32_t score = 0;
};
