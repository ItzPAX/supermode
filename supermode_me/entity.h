#pragma once

#include <Windows.h>
#include <iostream>
#include "vec3.h"

class entity
{
public:
	uintptr_t address;
	int health;
	vec3 position;
};