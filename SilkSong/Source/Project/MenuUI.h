#pragma once
#include "UI/UserInterface.h"

class Pointer;

class MenuUI :public UserInterface
{
	Image* Title;
	Image* Logo;
	Image* Icon;
	Image* Black;

	Text* Edition;
	Text* Options[5];
	Button* Buttons[5];

	Pointer* a;
	Pointer* b;

	int32 startFlag = 0;
public:
	MenuUI();

	virtual void Update(float deltaTime)override;
};