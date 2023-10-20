#include "supermode.h"

int main()
{
	remove("C:\\indices.json");
	system("supermode_me.exe SUPERMODE");

	while (!supermode::wnbios.get_process_base("supermode_me.exe"))
	{
		std::cout << "Waiting for target application...\n";
		Sleep(1000);
	}

	Sleep(5000);

	supermode::insert_first_malicious_pte();
	supermode::insert_second_malicious_pte();

	std::cout << "saving indices for target application...\n";
	supermode::save_indices_for_target();

	std::cout << "unloading wnbios...\n";
	supermode::wnbios.unload_driver();

	std::cout << "target is now supermoded lets pray the system doesnt BSOD :(\n";

	Sleep(1000);

	return 1;
}