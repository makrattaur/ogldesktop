#include "resload.h"
#include <Windows.h>
#include "resource.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;


bool LoadImageFromResources(sf::Image &image)
{
	HINSTANCE thisModule = (HINSTANCE)&__ImageBase;
	HRSRC resourceInfo = FindResource(thisModule, MAKEINTRESOURCE(IDR_LOCK_WP), RT_RCDATA);
	if(resourceInfo == NULL)
	{
		return false;
	}

	HGLOBAL resourceHandle = LoadResource(thisModule, resourceInfo);
	if(resourceHandle == 0)
	{
		return false;
	}

	void *resourceBytes = LockResource(resourceHandle);
	size_t resourceSize = SizeofResource(thisModule, resourceInfo);

	return image.loadFromMemory(resourceBytes, resourceSize);
}
