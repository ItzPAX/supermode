#pragma once
#include "inc.h"

namespace menu
{
	enum mf_flag : DWORD
	{
		flag_none = 0x0,
		flag_warning = 0x1,
		flag_red = 0x10,
	};

	enum mf_type : int
	{
		type_bool = 1,
		type_int
	};

	struct mf
	{
		void* data;
		int size, idx, step;
		std::string label;
		mf_type type;
		unsigned int flags;
	};

	std::vector<mf> m_menufeatures;

	int m_currentidx = 0;
	int m_idx = 0;


	void handle_input()
	{
		if (GetAsyncKeyState(VK_DOWN) & 0x01)
		{
			m_currentidx++;
			if (m_currentidx >= m_idx)
				m_currentidx = 0;
		}
		if (GetAsyncKeyState(VK_UP) & 0x01)
		{
			m_currentidx--;
			if (m_currentidx < 0)
				m_currentidx = m_idx - 1;
		}
	}

	void render_info(HDC dc)
	{
		render::cstm_string(render::font_title, 17, 10, "[raybot] cs2", false, false, RGB(255, 0, 0));
		render::cstm_string(render::font_normy, 20, 35, "by Deniz", false, false, RGB(255, 0, 0));
	}

	void render_features()
	{
		int y = 60;
		for (auto& e : m_menufeatures)
		{
			switch (e.type)
			{
			case type_bool:
			{
				std::stringstream ssdata;
				bool b = *(bool*)(e.data);
				if ((e.flags & flag_warning) == flag_warning)
					ssdata << (b ? "WARNING (ON)" : "OFF");
				else
					ssdata << (b ? "ON" : "OFF");

				if ((e.flags & flag_red) == flag_red)
				{
					std::stringstream ssfull;
					ssfull << e.label << " : ";
					render::string(20, y, ssfull.str(), false, m_currentidx == e.idx ? RGB(255, 255, 0) : RGB(255, 255, 255));
					SIZE s;
					render::get_text_size(ssfull.str(), &s);
					render::string(20 + s.cx, y, ssdata.str(), false, b ? RGB(255, 0, 0) : RGB(255, 255, 255));
				}
				else
				{
					std::stringstream ssfull;
					ssfull << e.label << " : " << ssdata.str();
					render::string(20, y, ssfull.str(), false, m_currentidx == e.idx ? RGB(255, 255, 0) : RGB(255, 255, 255));
				}

				if (m_currentidx == e.idx)
				{
					if (m_currentidx == e.idx && (GetAsyncKeyState(VK_RIGHT) & 0x1))
					{
						*(bool*)e.data = !(*(bool*)e.data);
					}
					if (m_currentidx == e.idx && (GetAsyncKeyState(VK_LEFT) & 0x1))
					{
						*(bool*)e.data = !(*(bool*)e.data);
					}
				}
				break;
			}
			case type_int:
			{
				std::stringstream ss;
				std::stringstream ssdata;
				int i = *(int*)(e.data);
				if ((e.flags & flag_warning) == flag_warning)
					ssdata << (i != 0 ? "WARNING " : "") << std::to_string(i);
				else
					ssdata << std::to_string(i);

				if ((e.flags & flag_red) == flag_red)
				{
					std::stringstream ssfull;
					ssfull << e.label << " : ";
					render::string(20, y, ssfull.str(), false, m_currentidx == e.idx ? RGB(255, 255, 0) : RGB(255, 255, 255));
					SIZE s;
					render::get_text_size(ssfull.str(), &s);
					render::string(20 + s.cx, y, ssdata.str(), false, i != 0 ? RGB(255, 0, 0) : RGB(255, 255, 255));
				}
				else
				{
					std::stringstream ssfull;
					ssfull << e.label << " : " << ssdata.str();
					render::string(20, y, ssfull.str(), false, m_currentidx == e.idx ? RGB(255, 255, 0) : RGB(255, 255, 255));
				}

				if (m_currentidx == e.idx && (GetAsyncKeyState(VK_RIGHT) & 0x1))
				{
					*(int*)e.data += e.step;
				}
				if (m_currentidx == e.idx && (GetAsyncKeyState(VK_LEFT) & 0x1))
				{
					*(int*)e.data -= e.step;
				}
				break;
			}
			default:
				break;
			}

			y += 16;
		}

		handle_input();
	}

	void element_bool(std::string label, bool* b, unsigned int flags = flag_none)
	{
		mf e;
		e.data = b;
		e.size = 1;
		e.idx = m_idx;
		e.label = label;
		e.type = mf_type::type_bool;
		e.flags = flags;

		m_menufeatures.push_back(e);

		m_idx++;
	}

	void element_int(std::string label, int* i, int step = 1, unsigned int flags = flag_none)
	{
		mf e;
		e.data = i;
		e.size = 1;
		e.idx = m_idx;
		e.label = label;
		e.type = mf_type::type_int;
		e.step = step;
		e.flags = flags;

		m_menufeatures.push_back(e);

		m_idx++;
	}

	static bool esp;
	void init()
	{
		element_bool("[ESP] Enabled", &esp);
	}
}
