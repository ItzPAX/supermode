#include "supermode.h"

int main()
{
	std::cout << "START ME SECOND!!!\n";

	while (!supermode_comm::load())
	{
		std::cout << "retrying to load...\n";
		Sleep(1000);
	}

	Sleep(5000);
	system("pause");

	uint64_t base = supermode::attach("explorer.exe");

	std::cout << base << std::endl;
	system("pause");

	byte buf[3] = { 0 };

	supermode_comm::read_physical_memory(0x1ad000, 2, (uint64_t*)buf);

	//sm.read_physical_memory(0x1ad000, (uintptr_t)buf, 2);
	std::cout << buf[0] << std::endl;

	system("pause");

	return 1;
}