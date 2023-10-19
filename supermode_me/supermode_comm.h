#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>
#include "json.hpp"

namespace supermode_comm
{
	typedef struct _PTE
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access the memory.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access the memory.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Dirty : 1;                // If 0, the memory backing this page has not been written to.
				ULONG64 PageAccessType : 1;       // Determines the memory type used to access the memory.
				ULONG64 Global : 1;                // If 1 and the PGE bit of CR4 is set, translations are global.
				ULONG64 Ignored2 : 3;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the backing physical page.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 7;
				ULONG64 ProtectionKey : 4;         // If the PKE bit of CR4 is set, determines the protection key.
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PTE, * PPTE;

	enum PAGING_STAGE
	{
		PML4,
		PDPT,
		PD,
		PT
	};

	static uint64_t mal_pointer_pte_ind[4];
	static uint64_t mal_pte_ind[4];

	static uint64_t system_cr3;

	static uint64_t current_pfn = 0;

	struct PTE_PFN
	{
		uint64_t pfn;
		uint64_t offset;
	};
	static PTE_PFN mal_pte_pfn;
	
	// rudamentary ahhhh data transmition
	static bool load()
	{
		std::ifstream ifs("C:\\indices.json");
		if (!ifs.is_open())
			return false;

		std::string content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));

		nlohmann::json j = nlohmann::json::parse(content);

		system_cr3 = j["cr3"].get<uint64_t>();
		std::cout << "CR3: 0x" << std::hex << system_cr3 << std::dec << std::endl;

		for (int i = 0; i <= PT; i++)
		{
			mal_pointer_pte_ind[i] = j["mal_pointer_pte_indices"][std::to_string(i)].get<uint64_t>();
			std::cout << i << ":" << mal_pointer_pte_ind[i] << std::endl;
		}
		for (int i = 0; i <= PT; i++)
		{
			if (mal_pte_ind[i] == 0)
				mal_pte_ind[i] = j["mal_pte_indices"][std::to_string(i)].get<uint64_t>();
			std::cout << i << ":" << mal_pte_ind[i] << std::endl;
		}

		return true;
	}

	static PTE_PFN calc_pfnpte_from_addr(uint64_t addr)
	{
		PTE_PFN pte_pfn;
		uint64_t pfn = addr >> 12;
		pte_pfn.pfn = pfn;
		pte_pfn.offset = addr - (pfn * 0x1000);
		return pte_pfn;
	}

	static uint64_t generate_virtual_address(uint64_t pml4, uint64_t pdpt, uint64_t pd, uint64_t pt, uint64_t offset)
	{
		uint64_t virtual_address =
			(pml4 << 39) |
			(pdpt << 30) |
			(pd << 21) |
			(pt << 12) |
			offset;

		return virtual_address;
	}

	static void change_mal_pt_pfn(uint64_t pfn)
	{
		current_pfn = pfn;

		PTE mal_pte;
		mal_pte.Present = 1;
		mal_pte.ReadWrite = 1;
		mal_pte.UserSupervisor = 1;
		mal_pte.PageFrameNumber = pfn;
		mal_pte.ExecuteDisable = 1;
		mal_pte.Dirty = 1;
		mal_pte.Accessed = 1;
		mal_pte.PageCacheDisable = 1;
		mal_pte.PageWriteThrough = 1;

		Sleep(1);

		uint64_t va = generate_virtual_address(mal_pointer_pte_ind[PML4], mal_pointer_pte_ind[PDPT], mal_pointer_pte_ind[PD], mal_pointer_pte_ind[PT], 0);
		memcpy((void*)va, &mal_pte, sizeof(PTE));
	}

	static void read_physical_memory(uint64_t addr, uint64_t size, uint64_t* buf)
	{
		PTE_PFN pfn = calc_pfnpte_from_addr(addr);

		if (current_pfn != pfn.pfn)
			change_mal_pt_pfn(pfn.pfn);

		uint64_t va = generate_virtual_address(mal_pte_ind[PML4], mal_pte_ind[PDPT], mal_pte_ind[PD], mal_pte_ind[PT], pfn.offset);
		memcpy((void*)buf, (void*)va, size);
	}

	static void write_physical_memory(uint64_t addr, uint64_t size, uint64_t* data)
	{
		PTE_PFN pfn = calc_pfnpte_from_addr(addr);

		if (current_pfn != pfn.pfn)
			change_mal_pt_pfn(pfn.pfn);

		uint64_t va = generate_virtual_address(mal_pte_ind[PML4], mal_pte_ind[PDPT], mal_pte_ind[PD], mal_pte_ind[PT], pfn.offset);
		memcpy((void*)va, (void*)data, size);
	}
}