#include "yaDestroy.h"
#include "yaGameObject.h"

namespace ya::object
{
	void Destroy(GameObject* obj)
	{
		obj->Death();
	}
}