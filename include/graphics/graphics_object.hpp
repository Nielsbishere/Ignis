#pragma once
#include "types/types.hpp"

namespace ignis {

	class Graphics;

	class GraphicsObject {

	public:

		GraphicsObject(Graphics &g, const String &name);
		~GraphicsObject();

		GraphicsObject(const GraphicsObject&) = delete;
		GraphicsObject(GraphicsObject&&) = delete;
		GraphicsObject &operator=(const GraphicsObject&) = delete;
		GraphicsObject &operator=(GraphicsObject&&) = delete;

	protected:

		inline Graphics &getGraphics() { return g; }
		inline const String &getName() { return name; }

	private:

		Graphics &g;
		String name;

	};

}