#pragma once
#include "graphics/commandlist.hpp"
#include "graphics/commandops.hpp"

//General GPU commands
//These have to be implemented for every CommandList implementation

namespace ignis {

	struct Pipeline;
	struct Surface;

	//Commands

	struct BindPipelineCmd : public Command {
		Pipeline *pipeline;
		BindPipelineCmd(Pipeline *pipeline): Command(CMD_BIND_PIPELINE, sizeof(*this)), pipeline(pipeline) {}
	};
	

}