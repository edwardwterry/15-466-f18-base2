#include "Game.hpp"

Game::Game(){
	{ // create hash map
		for (uint32_t r = 0; r < (mesh_size + 1); r++){
			uint32_t start_col = (r % 2 == 0) ? 1 : 0;
			for (uint32_t c = start_col; c < (mesh_size + 1); c+=2){
				std::pair< uint32_t, glm::uvec2 > coord (edge_index, glm::uvec2(r, c));
				std::pair< glm::uvec2, uint32_t > inv_coord (glm::uvec2(r, c), edge_index);
				grid.insert(coord);
				inv_grid.insert(inv_coord);
				edge_index++;
			}	
		}
		// for (auto it = grid.begin(); it != grid.end(); ++it){
		// 	std::cout<<it->first<<":"<<glm::to_string(it->second)<<std::endl;
		// }
		for (uint32_t i = 0; i < edge_index; i++){ // set all segments as inactive to start
			segment_status.insert(std::pair<uint32_t, uint32_t> (i, SegmentOptions::INACTIVE));
		}
		auto it = segment_status.find(active_segment);
		it->second = SegmentOptions::HOVER; // set the upper left element to active
	}

	{ // initialize number templates
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 1, 0, 1, 0, 1, 1}, 5));

		// subtraction for each element
		delta = 				{	0, 
									1 * (int)grid_size, 
									1 * (int)grid_size + 1, 
									2 * (int)grid_size + 1,
									3 * (int)grid_size + 1,
									3 * (int)grid_size + 2,
									4 * (int)grid_size + 2};
	}
}

void Game::update(float time) {
// void Game::update(const Controls &controls) {
	uint32_t dr = 0;
	uint32_t dc = 0;
	bool lock = false;
	if (controls.down) {
		dr++;
	} else if (controls.up) {
		dr--;
	} else if (controls.left) {
		dc--;
	} else if (controls.right) {
		dc++;
	} else if (controls.lock) {
		lock = true;
	}
	if (dc != 0) assert(dr == 0);
	if (dr != 0) assert(dc == 0);
	
	{
		glm::uvec2 current_coord = grid.find(active_segment)->second; 
		// can only move L/R on vert segments
		if (current_coord.y % 2 == 0 && dc != 0){ // if currently on a vertical segment and L/R command
			if ((current_coord.y + dc) <= 0 || (current_coord.y + dc) >= mesh_size) { // if move will take you out of bounds
				dc = 0; // do nothing
			} else {
				dc *= 2; // this will take you to adjacent vertical segment			
			}
		} else if (current_coord.y % 2 == 0 && dr != 0) { // if currently on a vertical segment and U/D command
			if ((current_coord.y + dr) <= 0 || (current_coord.y + dr) >= mesh_size) { // if move will take you out of bounds
				dr = 0; // do nothing
			} else {
				if (current_coord.y == mesh_size){ // if you're on RH boundary
					dc--; // shift left
				} else {
					dc++; // shift right by default
				}
			}
		} else if (current_coord.x % 2 == 0 && dc != 0){ // currently on a horiz segment
			dc = 0; // can't move left or right on a horiz segment
		} else if (current_coord.x % 2 == 0 && dr != 0){
			dc--; // shift left by default
		}

		glm::uvec2 next_coord = glm::uvec2(current_coord.x + dr, current_coord.y + dc);
		std::cout<<"Start: "<<glm::to_string(current_coord)<<" | End:"<<glm::to_string(next_coord)<<std::endl;
		active_segment = inv_grid.find(next_coord)->second;
		std::cout<<"New active segment: "<<active_segment<<std::endl;

		// TODO: make segments which have been visited but are neither locked nor scored, to be inactive
	}

	{ // slide template over screen
		auto get_score = [&](uint32_t &top){
			std::vector< bool > retrieved_status;
			for (uint32_t i = 0; i < num_digit_segments; i++){
				uint32_t segment_to_check = top + delta[i]; // uses the top segment, and the known deltas based on grid size
				// 1 if the segment is locked, 0 otherwise
				retrieved_status.emplace_back(segment_status.find(segment_to_check)->second == SegmentOptions::LOCKED);
			}
			if (number_templates.find(retrieved_status) != number_templates.end()){ // if exact bool match to any template
				for (uint32_t i = 0; i < num_digit_segments; i++){
					// set it to scored to prevent double counting
					segment_status.find(top + delta[i])->second = SegmentOptions::SCORED;
				}				
				return number_templates.find(retrieved_status)->second;
			} else {
				return (uint32_t)0;
			}
		};

		for (uint32_t r = 0; r < mesh_size - 3; r += 2){
			for (uint32_t c = 1; c < mesh_size; c += 2){
				score += get_score(inv_grid.find(glm::uvec2(r, c))->second);
			}	
		}
	}

	std::vector< bool > to_check;
	// segment_status.find(active_segment)->second = SegmentOptions::HOVER;

	// if (lock) {
	// 	segment_status.find(active_segment)->second = SegmentOptions::LOCKED;
	// }

	// ball += ball_velocity * time;
	// if (ball.x >= 0.5f * FrameWidth - BallRadius) {
	// 	ball_velocity.x = -std::abs(ball_velocity.x);
	// }
	// if (ball.x <=-0.5f * FrameWidth + BallRadius) {
	// 	ball_velocity.x = std::abs(ball_velocity.x);
	// }
	// if (ball.y >= 0.5f * FrameHeight - BallRadius) {
	// 	ball_velocity.y = -std::abs(ball_velocity.y);
	// }
	// if (ball.y <=-0.5f * FrameHeight + BallRadius) {
	// 	ball_velocity.y = std::abs(ball_velocity.y);
	// }

	// auto do_point = [this](glm::vec2 const &pt) {
	// 	glm::vec2 to = ball - pt;
	// 	float len2 = glm::dot(to, to);
	// 	if (len2 > BallRadius * BallRadius) return;
	// 	//if point is inside ball, make ball velocity outward-going:
	// 	float d = glm::dot(ball_velocity, to);
	// 	ball_velocity += ((std::abs(d) - d) / len2) * to;
	// };

	// do_point(glm::vec2(paddle.x - 0.5f * PaddleWidth, paddle.y));
	// do_point(glm::vec2(paddle.x + 0.5f * PaddleWidth, paddle.y));

	// auto do_edge = [&](glm::vec2 const &a, glm::vec2 const &b) {
	// 	float along = glm::dot(ball-a, b-a);
	// 	float max = glm::dot(b-a,b-a);
	// 	if (along <= 0.0f || along >= max) return;
	// 	do_point(glm::mix(a,b,along/max));
	// };

	// do_edge(glm::vec2(paddle.x + 0.5f * PaddleWidth, paddle.y), glm::vec2(paddle.x - 0.5f * PaddleWidth, paddle.y));

}
