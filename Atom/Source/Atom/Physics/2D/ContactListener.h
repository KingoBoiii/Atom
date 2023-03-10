#pragma once

#include <box2d/b2_world_callbacks.h>

namespace Atom
{

	class ContactListener : public b2ContactListener
	{
	public:
		/// Called when two fixtures begin to touch.
		virtual void BeginContact(b2Contact* contact) override;

		/// Called when two fixtures cease to touch.
		virtual void EndContact(b2Contact* contact) override;
	};

}