#pragma once
#include "types/types.hpp"
#define apimpl
#define plimpl

namespace ignis {

	class Graphics;

	class GraphicsObject {

	public:

		GraphicsObject(Graphics &g, const String &name);
		virtual ~GraphicsObject();

		GraphicsObject(const GraphicsObject&) = delete;
		GraphicsObject(GraphicsObject&&) = delete;
		GraphicsObject &operator=(const GraphicsObject&) = delete;
		GraphicsObject &operator=(GraphicsObject&&) = delete;

		template<typename T>
		bool canCast() { return dynamic_cast<T*>(this); }

		template<typename T>
		bool cast() { return dynamic_cast<T*>(this); }

	protected:

		inline Graphics &getGraphics() { return g; }
		inline const String &getName() { return name; }

	private:

		Graphics &g;
		String name;

	};

}