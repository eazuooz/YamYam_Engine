#include "yaToolScene.h"
#include "yaObject.h"
#include "yaTile.h"
#include "yaTilemapRenderer.h"
#include "yaResources.h"
#include "yaTexture.h"
#include "yaCamera.h"
#include "yaRenderer.h"
#include "yaInput.h"
#include "yaCameraScript.h"

namespace ya
{

	ToolScene::ToolScene()
	{
	}

	ToolScene::~ToolScene()
	{
	}

	void ToolScene::Initialize()
	{
		GameObject* camera = object::Instantiate<GameObject>(enums::eLayerType::Particle, Vector2(344.0f, 442.0f));
		Camera* cameraComp = camera->AddComponent<Camera>();
		camera->AddComponent<CameraScript>();

		renderer::mainCamera = cameraComp;

		Scene::Initialize();
	}

	void ToolScene::Update()
	{
		Scene::Update();
	}

	void ToolScene::LateUpdate()
	{
		Scene::LateUpdate();

		if (Input::GetKeyDown(eKeyCode::LButton))
		{
			createTileObject();
		}

		if (Input::GetKeyDown(eKeyCode::S))
		{
			Save();
		}
		if (Input::GetKeyDown(eKeyCode::L))
		{
			Load();
		}
	}

	void ToolScene::Render()
	{
		Scene::Render();
		renderGreed();
	}

	void ToolScene::OnEnter()
	{
		Scene::OnEnter();
	}

	void ToolScene::OnExit()
	{
		Scene::OnExit();
	}

	void ToolScene::Save()
	{
		// open a file name
		OPENFILENAME ofn = {};

		wchar_t szFilePath[256] = {};

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFilePath;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = 256;
		ofn.lpstrFilter = L"Tile\0*.tile\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (false == GetSaveFileName(&ofn))
			return;

		FILE* pFile = nullptr;
		_wfopen_s(&pFile, szFilePath, L"wb");

		for (Tile* tile : mTiles)
		{
			TilemapRenderer* tmr = tile->GetComponent<TilemapRenderer>();
			Transform* tr = tile->GetComponent<Transform>();

			Vector2 sourceIndex = tmr->GetIndex();
			Vector2 position = tr->GetPosition();

			int x = (int)sourceIndex.x;
			int y = (int)sourceIndex.y;

			if (pFile == nullptr)
				break;

			fwrite(&x, sizeof(int), 1, pFile);
			fwrite(&y, sizeof(int), 1, pFile);

			x = (int)position.x;
			y = (int)position.y;
			
			fwrite(&x, sizeof(int), 1, pFile);
			fwrite(&y, sizeof(int), 1, pFile);
		}

		if (pFile)
			fclose(pFile);
	}
	void ToolScene::Load()
	{
		OPENFILENAME ofn = {};

		wchar_t szFilePath[256] = {};

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFilePath;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = 256;
		ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (false == GetOpenFileName(&ofn))
			return;

		FILE* pFile = nullptr;
		_wfopen_s(&pFile, szFilePath, L"rb");

		while (true)
		{
			int idxX = 0;
			int idxY = 0;

			int posX = 0;
			int posY = 0;

			if (pFile == nullptr)
				break;

			if (fread(&idxX, sizeof(int), 1, pFile) == NULL)
				break;
			if (fread(&idxY, sizeof(int), 1, pFile) == NULL)
				break;
			if (fread(&posX, sizeof(int), 1, pFile) == NULL)
				break;
			if (fread(&posY, sizeof(int), 1, pFile) == NULL)
				break;

			Tile* tile = object::Instantiate<Tile>(eLayerType::Tile, Vector2((float)posX, (float)posY));
			TilemapRenderer* tmr = tile->AddComponent<TilemapRenderer>();
			tmr->SetTexture(Resources::Find<graphics::Texture>(L"SpringFloor"));
			tmr->SetIndex(Vector2((float)idxX, (float)idxY));

			mTiles.push_back(tile);
		}

		if (pFile)
			fclose(pFile);
	}
	void ToolScene::renderGreed()
	{

	}
	void ToolScene::createTileObject()
	{
		Vector2 pos = Input::GetMousePosition();
		pos = renderer::mainCamera->CaluateTilePosition(pos);

		if (pos.x >= 0.0f && pos.y >= 0.0f)
		{
			int idxX = (int)(pos.x / TilemapRenderer::TileSize.x);
			int idxY = (int)(pos.y / TilemapRenderer::TileSize.y);

			Tile* tile = object::Instantiate<Tile>(eLayerType::Tile);
			TilemapRenderer* tmr = tile->AddComponent<TilemapRenderer>();
			tmr->SetTexture(Resources::Find<graphics::Texture>(L"SpringFloor"));
			tmr->SetIndex(TilemapRenderer::SelectedIndex);

			tile->SetIndexPosition(idxX, idxY);
			mTiles.push_back(tile);
		}
		else
		{
			//
		}
	}
}

LRESULT CALLBACK WndTileProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
	{
		POINT mousePos = { };
		GetCursorPos(&mousePos);
		ScreenToClient(hWnd, &mousePos);

		ya::math::Vector2 mousePosition;
		mousePosition.x = (float)mousePos.x;
		mousePosition.y = (float)mousePos.y;

		int idxX = (int)(mousePosition.x / ya::TilemapRenderer::OriginTileSize.x);
		int idxY = (int)(mousePosition.y / ya::TilemapRenderer::OriginTileSize.y);

		ya::TilemapRenderer::SelectedIndex = Vector2((float)idxX, (float)idxY);
	}

	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		/*HDC hdc = */BeginPaint(hWnd, &ps);


		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
