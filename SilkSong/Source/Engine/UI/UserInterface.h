/**
 * @file	UserInterface.h
 * @brief   UI类模板，用于容纳并管理ui小部件
 * @author	Arty
 **/

#pragma once
#include"Widgets.h"



/*----------------------------------
			   UI类 
  ----------------------------------*/
class UserInterface : public Object
{
	friend Panel;
	std::unordered_set<Widget*>widgets;

	void BeginPlay()override {}
	void EndPlay()override {}

protected:
	Widget* const rootCanvas = new Widget;

public:
	UserInterface();
	~UserInterface() { for (auto& widget : widgets)delete widget; }

	virtual void Update(float deltaTime);

	void ShowInfoBoxs();

	void DrawDebugRect();

	//添加widget小部件到UI模板管理
	template<class T>
	T* AddWidget()
	{
		T* gameWid = new T;
		if (Widget* buf = static_cast<Widget*>(gameWid))
		{
			widgets.insert(buf);
			return gameWid;
		}
		delete gameWid;
		return nullptr;
	}

	//将UI添加到视口
	void AddToViewport();

	//将UI从视口暂时隐藏
	void HideFromViewport();

	//将UI从视口移除
	void RemoveFromViewport();
};