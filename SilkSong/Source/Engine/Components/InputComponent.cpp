#include "InputComponent.h"
#include "GameplayStatics.h"



void InputComponent::SetMapping(std::string mappingName, KeyCode value)
{
	mappings.insert({mappingName, value});
}


void InputComponent::BindAction(std::string actionName, InputType type, std::function<void()> func)
{
	if (mappings.find(actionName) != mappings.end())
		actionCallbacks.insert({ actionName, { func,type,false } });
}

Vector2D InputComponent::GetMousePosition()
{
	return bActive ? mousePos : Vector2D{};
}

bool InputComponent::IsMouseButtonPressed()
{
	return bActive ? (GetAsyncKeyState(VK_LBUTTON) & 0x8000) : false;
}

void InputComponent::EnableInput(bool enable)
{
	bActive = enable;
}

void InputComponent::PeekInfo()
{
	if (!bIsEnabled || !bActive)return;

	for (auto& mapping : mappings)
	{
		KeyBindInfo& info = actionCallbacks[mapping.first];
		if (info.type == InputType::Holding)continue;
		if (GetAsyncKeyState((int)mapping.second) & 0x8000)
		{
			if (info.type == InputType::Pressed && !info.pressFlag)info.func();
			if (info.type == InputType::DoubleClick && info.lastTime > 0)
			{
				if (GameplayStatics::GetTimeSeconds() - info.lastTime < 0.5f) { info.func(); info.lastTime = -1; }
				else info.lastTime = 0;
			}
			info.pressFlag = true;
		}
		else if (info.pressFlag)
		{
			if (info.type == InputType::Released)info.func();
			if (info.type == InputType::DoubleClick && info.lastTime == 0)info.lastTime = GameplayStatics::GetTimeSeconds();
			if (info.type == InputType::DoubleClick && info.lastTime == -1)info.lastTime = 0;
			info.pressFlag = false;
		}
	}
}

void InputComponent::PeekInfo_()
{
	if (!bIsEnabled || !bActive)return;

	for (auto& mapping : mappings)
	{
		KeyBindInfo& info = actionCallbacks[mapping.first];
		if (info.type != InputType::Holding)continue;
		if (GetAsyncKeyState((int)mapping.second) & 0x8000)
		{
			info.func();
		}
	}
}

void InputComponent::MouseTick()
{
	if (peekmessage(&msg))
	{
		mousePos = Vector2D(float(msg.x), float(msg.y));
	}
}



Vector2D InputComponent::mousePos = {};
bool InputComponent::bActive = true;
