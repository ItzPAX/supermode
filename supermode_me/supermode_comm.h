#pragma once
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <thread>
#include "json.hpp"

namespace supermode_comm
{
	typedef struct _PML4E
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // Must be 0 for PML4E.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PML4E, * PPML4E;

	typedef struct _PDPTE
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PD.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PD.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // If 1, this entry maps a 1GB page.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PD of this PDPTE.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PDPTE, * PPDPTE;

	typedef struct _PDE
	{
		union
		{
			struct
			{
				ULONG64 Present : 1;              // Must be 1, region invalid if 0.
				ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
				ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
				ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PT.
				ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PT.
				ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
				ULONG64 Ignored1 : 1;
				ULONG64 PageSize : 1;             // If 1, this entry maps a 2MB page.
				ULONG64 Ignored2 : 4;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the PT of this PDE.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 11;
				ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
			};
			ULONG64 Value;
		};
	} PDE, * PPDE;

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
				ULONG64 Global : 1;               // If 1 and the PGE bit of CR4 is set, translations are global.
				ULONG64 Ignored2 : 3;
				ULONG64 PageFrameNumber : 36;     // The page frame number of the backing physical page.
				ULONG64 Reserved : 4;
				ULONG64 Ignored3 : 7;
				ULONG64 ProtectionKey : 4;         // If the PKE bit of CR4 is set, determines the protection key.
				ULONG64 ExecuteDisable : 1;        // If 1, instruction fetches not allowed.
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
	static uint64_t mal_pml4_pte_ind[4];

	static uint64_t system_cr3;
	static uint64_t target_cr3;

	static uint64_t mal_pte_offset = 0;

	static uint64_t current_pfn = 0;

	static std::vector<uint64_t> free_pml4s;

	struct PTE_PFN
	{
		uint64_t pfn;
		uint64_t offset;
	};
	
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
		std::cout << "system_cr3: 0x" << std::hex << system_cr3 << std::dec << std::endl;

		target_cr3 = j["target_cr3"].get<uint64_t>();
		std::cout << "target_cr3: 0x" << std::hex << target_cr3 << std::dec << std::endl;

		mal_pte_offset = j["mal_pte_offset"].get<uint64_t>();
		std::cout << "mal_pte_offset: 0x" << std::hex << mal_pte_offset << std::dec << std::endl;

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
		for (int i = 0; i <= PT; i++)
		{
			if (mal_pml4_pte_ind[i] == 0)
				mal_pml4_pte_ind[i] = j["mal_pml4_pte_indices"][std::to_string(i)].get<uint64_t>();
			std::cout << i << ":" << mal_pml4_pte_ind[i] << std::endl;
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

	static void invalidate_pte_tlb()
	{
		uint64_t mal_pte_va = generate_virtual_address(mal_pte_ind[PML4], mal_pte_ind[PDPT], mal_pte_ind[PD], mal_pte_ind[PT], 0);

		while (true)
		{
			__try
			{
				if (*(PCHAR)mal_pte_va)
					continue;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return;
			}

			YieldProcessor();
		}
	}

	static uintptr_t change_mal_pt_pfn(uint64_t pfn)
	{
		current_pfn = pfn;

		uint64_t va = generate_virtual_address(mal_pointer_pte_ind[PML4], mal_pointer_pte_ind[PDPT], mal_pointer_pte_ind[PD], mal_pointer_pte_ind[PT], mal_pte_offset);

		PTE mal_pte;
		mal_pte.Value = 0;
		memcpy((void*)va, &mal_pte, sizeof(PTE));

		invalidate_pte_tlb();
		 
		mal_pte.PageFrameNumber = pfn;
		mal_pte.Present = 1;
		mal_pte.UserSupervisor = 1;
		mal_pte.PageCacheDisable = 1;
		mal_pte.ReadWrite = 1;
		mal_pte.ExecuteDisable = 1;
		mal_pte.Accessed = 1;
		mal_pte.Dirty = 1;

		memcpy((void*)va, &mal_pte, sizeof(PTE));
		return va;
	}

	static int cycle = 0;
	static uint64_t create_pml4(ULONG64 pml4e)
	{
		// only use upper half of free entries for whatever reason
		if (cycle >= (int)(free_pml4s.size() / 2))
			cycle = 0;

		uint64_t pml4e_ind = free_pml4s.at(cycle);
		cycle++;

		uint64_t va = generate_virtual_address(mal_pml4_pte_ind[PML4], mal_pml4_pte_ind[PDPT], mal_pml4_pte_ind[PD], mal_pml4_pte_ind[PT], pml4e_ind * sizeof(PPML4E));

	    memcpy((void*)va, &pml4e, sizeof(ULONG64));

		return pml4e_ind;
	}

	static void destroy_pml4e(uint64_t pml4e_ind)
	{
		uint64_t va = generate_virtual_address(mal_pml4_pte_ind[PML4], mal_pml4_pte_ind[PDPT], mal_pml4_pte_ind[PD], mal_pml4_pte_ind[PT], pml4e_ind * sizeof(PPML4E));

		PML4E pml4;
		pml4.Value = 0;

		memcpy((void*)va, &pml4, sizeof(PML4E));
	}

	static bool read_physical_memory(uint64_t addr, uint64_t size, uint64_t* buf)
	{
		PTE_PFN pfn = calc_pfnpte_from_addr(addr);

		if (pfn.pfn != current_pfn)
			change_mal_pt_pfn(pfn.pfn);

		MemoryFence();

		uint64_t va = generate_virtual_address(mal_pte_ind[PML4], mal_pte_ind[PDPT], mal_pte_ind[PD], mal_pte_ind[PT], pfn.offset);

		memcpy((void*)buf, (void*)va, size);
		return true;
	}

	static void write_physical_memory(uint64_t addr, uint64_t size, uint64_t* data)
	{
		PTE_PFN pfn = calc_pfnpte_from_addr(addr);

		if (pfn.pfn != current_pfn)
		{
			change_mal_pt_pfn(pfn.pfn);
			MemoryFence();
		}

		uint64_t va = generate_virtual_address(mal_pte_ind[PML4], mal_pte_ind[PDPT], mal_pte_ind[PD], mal_pte_ind[PT], pfn.offset);
		memcpy((void*)va, (void*)data, size);
	}
}