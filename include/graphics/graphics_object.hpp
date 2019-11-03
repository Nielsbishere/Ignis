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
		T *cast() { return dynamic_cast<T*>(this); }

		inline const String &getName() { return name; }

	protected:

		inline Graphics &getGraphics() { return g; }

	private:

		Graphics &g;
		String name;

	};

}