#ifndef EXTRACTS
#define EXTRACTS
#pragma once

#include "../../utils/vectors/vectors.hpp"

#include "../entity/entity.hpp"

class extraction : public entity{
public:
	std::string name;

	enum EExfiltrationStatus {
		NotPresent = 1,
		UncompleteRequirements,
		Countdown,
		RegularMode,
		Pending,
		AwaitsManualActivation
	};

	bool open_extract()
	{
		if (!entity)
			return false;

		BYTE status = OMG::KERNAL::read<BYTE>(entity + 0xA8); //_status (type: EFT.Interactive.EExfiltrationStatus)
		if (status == EExfiltrationStatus::UncompleteRequirements || EExfiltrationStatus::RegularMode || EExfiltrationStatus::AwaitsManualActivation)
			return true;

		return false;
	}

};
#endif