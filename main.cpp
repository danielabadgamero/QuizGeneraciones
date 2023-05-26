#include "Core.h"

int main(int, char**)
{
	Core::init();

	while (true)
		Core::loop();

	Core::quit();

	return 0;
}