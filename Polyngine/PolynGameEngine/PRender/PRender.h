#pragma once

#include <Windows.h>
#include <dxgi1_2.h>
#include <d3d11_2.h>
#include <vector>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")

#include "PDebugLines/PDebugLineRender.h"
#include "../PSystem/Blob/PBlob.h"
#include "../PRender/PEnvironment/PEnvironment.h"
#include "GUIToolbox/ImGui/imgui.h"
#include "GUIToolbox/ImGui/imgui_impl_win32.h"
#include "GUIToolbox/ImGui/imgui_impl_dx11.h"
#include "../PSystem/FBX/FBXExporter/FBXExporter.h"

#define SC_REFRESHRATE	144
#define SC_MSAA_COUNT	GetPrivateProfileInt("Renderer.Scalability", "MSAA.Quality", 0, "../Configurations/Engine.ini")

namespace
{
	template<typename T>
	void safe_release(T* t)
	{
		if (t)
			t->Release();
	}
}

// This defines a texture type for functions requiring texture input.
enum ETextureTypes
{
	DIFFUSE = 0,
	NORMAL = 1,
	SPECULAR = 2,
	EMISSIVE = 3
};

using namespace DirectX;
using namespace PBlob;
using namespace DebugLines;

namespace Renderer
{
	class PRender
	{
	public:
		HWND						hwnd;
		ID3D11Device*				Device = nullptr;
		ID3D11DeviceContext*		Context = nullptr;
		IDXGISwapChain*				SwapChain = nullptr;
		ID3D11RenderTargetView*		RenderTarget = nullptr;
		ID3D11DepthStencilView*		DepthStencilView = nullptr;
		ID3D11DepthStencilState*	DepthStencilState = nullptr;
		ID3D11RasterizerState*		RasterizerState = nullptr;
		ID3D11Buffer*				VertexBuffer = nullptr;
		ID3D11Buffer*				IndexBuffer = nullptr;
		ID3D11InputLayout*			InputLayout = nullptr;
		ID3D11InputLayout*			InputLayout_GeneralShaders = nullptr;
		ID3D11VertexShader*			VS_DebugLines = nullptr;
		ID3D11PixelShader*			PS_DebugLines = nullptr;
		ID3D11VertexShader*			VS_ShadedGeneral = nullptr;
		ID3D11PixelShader*			PS_ShadedGeneral = nullptr;
		ID3D11SamplerState*			LinearSamplerState = nullptr;
		ID3D11BlendState*			BlendState = nullptr;
		ID3D11Texture2D*			DiffuseTexture = nullptr;
		ID3D11Texture2D*			EmissiveTexture = nullptr;
		ID3D11Buffer*				ConstantBuffer = nullptr;
		D3D11_VIEWPORT				Viewport;

	private:
		void DrawView();

		// Print some text to the output log (the console window behind the renderer). Status: 0) Text, 1) Success, 2) Error, 3) Warning.
		void PrintToConsole(std::string OutString, int Status = 0);

		// DirectX object creation helpers.
		void D3DCreateDeviceAndSwapChain();
		void D3DCreateRenderTarget();
		void D3DCreateDepthStencil();
		void D3DCreateRasterizer();
		void D3DCreateShaders();
		void D3DCreateInputLayout();
		void D3DCreateConstantBuffer();

		// ImGui handling of windows and toolkits.
		void CreateImGuiToolkits();
		void CreateImGuiTool_TitleBar();
		void CreateImGuiTool_WindowToolbar();
		void CreateImGuiTool_WorldHierarchy();
		void CreateImGuiTool_OutputLog();
		void CreateImGuiTool_EngineIniEditor();
		void CreateImGuiTool_GameIniEditor();

		// Dialog boxes and basic Windows windows.
		int CreateWindow_OpenLevel();									// Open file dialog for level.
		int CreateWindow_SaveLevel();									// Save file as dialog for level.
		int CreateWindow_SaveCustomClass(EClasses Type);	// Create a new custom class.
		int CreateWindow_OpenTexture(ETextureTypes Type = ETextureTypes::DIFFUSE);	// Open file dialog for a texture.
		int CreateWindow_OpenModelFile();								// Open file dialog for a model.
		int CreateWindow_OpenMeshFile();								// Open file dialog for a mesh.
		int CreateWindow_OpenAnimFile();								// Open animation dialog for a mesh.
		int CreateWindow_ImportFBXFile();								// Import file dialog for a mesh.
		int CreateWindow_ImportFBXAnimation();							// Import file dialog for an FBX animation.
		std::string CreateWindow_PickLevelDialog();						// Pick a plevel file from the project. Returns filepath.

