#include "Core/Application.h"
#include "Core/Entrypoint.h"

class PEditorApplication : public PApplication
{
};

PApplication* CreateApplication()
{
	return new PEditorApplication();
}