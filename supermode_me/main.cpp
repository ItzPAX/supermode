#include "supermode_ctrl.h"

int main()
{
	std::cout << "START ME SECOND!!!\n";

	while (!supermode::load())
	{
		std::cout << "retrying to load...\n";
		Sleep(1000);
	}

	Sleep(5000);
	system("pause");

	byte buf[16] = { 0 };
	
	std::cout << "Reading from phys 0x1ad000 from now on...\n";
	
	while (true)
	{
		supermode::read_physical_memory(0x1ad000, 15, (uint64_t*)buf);
		for (int i = 0; i < 16; i++)
			std::cout << std::hex << (int)buf[i] << std::endl;
		Sleep(10000000000);
	}

	return 1;
}