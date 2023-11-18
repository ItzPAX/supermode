#pragma once

#include <Windows.h>
#include <iostream>
#include "vec.h"

class entity
{
public:
	uintptr_t address;
	int health;
	int team;
	vec3 origin;
	vec3 head;
};