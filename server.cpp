#include "Connection.hpp"
#include "Game.hpp"

#include <iostream>
#include <set>
#include <chrono>

#include <map>

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage:\n\t./server <port>" << std::endl;
		return 1;
	}
	
	Server server(argv[1]);

	Game state;

	std::unordered_map< Connection *,  Game::Controls> data;

	while (1) {
		server.poll([&](Connection *c, Connection::Event e) {
			if (e == Connection::OnOpen) {
				assert(!data.count(c));
				data.insert(std::make_pair(c, state.controls));
				std::cout<<"made a connection"<<std::endl;
			} else if (e == Connection::OnClose) {
				auto f =  data.find(c);
				assert(f != data.end());
				data.erase(f);
			} else if (e == Connection::OnRecv) {
				if (c->recv_buffer[0] == 'h') {
					c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1);
					std::cout << c << ": Got hello." << std::endl;
				} else if (c->recv_buffer[0] == 'c') {
					// std::cout<<"Getting stuff..."<<std::endl;
					if (c->recv_buffer.size() < 1 + sizeof(Game::Controls)) {
						return; //wait for more data
					} else {
						auto f =  data.find(c); // thanks Jim
						assert(f != data.end());
						memcpy(&f->second, c->recv_buffer.data() + 1, sizeof(Game::Controls));
						// std::cout<<f->second.up<<f->second.down<<f->second.left<<f->second.right<<f->second.lock;
						// std::cout<<"\n";
						c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 1 + sizeof(Game::Controls));
					}
				}
			}
		}, 0.01);

		// state.update();
		// std::cout<<"Active: "<<state.active_segment<<std::endl;

		// std::cout<<"sizeof(state.segment_status)"<<sizeof(state.segment_status)<<std::endl;
		// std::cout<<"sizeof(Game::segment_status)"<<sizeof(Game::segment_status)<<std::endl;

		// auto it = state.segment_status.begin();
		
		// while(it != state.segment_status.end())
		// {
		// 	std::cout<<*it;//<<std::endl;
		// 	it++;
		// }
		// std::cout<<"\n";

		if (!server.connections.empty()) {
			//send game state to client:		
			for (auto &c : server.connections){
				// c.send_raw("s", 1);
				// // for (uint32_t i = 0; i < state.segment_status.size(); i++){	
				// // 	c.send_raw(&state.segment_status[i], sizeof(uint32_t));		 <-- could never make this work
				// // }
				// c.send_raw(&state.segment_status, sizeof(Game::segment_status));
			}
		}		
	}
}
