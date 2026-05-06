#include "ramen.h"
#include <cstdlib>

ramen::ramen() {
	clear();
}

void ramen::clear() {
	noodle = false;
	soup = false;
	egg = false;
	cheese = false;
	dumpling = false;
	water = false;
}

void ramen::makebasicramen() {
	clear();
	noodle = true;
	water = true;
	egg = true;
}

void ramen::makerandramen() {
	makebasicramen();

	if (rand() % 2 == 0)
	{
		egg = true;
	}

	if (rand() % 2 == 0)
	{
		cheese = true;
	}

	if (rand() % 2 == 0)
	{
		dumpling = true;
	}
}

bool ramen::ramenSame(const ramen& other) {
	if (noodle == other.noodle &&
		soup == other.soup &&
		water == other.water &&
		egg == other.egg &&
		cheese == other.cheese &&
		dumpling == other.dumpling)
	{
		return true;
	}

	return false;
}