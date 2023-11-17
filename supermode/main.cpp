#include "supermode.h"
#include "../settings.h"

void start_thread()
{
	std::string command = LOCAL_NAME + " SUPERMODE";
	system(command.c_str());
}

int main()
{
	remove("C:\\indices.json");
	HANDLE h = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)start_thread, NULL, NULL, NULL);
	if (h)
		CloseHandle(h);

	uintptr_t target_cr3 = 0;

	if (EACCR3)
	{
		uintptr_t target_base = supermode::wnbios.get_process_base(TARGET_NAME.c_str());
		std::cout << "bruteforcing process dtb, this might take a while...\n";
		target_cr3 = supermode::wnbios.find_dtb_from_base(target_base);
		std::cout << std::hex << target_cr3 << std::endl;
	}

	uintptr_t base = 0;
	while (!base)
	{
		base = supermode::wnbios.get_process_base(LOCAL_NAME.c_str());
		std::cout << "Waiting for target application...\n";
		Sleep(1000);
	}

	Sleep(1000);
	std::cout << "populating forbidden zones...\n";
	supermode::fill_forbidden_zones(supermode::wnbios.cr3, supermode::wnbios.attached_eproc);
	
	supermode::insert_first_malicious_pte();
	supermode::insert_second_malicious_pte();
	supermode::insert_third_malicious_pte(supermode::wnbios.cr3);
	
	std::cout << "saving indices for target application...\n";
	supermode::save_indices_for_target(target_cr3);
	
	std::cout << "unloading wnbios...\n";
	supermode::wnbios.unload_driver();
	
	std::cout << "target is now supermoded lets pray the system doesnt BSOD :(\n";

	return 1;
}