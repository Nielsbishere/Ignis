#pragma once
#include "types/types.hpp"
#define apimpl
#define plimpl

namespace ignis {

	class Graphics;

	class GraphicsObject {

	public:

		//Creates a GraphicsObject with one reference
		GraphicsObject(Graphics &g, const String &name);

		GraphicsObject(const GraphicsObject&) = delete;
		GraphicsObject(GraphicsObject&&) = delete;
		GraphicsObject &operator=(const GraphicsObject&) = delete;
		GraphicsObject &operator=(GraphicsObject&&) = delete;

		template<typename T>
		bool canCast() { return dynamic_cast<T*>(this); }

		template<typename T>
		T *cast() { return dynamic_cast<T*>(this); }

		inline const String &getName() { return name; }

		//When a ref is added; it will have to be removed or the resource will be left over
		inline void addRef() { refCount++; }

		//Lose a reference; only way to destruct the object
		inline void loseRef() {
			if (!(--refCount)) {
				erase();
				delete this;
			}
		}

		inline Graphics &getGraphics() const { return g; }

	protected:

		virtual ~GraphicsObject() {}

	private:

		void erase();

		Graphics &g;
		String name;

		usz refCount = 1;

	};

}