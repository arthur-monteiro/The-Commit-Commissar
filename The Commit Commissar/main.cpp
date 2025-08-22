#include <ResourceUniqueOwner.h>

#include "Commissar.h"

int main()
{
	Wolf::ResourceUniqueOwner<Commissar> commissar(new Commissar("config/config.json"));
	commissar->run();

	return EXIT_SUCCESS;
}