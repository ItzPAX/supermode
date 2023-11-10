#include "rwptm.h"
#include "entity.h"
#include "offsets.h"

int start_sm(int argc, char* argv[])
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

	rwptm::init(TARGET_NAME.c_str(), LOCAL_NAME.c_str());
	system("pause");

	return 1;
}

void update_entity(entity* pent)
{
	pent->position = rwptm::read_virtual_memory<vec3>(pent->address + player::vOrigin);
	pent->health = rwptm::read_virtual_memory<int>(pent->address + player::iHealth);
}

void cls()
{
	static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD topLeft = { 0, 0 };
	std::cout.flush();

	if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
		abort();
	}
	DWORD length = csbi.dwSize.X * csbi.dwSize.Y;

	DWORD written;

	FillConsoleOutputCharacter(hOut, TEXT(' '), length, topLeft, &written);

	FillConsoleOutputAttribute(hOut, csbi.wAttributes, length, topLeft, &written);

	SetConsoleCursorPosition(hOut, topLeft);
}

void cheat_thread()
{
	DWORD csid = supermode::get_process_id("cs2.exe");
	std::cout << "csid: " << csid << std::endl;

	entity local_player{};
	std::vector<entity*> players;

	uintptr_t client, engine;

	client = supermode::get_module_base(L"client.dll", rwptm::target_eproc, rwptm::target_cr3);
	std::cout << "clientdll: " << client << std::endl;

	engine = supermode::get_module_base(L"engine2.dll", rwptm::target_eproc, rwptm::target_cr3);
	std::cout << "clientdll: " << client << std::endl;

	while (true)
	{
		uintptr_t gameclient = rwptm::read_virtual_memory<uintptr_t>(engine + engine2_dll::dwNetworkGameClient);
		int max_clients = rwptm::read_virtual_memory<int>(gameclient + engine2_dll::dwNetworkGameClient_maxClients);

		if (max_clients <= 1)
		{
			Sleep(500);
			continue;
		}

		local_player.address = rwptm::read_virtual_memory<uintptr_t>(client + client_dll::dwLocalPlayerPawn);
		if (!local_player.address)
		{
			continue;
		}
		
		cls();
		const auto entity_list = rwptm::read_virtual_memory<uintptr_t>(client + client_dll::dwEntityList);
		for (auto i = 1; i < 64; i++) {
			uintptr_t list_entry = rwptm::read_virtual_memory<uintptr_t>(entity_list + (8 * (i & 0x7FFF) >> 9) + 16);
			uintptr_t cplayer = rwptm::read_virtual_memory<uintptr_t>(list_entry + 120 * (i & 0x1FF));

			if (cplayer == 0)
				continue;

			const std::uint32_t player_pawn = rwptm::read_virtual_memory<std::uint32_t>(cplayer + player::m_hPlayerPawn);
			const uintptr_t list_entry2 = rwptm::read_virtual_memory<uintptr_t>(entity_list + 0x8 * ((player_pawn & 0x7FFF) >> 9) + 16);
			if (!list_entry2) 
				continue;

			entity ent;
			ent.address = rwptm::read_virtual_memory<uintptr_t>(list_entry2 + 120 * (player_pawn & 0x1FF));

			if (ent.address == local_player.address)
				continue;

			std::cout << "p: " << ent.address << std::endl;

			update_entity(&ent);
			
			std::cout << "health: " << std::dec << ent.health << std::endl;
			std::cout << "pos x: " << std::dec << ent.position.x << " pos y: " << ent.position.y << " pos z: " << ent.position.z << std::endl;
		}

		Sleep(1);
	}
}

int main(int argc, char* argv[])
{
	if (start_sm(argc, argv) != 1)
		return -1;

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)cheat_thread, NULL, NULL, NULL);
	_fgetchar();

	return 1;
}