#ifndef gameworld_hpp
#define gameworld_hpp
#pragma once
#include "../eft.hpp"

class gameworld : public entity {
private:

public:    

	void set_time(float time) {
		auto time_class = OMG::KERNAL::read<uintptr_t>(entity + 0x28);
		if (!time_class)
			return;

		OMG::KERNAL::write<uintptr_t>(time_class + 0x20, (time / 24.f) * 1000000000000);
	};

};
#endif