/**
 * @file	GameplayStatics.h
 * @brief   全局静态库定义，提供处理游戏常见逻辑所需的接口，更方便访问并管理游戏对象
 * @author	Arty
 **/



#pragma once
#include"CoreMinimal.h"
#include"Core/World.h"


/*----------------------------------
			游戏逻辑静态库
  ----------------------------------*/
class GameplayStatics final
{
public:
	//获取游戏实例
	static GameInstance* GetGameInstance();
	
	//获取玩家控制器
	static class Controller* GetController();

	//创建游戏对象
	template<typename T>
	static T* CreateObject(Vector2D pos = Vector2D(0, 0), float angle = 0, Vector2D scale = Vector2D(1, 1));

	//通过类查找游戏对象
	template<typename T>
	static std::vector<T*>FindObjectsOfClass();

	//通过类查找游戏对象
	template<typename T>
	static T* FindObjectOfClass();

	//通过标签名查找游戏对象
	template<typename T>
	static T* FindObjectOfName(std::string tagName);

	//创建UI对象
	template<class T>
	static T* CreateUI();

	//加载地图
	static void OpenLevel(std::string levelName);

	//退出游戏
	static void QuitGame();

	/**
	 * @brief 相机震动
	 * @param[in] intensity			  震动强度（幅度）
	 * @param[in] decay               震动衰减系数，范围为1`100，越大则衰减速率越快
	 **/
	static void PlayCameraShake(int intensity, int decay = 10);

	//获取游戏开始时间
	static double GetTimeSeconds();

	//启用高斯模糊抗锯齿，每一帧都会处理，很耗性能。如果有人怂恿你使用这个函数那他非蠢即坏
	static void SetGaussianFilterOn(bool enable,int level = 2);

	//暂停游戏数秒
	static void Pause(float delay);

	//播放音效片段
	static void PlaySound2D(std::string name);

	//全局化对象，该对象场景切换后不会被删除
	static void DontDestroyOnLoad(class Actor* obj);

	//全局化对象，该对象场景切换后不会被删除
	static void DontDestroyOnLoad(class UserInterface* obj);

	//将屏幕坐标投射到世界
	static Vector2D ProjectScreenToWorld(Vector2D pos);

	//将世界坐标投射到屏幕
	static Vector2D ProjectWorldToScreen(Vector2D pos);
};






template<typename T>
inline T* GameplayStatics::CreateObject(Vector2D pos, float angle, Vector2D scale)
{
	T* pObj = new T;
	if (pObj && static_cast<Actor*>(pObj))
	{
		mainWorld.GameActors_to_add.push_back(pObj);
		pObj->SetLocalPosition(pos);
		pObj->SetLocalRotation(angle);
		pObj->SetLocalScale(scale);
		return pObj;
	}
	delete pObj;
	return nullptr;
}

template<typename T>
inline std::vector<T*> GameplayStatics::FindObjectsOfClass()
{
	std::vector<T*> result;
	result.reserve(mainWorld.GameActors.size());
	for (auto& obj : mainWorld.GameActors) 
	{
		if (T* pObj = dynamic_cast<T*>(obj))result.push_back(pObj);
	}
	return result;
}

template<typename T>
inline T* GameplayStatics::FindObjectOfClass()
{
	for (auto& obj : mainWorld.GameActors)
	{
		if (T* pObj = dynamic_cast<T*>(obj))return pObj;
	}
	return nullptr;
}

template<typename T>
inline T* GameplayStatics::FindObjectOfName(std::string tagName)
{
	for (auto& obj : mainWorld.GameActors)
	{
		if (obj->GetName() == tagName)return obj;
	}
	return nullptr;
}

template<class T>
inline T* GameplayStatics::CreateUI()
{
	T* pUI = new T;
	if (pUI && static_cast<UserInterface*>(pUI)) 
	{
		mainWorld.GameUIs_to_add.push_back(pUI);
		return pUI;
	}
	delete pUI;
	return nullptr;
}

