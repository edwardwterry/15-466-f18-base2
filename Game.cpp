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

		for (uint32_t i = 0; i < edge_index; i++){ // set all segments as inactive to start
			// segment_status.insert(std::pair<uint32_t, uint32_t> (i, SegmentOptions::INACTIVE));
			segment_status.push_back(SegmentOptions::INACTIVE);
		}
		// segment_status.find(active_segment)->second = SegmentOptions::HOVER; // set the upper left element to active
		segment_status[active_segment] = SegmentOptions::HOVER; // set the upper left element to active
	}

	{ // initialize number templates
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({0, 0, 1, 0, 0, 1, 0}, 1));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 0, 1, 1, 1, 0, 1}, 2));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 0, 1, 1, 0, 1, 1}, 3));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({0, 1, 1, 1, 0, 1, 0}, 4));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 1, 0, 1, 0, 1, 1}, 5));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 1, 0, 1, 1, 1, 1}, 6));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 1, 1, 0, 0, 1, 0}, 7));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 1, 1, 1, 1, 1, 1}, 8));
		number_templates.emplace(std::make_pair<std::vector<bool>,uint32_t>({1, 1, 1, 1, 0, 1, 1}, 9));

		// subtraction for each element
		delta = 				{	0, 
									1 * (int)grid_size, 
									1 * (int)grid_size + 1, 
									2 * (int)grid_size + 1,
									3 * (int)grid_size + 1,
									3 * (int)grid_size + 2,
									4 * (int)grid_size + 2};

		std::vector< uint32_t > test_5 {17, 24, 32, 40, 47};
		for (uint32_t i = 0; i < test_5.size(); i++){
			segment_status[i] = SegmentOptions::LOCKED;
		}
	}
}

glm::vec3 Game::segment_id_to_coord(const uint32_t &id){
	glm::vec3 coord;
	coord.x = static_cast<float>(grid.find(id)->second.y) * 0.5f * grid_edge_length;
	coord.y = static_cast<float>(mesh_size - grid.find(id)->second.x) * 0.5f * grid_edge_length;
	coord.z = 0.01f;
	return coord;
}

void Game::update() {
	int32_t dr = 0;
	int32_t dc = 0;
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
	int32_t dc_input = dc;
	int32_t dr_input = dr;
	{ // work out next segment
		glm::uvec2 current_coord = grid.find(active_segment)->second; 
		// std::cout<<"(current_coord.x + dr): "<<(current_coord.x + dr)<<std::endl;
		// can only move L/R on vert segments
		if (current_coord.y % 2 == 0){ // if on a vertical segment
			if (dc_input != 0){ // and L/R command
				if ((current_coord.y + dc) <= 0 || (current_coord.y + dc) >= mesh_size) { // if move will take you out of bounds
					dc = 0; // do nothing
				} else {
					dc *= 2; // this will take you to adjacent vertical segment			
				}
			} else if (dr_input != 0){ // and U/D command
				if (current_coord.y == mesh_size){ // if you're on RH boundary
					dc--; // shift left
				} else {
					dc++; // shift right by default
				}
			}
		} else if (current_coord.x % 2 == 0){ // if on a horiz segment
			if (dc_input != 0){ // and L/R command
				dc = 0;		
			} else if (dr_input != 0){ // and U/D command
				if ((current_coord.x + dr) <= 0 || (current_coord.x + dr) >= mesh_size) { // if move will take you out of bounds
					dr = 0; // do nothing
				} else {
					dc--;
				}
			}
		}

		glm::uvec2 next_coord = glm::uvec2(current_coord.x + dr, current_coord.y + dc);
		// std::cout<<"Start: "<<glm::to_string(current_coord)<<" | End:"<<glm::to_string(next_coord)<<std::endl;
		active_segment = inv_grid.find(next_coord)->second;
		// std::cout<<"New active segment: "<<active_segment<<std::endl;

		// TODO: make segments which have been visited but are neither locked nor scored, to be inactive
	}

	{ // slide template over screen
		auto get_score = [&](uint32_t &top){
			std::vector< bool > retrieved_status;
			for (uint32_t i = 0; i < num_digit_segments; i++){
				uint32_t segment_to_check = top + delta[i]; // uses the top segment, and the known deltas based on grid size
				// 1 if the segment is locked, 0 otherwise
				retrieved_status.emplace_back(segment_status[segment_to_check] == SegmentOptions::LOCKED);
			}
			if (number_templates.find(retrieved_status) != number_templates.end()){ // if exact bool match to any template
				for (uint32_t i = 0; i < num_digit_segments; i++){
					// set it to scored to prevent double counting
					segment_status[top + delta[i]] = SegmentOptions::SCORED;
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
		std::cout<<"Score: "<<score<<std::endl;
	}

	if (lock) {
		if (segment_status[active_segment] != SegmentOptions::SCORED){
			segment_status[active_segment] = SegmentOptions::LOCKED;
		}
	}

	for (uint32_t i = 0; i < edge_index; i++){ // set all remaining ones to inactive
		if (segment_status[i] != SegmentOptions::SCORED ||
			segment_status[i] != SegmentOptions::LOCKED){
			segment_status[i] = SegmentOptions::INACTIVE;
		}
	}	

	// leave with only 1 as active
	segment_status[active_segment] = SegmentOptions::HOVER;
}
