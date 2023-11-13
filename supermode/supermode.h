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

	struct FORBIDDEN_ZONE
	{
		uintptr_t begin;
		uintptr_t end;
	};

	wnbios_lib wnbios;

	uint64_t mal_pointer_pte_ind[4];
	uint64_t mal_pte_ind[4];
	uint64_t mal_pml4_pte_ind[4];

	uint64_t mal_pointer_pte_struct[4];
	uint64_t mal_pte_struct[4];
	uint64_t mal_pml4_pte_struct[4];

	std::vector<FORBIDDEN_ZONE> forbidden_zones;
	std::vector<int> banned_pml4_indices;
	std::vector<int> banned_pdpt_indices;
	std::vector<int> banned_pd_indices;
	std::vector<int> banned_pt_indices;

	struct PTE_PFN
	{
		uint64_t pfn;
		uint64_t offset;
	};
	PTE_PFN mal_pte_pfn;

	typedef struct VAD_NODE {
		VAD_NODE* Left;
		VAD_NODE* Right;
		VAD_NODE* Parent;
		ULONG StartingVpn;
		ULONG EndingVpn;
		ULONG ulVpnInfo;
		ULONG ReferenceCount;
		PVOID PushLock;
		ULONG u;
		ULONG u1;
		PVOID u5;
		PVOID u2;
		void* Subsection; // 0x48 - 0x50
		PVOID FirstProtoPte; // 0x50 - 0x58
		PVOID LastPte; // 0x58 - 0x60
		_LIST_ENTRY ViewLinks;
		void* VadsProcess; // 0x60 - 0x68
		PVOID u4;
		PVOID FileObject;
	}VAD_NODE, * PVAD_NODE;

	uintptr_t get_adjusted_va(BOOLEAN start, VAD_NODE vad)
	{
		UCHAR byte_offset = (start ? ((UCHAR*)&vad.ulVpnInfo)[0] : ((UCHAR*)&vad.ulVpnInfo)[1]);
		DWORD64 hi_va_start = 0x100000000 * byte_offset;
		hi_va_start += start ? vad.StartingVpn : vad.EndingVpn;

		return (uintptr_t)hi_va_start;
	}

	void avl_iterate_over(VAD_NODE node, uintptr_t dtb)
	{
		DWORD64 start_va_adjusted = get_adjusted_va(TRUE, node);
		DWORD64 end_va_adjusted = get_adjusted_va(FALSE, node);

		std::cout << "VAD Entry at: 0x" << std::hex << &node << " start: " << std::hex << start_va_adjusted << " end : " << end_va_adjusted << std::endl;
		forbidden_zones.push_back({ start_va_adjusted, end_va_adjusted });

		if (node.Left != nullptr) {
			VAD_NODE leftNode{};
			if (wnbios.read_virtual_memory((uintptr_t)node.Left, (uint64_t*)&leftNode, sizeof(VAD_NODE), dtb)) {
				avl_iterate_over(leftNode, dtb);
			}
		}

		if (node.Right != nullptr) {
			VAD_NODE rightNode{};
			if (wnbios.read_virtual_memory((uintptr_t)node.Right, (uint64_t*)&rightNode, sizeof(VAD_NODE), dtb)) {
				avl_iterate_over(rightNode, dtb);
			}
		}
	}

	void fill_forbidden_zones(uintptr_t dtb, uintptr_t eproc)
	{
		PVAD_NODE lpVadRoot;
		wnbios.get_eprocess_offsets();
		uintptr_t system_cr3 = wnbios.get_system_dirbase();
		if (!wnbios.read_virtual_memory((eproc + wnbios.EP_VADROOT), (uint64_t*)&lpVadRoot, sizeof(PVAD_NODE), system_cr3))
			return;

		VAD_NODE vad;
		if (!wnbios.read_virtual_memory((uintptr_t)lpVadRoot, (uint64_t*)&vad, sizeof(VAD_NODE), dtb))
			return;

		avl_iterate_over(vad, dtb);
		std::cout << "added " << forbidden_zones.size() << " forbidden zones\n";
	}

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
				bool banned = false;
				for (auto& banned_ind : banned_pml4_indices)
				{
					if (banned_ind == i)
						banned = true;
				}

				if (!banned)
				{
					std::cout << "Valid pml4 found at index " << i << std::endl;
					*pml4ind = i;
					*pdptstruct = pml4e.PageFrameNumber * 0x1000;
					return;
				}
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
				bool banned = false;
				for (auto& banned_ind : banned_pdpt_indices)
				{
					if (banned_ind == i)
						banned = true;
				}

				if (!banned)
				{
					std::cout << "Valid pdpte found at index " << i << std::endl;
					*pdpteind = i;
					*pdstruct = pdpte.PageFrameNumber * 0x1000;
					return;
				}
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
				bool banned = false;
				for (auto& banned_ind : banned_pd_indices)
				{
					if (banned_ind == i)
						banned = true;
				}

				if (!banned)
				{

					std::cout << "Valid pde found at index " << i << std::endl;
					*pdind = i;
					*ptstruct = pde.PageFrameNumber * 0x1000;
					return;
				}
			}
		}
	}

	void free_pte(uint64_t ptstruct, uint64_t* ptind, bool must_be_first = false)
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
				bool banned = false;
				for (auto& banned_ind : banned_pt_indices)
				{
					if (banned_ind == i)
						banned = true;
				}

				if (!banned)
				{
					std::cout << "Free pte found at index " << i << std::endl;
					*ptind = i;
					return;
				}
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
		find_indices:
		// find a free pte and populate other indices while at it
		valid_pml4e(&mal_pte_ind[PML4], &mal_pte_struct[PDPT]);
		valid_pdpte(mal_pte_struct[PDPT], &mal_pte_ind[PDPT], &mal_pte_struct[PD]);
		valid_pde(mal_pte_struct[PD], &mal_pte_ind[PD], &mal_pte_struct[PT]);
		free_pte(mal_pte_struct[PT], &mal_pte_ind[PT]);

		uintptr_t va = generate_virtual_address(mal_pte_ind[PML4], mal_pte_ind[PDPT], mal_pte_ind[PD], mal_pte_ind[PT], 0);
		uintptr_t vad_vpn = (va & 0xFFFFFFFFFFFFF000) / 0x1000;
		for (auto& zone : forbidden_zones)
		{
			if (zone.begin <= vad_vpn && vad_vpn <= zone.end)
			{
				banned_pml4_indices.push_back(mal_pte_ind[PML4]);
				banned_pdpt_indices.push_back(mal_pte_ind[PDPT]);
				banned_pd_indices.push_back(mal_pte_ind[PD]);
				banned_pt_indices.push_back(mal_pte_ind[PT]);

				goto find_indices;
			}
		}

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

		std::cout << "Offset: " << mal_pte_pfn.offset << std::endl;

		wnbios.write_physical_memory(mal_pte_phys, &mal_pte, sizeof(PTE));
		std::cout << "inserted first malicious pte at index " << mal_pte_ind[PT] << " [" << std::hex << mal_pte_phys << "] " << std::dec << std::endl;
	}

	// this pte will point to our other malicious pte we can modify from our UM program then
	void insert_second_malicious_pte()
	{
		// find a free pte and populate other indices while at it
		find_indices:
		valid_pml4e(&mal_pointer_pte_ind[PML4], &mal_pointer_pte_struct[PDPT]);
		valid_pdpte(mal_pointer_pte_struct[PDPT], &mal_pointer_pte_ind[PDPT], &mal_pointer_pte_struct[PD]);
		valid_pde(mal_pointer_pte_struct[PD], &mal_pointer_pte_ind[PD], &mal_pointer_pte_struct[PT]);
		free_pte(mal_pointer_pte_struct[PT], &mal_pointer_pte_ind[PT]);

		uintptr_t va = generate_virtual_address(mal_pointer_pte_ind[PML4], mal_pointer_pte_ind[PDPT], mal_pointer_pte_ind[PD], mal_pointer_pte_ind[PT], 0);
		uintptr_t vad_vpn = (va & 0xFFFFFFFFFFFFF000) / 0x1000;
		for (auto& zone : forbidden_zones)
		{
			if (zone.begin <= vad_vpn && vad_vpn <= zone.end)
			{
				banned_pml4_indices.push_back(mal_pointer_pte_ind[PML4]);
				banned_pdpt_indices.push_back(mal_pointer_pte_ind[PDPT]);
				banned_pd_indices.push_back(mal_pointer_pte_ind[PD]);
				banned_pt_indices.push_back(mal_pointer_pte_ind[PT]);

				goto find_indices;
			}
		}

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
		std::cout << "inserted second malicious pte at index " << mal_pointer_pte_ind[PT] << " [" << std::hex << mal_pte_phys << "] " << " pointing to pfn " << mal_pte_pfn.pfn << std::endl;
	}

	void insert_third_malicious_pte(uintptr_t target_pml4)
	{
		std::cout << "inserting third pte for dbt: " << std::hex << target_pml4 << std::dec << std::endl;

		// find a free pte and populate other indices while at it
		find_indices:
		valid_pml4e(&mal_pml4_pte_ind[PML4], &mal_pml4_pte_struct[PDPT]);
		valid_pdpte(mal_pml4_pte_struct[PDPT], &mal_pml4_pte_ind[PDPT], &mal_pml4_pte_struct[PD]);
		valid_pde(mal_pml4_pte_struct[PD], &mal_pml4_pte_ind[PD], &mal_pml4_pte_struct[PT]);
		free_pte(mal_pml4_pte_struct[PT], &mal_pml4_pte_ind[PT]);

		uintptr_t va = generate_virtual_address(mal_pml4_pte_ind[PML4], mal_pml4_pte_ind[PDPT], mal_pml4_pte_ind[PD], mal_pml4_pte_ind[PT], 0);
		uintptr_t vad_vpn = (va & 0xFFFFFFFFFFFFF000) / 0x1000;
		for (auto& zone : forbidden_zones)
		{
			if (zone.begin <= vad_vpn && vad_vpn <= zone.end)
			{
				banned_pml4_indices.push_back(mal_pml4_pte_ind[PML4]);
				banned_pdpt_indices.push_back(mal_pml4_pte_ind[PDPT]);
				banned_pd_indices.push_back(mal_pml4_pte_ind[PD]);
				banned_pt_indices.push_back(mal_pml4_pte_ind[PT]);

				goto find_indices;
			}
		}

		std::cout << "PML4: " << mal_pml4_pte_ind[PML4] << std::endl
			<< "PDPT: " << mal_pml4_pte_ind[PDPT] << std::endl
			<< "PD: " << mal_pml4_pte_ind[PD] << std::endl
			<< "PT: " << mal_pml4_pte_ind[PT] << std::endl;

		auto pml4_pte_pfn = calc_pfnpte_from_addr(target_pml4);

		PTE mal_pte;
		mal_pte.Present = 1;
		mal_pte.ReadWrite = 1;
		mal_pte.UserSupervisor = 1;
		mal_pte.PageFrameNumber = pml4_pte_pfn.pfn;
		mal_pte.ExecuteDisable = 1;
		mal_pte.Dirty = 1;
		mal_pte.Accessed = 1;

		uintptr_t mal_pte_phys = mal_pml4_pte_struct[PT] + mal_pml4_pte_ind[PT] * sizeof(uintptr_t);

		wnbios.write_physical_memory(mal_pte_phys, &mal_pte, sizeof(PTE));
		std::cout << "inserted third malicious pte at index " << mal_pml4_pte_ind[PT] << " [" << std::hex << mal_pte_phys << "] " << " pointing to pfn " << pml4_pte_pfn.pfn << " offset: " << pml4_pte_pfn.offset << std::dec << std::endl;
	}

	// goofy ahh
	void save_indices_for_target(uintptr_t target_cr3)
	{
		nlohmann::json j;
		j["mal_pte_indices"] = {};
		j["mal_pointer_pte_indices"] = {};
		j["mal_pml4_pte_indices"] = {};

		for (int i = 0; i <= PT; i++)
		{
			j["mal_pte_indices"][std::to_string(i)] = mal_pte_ind[i];
			j["mal_pointer_pte_indices"][std::to_string(i)] = mal_pointer_pte_ind[i];
			j["mal_pml4_pte_indices"][std::to_string(i)] = mal_pml4_pte_ind[i];
		}

		j["cr3"] = wnbios.get_system_dirbase();
		j["target_cr3"] = target_cr3;
		j["mal_pte_offset"] = mal_pte_pfn.offset;

		std::ofstream out_file("C:\\indices.json");
		out_file << j.dump();
	}
}