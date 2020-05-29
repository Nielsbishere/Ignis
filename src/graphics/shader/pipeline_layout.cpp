#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/enums.hpp"
#include "system/system.hpp"
#include "system/log.hpp"

namespace ignis {

	//Construct valid register layouts

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, GPUBufferType type,
		u32 localId, ShaderAccess access, usz bufferSize, bool isWritable
	): 
		name(name), bufferSize(bufferSize), globalId(globalId), localId(localId), 
		type(
			type == GPUBufferType::UNIFORM ? ResourceType::CBUFFER : 
			ResourceType::BUFFER
		),
		bufferType(type), access(access), isWritable(isWritable) {}

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, TextureType type, u32 localId,
		ShaderAccess access, GPUFormat textureFormat, bool isWritable
	):
		name(name), textureFormat(textureFormat), globalId(globalId), localId(localId), 
		type(ResourceType::IMAGE), textureType(type), access(access), isWritable(isWritable) {}

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, TextureType type, u32 localId,
		ShaderAccess access
	):
		name(name), textureFormat(GPUFormat::NONE), globalId(globalId), localId(localId), 
		type(ResourceType::TEXTURE), textureType(type), 
		access(access), isWritable(isWritable) {}

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, SamplerType type, u32 localId,
		ShaderAccess access
	):
		name(name), globalId(globalId), localId(localId), 
		type(
			type == SamplerType::SAMPLER ? ResourceType::SAMPLER :
			ResourceType::COMBINED_SAMPLER
		), samplerType(type),
		access(access), isWritable(false) {}

	RegisterLayout::RegisterLayout(): 
		name(), globalId(), localId(), type(), access(), isWritable() { }

	bool RegisterLayout::operator==(const RegisterLayout &other) const {

		const bool contentsEqual = 
			(u64&)globalId == (u64&)other.globalId &&
			(u16&)type == (u16&)other.type &&
			isWritable == other.isWritable;
		
		return

			//General equal (everything but union part and name)
			contentsEqual &&

			//Union part compare
			(
				(
					(
						type == ResourceType::IMAGE || type == ResourceType::TEXTURE
					) && 
					(
						!isWritable || textureFormat == other.textureFormat
					)
				) ||
				(
					(type == ResourceType::BUFFER || type == ResourceType::CBUFFER) && 
					(!bufferSize || bufferSize == other.bufferSize)
				) ||
				type == ResourceType::SAMPLER ||
				type == ResourceType::COMBINED_SAMPLER
			);
	}

	//Validate pipeline layout and create look-up tables

	PipelineLayout::Info::Info(const List<RegisterLayout> &layout) {

		for (auto &l : layout)

			if (l.type == ResourceType::BUFFER && !l.bufferSize)
				oic::System::log()->fatal("Buffer requires a stride or size");

			else if(l.type == ResourceType::TEXTURE && l.isWritable && !u8(l.textureType))
				oic::System::log()->fatal("Image requires a texture format");

			else if(l.type == ResourceType::NONE)
				oic::System::log()->fatal("Register requires a valid resource type");

		layoutsByName.reserve(layout.size());
		layoutsById.reserve(layout.size());

		for (auto &reg : layout) {

			if (reg.name.size()) {

				if (operator[](reg.name) != layoutsByName.end())
					oic::System::log()->fatal("Register names overlap");

				layoutsByName[reg.name] = reg;
			}

			if (operator[](reg.globalId) != layoutsById.end())
				oic::System::log()->fatal("Register global ids overlap");

			layoutsById[reg.globalId] = reg;

			if (find(reg.type, reg.localId) != layoutsByLocalId.end())
				oic::System::log()->fatal("Register local ids overlap");

			layoutsByLocalId[localHash(reg.type, reg.localId)] = reg;
		}
	}

	bool PipelineLayout::supportsLayout(const PipelineLayout *other) const {

		if (!other) return true;

		for (auto &elem : info.layoutsById) {

			auto &reg = elem.second;
			auto regIt = other->info[elem.first];

			if (regIt == other->info.end() || regIt->second != reg)
				return false;
		}

		return true;
	}
}