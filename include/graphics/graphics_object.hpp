#pragma once

namespace ignis {

	class Graphics;

	class GraphicsObject {

	public:

		GraphicsObject(Graphics &g);
		~GraphicsObject();

		GraphicsObject(const GraphicsObject&) = delete;
		GraphicsObject(GraphicsObject&&) = delete;
		GraphicsObject &operator=(const GraphicsObject&) = delete;
		GraphicsObject &operator=(GraphicsObject&&) = delete;

	protected:

		inline Graphics &getGraphics() { return g; }

	private:

		Graphics &g;

	};

}