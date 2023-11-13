#pragma once
#include "inc.h"

namespace render
{
	HDC dc;
	HWND hwnd;

	HFONT font_title = CreateFontA(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Consolas"));
	HFONT font_normy = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Consolas"));
	HFONT font_esp = CreateFontA(8, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Consolas"));

	void get_text_size(std::string text, SIZE* p)
	{
		GetTextExtentPoint32(dc, text.c_str(), text.size(), p);
	}

	void string(int x, int y, std::string s, bool bg = false, COLORREF col = RGB(255, 255, 255), COLORREF bgcol = RGB(0, 0, 0))
	{
		SetTextColor(dc, col);
		if (bg)
			SetBkColor(dc, bgcol);
		else
			SetBkMode(dc, TRANSPARENT);

		TextOut(dc, x, y, s.c_str(), s.size());

		SetBkMode(dc, OPAQUE);
	}

	void cstm_string(HFONT font, int x, int y, std::string s, bool centered, bool bg = false, COLORREF col = RGB(255, 255, 255), COLORREF bgcol = RGB(0, 0, 0))
	{
		SetTextColor(dc, col);
		if (bg)
			SetBkColor(dc, bgcol);
		else
			SetBkMode(dc, TRANSPARENT);

		HFONT of = (HFONT)GetCurrentObject(dc, OBJ_FONT);
		SelectObject(dc, font);

		if (centered)
		{
			SIZE textSize;
			GetTextExtentPoint32A(dc, s.c_str(), 1, &textSize);
			int startX = x - textSize.cx / 2;
			int startY = y - textSize.cy / 2;
			TextOutA(dc, startX, startY, s.c_str(), 1);
		}
		else
		{
			TextOutA(dc, x, y, s.c_str(), s.size());
		}

		SelectObject(dc, of);
		SetBkMode(dc, OPAQUE);
	}

	void rectangle(int x, int y, int w, int h, COLORREF col, bool outline)
	{
		HPEN hPen = CreatePen(PS_INSIDEFRAME, 1, col);
		HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);

		SelectObject(dc, hBrush);
		SelectObject(dc, hPen);

		Rectangle(dc, x, y, x + w, y + h);

		DeleteObject(hPen);
		DeleteObject(hBrush);

		if (!outline)
			return;

		HPEN hPenOutline = CreatePen(PS_INSIDEFRAME, 1, RGB(0, 0, 0));
		HBRUSH hBrushOutline = (HBRUSH)GetStockObject(NULL_BRUSH);

		SelectObject(dc, hBrushOutline);
		SelectObject(dc, hPenOutline);

		Rectangle(dc, x + 1, y + 1, x + w - 1, y + h - 1);
		Rectangle(dc, x - 1, y - 1, x + w + 1, y + h + 1);

		DeleteObject(hPenOutline);
		DeleteObject(hBrushOutline);
	}

	void rectangle_filled(int x, int y, int w, int h, COLORREF col)
	{
		HBRUSH hBrush = CreateSolidBrush(col);
		RECT r{ x, y, x + w, y + h };
		FillRect(dc, &r, hBrush);
		DeleteObject(hBrush);
	}

	void line(int x1, int y1, int x2, int y2, COLORREF col)
	{
		HPEN hPen = CreatePen(PS_SOLID, 1, col);
		SelectObject(dc, hPen);
		MoveToEx(dc, x1, y1, NULL);
		LineTo(dc, x2, y2);
		DeleteObject(hPen);
	}
}