#include <windows.h>
#include "Input.h"

void MInput::Init()
{
	for (int i = 0; i < 256; ++i)
	{
		Key[i] = KEY_UP;
	}
}


void MInput::Tick()
{
	for (int i = 0; i < 256; i++)
	{
		bool bKeyDown = GetAsyncKeyState(i) & 0x8000;
		int& CurrentKey = Key[i];
		if (CurrentKey == KEY_UP_CONT)
		{
			if (bKeyDown)
			{
				CurrentKey = KEY_DOWN;
			}
			else
			{
				CurrentKey = KEY_UP_CONT;
			}
		}

		if (CurrentKey == KEY_UP)
		{
			if (bKeyDown)
			{
				CurrentKey = KEY_DOWN;
			}
			else
			{
				CurrentKey = KEY_UP_CONT;
			}
		}


		if (CurrentKey == KEY_DOWN_CONT)
		{
			if (bKeyDown)
			{
				CurrentKey = KEY_DOWN_CONT;
			}
			else
			{
				CurrentKey = KEY_UP;
			}
		}

		if (CurrentKey == KEY_DOWN)
		{
			if (bKeyDown)
			{
				CurrentKey = KEY_DOWN_CONT;
			}
			else
			{
				CurrentKey = KEY_UP;
			}
		}
	}
}


void MInput::Exit()
{
	for (int i = 0; i < 256; ++i)
	{
		Key[i] = KEY_UP;
	}
}
