#include "rwptm.h"
#include "entity.h"
#include "offsets.h"
#include "communication.h"
#include "utils.h"

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
	pent->origin = rwptm::read_virtual_memory<vec3>(pent->address + player::vOrigin);

	uintptr_t gamescene = rwptm::read_virtual_memory<uintptr_t>(pent->address + player::dwGameScene);
	uintptr_t bonearray = rwptm::read_virtual_memory<uintptr_t>(gamescene + player::dwBoneArray1 + player::dwBoneArray2);
	pent->head = rwptm::read_virtual_memory<vec3>(bonearray + 6 * 32);

	pent->health = rwptm::read_virtual_memory<int>(pent->address + player::iHealth);
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
	std::cout << "engine2dll: " << engine << std::endl;

	view_matrix_t vm;
	vec3 screensize = { 1920, 1080, 0 };

	while (true)
	{
		comm::boxes.clear();
		comm::box b{};
		comm::boxes.push_back(b);

		uintptr_t gameclient = rwptm::read_virtual_memory<uintptr_t>(engine + engine2_dll::dwNetworkGameClient);
		int max_clients = rwptm::read_virtual_memory<int>(gameclient + engine2_dll::dwNetworkGameClient_maxClients);
		
		if (max_clients <= 1)
		{
			Sleep(500);
			continue;
		}
		
		vm = rwptm::read_virtual_memory<view_matrix_t>(client + client_dll::dwViewMatrix);
		
		local_player.address = rwptm::read_virtual_memory<uintptr_t>(client + client_dll::dwLocalPlayerPawn);
		if (!local_player.address)
		{
			continue;
		}
		
		const auto entity_list = rwptm::read_virtual_memory<uintptr_t>(client + client_dll::dwEntityList);

		for (auto i = 1; i < 64; i++) 
		{
			uintptr_t list_entry = rwptm::read_virtual_memory<uintptr_t>(entity_list + (8 * (i & 0x7FFF) >> 9) + 16);
			uintptr_t cplayer = rwptm::read_virtual_memory<uintptr_t>(list_entry + 120 * (i & 0x1FF));
		
			if (cplayer == 0)
				continue;
		
			const std::uint32_t player_pawn = rwptm::read_virtual_memory<std::uint32_t>(cplayer + player::hPlayerPawn);
			const uintptr_t list_entry2 = rwptm::read_virtual_memory<uintptr_t>(entity_list + 0x8 * ((player_pawn & 0x7FFF) >> 9) + 16);
			if (!list_entry2) 
				continue;
		
			entity ent;
			ent.address = rwptm::read_virtual_memory<uintptr_t>(list_entry2 + 120 * (player_pawn & 0x1FF));
		
			if (ent.address == local_player.address)
				continue;
		
			update_entity(&ent);
		
			if (ent.health > 100 || ent.health < 1)
				continue;
		
			vec3 w2s_origin{};
			if (!world_to_screen(screensize, ent.origin, w2s_origin, vm))
				continue;
		
			vec3 w2s_head{};
			if (!world_to_screen(screensize, ent.head, w2s_head, vm))
				continue;
		
			// make some shitty box
			float height = w2s_origin.y - w2s_head.y;
			float width = height / 2;
			
			int health_to_255 = ent.health * 2.55;
			DWORD col = RGB(255 - health_to_255, health_to_255, 0);

			comm::box b;
			b.xy = vec2{ w2s_origin.x - width / 2, w2s_head.y };
			b.size = vec2{ width, height };
			b.col = col;
			comm::boxes.push_back(b);
		}
		
		comm::write_draw_data();
		
		Sleep(1);
	}
}

int main(int argc, char* argv[])
{
	if (start_sm(argc, argv) != 1)
		return -1;

	comm::init();
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)cheat_thread, NULL, NULL, NULL);
	
	_fgetchar();

	return 1;
}