		// Below are tools for interacting with the Engine.ini configuration file.
		//
		// This function returns whether the file was successfully opened for writing. If it fails, this will return false.
		bool LoadEngineIni();

		// GUI object visibilities.
		bool bGUITool_EngineIniEditor = false;
		bool bGUITool_GameIniEditor = false;
		bool bGUITool_WorldHierarchy = false;
		bool bGUITool_Inspector = false;
		bool bGUITool_OutputLog = false;

		// This tracks how many lines are in the output vector for the output log. When the number is different than the current amount, this is updated and the output log scrolls to the most recent ouput.
		unsigned int OutputLine = 0;

	public:
		PRender();
		void InitializeRenderer(HWND WindowHandle);
		void DestroyRenderer();
		void Draw();

		// Turn fullscreen mode on or off.
		void SetFullscreenMode(bool bEnable = true);
		bool GetFullscreenMode();

		struct MVP_t
		{
			XMMATRIX Model;
			XMMATRIX View;
			XMMATRIX Projection;
			float4 DirLightClr;
			float4 HighlightedObjClr;
			float3 DirLightDir;
			float bShowLighting;
			float AmbientLightIntensity;
			float ObjSelected;
			float PointLightCount;
			float DirLightIntensity;
			float4 PointLightLoc[112];
			float4 PointLightClr[112];
			float4 PointLightRad[112];
			float4 PointLightInt[112];
			float4 CameraPos;
			float4 BaseSpecular;
			float4 BaseEmissive;

			float4 PaddingB;

			float bHasEmissiveTex;
			float bHasSpecularTex;

			float2 PaddingC;
			float4 PaddingD;
			float4 PaddingE;
			float4 PaddingF;
		};

		PEnvironment Environment;

		bool bFlag_ShowGrid = true;
		bool bFlag_ShowLighting = true;
		bool bFlag_ImmersiveMode = false;

		// Renderer quality settings options.
		int		Render_Set_LightingQuality				= 1;

		// Editor visual settings that change the GUI colors and details.
		bool		GUI_Pref_UnselectOnHover				= true;
		std::string GUI_Pref_StartupLevel					= "Default";
		float4		GUI_Color_SeletedObjectHighlight		= { 0.93f, 0.05f, 0.96f, 0.2f };
		float4		GUI_Color_DebugGrid						= { 0.3f, 0.3f, 0.3f, 1.0f };
		float4		GUI_Color_TextColor						= { 1.0f, 1.0f, 1.0f, 1.0f };
		float4		GUI_Color_TitleBg						= { 0.33f, 0.22f, 0.32f, 0.9f };
		float4		GUI_Color_TitleActiveBg					= { 0.69f, 0.45f, 0.67f, 0.8f };
		float4		GUI_Color_TitleBorderClr				= { 1.0f, 0.35f, 0.95f, 0.9f };
		float4		GUI_Color_WindowBg						= { 0.0f, 0.0f, 0.0f, 0.5f };
		float4		GUI_Color_HierarchyButtonText			= { 1.0f, 1.0f, 1.0f, 1.0f };
		float4		GUI_Color_HierarchyButtonTextSelect		= { 0.0f, 0.0f, 0.0f, 0.8f };
		float4		GUI_Color_HierarchyButton				= { 0.41f, 0.31f, 0.42f, 0.8f };
		float4		GUI_Color_HierarchyButtonHover			= { 0.41f, 0.31f, 0.42f, 0.8f };
		float4		GUI_Color_HierarchyButtonActiveHover	= { 0.41f, 0.31f, 0.42f, 0.8f };
		float4		GUI_Color_HierarchyButtonSelected		= { 0.69f, 0.45f, 0.67f, 1.0f };

		char	Test_ProjName		[200];
		char	Test_ProjDesc		[1000];
		char	Test_ProjVersion	[10];
	};
}

