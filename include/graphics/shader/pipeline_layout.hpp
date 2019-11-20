#pragma once
#include "types/types.hpp"

namespace ignis {

	enum class ShaderAccess : u32;
	enum class GPUBufferType : u8;
	enum class TextureType : u8;
	enum class ResourceType : u8;
	enum class SamplerType : u8;
	enum class GPUFormat : u16;

	//The layout of a register (texture, buffer or sampler)
	struct RegisterLayout {

		//TODO: Arrays

		String name;

		union {

			//(If buffer)
			//	(If structured) represents stride
			//	(Else)			represents required buffer size
			usz	  bufferSize;

			//(If texture)
			GPUFormat textureFormat;
		};

		u32 globalId;						//Global bind point (increases per register)

		//Local bind point, increase for every similar type;
		//(UNIFORM buffer, other buffer, texture, sampler)
		u32 localId;

		ResourceType type;					//The register type (buffer, texture, sampler)

		union {								//Subtype definition; use these depending on RegisterType
			GPUBufferType bufferType;		//(If buffer)
			TextureType   textureType;		//(If texture)
			SamplerType   samplerType;		//(If sampler)
			u8			  subType;			//Type dependent subtype
		};

		ShaderAccess access;				//By which shaders this can be accessed

		bool isWritable;					//If the register is GPU writable

		//Null register (only valid for use in Lists)
		RegisterLayout();

		//Construct a buffer register
		RegisterLayout(
			const String &name, u32 globalId, GPUBufferType type, u32 localId,
			ShaderAccess access, usz bufferSize, bool isWritable = false
		);

		//Construct a RW texture register
		RegisterLayout(
			const String &name, u32 globalId, TextureType type, u32 localId,
			ShaderAccess access, GPUFormat textureFormat, bool writable = false
		);

		//Construct a texture register
		RegisterLayout(
			const String &name, u32 globalId, TextureType type, u32 localId,
			ShaderAccess access, bool writable = false
		);

		//Construct a sampler register
		RegisterLayout(
			const String &name, u32 globalId, SamplerType type, u32 localId,
			ShaderAccess access
		);

		//Check if two registers are equal (ignore name)
		bool operator==(const RegisterLayout &other) const;
		inline bool operator!=(const RegisterLayout &other) const { return !operator==(other); }

	};

	//The layout of a pipeline (all registers that can span across multiple pipelines)
	struct PipelineLayout {

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

		//Construct a pipeline layout from register layouts
		PipelineLayout(const List<RegisterLayout> &layout);

		//Construct a pipeline layout from register layouts
		template<typename ...args>
		PipelineLayout(const RegisterLayout &layout0, const args &...arg):
			PipelineLayout(List<RegisterLayout>{ layout0, arg... }) { }

		//Helper functions

		inline auto begin() const { return layoutsById.begin(); }
		inline auto end() const { return layoutsById.end(); }
		inline usz size() const { return layoutsById.size(); }

		//Lookup by global id
		inline auto operator[](const u32 globalId) const { return layoutsById.find(globalId); }

		//Lookup by name
		//Use HASH("YourVariable") if constructed with it; this will not expose "YourVariable" to the program though.
		inline auto operator[](const String &str) const { return layoutsByName.find(str); }

		//Get hash for local id by using register type
		inline static constexpr u64 localHash(const ResourceType type, const u32 localId){
			return (u64(type) << 32) | localId;
		}

		//Find by local id and register type
		inline auto find(const ResourceType type, const u32 localId) const {
			return layoutsByLocalId.find(localHash(type, localId));
		}

		//Check if this pipeline is compatible with the other
		//Ex:
		//Shader: uses 0, 2
		//Descriptors: 0, 1, 2, 3
		//If both r0 and r2 are compatible, this passes.
		//The syntax would be shaderLayout.supportsLayout(descriptorLayout); as it checks if all registers it requires are present
		bool supportsLayout(const PipelineLayout &other) const;
	};

}