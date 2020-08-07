#include "graphics/shader/pipeline_layout.hpp"
#include "graphics/enums.hpp"
#include "system/system.hpp"
#include "system/log.hpp"
#include "graphics/shader/descriptors.hpp"
#include "graphics/memory/gpu_buffer.hpp"
#include "graphics/memory/texture_object.hpp"
#include "graphics/shader/sampler.hpp"

namespace ignis {

	//Construct valid register layouts

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, GPUBufferType type,
		u16 localId, u16 descriptorSetId, ShaderAccess access, usz bufferSize, bool isWritable
	): 
		name(name), bufferSize(bufferSize), globalId(globalId), localId(localId), descriptorSetId(descriptorSetId),
		type(
			type == GPUBufferType::UNIFORM ? ResourceType::CBUFFER : 
			ResourceType::BUFFER
		),
		bufferType(type), access(access), isWritable(isWritable) {}

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, TextureType type, u16 localId, u16 descriptorSetId,
		ShaderAccess access, GPUFormat textureFormat, bool isWritable
	):
		name(name), textureFormat(textureFormat.value), globalId(globalId), localId(localId), descriptorSetId(descriptorSetId), 
		type(ResourceType::IMAGE), textureType(type), access(access), isWritable(isWritable) {}

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, TextureType type, u16 localId, u16 descriptorSetId,
		ShaderAccess access
	):
		name(name), textureFormat(GPUFormat::NONE), globalId(globalId), localId(localId), descriptorSetId(descriptorSetId), 
		type(ResourceType::TEXTURE), textureType(type), 
		access(access), isWritable(isWritable) {}

	RegisterLayout::RegisterLayout(
		const String &name, u32 globalId, SamplerType type, u16 localId, u16 descriptorSetId,
		ShaderAccess access
	):
		name(name), globalId(globalId), localId(localId), descriptorSetId(descriptorSetId),
		type(
			type == SamplerType::SAMPLER ? ResourceType::SAMPLER :
			ResourceType::COMBINED_SAMPLER
		), samplerType(type),
		access(access), isWritable(false) {}

	RegisterLayout::RegisterLayout(): 
		name(), globalId(), localId(), descriptorSetId(), type(), access(), isWritable(), subType() { }

	bool RegisterLayout::operator==(const RegisterLayout &other) const {

		const bool contentsEqual = 
			(const u64&)globalId == (const u64&)other.globalId &&
			(const u64&)type == (const u64&)other.type;
		
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

		layoutsByName.reserve(layout.size());
		layoutsById.reserve(layout.size());
		layoutsByLocalId.reserve(layout.size());

		for (auto &l : layout) {

			if (l.type == ResourceType::BUFFER && !l.bufferSize)
				oic::System::log()->fatal("Buffer requires a stride or size");

			else if(l.type == ResourceType::TEXTURE && l.isWritable && !u8(l.textureType))
				oic::System::log()->fatal("Image requires a texture format");

			else if(l.type == ResourceType::NONE)
				oic::System::log()->fatal("Register requires a valid resource type");

			if(layoutsByName.find(l.name) != layoutsByName.end())
				oic::System::log()->fatal("Register names overlap");

			if(layoutsById.find(l.globalId) != layoutsById.end())
				oic::System::log()->fatal("Register global ids overlap");

			if (find(l.type, l.localId) != layoutsByLocalId.end())
				oic::System::log()->fatal("Register local ids overlap");

			layoutsByName[l.name] = l;
			layoutsById[l.globalId] = l;

			layoutsByLocalId[localHash(l.type, l.localId)] = l;
		}
	}

	bool PipelineLayout::isTextureCompatible(const RegisterLayout &layout, const GPUSubresource &subres, TextureObject *tex) {

		auto &res = subres.textureRange;

		if (layout.isWritable && !HasFlags(tex->getInfo().usage, GPUMemoryUsage::GPU_WRITE)) {
			oic::System::log()->error("TextureObject is not writable");
			return false;
		}

		if(layout.isWritable && tex->getInfo().format != layout.textureFormat) {
			oic::System::log()->error("TextureObject doesn't have correct texture format");
			return false;
		}

		if(
			(layout.subType & u8(SamplerType::PROPERTY_AS_TEXTURE)) != u8(tex->getInfo().textureType) ||
			(layout.subType & u8(SamplerType::PROPERTY_AS_TEXTURE)) != u8(res.subType)
		) {
			oic::System::log()->error("TextureObject doesn't have compatible texture type");
			return false;
		}

		if(res.minLevel + res.levelCount > tex->getInfo().mips) {
			oic::System::log()->error("Texture mip out of bounds");
			return false;
		}

		if(res.minLayer + res.layerCount > tex->getInfo().layers) {
			oic::System::log()->error("Texture layer out of bounds");
			return false;
		}

		return true;
	}

	bool PipelineLayout::isCompatible(const List<Descriptors*> &descriptors) const {

		for (auto &elem : info.layoutsById) {

			//General descriptor checks

			const RegisterLayout &layout = elem.second;

			if (layout.descriptorSetId >= descriptors.size()) {
				oic::System::log()->error("Descriptor set index out of bounds");
				return false;
			}

			Descriptors *desc = descriptors[layout.descriptorSetId];

			if (!desc) {
				oic::System::log()->error("Invalid descriptor set");
				return false;
			}

			auto it = desc->getInfo().resources.find(layout.globalId);

			if (it == desc->getInfo().resources.end()) {
				oic::System::log()->error("Layout element not contained in descriptor");
				return false;
			}

			const GPUSubresource &subres = it->second;

			if (subres.resource == nullptr)
				return true;

			//Type dependent checks

			//Texture validation

			if (TextureObject *tex = dynamic_cast<TextureObject*>(subres.resource)) {

				if (layout.isWritable && layout.type != ResourceType::IMAGE) {
					oic::System::log()->error("Incompatible resource type, expected image");
					return false;
				}

				else if (!layout.isWritable && layout.type != ResourceType::TEXTURE) {
					oic::System::log()->error("Incompatible resource type, expected texture");
					return false;
				}

				if (!isTextureCompatible(layout, subres, tex)) {
					oic::System::log()->error("Texture or image is incompatible");
					return false;
				}
			}

			//Buffer validation

			else if (GPUBuffer *b = dynamic_cast<GPUBuffer*>(subres.resource)) {

				if (b->getInfo().type != GPUBufferType::UNIFORM) {
					if (layout.type != ResourceType::BUFFER) {
						oic::System::log()->error("Invalid buffer type");
						return false;
					}
				}
				else if (layout.type != ResourceType::CBUFFER) {
					oic::System::log()->error("Invalid buffer type");
					return false;
				}

				if (subres.bufferRange.offset + subres.bufferRange.size > b->size()) {
					oic::System::log()->error("Buffer out of bounds");
					return false;
				}

				if (layout.bufferType != b->getInfo().type) {
					oic::System::log()->error("Incompatible buffer types");
					return false;
				}

				if (layout.isWritable && !HasFlags(b->getInfo().usage, GPUMemoryUsage::GPU_WRITE)) {
					oic::System::log()->error("GPU Buffer is not writable");
					return false;
				}

				if (layout.bufferType != GPUBufferType::STRUCTURED) {

					if (layout.bufferSize != subres.bufferRange.size) {
						oic::System::log()->error("Incompatible buffer sizes");
						return false;
					}

				}

				else if (subres.bufferRange.size % layout.bufferSize) {
					oic::System::log()->error("Invalid structured buffer stride");
					return false;
				}

			}

			//Sampler validation

			else if (Sampler *s = dynamic_cast<Sampler*>(subres.resource)) {

				if (subres.samplerData.texture) {

					if (layout.type != ResourceType::COMBINED_SAMPLER) {
						oic::System::log()->error("Texture can only be provided to sampler if they are combined");
						return false;
					}

					if (!isTextureCompatible(layout, subres, subres.samplerData.texture))
						return false;

				}

				else if (
					layout.type != ResourceType::SAMPLER || 
					layout.isWritable || 
					layout.samplerType != SamplerType::SAMPLER
				) {
					oic::System::log()->error("Sampler type invalid or writable sampler encountered");
					return false;
				}
			}

			//Other validation

			else return false;

		}

		return true;
	}
}