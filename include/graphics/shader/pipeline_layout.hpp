#pragma once
#include "types/types.hpp"
#include "graphics/graphics.hpp"

namespace ignis {

	struct GPUSubresource;

	//The layout of a register (texture, buffer or sampler)
	struct RegisterLayout {

		//TODO: Arrays

		String name;

		union {

			//(If buffer)
			//	(If structured) represents stride
			//	(Else)			represents required buffer size
			usz	  bufferSize{};

			//(If texture)
			GPUFormat::_E textureFormat;
		};

		u32 globalId;						//Global bind point (increases per register)

		//Local bind point, increase for every similar type;
		//(UNIFORM buffer, other buffer, texture, sampler)
		u16 localId;

		//Index into descriptors passed to SetDescriptors command
		u16 descriptorSetId;

		ResourceType type;					//The register type (buffer, texture, sampler)

		union {								//Subtype definition; use these depending on RegisterType
			GPUBufferType bufferType;		//(If buffer)
			TextureType   textureType;		//(If texture)
			SamplerType   samplerType;		//(If sampler)
			u8			  subType;			//Type dependent subtype
		};

		bool isWritable;					//If the register is GPU writable

		u8 pad{};

		ShaderAccess access;				//By which shaders this can be accessed

		//Null register (only valid for use in Lists)
		RegisterLayout();

		//Construct a buffer register
		RegisterLayout(
			const String &name, u32 globalId, GPUBufferType type, u16 localId, u16 descriptorSetId,
			ShaderAccess access, usz bufferSize, bool isWritable = false
		);

		//Construct an image register
		RegisterLayout(
			const String &name, u32 globalId, TextureType type, u16 localId, u16 descriptorSetId,
			ShaderAccess access, GPUFormat textureFormat, bool writable = false
		);

		//Construct a texture register
		RegisterLayout(
			const String &name, u32 globalId, TextureType type, u16 localId, u16 descriptorSetId,
			ShaderAccess access
		);

		//Construct a sampler register
		RegisterLayout(
			const String &name, u32 globalId, SamplerType type, u16 localId, u16 descriptorSetId,
			ShaderAccess access
		);

		//Check if two registers are equal (ignore name)
		bool operator==(const RegisterLayout &other) const;
		inline bool operator!=(const RegisterLayout &other) const { return !operator==(other); }

	};

	//The layout of a pipeline (all registers that can span across multiple pipelines)
	class PipelineLayout : public GPUObject {

	public:

		struct Info {

			//Don't rely on the Strings to be human readable; they might use HASH("Name")
			//If HASH("YourVariable") is used in operator[] it can still be used though.
			//Names can also be completely stripped, then all lookups by name will fail
			HashMap<String, RegisterLayout> layoutsByName;

			//Index using global register index
			HashMap<u32, RegisterLayout> layoutsById;

			//Index using local register index
			//higher u32: registerType
			//lower u32: localId
			HashMap<u64, RegisterLayout> layoutsByLocalId;

			//Empty layout
			Info() {}

			//Construct a pipeline layout from register layouts
			Info(const List<RegisterLayout> &layout);

			//Construct a pipeline layout from register layouts
			template<typename ...args>
			Info(const RegisterLayout &layout0, const args &...arg):
				Info(List<RegisterLayout>{ layout0, arg... }) { }

			//Helper functions

			inline auto begin() const { return layoutsById.begin(); }
			inline auto end() const { return layoutsById.end(); }
			inline usz size() const { return layoutsById.size(); }

			//Lookup by global id
			inline auto operator[](const u32 globalId) const { return layoutsById.find(globalId); }

			//Lookup by name
			//Use HASH("YourVariable") if constructed with it; this will not expose "YourVariable" to the program though.
			inline auto operator[](const String &str) const { return layoutsByName.find(str); }

			//
			inline bool operator==(const Info &other) const {
				return layoutsById == other.layoutsById;
			}

			//Get hash for local id by using register type
			inline static constexpr u64 localHash(const ResourceType type, const u16 localId) {
				return (u64(type) << 32) | localId;
			}

			//Find by local id and register type
			inline auto find(const ResourceType type, const u16 localId) const {
				return layoutsByLocalId.find(localHash(type, localId));
			}
		};

		apimpl struct Data;

		inline const Info &getInfo() const { return info; }
		inline Data *getData() const { return data; }

		apimpl PipelineLayout(Graphics &g, const String &name, const Info &info);

		bool isCompatible(const List<Descriptors*> &descriptors) const;

	private:

		static bool isTextureCompatible(const RegisterLayout &layout, const GPUSubresource &res, TextureObject *tex);

		apimpl ~PipelineLayout();

		Info info;
		Data *data{};
	};

	using PipelineLayoutRef = GraphicsObjectRef<PipelineLayout>;
}