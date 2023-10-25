#include "rwptm.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cout << "Please start supermode.exe instead of this application\n";
		system("pause");
		return -1;
	}
	else if (strcmp(argv[1], "SUPERMODE") != 0)
	{
		std::cout << "Please start supermode.exe instead of this application\n";
		system("pause");
		return -1;
	}

	std::cout << "Starting...\n";

	while (!supermode_comm::load())
	{
		std::cout << "retrying to load...\n";
		Sleep(1000);
	}
	
	std::cout << "Open the game...\n";
	system("pause");

	uintptr_t supermode_dtb, supermode_kproc;
	supermode::attach("explorer.exe", &supermode_dtb, &supermode_kproc);

	supermode::attach("supermode_me.exe", &supermode_dtb, &supermode_kproc);

	uint64_t explorer_pid = supermode::get_process_id("explorer.exe");
	uint64_t explorer_base = supermode::get_process_base_um(explorer_pid, "explorer.exe");

	std::cout << "explorer base " << std::hex << explorer_base << std::dec << std::endl;

	system("pause");

	supermode::get_dtb_from_process_base(explorer_base, supermode_dtb);

	system("pause");


	return 1;
}