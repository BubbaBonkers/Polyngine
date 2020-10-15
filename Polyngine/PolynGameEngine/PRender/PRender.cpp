#include "PRender.h"
#include "wrl/client.h"
#include "../PStatics/PGameplayStatics/PGameplayStatics.h"
#include <sstream>

#define IDM_NEW 100
#define IDM_OPEN 101
#define IDM_SAVE 102

namespace Renderer
{
	PRender::PRender()
	{

	}

	void PRender::InitializeRenderer(HWND WindowHandle)
	{
		PrintToConsole("Initializing Renderer.", 4);

		hwnd = WindowHandle;

		D3DCreateDeviceAndSwapChain();
		D3DCreateRenderTarget();
		D3DCreateDepthStencil();
		D3DCreateRasterizer();
		D3DCreateShaders();
		D3DCreateConstantBuffer();
		D3DCreateInputLayout();

		// Setup the ImGui graphical interfacer.
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& IG_IO = ImGui::GetIO();
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(Device, Context);
		ImGui::StyleColorsDark();

		Environment.Device = Device;
		Environment.DeviceContext = Context;
		Environment.BeginPlay();

		PrintToConsole("Renderer was initialized.", 1);

		// Load the Engine.ini configuration.
		if (LoadEngineIni())
		{
			PrintToConsole("Engine configuration was loaded.", 1);
		}
		else
		{
			PrintToConsole("Could not load engine configuration.", 2);
		}
	}

	void PRender::DestroyRenderer()
	{
		SwapChain->SetFullscreenState(false, nullptr);

		// Destroy ImGui objects and close out the GUI.
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui::DestroyContext();

		Environment.Destroy();

		safe_release(ConstantBuffer);
		safe_release(PS_DebugLines);
		safe_release(VS_DebugLines);
		safe_release(PS_ShadedGeneral);
		safe_release(VS_ShadedGeneral);
		safe_release(InputLayout);
		safe_release(InputLayout_GeneralShaders);
		safe_release(IndexBuffer);
		safe_release(VertexBuffer);
		safe_release(RasterizerState);
		safe_release(DepthStencilState);
		safe_release(DepthStencilView);
		safe_release(RenderTarget);
		safe_release(Context);
		safe_release(SwapChain);
		safe_release(Device);
		safe_release(LinearSamplerState);
		safe_release(BlendState);
		safe_release(DiffuseTexture);
		safe_release(EmissiveTexture);
	}

	void PRender::DrawView()
	{
		// Handle the grid.
		if (bFlag_ShowGrid && Environment.bFlag_ShowDebugGrid && Environment.CurrentState != ERenderStates::SHIP)
		{
			DrawGrid(500, GUI_Color_DebugGrid);
		}

		std::vector<PObject*> WorldObjects = Environment.WorldObjects;

		PCamera* ActiveCamera = Environment.GetActiveCamera();

		// Only draw objects if a render camera is present.
		if (ActiveCamera)
		{
			PMath::view_t View = ActiveCamera->GetWorld();

			const PMath::float4 Black{ 0.0f, 0.0f, 0.0f, 1.0f };

			Context->ClearRenderTargetView(RenderTarget, Black.data());
			Context->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			Context->OMSetDepthStencilState(DepthStencilState, 1);

			Context->RSSetViewports(1, &Viewport);
			Context->OMSetRenderTargets(1, &RenderTarget, DepthStencilView);
			Context->IASetInputLayout(InputLayout);
			Context->RSSetState(RasterizerState);

			Context->VSSetConstantBuffers(0, 1, &ConstantBuffer);

			PCamera* RenderCam = ActiveCamera;
			PDirectionalLight* Sunlight = Environment.GetSunLight();
			MVP_t MVP;
			ZeroMemory(&MVP, sizeof(MVP));

			MVP.Model = XMMatrixTranspose(XMMatrixIdentity());
			MVP.Projection = XMMatrixTranspose((XMMATRIX&)View.ProjectionMatrix);
			MVP.View = XMMatrixTranspose(XMMatrixInverse(nullptr, (XMMATRIX&)View.ViewMatrix));
			MVP.DirLightIntensity = Sunlight ? Sunlight->GetIntensity() : 0.0f;
			MVP.DirLightDir = Sunlight ? Sunlight->GetRotation() : float3{ 0.0f, 0.0f, 0.0f };
			MVP.DirLightClr = Sunlight ? Sunlight->GetColor() : float4{ 0.0f, 0.0f, 0.0f, 0.0f };
			MVP.PointLightCount = (float)Environment.GetPointLights().size();
			MVP.bShowLighting = bFlag_ShowLighting;
			MVP.AmbientLightIntensity = Environment.AmbientLightIntensity;
			MVP.CameraPos = float4({ RenderCam->GetLocation().x, RenderCam->GetLocation().y, RenderCam->GetLocation().z, 0.0f });

			// i_offset is used to ensure that the constant buffer is updated slot after slot and doesn't skip a slot because the i from WorldLights was not the correct type.
			unsigned int i_offset = 0;

			// Setup lighting in constant buffer before drawing all of the world objects.
			std::vector<PLight*> WorldLights = Environment.GetLights();
			for (unsigned int i = 0; i < WorldLights.size(); ++i)
			{
				PPointLight* TestPoint = dynamic_cast<PPointLight*>(WorldLights[i]);
				if (TestPoint && TestPoint->GetVisibility() && ((Environment.CurrentState == ERenderStates::DEBUG) || (!TestPoint->GetHiddenInGame())))
				{
					MVP.PointLightLoc[i - i_offset] = float4{ TestPoint->GetLocation().x, TestPoint->GetLocation().y, TestPoint->GetLocation().z, 0 };
					MVP.PointLightClr[i - i_offset] = TestPoint->GetColor();
					MVP.PointLightRad[i - i_offset] = { TestPoint->GetRadius(), TestPoint->GetRadius(), TestPoint->GetRadius(), TestPoint->GetRadius() };
					MVP.PointLightInt[i - i_offset] = { TestPoint->GetIntensity(), TestPoint->GetIntensity(), TestPoint->GetIntensity(), TestPoint->GetIntensity() };
				}
				else
				{
					i_offset++;
				}
			}

			Context->UpdateSubresource(ConstantBuffer, 0, NULL, &MVP, 0, 0);

			// Debug line renderer.
			Context->VSSetShader(VS_DebugLines, nullptr, 0);
			Context->PSSetShader(PS_DebugLines, nullptr, 0);

			Context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			UINT MeshStrides = sizeof(PMath::colored_vertex);
			UINT Offset = 0;

			Context->UpdateSubresource(VertexBuffer, 0, NULL, DebugLines::GetLineVerts(), 0, 0);
			Context->IASetVertexBuffers(0, 1, &VertexBuffer, &MeshStrides, &Offset);
			Context->Draw(DebugLines::GetLineVertCount(), 0);

			// End Debug Line Drawing ------------------------------------------->

			// Change the Input Layout to Generalized Lighted Objects, and set the Topology to Trianglelist to draw 3D objects with triangles.
			Context->IASetInputLayout(InputLayout_GeneralShaders);
			Context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Draw the world objects that exist.
			for (unsigned int i = 0; i < Environment.WorldObjects.size(); ++i)
			{
				if (Environment.WorldObjects[i] && Environment.WorldObjects[i]->GetVisibility() && ((Environment.CurrentState == ERenderStates::DEBUG) || (!Environment.WorldObjects[i]->GetHiddenInGame())))
				{
					// Ensure the World Object is able to cast to a StaticMesh.
					PStaticMesh* SMesh = dynamic_cast<PStaticMesh*>(Environment.WorldObjects[i]);

					// Ensure the World Object has been casted and is not nullptr before setting up and drawing it.
					if (SMesh)
					{
						RECT ClientRectangle;
						GetClientRect(hwnd, &ClientRectangle);

						XMMATRIX Proj_Mat = DirectX::XMMatrixPerspectiveFovLH(PDegrees_Radians(ActiveCamera->GetFieldOfView()), ((float)ClientRectangle.right / (float)ClientRectangle.bottom), 1.0f, 1000.0f);

						view_t NewView = View;
						NewView.ProjectionMatrix = (float4x4_a&)Proj_Mat;
						NewView.ViewMatrix = (float4x4_a&)(XMMatrixInverse(0, (XMMATRIX&)View.ViewMatrix));

						if (PAABBToFrustum(SMesh->Col_BoundingBox, PCalculateFrustum(NewView, ClientRectangle.right, ClientRectangle.bottom)))
						{
							MeshStrides = sizeof(Vertex);
							Offset = 0;
							Context->IASetVertexBuffers(0, 1, { &SMesh->VertexBuffer }, &MeshStrides, &Offset);
							Context->IASetIndexBuffer(SMesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

							MVP.Model = (XMMATRIX&)Environment.WorldObjects[i]->GetWorld().ViewMatrix;
							MVP.View = XMMatrixInverse(0, (XMMATRIX&)View.ViewMatrix);
							MVP.Projection = (XMMATRIX&)View.ProjectionMatrix;
							MVP.ObjSelected = (((Environment.SelectedObject != nullptr) && (Environment.SelectedObject->GetDisplayName() == WorldObjects[i]->GetDisplayName()) && (Environment.CurrentState == ERenderStates::DEBUG)) ? 1.0f : 0.0f);
							MVP.HighlightedObjClr = GUI_Color_SeletedObjectHighlight;
							MVP.BaseSpecular = { SMesh->Material.Specular[0], SMesh->Material.Specular[0], SMesh->Material.Specular[0], SMesh->Material.Specular[0] };
							MVP.BaseEmissive = { SMesh->Material.Emissive[0], SMesh->Material.Emissive[1], SMesh->Material.Emissive[2], SMesh->Material.Emissive[3] };
							MVP.bHasEmissiveTex = SMesh->Emissive_DDSFile != "" ? 1.0f : 0.0f;
							MVP.bHasSpecularTex = SMesh->Specular_DDSFile != "" ? 1.0f : 0.0f;

							Context->UpdateSubresource(ConstantBuffer, 0, NULL, &MVP, 0, 0);

							Context->VSSetShader(VS_ShadedGeneral, nullptr, 0);
							Context->VSSetConstantBuffers(0, 1, &ConstantBuffer);
							Context->PSSetShader(PS_ShadedGeneral, nullptr, 0);
							Context->PSSetConstantBuffers(0, 1, &ConstantBuffer);
							Context->PSSetSamplers(0, 1, &LinearSamplerState);
							Context->PSSetShaderResources(0, 1, &SMesh->D_ShaderResourceView);
							Context->PSSetShaderResources(1, 1, &SMesh->E_ShaderResourceView);
							Context->PSSetShaderResources(2, 1, &SMesh->S_ShaderResourceView);

							Context->DrawIndexed(SMesh->Indices.size(), 0, 0);
						}
					}
				}
			}
		}
		else
		{
			PrintToConsole("No render camera.", 3);
		}

		// If not in SHIP state, render the editor GUI.
		if (Environment.CurrentState == ERenderStates::DEBUG)
		{
			// Render ImGui Toolkits.
			CreateImGuiToolkits();
		}

		SwapChain->Present(0, 0);
	}

	void PRender::Draw()
	{
		if (GUI_Pref_UnselectOnHover && Environment.InputManager->IsInputDown(PInputManager::PInputMap::PIN_MOUSE_L) && ((Environment.CurrentState == ERenderStates::SHIP) || !(ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse)))
		{
			Environment.SelectedObject = nullptr;
		}

		DrawView();
	}

	// Print some text to the output log. OutString is the string that will be printed and Status will decide the
	// color and type of the string to print.
	//
	// Status:
	//	0. Text
	//	1. Success
	//	2. Failure
	//	3. Warning
	//	4. System Command
	void PRender::PrintToConsole(std::string OutString, int Status)
	{
		Environment.PrintToConsole(OutString, Status, "Renderer");
	}

	// Create the DirectX Context and Device to be used for rendering.
	void PRender::D3DCreateDeviceAndSwapChain()
	{
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);

		// Setup the viewport.
		D3D11_VIEWPORT& d3dViewport = Viewport;
		d3dViewport.Width = (float)ClientRectangle.right;
		d3dViewport.Height = (float)ClientRectangle.bottom;
		d3dViewport.MinDepth = 0.0f;
		d3dViewport.MaxDepth = 1.0f;
		d3dViewport.TopLeftX = 0;
		d3dViewport.TopLeftY = 0;

		// Setup the swapchain.
		DXGI_SWAP_CHAIN_DESC SwapDesc;
		ZeroMemory(&SwapDesc, sizeof(SwapDesc));
		SwapDesc.BufferCount = 1;
		SwapDesc.BufferDesc.Width = ClientRectangle.right;
		SwapDesc.BufferDesc.Height = ClientRectangle.bottom;
		SwapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapDesc.BufferDesc.RefreshRate.Numerator = SC_REFRESHRATE;
		SwapDesc.BufferDesc.RefreshRate.Denominator = 1;
		SwapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapDesc.OutputWindow = hwnd;
		SwapDesc.SampleDesc.Quality = 0;
		SwapDesc.Windowed = TRUE;
		SwapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		if (SC_MSAA_COUNT > 0)
		{
			SwapDesc.SampleDesc.Count = SC_MSAA_COUNT;
		}
		else
		{
			SwapDesc.SampleDesc.Count = 1;
		}

		D3D_FEATURE_LEVEL FeatureLevelsSupported;

		const D3D_FEATURE_LEVEL FeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1
		};
		const UINT NumFeatureLevels = 7;

		UINT DeviceFlags = 0;

#ifdef _DEBUG
		DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, DeviceFlags, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &SwapDesc, &SwapChain, &Device, &FeatureLevelsSupported, &Context);
		assert(!FAILED(hr));

		PrintToConsole("Created Device and SwapChain.");
	}

	void PRender::D3DCreateRenderTarget()
	{
		ID3D11Texture2D* BackBuffer;

		HRESULT hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
		assert(!FAILED(hr));

		hr = Device->CreateRenderTargetView(BackBuffer, NULL, &RenderTarget);

		BackBuffer->Release();

		assert(!FAILED(hr));

		PrintToConsole("Created Renter Target.");
	}

	void PRender::D3DCreateDepthStencil()
	{
		// The following is the DEPTH BUFFER.
		D3D11_TEXTURE2D_DESC DepthBufferDesc;
		ID3D11Texture2D* DepthStencilBuffer;
		ZeroMemory(&DepthBufferDesc, sizeof(DepthBufferDesc));

		DepthBufferDesc.Width = (UINT)Viewport.Width;
		DepthBufferDesc.Height = (UINT)Viewport.Height;
		DepthBufferDesc.MipLevels = 1;
		DepthBufferDesc.ArraySize = 1;
		DepthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		DepthBufferDesc.SampleDesc.Quality = 0;
		DepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		DepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		DepthBufferDesc.CPUAccessFlags = 0;
		DepthBufferDesc.MiscFlags = 0;

		if (SC_MSAA_COUNT > 0)
		{
			DepthBufferDesc.SampleDesc.Count = SC_MSAA_COUNT;
		}
		else
		{
			DepthBufferDesc.SampleDesc.Count = 1;
		}

		HRESULT hr = Device->CreateTexture2D(&DepthBufferDesc, NULL, &DepthStencilBuffer);
		assert(!FAILED(hr));

		// The following is the DEPTH STENCIL.
		D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
		ZeroMemory(&DepthStencilViewDesc, sizeof(DepthStencilViewDesc));

		DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		DepthStencilViewDesc.Texture2D.MipSlice = 0;

		// Do MSAA check and apply settings.
		if (SC_MSAA_COUNT > 0)
		{
			DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		}
		else
		{
			DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		}

		hr = Device->CreateDepthStencilView(DepthStencilBuffer, &DepthStencilViewDesc, &DepthStencilView);
		assert(!FAILED(hr));

		DepthStencilBuffer->Release();

		// The following is the DEPTH STENCIL DESC.
		D3D11_DEPTH_STENCIL_DESC DepthStencilDesc;
		ZeroMemory(&DepthStencilDesc, sizeof(DepthStencilDesc));

		DepthStencilDesc.DepthEnable = true;
		DepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		DepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		hr = Device->CreateDepthStencilState(&DepthStencilDesc, &DepthStencilState);
		assert(!FAILED(hr));

		PrintToConsole("Created Depth Stencil State.");

		// Load the texture into the graphics card.
		D3D11_TEXTURE2D_DESC TextureDesc;
		D3D11_SUBRESOURCE_DATA TextureSource;
		ZeroMemory(&TextureDesc, sizeof(TextureDesc));
		ZeroMemory(&TextureSource, sizeof(TextureSource));

		TextureDesc.ArraySize = 1;
		TextureDesc.MipLevels = 1;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
		TextureDesc.Height = 256;
		TextureDesc.Width = 256;
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		TextureDesc.MiscFlags = 0;

		hr = Device->CreateTexture2D(&TextureDesc, nullptr, &DiffuseTexture);
		assert(!FAILED(hr));

		hr = Device->CreateTexture2D(&TextureDesc, nullptr, &EmissiveTexture);
		assert(!FAILED(hr));

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Load the Texture
		hr = Device->CreateSamplerState(&sampDesc, &LinearSamplerState);
		assert(!FAILED(hr));

		PrintToConsole("Created Sampler State.");
	}

	void PRender::D3DCreateRasterizer()
	{
		D3D11_RASTERIZER_DESC RasterizerDesc;
		ZeroMemory(&RasterizerDesc, sizeof(RasterizerDesc));

		RasterizerDesc.AntialiasedLineEnable = true;
		RasterizerDesc.DepthClipEnable = true;
		RasterizerDesc.FillMode = D3D11_FILL_SOLID;
		RasterizerDesc.FrontCounterClockwise = false;
		RasterizerDesc.ScissorEnable = false;

		// Check for MSAA and apply MSAA settings.
		if (SC_MSAA_COUNT > 0)
		{
			RasterizerDesc.CullMode = D3D11_CULL_NONE;
			RasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
			RasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
			RasterizerDesc.MultisampleEnable = true;
			RasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		}
		else
		{
			RasterizerDesc.CullMode = D3D11_CULL_BACK;
			RasterizerDesc.DepthBias = 0;
			RasterizerDesc.DepthBiasClamp = 0.0f;
			RasterizerDesc.MultisampleEnable = false;
			RasterizerDesc.SlopeScaledDepthBias = 0.0f;
		}

		HRESULT hr = Device->CreateRasterizerState(&RasterizerDesc, &RasterizerState);
		assert(!FAILED(hr));

		PrintToConsole("Created Rasterizer State.");

		D3D11_BLEND_DESC BlendDescState;
		D3D11_RENDER_TARGET_BLEND_DESC TargetBlendState;
		ZeroMemory(&BlendDescState, sizeof(D3D11_BLEND_DESC));

		TargetBlendState.BlendEnable = TRUE;
		TargetBlendState.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		TargetBlendState.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		TargetBlendState.BlendOp = D3D11_BLEND_OP_ADD;
		TargetBlendState.SrcBlendAlpha = D3D11_BLEND_ONE;
		TargetBlendState.DestBlendAlpha = D3D11_BLEND_ZERO;
		TargetBlendState.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		TargetBlendState.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		BlendDescState.AlphaToCoverageEnable = FALSE;
		BlendDescState.IndependentBlendEnable = FALSE;
		BlendDescState.RenderTarget[0] = TargetBlendState;

		hr = Device->CreateBlendState(&BlendDescState, &BlendState);
		assert(!FAILED(hr));

		PrintToConsole("Created Blend State.");
	}

	void PRender::D3DCreateShaders()
	{
		// Debug line shaders.
		binary_blob_t VS_Blob = LoadBinaryBlob("Shaders/CSO/VS_DebugLines.cso");
		binary_blob_t PS_Blob = LoadBinaryBlob("Shaders/CSO/PS_DebugLines.cso");

		HRESULT hr = Device->CreateVertexShader(VS_Blob.data(), VS_Blob.size(), NULL, &VS_DebugLines);
		assert(!FAILED(hr));

		hr = Device->CreatePixelShader(PS_Blob.data(), PS_Blob.size(), NULL, &PS_DebugLines);
		assert(!FAILED(hr));

		// Generalized shaded object shader for 3D models.
		VS_Blob = LoadBinaryBlob("Shaders/CSO/VS_ShadedGeneral.cso");
		PS_Blob = LoadBinaryBlob("Shaders/CSO/PS_ShadedGeneral.cso");

		hr = Device->CreateVertexShader(VS_Blob.data(), VS_Blob.size(), NULL, &VS_ShadedGeneral);
		assert(!FAILED(hr));

		PrintToConsole("Created Vertex Shader.");

		hr = Device->CreatePixelShader(PS_Blob.data(), PS_Blob.size(), NULL, &PS_ShadedGeneral);
		assert(!FAILED(hr));

		PrintToConsole("Created Pixel Shader.");
	}

	void PRender::D3DCreateInputLayout()
	{
		binary_blob_t VS_Blob = LoadBinaryBlob("Shaders/CSO/VS_DebugLines.cso");

		D3D11_INPUT_ELEMENT_DESC InputDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		HRESULT hr = Device->CreateInputLayout(InputDesc, 2, VS_Blob.data(), VS_Blob.size(), &InputLayout);
		assert(!FAILED(hr));

		PrintToConsole("Created Debug Line Input Layout.");

		VS_Blob = LoadBinaryBlob("Shaders/CSO/VS_ShadedGeneral.cso");

		// Describe it to DirectX.
		D3D11_INPUT_ELEMENT_DESC Gen_InputDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		hr = Device->CreateInputLayout(Gen_InputDesc, 3, VS_Blob.data(), VS_Blob.size(), &InputLayout_GeneralShaders);
		assert(!FAILED(hr));

		PrintToConsole("Created Shaded Input Layout.");
	}

	void PRender::CreateImGuiToolkits()
	{
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);

		ImGui::GetIO().DisplaySize.x = ClientRectangle.right;
		ImGui::GetIO().DisplaySize.y = ClientRectangle.bottom;

		ImGui::GetStyle().AntiAliasedFill = true;
		ImGui::GetStyle().AntiAliasedLines = true;

		// Start the ImGui frame under both implimentations, then the master.
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Create the main window toolbar.
		CreateImGuiTool_WindowToolbar();

		CreateImGuiTool_TitleBar();
		CreateImGuiTool_WorldHierarchy();

		CreateImGuiTool_OutputLog();

		// Gather the draw data together to pass to Direct X.
		ImGui::Render();

		// Render the data compiled using the Direct X pipeline.
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void PRender::CreateImGuiTool_TitleBar()
	{
		// Get window size.
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);

		if (ImGui::BeginMainMenuBar())
		{
			// Set the font scale in the window.
			ImGui::SetWindowFontScale(1.33f);

			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Level"))
				{
					PrintToConsole("Clearing environment for a new level.", 4);

					Environment.CurrentLevel = "";
					SetWindowTextA(hwnd, "Polyn v0.5");

					Environment.ClearEnvironment(false, true);

					PrintToConsole("Environment was cleared. Ready for use.", 1);
				}

				if (Environment.CurrentLevel != "" && ImGui::MenuItem("Save Level"))
				{
					Environment.SaveLevel(Environment.CurrentLevel);
				}

				if (ImGui::MenuItem("Save As"))
				{
					CreateWindow_SaveLevel();
				}

				if (ImGui::MenuItem("Open Level"))
				{
					CreateWindow_OpenLevel();
				}

				if (ImGui::MenuItem("Exit Polyn"))
				{
					PostQuitMessage(0);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Copy"))
				{

				}

				if (ImGui::MenuItem("Paste"))
				{

				}

				if (ImGui::MenuItem("Duplicate"))
				{

				}

				if (ImGui::MenuItem("Delete"))
				{
					if (Environment.SelectedObject)
					{
						Environment.DestroyObject(Environment.SelectedObject);
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Project"))
			{
				if (ImGui::BeginMenu("Properties"))
				{
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem("Engine Properties"))
					{
						bGUITool_EngineIniEditor = true;
					}

					if (ImGui::MenuItem("Game Properties"))
					{
						if (!bGUITool_GameIniEditor)
						{
							ImGui::SetNextWindowPos({ (ClientRectangle.right - (ClientRectangle.right * 0.35f)), (ClientRectangle.bottom - (ClientRectangle.bottom * 0.35f)) });
						}

						bGUITool_GameIniEditor = true;
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::BeginMenu("Custom Type"))
				{
					if (ImGui::MenuItem("PObject"))
					{
						CreateWindow_SaveCustomClass(EClasses::OBJECT);
					}

					if (ImGui::MenuItem("PController"))
					{
						CreateWindow_SaveCustomClass(EClasses::CONTROLLER);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Primitives"))
				{
					// Set the font scale in the window.
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem("Plane"))
					{
						Environment.CreatePrimitive(EPrimitives::PLANE);
					}

					if (ImGui::MenuItem("Cube"))
					{
						Environment.CreatePrimitive(EPrimitives::CUBE);
					}

					if (ImGui::MenuItem("Sphere"))
					{
						Environment.CreatePrimitive(EPrimitives::SPHERE);
					}

					if (ImGui::MenuItem("Cone"))
					{
						Environment.CreatePrimitive(EPrimitives::CONE);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("3D Objects"))
				{
					// Set the font scale in the window.
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem("Static Mesh"))
					{
						Environment.CreateStaticMesh("Models/Lamp.obj", "Textures/Lantern.dds");
					}

					if (ImGui::MenuItem("Skeletal Mesh"))
					{
						Environment.CreateSkeletalMesh("Meshes/BattleMage.mesh", "Textures/BattleMage/BattleMage_D.dds", "Textures/BattleMage/BattleMage_S.dds", "Textures/BattleMage/BattleMage_E.dds");
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Cinematics"))
				{
					// Set the font scale in the window.
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem("Camera"))
					{
						Environment.CreateCamera();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Lighting"))
				{
					// Set the font scale in the window.
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem("Directional Light"))
					{
						Environment.CreateDirectionalLight(1.0f, { 0.33f, 0.0f, 0.0f });
					}
					else if (ImGui::MenuItem("Point Light"))
					{
						Environment.CreatePointLight(1.0f, 125.0f);
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Import"))
			{
				if (ImGui::BeginMenu("3D Objects"))
				{
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem(".FBX - Skeletal Mesh"))
					{
						CreateWindow_ImportFBXFile();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Convert a .FBX Seletal Mesh into a .MESH file and import it into your project directory.");
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Materials"))
				{
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem(".FBX - Mesh Material"))
					{
						CreateWindow_ImportFBXFile();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Import Materials from a .FBX file and store them in the project directory as a .MAT file.");
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Animations"))
				{
					ImGui::SetWindowFontScale(1.33f);

					if (ImGui::MenuItem(".FBX - Mesh Animation"))
					{
						CreateWindow_ImportFBXAnimation();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Import Animations from a .FBX file and store them in the project directory as a .ANIM file.");
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Documentation"))
				{
					ShellExecute(NULL, "open", (PGameplayStatics::GetMainDirectory() + "../Documentation/PolyngineBreakdown.docx").c_str(), NULL, NULL, SW_SHOW);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// Create GUI Tools needed.
		if (bGUITool_EngineIniEditor)
		{
			CreateImGuiTool_EngineIniEditor();
		}

		if (bGUITool_GameIniEditor)
		{
			CreateImGuiTool_GameIniEditor();
		}
	}

	void PRender::CreateImGuiTool_WindowToolbar()
	{
		// Get window size.
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);

		// Setup the toolbar style before drawing.
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { GUI_Color_WindowBg.x, GUI_Color_WindowBg.y, GUI_Color_WindowBg.z, GUI_Color_WindowBg.w });
		ImGui::PushStyleColor(ImGuiCol_Border, { GUI_Color_TitleBorderClr.x, GUI_Color_TitleBorderClr.y, GUI_Color_TitleBorderClr.z, GUI_Color_TitleBorderClr.w });
		ImGui::PushStyleColor(ImGuiCol_TitleBg, { GUI_Color_TitleBg.x, GUI_Color_TitleBg.y, GUI_Color_TitleBg.z, GUI_Color_TitleBg.w });
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, { GUI_Color_TitleActiveBg.x, GUI_Color_TitleActiveBg.y, GUI_Color_TitleActiveBg.z, GUI_Color_TitleActiveBg.w });
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.41f, 0.31f, 0.42f, 0.8f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.69f, 0.45f, 0.67f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.0f, 0.0f, 0.0f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Text, { GUI_Color_TextColor.x, GUI_Color_TextColor.y, GUI_Color_TextColor.z, GUI_Color_TextColor.w });

		ImGui::SetNextWindowSize({ ((float)ClientRectangle.right * 0.25f), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.07f)) });
		ImGui::SetNextWindowPos({ ((float)ClientRectangle.right - ((float)ClientRectangle.right * 0.25f)), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.93f)) });

		ImGui::SetNextWindowSize({ ((float)ClientRectangle.right * 0.99f), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.945f)) });
		ImGui::SetNextWindowPos({ ((float)ClientRectangle.right * 0.005f), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.975f)) });

		// Create Toolbar window.
		ImGui::Begin("Environment Toolbar");

		// Set the font scale in the window.
		ImGui::SetWindowFontScale(1.62f);

		if (ImGui::Button("Options"))
		{
			ImGui::OpenPopup("Renderer");
		}

		if (ImGui::BeginPopup("Renderer"))
		{
			if (ImGui::BeginMenu("Show"))
			{
				// Set the font scale in the window.
				ImGui::SetWindowFontScale(1.45f);

				bool bShowAllButton = (bFlag_ShowGrid && bFlag_ShowLighting && Environment.bFlag_ShowDebugMatrices && Environment.bFlag_ShowCollisionBoxes);
				if (ImGui::Selectable("Show All", bShowAllButton))
				{
					bFlag_ShowGrid = true;
					bFlag_ShowLighting = true;
					Environment.bFlag_ShowDebugMatrices = true;
					Environment.bFlag_ShowCollisionBoxes = true;
					Environment.bFlag_ShowDebugGrid = true;

					PrintToConsole("All renderer debug flags have been enabled.");
				}

				if (ImGui::Selectable("World Grid", bFlag_ShowGrid))
				{
					bFlag_ShowGrid = !bFlag_ShowGrid;
					Environment.bFlag_ShowDebugGrid = bFlag_ShowGrid;
					PrintToConsole("World Grid flag has been " + std::string(bFlag_ShowGrid ? "enabled" : "disabled") + ".");
				}
				
				if (ImGui::Selectable("Debug Matrices", Environment.bFlag_ShowDebugMatrices))
				{
					Environment.bFlag_ShowDebugMatrices = !Environment.bFlag_ShowDebugMatrices;
					PrintToConsole("Debug Matrices flag has been " + std::string(Environment.bFlag_ShowDebugMatrices ? "enabled" : "disabled") + ".");
				}

				if (ImGui::Selectable("Collision Boxes", Environment.bFlag_ShowCollisionBoxes))
				{
					Environment.bFlag_ShowCollisionBoxes = !Environment.bFlag_ShowCollisionBoxes;
					PrintToConsole("Bounding Boxes flag has been " + std::string(Environment.bFlag_ShowCollisionBoxes ? "enabled" : "disabled") + ".");
				}

				if (ImGui::Selectable("Object Lighting", bFlag_ShowLighting))
				{
					bFlag_ShowLighting = !bFlag_ShowLighting;
					PrintToConsole("Object Lighting flag has been " + std::string(bFlag_ShowLighting ? "enabled" : "disabled") + ".");
				}

				bool bHideAllButton = (!bFlag_ShowGrid && !bFlag_ShowLighting && !Environment.bFlag_ShowDebugMatrices && !Environment.bFlag_ShowCollisionBoxes);
				if (ImGui::Selectable("Hide All", bHideAllButton))
				{
					bFlag_ShowGrid = false;
					bFlag_ShowLighting = false;
					Environment.bFlag_ShowDebugMatrices = false;
					Environment.bFlag_ShowCollisionBoxes = false;
					Environment.bFlag_ShowDebugGrid = false;

					PrintToConsole("All renderer debug flags have been disabled.");
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Renderer"))
			{
				if (ImGui::Selectable("Fullscreen Mode", GetFullscreenMode()))
				{
					RECT WindowSize;
					GetClientRect(hwnd, &WindowSize);

					SetFullscreenMode(!GetFullscreenMode());
					Environment.RefreshCameraAspectRatios(WindowSize.right / WindowSize.bottom);
				}

				ImGui::EndMenu();
			}
			
			ImGui::EndPopup();
		}

		if (Environment.CurrentLevel != "")
		{
			ImGui::SameLine();

			if (ImGui::Button("Launch"))
			{
				if (Environment.GetLevelStartCamera() != nullptr)
				{
					Environment.BeginPlaytest();
				}
				else
				{
					PrintToConsole("There is no camera set to be the view on Level Start!", 2);
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Test will launch your game in Test mode where you can test the currently loaded level.");
			}
		}

		ImGui::SameLine();

		ImGui::Text("FPS:");

		ImGui::SameLine();

		char FPSBuf[8];
		float TempFPS = Environment.FPSRef;
		sprintf(FPSBuf, "%.0f", TempFPS);
		ImGui::Text(FPSBuf);

		// Set the font scale in the window.
		ImGui::SetWindowFontScale(1.0f);

		// End window creation.
		ImGui::End();
	}

	void PRender::CreateImGuiTool_WorldHierarchy()
	{
		// Get window size.
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);

		if (!bGUITool_WorldHierarchy)
		{
			ImGui::SetNextWindowSize({ ((float)ClientRectangle.right * 0.15f), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.595f)) });
			ImGui::SetNextWindowPos({ ((float)ClientRectangle.right - ((float)ClientRectangle.right * 0.155f)), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.917f)) });
		}

		ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;

		// Create Toolbar window.
		if (ImGui::Begin("World Hierarchy", NULL))
		{
			// Set the font scale in the window.
			ImGui::SetWindowFontScale(1.33f);

			for (unsigned int i = 0; i < Environment.WorldObjects.size(); ++i)
			{
				PObject* CurrObj = Environment.WorldObjects[i];

				if (CurrObj && CurrObj->bShowInHierarchy)
				{
					if ((Environment.SelectedObject != nullptr) && (Environment.SelectedObject->GetDisplayName() == CurrObj->GetDisplayName()))
					{
						ImGui::PushStyleColor(ImGuiCol_Button, { GUI_Color_HierarchyButtonSelected.x, GUI_Color_HierarchyButtonSelected.y, GUI_Color_HierarchyButtonSelected.z, GUI_Color_HierarchyButtonSelected.w });
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { GUI_Color_HierarchyButtonActiveHover.x, GUI_Color_HierarchyButtonActiveHover.y, GUI_Color_HierarchyButtonActiveHover.z, GUI_Color_HierarchyButtonActiveHover.w });
						ImGui::PushStyleColor(ImGuiCol_Text, { GUI_Color_HierarchyButtonTextSelect.x, GUI_Color_HierarchyButtonTextSelect.y, GUI_Color_HierarchyButtonTextSelect.z, GUI_Color_HierarchyButtonTextSelect.w });
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Button, { GUI_Color_HierarchyButton.x, GUI_Color_HierarchyButton.y, GUI_Color_HierarchyButton.z, GUI_Color_HierarchyButton.w });
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { GUI_Color_HierarchyButtonHover.x, GUI_Color_HierarchyButtonHover.y, GUI_Color_HierarchyButtonHover.z, GUI_Color_HierarchyButtonHover.w });
						ImGui::PushStyleColor(ImGuiCol_Text, { GUI_Color_HierarchyButtonText.x, GUI_Color_HierarchyButtonText.y, GUI_Color_HierarchyButtonText.z, GUI_Color_HierarchyButtonText.w });
					}

					std::string NameButtonText = std::string((CurrObj->ParentObject == nullptr) ? "" : "  + ") + CurrObj->GetDisplayName();
					if (ImGui::Button(NameButtonText.c_str(), { ((float)ClientRectangle.right * 0.1425f), ((float)ClientRectangle.bottom * 0.02f) }))
					{
						if ((Environment.SelectedObject != nullptr) && Environment.SelectedObject == CurrObj)
						{
							Environment.SelectedObject = nullptr;
						}
						else
						{
							Environment.SelectedObject = Environment.WorldObjects[i];
						}
					}

					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}
			}

			ImGui::SetWindowFontScale(1.0f);
			ImGui::End();
		}

		if (!bGUITool_WorldHierarchy)
		{
			bGUITool_WorldHierarchy = true;
		}

		if (!bGUITool_Inspector)
		{
			ImGui::SetNextWindowSize({ ((float)ClientRectangle.right * 0.15f), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.495f)) });
			ImGui::SetNextWindowPos({ ((float)ClientRectangle.right - ((float)ClientRectangle.right * 0.155f)), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.51f)) });
		}

		// Create Toolbar window.
		if (ImGui::Begin("World Editor", NULL))
		{
			// Set the font scale in the window.
			ImGui::SetWindowFontScale(1.33f);

			if (ImGui::BeginTabBar("Tabs", ImGuiTabBarFlags_Reorderable))
			{
				if (ImGui::BeginTabItem("Inspector"))
				{
					if (Environment.SelectedObject)
					{
						PObject* CurrObj = Environment.SelectedObject;

						if ((CurrObj != nullptr) && CurrObj->GetDisplayName() != "RenderCamera")
						{
							ImGui::Text("Object Information");

							char InName[500];
							strcpy(InName, CurrObj->GetDisplayName().c_str());
							bool bNameAvailable = (Environment.GetObjectByName(InName) == nullptr);

							if (ImGui::InputText("Name", InName, 500))
							{
								if (Environment.GetObjectByName(InName) == nullptr)
								{
									CurrObj->DisplayName = InName;
								}
							}

							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip("Change the Object Name");
							}


							XMVECTOR XMRotation;
							XMVECTOR XMScale;
							XMVECTOR XMTrans;

							XMMatrixDecompose(&XMScale, &XMRotation, &XMTrans, CurrObj->ParentObject ? (XMMATRIX&)CurrObj->GetLocal() : (XMMATRIX&)CurrObj->GetWorld().ViewMatrix);

							float3 ObjPos = { XMVectorGetX(XMTrans), XMVectorGetY(XMTrans), XMVectorGetZ(XMTrans) };
							float3 ObjRot = { XMVectorGetX(XMRotation), XMVectorGetY(XMRotation), XMVectorGetZ(XMRotation) };
							float3 ObjScale = { XMVectorGetX(XMScale), XMVectorGetY(XMScale), XMVectorGetZ(XMScale) };

							float ObjPosVec[4] = { ObjPos.x, ObjPos.y, ObjPos.z, 1.0f };
							float ObjRotVec[4] = { ObjRot.x, ObjRot.y, ObjRot.z, 1.0f };
							float ObjScaleVec[4] = { ObjScale.x, ObjScale.y, ObjScale.z, 1.0f };

							if (CurrObj->ParentObject != nullptr)
							{
								ImGui::Text(("Parent: " + CurrObj->ParentObject->GetDisplayName()).c_str());
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("This is the parent to this object.");
								}

								ImGui::Text("\nLocal Orientation");
							}

							ImGui::PushItemWidth(285.0f);

							if (ImGui::DragFloat3("Location", ObjPosVec, 0.065f, -999999.0f, 999999.0f, "%.2f"))
							{
								if (CurrObj->ParentObject)
								{
									CurrObj->SetLocalLocation({ ObjPosVec[0], ObjPosVec[1], ObjPosVec[2] });
								}
								else
								{
									CurrObj->SetLocation({ ObjPosVec[0], ObjPosVec[1], ObjPosVec[2] });
								}
							}
							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip(CurrObj->ParentObject ? "Object Local Location" : "Object Location");
							}

							if (ImGui::DragFloat3("Rotation", ObjRotVec, 0.25f, -179.85f, 179.85f, "%.2f"))
							{
								//CurrObj->SetRotation({ PDegrees_Radians(ObjRotVec[0]), PDegrees_Radians(ObjRotVec[1]), PDegrees_Radians(ObjRotVec[2]) });
							}
							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip("Object Rotation");
							}

							if (ImGui::DragFloat3("Scale", ObjScaleVec, 0.065f, -5000.0f, 5000.0f, "%.2f"))
							{
								CurrObj->SetScale({ ObjScaleVec[0], ObjScaleVec[1], ObjScaleVec[2] });
							}
							if (ImGui::IsItemHovered())
							{
								ImGui::SetTooltip("Object Scale");
							}

							ImGui::PopItemWidth();

							// Add lighting stuffs.
							PLight* TestLight = dynamic_cast<PLight*>(CurrObj);
							if (TestLight)
							{
								ImGui::Separator();

								ImGui::Text("Light Details");

								ImGui::PushItemWidth(285.0f);

								float LightClr[4] = { TestLight->GetColor().x, TestLight->GetColor().y, TestLight->GetColor().z, TestLight->GetColor().w };
								if (ImGui::ColorEdit4("Color", LightClr))
								{
									TestLight->SetColor({ LightClr[0], LightClr[1], LightClr[2], LightClr[3] });
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("The color of the light emitted from this light-source.");
								}

								ImGui::PopItemWidth();

								ImGui::PushItemWidth(200.0f);

								float LightInt = TestLight->GetIntensity();

								if (ImGui::DragFloat("Intensity", &LightInt, 0.005f, 0.0f, 4.0f, "%.2f"))
								{
									TestLight->SetIntensity(LightInt);
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("The strength of the light emitted from this source imposed on the objects around them.");
								}

								// Add Point Light stuffs.
								PPointLight* TestPLight = dynamic_cast<PPointLight*>(CurrObj);
								if (TestPLight)
								{
									float PLightInt = TestPLight->GetRadius();

									if (ImGui::DragFloat("Radius", &PLightInt, 0.25f, 0.0f, 20000.0f, "%.2f"))
									{
										TestPLight->SetRadius(PLightInt);
									}
									if (ImGui::IsItemHovered())
									{
										ImGui::SetTooltip("The distance light travels from this object before not being affacted anymore.");
									}
								}

								ImGui::PopItemWidth();
							}

							// Add camera stuffs.
							PCamera* TestCam = dynamic_cast<PCamera*>(CurrObj);
							if (TestCam)
							{
								ImGui::Separator();

								ImGui::Text("Camera Details");

								float CamFOV = TestCam->GetFieldOfView();

								// FOV for camera.
								if (ImGui::DragFloat("FOV", &CamFOV, 0.025f, 0.0f, 120.0f, "%.2f"))
								{
									TestCam->SetFieldOfView(CamFOV);
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("The vertical field of view is the amount of area the player can see when viewing through a camera. Higher numbers mean more in-cam at once, likewise lower numbers decrease the amount you can see at once.");
								}

								// Should the object be active when the game starts.
								ImGui::Checkbox("Set Active on Start", &TestCam->GetIsActiveOnStart());
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("If checked, this camera will be set to the active camera when the game begins. Only one camera can be set active at any given time.");
								}
							}

							// Add Static Mesh stuffs.
							PStaticMesh* TestMesh = dynamic_cast<PStaticMesh*>(CurrObj);
							if (TestMesh)
							{
								ImGui::Separator();

								ImGui::Text("Static Mesh Details");

								ImGui::SetWindowFontScale(1.0f);

								std::string ModelName = TestMesh->ModelFile;
								std::string TextureName = TestMesh->DDSFile;
								std::string N_TextureName = TestMesh->Normal_DDSFile;
								std::string S_TextureName = TestMesh->Specular_DDSFile;
								std::string E_TextureName = TestMesh->Emissive_DDSFile;
								std::string Anim_Curr = "";

								if (TestMesh->Animator.GetFile() != "")
								{
									Anim_Curr = TestMesh->Animator.GetFile();
								}

								if (ImGui::Button(("Model: " + PGameplayStatics::SplitStringGetLast(ModelName, '/')).c_str()))
								{
									if (dynamic_cast<PSkeletalMesh*>(CurrObj))
									{
										CreateWindow_OpenMeshFile();
									}
									else
									{
										CreateWindow_OpenModelFile();
									}
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip(ModelName.c_str());
								}

								/*if (ImGui::Button(("Normal: " + SplitStringGetLast(N_TextureName, '/')).c_str()))
								{
									CreateWindow_OpenTexture(TEXTURE::NORMAL);
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip(N_TextureName.c_str());
								}*/

								ImGui::Separator();

								ImGui::SetWindowFontScale(1.33f);
								ImGui::Text("Material Details");

								ImGui::SetWindowFontScale(1.0f);
								if (ImGui::Button(("Diffuse: " + PGameplayStatics::SplitStringGetLast(TextureName, '/')).c_str()))
								{
									CreateWindow_OpenTexture();
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip(TextureName.c_str());
								}

								if (ImGui::Button(("Specular: " + PGameplayStatics::SplitStringGetLast(S_TextureName, '/')).c_str()))
								{
									CreateWindow_OpenTexture(ETextureTypes::SPECULAR);
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip(S_TextureName.c_str());
								}

								if (ImGui::Button(("Emissive: " + PGameplayStatics::SplitStringGetLast(E_TextureName, '/')).c_str()))
								{
									CreateWindow_OpenTexture(ETextureTypes::EMISSIVE);
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip(E_TextureName.c_str());
								}

								ImGui::SetWindowFontScale(1.25f);

								ImGui::DragFloat("Specular", TestMesh->Material.Specular, 0.0065f, 0.0f, 1.0f, "%.2f");
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("This specular value will be added to any texture specular associated with this model.");
								}

								ImGui::DragFloat3("Emissive", TestMesh->Material.Emissive, 0.0065f, 0.0f, 1.0f, "%.2f");
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("This emissive value will be added to any texture emissive associated with this model.");
								}

								ImGui::Separator();

								if (dynamic_cast<PSkeletalMesh*>(CurrObj))
								{
									ImGui::SetWindowFontScale(1.33f);
									ImGui::Text("Animations");

									if (ImGui::Button(("Preview: " + PGameplayStatics::SplitStringGetLast(Anim_Curr, '/')).c_str()))
									{
										CreateWindow_OpenAnimFile();
									}
									if (ImGui::IsItemHovered())
									{
										ImGui::SetTooltip(Anim_Curr.c_str());
									}

									if (CurrObj->Animator.GetReady())
									{
										if (ImGui::Button("-"))
										{
											if (!CurrObj->Animator.GetPaused())
											{
												CurrObj->Animator.Pause();
											}

											CurrObj->Animator.FrameJump(-1);
										}

										ImGui::SameLine();

										if (ImGui::Button("+"))
										{
											if (!CurrObj->Animator.GetPaused())
											{
												CurrObj->Animator.Pause();
											}

											CurrObj->Animator.FrameJump(1);
										}

										ImGui::SameLine();

										if (CurrObj->Animator.GetPaused())
										{
											if (ImGui::Button("Play"))
											{
												CurrObj->Animator.Play();
											}
										}
										else
										{
											if (ImGui::Button("Pause"))
											{
												CurrObj->Animator.Pause();
											}
										}

										ImGui::SameLine();

										(CurrObj->Animator.Anim.GetPaused()) ? ImGui::Text("Paused") : ImGui::Text("Previewing");

										float AnimTime = CurrObj->Animator.Anim.Time;
										if (CurrObj->Animator.GetReady())
										{
											if (ImGui::SliderFloat("Time", &AnimTime, 0.0f, CurrObj->Animator.GetDuration(), "%.2f", 1.0f))
											{
												if (!CurrObj->Animator.GetPaused())
												{
													CurrObj->Animator.Pause();
												}

												CurrObj->Animator.TimeSet(AnimTime);
											}
										}
									}

									ImGui::Text(("Time: " + std::to_string(CurrObj->Animator.Anim.Time) + " / " + std::to_string(CurrObj->Animator.Anim.GetDuration())).c_str());

									if (CurrObj->Animator.GetReady())
									{
										ImGui::Text(("Frame: " + std::to_string(CurrObj->Animator.Anim.GetFrameAtTime())).c_str());
									}
								}

								ImGui::Separator();

								ImGui::SetWindowFontScale(1.33f);
								ImGui::Text("Collision Details");

								bool bEnableCollision = true;
								float3 ObjBBExt = TestMesh->GetBoundingBoxExtents();
								float3 ObjBBOffset = TestMesh->GetBoundingBoxOffset();
								float ObjBBExtVec[4] = { ObjBBExt.x, ObjBBExt.y, ObjBBExt.z, 1.0f };
								float ObjBBOffsetVec[4] = { ObjBBOffset.x, ObjBBOffset.y, ObjBBOffset.z, 1.0f };

								ImGui::Checkbox("Enable Collision", &TestMesh->Col_bEnableCollision);
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("This will enable or disable collision with other objects in the world. When off, this will not interact with anything and vice versa.");
								}

								if (ImGui::DragFloat3("Extents", ObjBBExtVec, 0.065f, 0.1f, 5000.0f, "%.2f"))
								{
									TestMesh->SetBoundingBoxExtents({ ObjBBExtVec[0], ObjBBExtVec[1], ObjBBExtVec[2] });
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("This is size of the collision box for this object.");
								}

								if (ImGui::DragFloat3("Offset", ObjBBOffsetVec, 0.065f, -5000.0f, 5000.0f, "%.2f"))
								{
									TestMesh->SetBoundingBoxOffset({ ObjBBOffsetVec[0], ObjBBOffsetVec[1], ObjBBOffsetVec[2] });
								}
								if (ImGui::IsItemHovered())
								{
									ImGui::SetTooltip("Change where the bounding box attaches to the object.");
								}
							}
						}

						// Do stuff for start with controller.
						ImGui::Separator();

						ImGui::Text("Input");

						bool bInputEnable = CurrObj->Ctrl_bEnableInput;
						if (ImGui::Checkbox("Allow Input", &CurrObj->Ctrl_bEnableInput))
						{
							//CurrObj->SetInputEnabled(&CurrObj->Ctrl_bEnableInput);
						}
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("If checked, this object is allowed to gather input from a controller, if it is possessed with one.");
						}

						bool bStartController = CurrObj->Ctrl_bStartWithController;
						ImGui::Checkbox("Possess on Start", &CurrObj->Ctrl_bStartWithController);
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("If checked, a controller will be created for and assigned to this object on game-start.");
						}

						// Do visibility stuff.
						ImGui::Separator();

						ImGui::Text("Visibility");

						bool bIsVisible = CurrObj->GetVisibility();
						if (ImGui::Checkbox("Visible", &bIsVisible))
						{
							CurrObj->SetVisibility(!CurrObj->GetVisibility());
						}
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("If checked, this object will be rendered in the editor, outside of debug mode, and while playing. If unchecked, this object will not render at all, ever.");
						}

						bool bHiddenInGame = CurrObj->GetHiddenInGame();
						if (ImGui::Checkbox("Hidden in Game", &bHiddenInGame))
						{
							CurrObj->SetHiddenInGame(!CurrObj->GetHiddenInGame());
						}
						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("If checked, this object will be rendered only when in the editor and when the Environment state is DEBUG. If unchecked, this object will render so long as GetVisibility() returns true.");
						}

						ImGui::Separator();
					}
					else
					{
						ImGui::Text("No object selected to inspect.");
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("World"))
				{
					ImGui::PushItemWidth(100.0f);

					float AmbLight = Environment.AmbientLightIntensity;
					if (ImGui::DragFloat("Ambient Light", &AmbLight, 0.005f, 0.0f, 1.0f, "%.2f"))
					{
						Environment.AmbientLightIntensity = AmbLight;
					}

					ImGui::PopItemWidth();
					ImGui::EndTabItem();
				}

				PCamera* RenderCam = Environment.GetRenderCamera();
				if (RenderCam && ImGui::BeginTabItem("Camera"))
				{
					ImGui::PushItemWidth(100.0f);

					float FOV = RenderCam->GetFieldOfView();
					if (ImGui::DragFloat("Field of View", &FOV, 0.01f, 40.0f, 120.0f, "%.2f"))
					{
						RenderCam->SetFieldOfView(FOV);
					}

					ImGui::DragFloat("Camera Speed", &RenderCam->Ctrl_MovementSpeed, 0.05f, 2.0f, 100.0f, "%.2f");
					ImGui::DragFloat("Camera Turn Rate", &RenderCam->Ctrl_RotationRate, 0.05f, 1.0f, 100.0f, "%.2f");

					ImGui::PopItemWidth();
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::SetWindowFontScale(1.0f);

			ImGui::End();
		}

		if (!bGUITool_Inspector)
		{
			bGUITool_Inspector = true;
		}

		ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_Left;
	}

	void PRender::CreateImGuiTool_OutputLog()
	{
		// Get window size.
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);

		if (!bGUITool_OutputLog)
		{
			ImGui::SetNextWindowSize({ ((float)ClientRectangle.right * 0.838f), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.78f)) });
			ImGui::SetNextWindowPos({ ((float)ClientRectangle.right - ((float)ClientRectangle.right * 0.995f)), ((float)ClientRectangle.bottom - ((float)ClientRectangle.bottom * 0.225f)) });
		}

		ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;

		// Create Toolbar window.
		if (ImGui::Begin("Output Log", NULL))
		{
			// Set the font scale in the window.
			ImGui::SetWindowFontScale(1.15f);

			if (ImGui::Button("Clear"))
			{
				Environment.OutputLogData.clear();
			}

			for (unsigned int i = 0; i < Environment.OutputLogData.size(); ++i)
			{
				ImGuiIO& IG_IO = ImGui::GetIO();
				std::string SBuilder = Environment.OutputLogData[i].Source + " >> ";

				switch (Environment.OutputLogData[i].Status)
				{
					case 0:
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 1.0f, 1.0f, 1.0f });
						break;
					}
					case 1:
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 1.0f, 0.0f, 1.0f });
						SBuilder += "SUCCESS:: ";
						break;
					}
					case 2:
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f });
						SBuilder += "FAILURE:: ";
						break;
					}
					case 3:
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 0.86f, 0.93f, 0.14f, 1.0f });
						SBuilder += "WARNING:: ";
						break;
					}
					case 4:
					{
						ImGui::PushStyleColor(ImGuiCol_Text, { 0.16f, 0.93f, 0.94f, 1.0f });
						SBuilder += "SYS:: ";
						break;
					}
				}

				ImGui::Text((SBuilder + Environment.OutputLogData[i].Data).c_str());

				ImGui::PopStyleColor();
			}

			ImGui::SetWindowFontScale(1.0f);

			ImGui::End();
		}

		if (OutputLine != Environment.OutputLogData.size())
		{
			OutputLine = Environment.OutputLogData.size();
			ImGui::SetScrollHere(1.0f);
		}

		if (!bGUITool_OutputLog)
		{
			bGUITool_OutputLog = true;
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_Left;
	}

	void PRender::CreateImGuiTool_EngineIniEditor()
	{
		// Get window size.
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);
		(float)ClientRectangle.right;

		ImGui::SetNextWindowSize({ ((float)ClientRectangle.right * 0.9f), ((float)ClientRectangle.bottom * 0.9f) });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.85f });
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, { 0.0f, 0.0f, 0.0f, 0.85f });

		// Set the font scale in the window.
		ImGui::SetWindowFontScale(1.2f);

		if (!ImGui::IsPopupOpen("Engine Properties"))
		{
			ImGui::OpenPopup("Engine Properties");
		}

		if (ImGui::BeginPopupModal("Engine Properties"))
		{
			// Set the font scale in the window.
			ImGui::SetWindowFontScale(1.2f);

			// Close button.
			if (ImGui::Button("Close Editor", { ((float)ClientRectangle.right * 0.9f) - 15.0f, 30 }))
			{
				std::string IniValue;

				// RENDERER.COLORS
				//
				// Save the new ini settings.
				IniValue = std::to_string(GUI_Color_SeletedObjectHighlight.x) + " " + std::to_string(GUI_Color_SeletedObjectHighlight.y) + " " + std::to_string(GUI_Color_SeletedObjectHighlight.z) + " " + std::to_string(GUI_Color_SeletedObjectHighlight.w);
				WritePrivateProfileString("Renderer.Colors", "ObjectHighlight", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_DebugGrid.x) + " " + std::to_string(GUI_Color_DebugGrid.y) + " " + std::to_string(GUI_Color_DebugGrid.z) + " " + std::to_string(GUI_Color_DebugGrid.w);
				WritePrivateProfileString("Renderer.Colors", "DebugGridColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				// EDITOR.COLORS
				//
				IniValue = std::to_string(GUI_Color_TextColor.x) + " " + std::to_string(GUI_Color_TextColor.y) + " " + std::to_string(GUI_Color_TextColor.z) + " " + std::to_string(GUI_Color_TextColor.w);
				WritePrivateProfileString("Editor.Colors", "TextColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
				
				IniValue = std::to_string(GUI_Color_TitleBg.x) + " " + std::to_string(GUI_Color_TitleBg.y) + " " + std::to_string(GUI_Color_TitleBg.z) + " " + std::to_string(GUI_Color_TitleBg.w);
				WritePrivateProfileString("Editor.Colors", "TitleBackgroundColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_TitleActiveBg.x) + " " + std::to_string(GUI_Color_TitleActiveBg.y) + " " + std::to_string(GUI_Color_TitleActiveBg.z) + " " + std::to_string(GUI_Color_TitleActiveBg.w);
				WritePrivateProfileString("Editor.Colors", "ActiveTitleBackgroundColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_WindowBg.x) + " " + std::to_string(GUI_Color_WindowBg.y) + " " + std::to_string(GUI_Color_WindowBg.z) + " " + std::to_string(GUI_Color_WindowBg.w);
				WritePrivateProfileString("Editor.Colors", "WindowBackgroundColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_TitleBorderClr.x) + " " + std::to_string(GUI_Color_TitleBorderClr.y) + " " + std::to_string(GUI_Color_TitleBorderClr.z) + " " + std::to_string(GUI_Color_TitleBorderClr.w);
				WritePrivateProfileString("Editor.Colors", "WindowBorderColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				// HIERARCHY.COLORS
				//
				IniValue = std::to_string(GUI_Color_HierarchyButtonText.x) + " " + std::to_string(GUI_Color_HierarchyButtonText.y) + " " + std::to_string(GUI_Color_HierarchyButtonText.z) + " " + std::to_string(GUI_Color_HierarchyButtonText.w);
				WritePrivateProfileString("Hierarchy.Colors", "HiTextColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_HierarchyButtonTextSelect.x) + " " + std::to_string(GUI_Color_HierarchyButtonTextSelect.y) + " " + std::to_string(GUI_Color_HierarchyButtonTextSelect.z) + " " + std::to_string(GUI_Color_HierarchyButtonTextSelect.w);
				WritePrivateProfileString("Hierarchy.Colors", "HiSelectedTextColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_HierarchyButton.x) + " " + std::to_string(GUI_Color_HierarchyButton.y) + " " + std::to_string(GUI_Color_HierarchyButton.z) + " " + std::to_string(GUI_Color_HierarchyButton.w);
				WritePrivateProfileString("Hierarchy.Colors", "HiButtonColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_HierarchyButtonHover.x) + " " + std::to_string(GUI_Color_HierarchyButtonHover.y) + " " + std::to_string(GUI_Color_HierarchyButtonHover.z) + " " + std::to_string(GUI_Color_HierarchyButtonHover.w);
				WritePrivateProfileString("Hierarchy.Colors", "HiButtonHoverColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_HierarchyButtonSelected.x) + " " + std::to_string(GUI_Color_HierarchyButtonSelected.y) + " " + std::to_string(GUI_Color_HierarchyButtonSelected.z) + " " + std::to_string(GUI_Color_HierarchyButtonSelected.w);
				WritePrivateProfileString("Hierarchy.Colors", "HiSelectedButtonColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				IniValue = std::to_string(GUI_Color_HierarchyButtonActiveHover.x) + " " + std::to_string(GUI_Color_HierarchyButtonActiveHover.y) + " " + std::to_string(GUI_Color_HierarchyButtonActiveHover.z) + " " + std::to_string(GUI_Color_HierarchyButtonActiveHover.w);
				WritePrivateProfileString("Hierarchy.Colors", "HiButtonHoverSelectedColor", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());


				// EDITOR.PREFERENCES
				//
				IniValue = (GUI_Pref_UnselectOnHover == true ? "true" : "false");
				WritePrivateProfileString("Editor.Preferences", "UnselectOnLoseFocus", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				// EDITOR.STARTUP
				//
				// What map should open when the engine opens?
				IniValue = GUI_Pref_StartupLevel;
				WritePrivateProfileString("Editor.Startup", "StartupMap", IniValue.c_str(), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());

				// Set Engine.ini as hidden.
				bGUITool_EngineIniEditor = false;
			}
			ImGui::Separator();

			if (ImGui::CollapsingHeader("Preferences"))
			{
				ImGui::Checkbox("Unselect on Focus Lost", &GUI_Pref_UnselectOnHover);
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("When enabled, clicking within the world viewport (where the game is renderer) will unselect the object that is currently selected in the environment.");
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Scalability"))
			{
				// Select MSAA level.
				const char* CreateMSAAQ[] = { "Off", "2", "4", "8" };
				static const char* CurrMSAAQ = "8";
				ImGui::PushItemWidth(100);
				if (ImGui::BeginCombo("MSAA Quality", CurrMSAAQ))
				{
					for (unsigned int i = 0; i < IM_ARRAYSIZE(CreateMSAAQ); ++i)
					{
						bool bSelected = (CurrMSAAQ == CreateMSAAQ[i]);
						if (ImGui::Selectable(CreateMSAAQ[i], bSelected))
						{
							CurrMSAAQ = CreateMSAAQ[i];
						}
					}

					ImGui::EndCombo();
					ImGui::PopItemWidth();
				}
			}
			ImGui::Separator();

			if (ImGui::CollapsingHeader("Startup Settings"))
			{
				std::string TempStr = "Startup Level: " + GUI_Pref_StartupLevel;
				if (ImGui::Button(TempStr.c_str()))
				{
					std::string FilePath = CreateWindow_PickLevelDialog();

					if (FilePath != "")
					{
						GUI_Pref_StartupLevel = FilePath;
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("This is the level that will be loaded when the engine starts up.");
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Editor Customization"))
			{
				// Set the font scale in the window.
				ImGui::SetWindowFontScale(1.2f);

				float4 HighlightClr= GUI_Color_SeletedObjectHighlight;
				float4 DebugGridClr = GUI_Color_DebugGrid;
				float4 TextClr = GUI_Color_TextColor;
				float4 TitleBgClr = GUI_Color_TitleBg;
				float4 TitleActiveBgClr = GUI_Color_TitleActiveBg;
				float4 WindowBgClr = GUI_Color_WindowBg;
				float4 WindowBorderClr = GUI_Color_TitleBorderClr;
				float4 Hi_ButtonTextClr = GUI_Color_HierarchyButtonText;
				float4 Hi_ButtonTextSelectedClr = GUI_Color_HierarchyButtonTextSelect;
				float4 Hi_ButtonClr = GUI_Color_HierarchyButton;
				float4 Hi_ButtonHoverClr = GUI_Color_HierarchyButtonHover;
				float4 Hi_ButtonSelectedClr = GUI_Color_HierarchyButtonSelected;
				float4 Hi_ButtonHoverSelectedClr = GUI_Color_HierarchyButtonActiveHover;
				float HighlightClrVec[4] = { HighlightClr.x, HighlightClr.y, HighlightClr.z, HighlightClr.w };
				float DebugGridClrVec[4] = { DebugGridClr.x, DebugGridClr.y, DebugGridClr.z, DebugGridClr.w };
				float TextClrVec[4] = { TextClr.x, TextClr.y, TextClr.z, TextClr.w };
				float TitleBgClrVec[4] = { TitleBgClr.x, TitleBgClr.y, TitleBgClr.z, TitleBgClr.w };
				float TitleActiveBgClrVec[4] = { TitleActiveBgClr.x, TitleActiveBgClr.y, TitleActiveBgClr.z, TitleActiveBgClr.w };
				float WindowBgClrVec[4] = { WindowBgClr.x, WindowBgClr.y, WindowBgClr.z, WindowBgClr.w };
				float WindowBorderClrVec[4] = { WindowBorderClr.x, WindowBorderClr.y, WindowBorderClr.z, WindowBorderClr.w };
				float Hi_ButtonClrVec[4] = { Hi_ButtonClr.x, Hi_ButtonClr.y, Hi_ButtonClr.z, Hi_ButtonClr.w };
				float Hi_ButtonHoverClrVec[4] = { Hi_ButtonHoverClr.x, Hi_ButtonHoverClr.y, Hi_ButtonHoverClr.z, Hi_ButtonHoverClr.w };
				float Hi_ButtonSelectedClrVec[4] = { Hi_ButtonSelectedClr.x, Hi_ButtonSelectedClr.y, Hi_ButtonSelectedClr.z, Hi_ButtonSelectedClr.w };
				float Hi_ButtonHoverSelectedClrVec[4] = { Hi_ButtonHoverSelectedClr.x, Hi_ButtonHoverSelectedClr.y, Hi_ButtonHoverSelectedClr.z, Hi_ButtonHoverSelectedClr.w };
				float Hi_ButtonTextClrVec[4] = { Hi_ButtonTextClr.x, Hi_ButtonTextClr.y, Hi_ButtonTextClr.z, Hi_ButtonTextClr.w };
				float Hi_ButtonTextSelectedClrVec[4] = { Hi_ButtonTextSelectedClr.x, Hi_ButtonTextSelectedClr.y, Hi_ButtonTextSelectedClr.z, Hi_ButtonTextSelectedClr.w };

				ImGui::Indent(25.0f);

				if (ImGui::CollapsingHeader("Renderer Colors"))
				{
					ImGui::PushItemWidth(1000.0f);

					if (ImGui::ColorEdit4("Object Highlight", HighlightClrVec))
					{
						GUI_Color_SeletedObjectHighlight = { HighlightClrVec[0], HighlightClrVec[1], HighlightClrVec[2], HighlightClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color the object you select in the editor will be highlighted. It takes a float4 consisting of RGB + Intensity. The intensity will alter the strength of the highlight effect on the object.");
					}

					if (ImGui::ColorEdit4("Debug Grid Color", DebugGridClrVec))
					{
						GUI_Color_DebugGrid = { DebugGridClrVec[0], DebugGridClrVec[1], DebugGridClrVec[2], DebugGridClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of the grid draw at the world 0 on the Y-axis when enabled through the renderer options.");
					}

					ImGui::PopItemWidth();
				}

				if (ImGui::CollapsingHeader("Editor Colors"))
				{
					ImGui::PushItemWidth(1000.0f);

					if (ImGui::ColorEdit4("Text Color", TextClrVec))
					{
						GUI_Color_TextColor = { TextClrVec[0], TextClrVec[1], TextClrVec[2], TextClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of editor generic text. This resembles most button text, window text, and other text.");
					}

					if (ImGui::ColorEdit4("Title Background Color", TitleBgClrVec))
					{
						GUI_Color_TitleBg = { TitleBgClrVec[0], TitleBgClrVec[1], TitleBgClrVec[2], TitleBgClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of the backgrounds for window titles. The titles are where the collapse button and window name exist.");
					}

					if (ImGui::ColorEdit4("Active Title Background Color", TitleActiveBgClrVec))
					{
						GUI_Color_TitleActiveBg = { TitleActiveBgClrVec[0], TitleActiveBgClrVec[1], TitleActiveBgClrVec[2], TitleActiveBgClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of the backgrounds for window titles when they are selected and active. The titles are where the collapse button and window name exist.");
					}

					if (ImGui::ColorEdit4("Window Background Color", WindowBgClrVec))
					{
						GUI_Color_WindowBg = { WindowBgClrVec[0], WindowBgClrVec[1], WindowBgClrVec[2], WindowBgClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of the backgrounds of windows. This is normally where buttons and text reside inside of.");
					}

					if (ImGui::ColorEdit4("Window Border Color", WindowBorderClrVec))
					{
						GUI_Color_TitleBorderClr = { WindowBorderClrVec[0], WindowBorderClrVec[1], WindowBorderClrVec[2], WindowBorderClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of the border that surrounds each window.");
					}

					ImGui::PopItemWidth();
				}

				if (ImGui::CollapsingHeader("World Hierarchy Colors"))
				{
					ImGui::PushItemWidth(1000.0f);

					if (ImGui::ColorEdit4("Hierarchy Text Color", Hi_ButtonTextClrVec))
					{
						GUI_Color_HierarchyButtonText = { Hi_ButtonTextClrVec[0], Hi_ButtonTextClrVec[1], Hi_ButtonTextClrVec[2], Hi_ButtonTextClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of the text within an idle World Hierarchy button.");
					}

					if (ImGui::ColorEdit4("Hierarchy Selected Text Color", Hi_ButtonTextSelectedClrVec))
					{
						GUI_Color_HierarchyButtonTextSelect = { Hi_ButtonTextSelectedClrVec[0], Hi_ButtonTextSelectedClrVec[1], Hi_ButtonTextSelectedClrVec[2], Hi_ButtonTextSelectedClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of the text within a selected World Hierarchy button.");
					}

					if (ImGui::ColorEdit4("Hierarchy Button Color", Hi_ButtonClrVec))
					{
						GUI_Color_HierarchyButton = { Hi_ButtonClrVec[0], Hi_ButtonClrVec[1], Hi_ButtonClrVec[2], Hi_ButtonClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of a button when idle in the World Hierarchy viewer.");
					}

					if (ImGui::ColorEdit4("Hierarchy Hovered Button Color", Hi_ButtonHoverClrVec))
					{
						GUI_Color_HierarchyButtonHover = { Hi_ButtonHoverClrVec[0], Hi_ButtonHoverClrVec[1], Hi_ButtonHoverClrVec[2], Hi_ButtonHoverClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of a button when idle and hovered in the World Hierarchy viewer.");
					}

					if (ImGui::ColorEdit4("Hierarchy Selected Hovered Button Color", Hi_ButtonHoverSelectedClrVec))
					{
						GUI_Color_HierarchyButtonActiveHover = { Hi_ButtonHoverSelectedClrVec[0], Hi_ButtonHoverSelectedClrVec[1], Hi_ButtonHoverSelectedClrVec[2], Hi_ButtonHoverSelectedClrVec[3] };
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("This is the color of a button when the button's object is selected in the World Hierarchy viewer and the button is highlighted.");
					}

					ImGui::PopItemWidth();
				}

				ImGui::Unindent(25.0f);
			}
			ImGui::Separator();

			if (ImGui::CollapsingHeader("Render Features"))
			{

			}
			ImGui::Separator();

			ImGui::EndPopup();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		// Set the font scale in the window.
		ImGui::SetWindowFontScale(1.0f);
	}

	void PRender::CreateImGuiTool_GameIniEditor()
	{
		// Get window size.
		RECT ClientRectangle;
		GetClientRect(hwnd, &ClientRectangle);
		(float)ClientRectangle.right;

		ImGui::SetNextWindowSize({ ((float)ClientRectangle.right * 0.9f), ((float)ClientRectangle.bottom * 0.9f) });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.85f });
		ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, { 0.0f, 0.0f, 0.0f, 0.85f });

		// Set the font scale in the window.
		ImGui::SetWindowFontScale(1.2f);

		if (!ImGui::IsPopupOpen("Game Properties"))
		{
			ImGui::OpenPopup("Game Properties");
		}

		if (ImGui::BeginPopupModal("Game Properties"))
		{
			// Set the font scale in the window.
			ImGui::SetWindowFontScale(1.2f);

			// Close button.
			if (ImGui::Button("Close Editor", { ((float)ClientRectangle.right * 0.9f) - 15.0f, 30 }))
			{
				bGUITool_GameIniEditor = false;
			}

			// Project name.
			if (ImGui::CollapsingHeader("About my Project"))
			{
				ImGui::Indent(25.0f);

				if (ImGui::CollapsingHeader("Game Details"))
				{
					// Set the font scale in the window.
					ImGui::SetWindowFontScale(1.2f);

					// Project name.
					ImGui::InputTextWithHint("Project Name", "", Test_ProjName, sizeof(Test_ProjName));
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Project Name is the name of your project. A \"Project\" is the levels, objects, 3D models, controllers, code, and everything else within your game. No more than 200 characters.");
					}

					// Project description.
					ImGui::InputTextWithHint("Project Description", "", Test_ProjDesc, sizeof(Test_ProjDesc));
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Project Description is the breakdown of your game, and should be short, sweet, and to the point. No more than 1000 characters.");
					}

					// Project version.
					ImGui::InputTextWithHint("Project Version", "", Test_ProjVersion, sizeof(Test_ProjVersion));
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Project Version is the build the game is currently running on. This is used primarily for ensuring every team member is running on the same build. No more than 10 characters.");
					}
				}

				ImGui::Unindent(25.0f);
			}

			ImGui::EndPopup();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}

		// Set the font scale in the window.
		ImGui::SetWindowFontScale(1.0f);
	}

	void PRender::D3DCreateConstantBuffer()
	{
		D3D11_BUFFER_DESC MVPBufferDesc;
		ZeroMemory(&MVPBufferDesc, sizeof(MVPBufferDesc));

		MVPBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		MVPBufferDesc.ByteWidth = sizeof(MVP_t);
		MVPBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		MVPBufferDesc.CPUAccessFlags = 0;

		HRESULT hr = Device->CreateBuffer(&MVPBufferDesc, NULL, &ConstantBuffer);
		assert(!FAILED(hr));

		PrintToConsole("Created Constant Buffer.");

		D3D11_BUFFER_DESC d3dVertexBuffer;
		ZeroMemory(&d3dVertexBuffer, sizeof(d3dVertexBuffer));

		d3dVertexBuffer.Usage = D3D11_USAGE_DEFAULT;
		d3dVertexBuffer.ByteWidth = (sizeof(PMath::colored_vertex) * (DebugLines::GetLineVertCapacity() + 22));
		d3dVertexBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		hr = Device->CreateBuffer(&d3dVertexBuffer, NULL, &VertexBuffer);
		assert(!FAILED(hr));

		PrintToConsole("Created Vertex Buffer.");
	}

	void PRender::SetFullscreenMode(bool bEnable)
	{
		SwapChain->SetFullscreenState(bEnable, nullptr);
	}

	bool PRender::GetFullscreenMode()
	{
		BOOL IsInFullScreen;
		SwapChain->GetFullscreenState(&IsInFullScreen, nullptr);
		return IsInFullScreen;
	}

	int PRender::CreateWindow_OpenLevel()
	{
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "PLevel files (*.plevel)\0*.plevel";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick a Level to Open";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&OFN))
		{
			PrintToConsole("Creating open level dialog.");

			// Get the level name from the selected file.
			//std::vector<std::string> Chunks = SplitString(Filename, '\\');
			std::string LevelName = PGameplayStatics::SliceFilepathByDir(Filename, "PolynGame");

			// Tell the environment to load the level that was selected.
			//Environment.LoadLevel(Filename);
			Environment.LoadLevel(PGameplayStatics::GetGameDirectory() + LevelName);

			SetWindowTextA(hwnd, ("Polyn v0.5 - " + Environment.CurrentLevel).c_str());
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
				case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
				case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
				case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
				case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
				case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
				case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
				case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
				case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
				case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
				case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
				case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
				case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
				case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
				case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
				case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
				default:					PrintToConsole("You cancelled the Open File operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_SaveLevel()
	{
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "PLevel files (*.plevel)\0*.plevel";
		OFN.lpstrDefExt = ".plevel";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick Where to Save the Level";
		OFN.Flags = OFN_DONTADDTORECENT;

		if (GetSaveFileNameA(&OFN))
		{
			PrintToConsole("Creating save level as dialog.");

			// Get the level name from the selected file.
			std::vector<std::string> Chunks = PGameplayStatics::SplitString(Filename, '\\');
			std::string LevelName = PGameplayStatics::SplitString(Chunks[Chunks.size() - 1], '.')[0];

			// Tell the environment to load the level that was selected.
			Environment.SaveLevel(Filename);

			Environment.LoadLevel(Filename);

			SetWindowTextA(hwnd, ("Polyn v0.5 - " + Environment.CurrentLevel).c_str());
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
				case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
				case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
				case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
				case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
				case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
				case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
				case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
				case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
				case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
				case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
				case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
				case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
				case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
				case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
				case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
				default:					PrintToConsole("You cancelled the Save File operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_SaveCustomClass(EClasses Type)
	{
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "Header files (*.h)\0*.h";
		OFN.lpstrDefExt = ".h";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick Where to Save the Class";
		OFN.Flags = OFN_DONTADDTORECENT;

		if (GetSaveFileNameA(&OFN))
		{
			PrintToConsole("Creating custom class dialog.");

			// Get the level name from the selected file.
			std::vector<std::string> Chunks = PGameplayStatics::SplitString(Filename, '\\');
			std::string LevelName = PGameplayStatics::SplitString(Chunks[Chunks.size() - 1], '.')[0];

			// Tell the environment to load the level that was selected.
			Environment.CreateCustomClass(Filename, Type);
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Save File operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_OpenTexture(ETextureTypes Type)
	{
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "Direct Draw Surface files (*.dds)\0*.dds";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick a Texture";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&OFN))
		{
			std::string FilenameStr = Filename;
			PrintToConsole("Loading texture: " + FilenameStr);

			// Get the level name from the selected file.
			std::string TextureName = PGameplayStatics::SliceFilepathByDir(FilenameStr, "Assets");

			PStaticMesh* CurrMesh = dynamic_cast<PStaticMesh*>(Environment.SelectedObject);
			PSkeletalMesh* CurrSkeleton = dynamic_cast<PSkeletalMesh*>(Environment.SelectedObject);

			bool LoadStat = false;

			PrintToConsole("Applying new Texture to selected object in the world: " + TextureName);

			if (CurrSkeleton)
			{
				LoadStat = CurrSkeleton->LoadTexture(TextureName.c_str(), Device, Type);
			}

			if (CurrMesh && !CurrSkeleton)
			{
				LoadStat = CurrMesh->LoadTexture(TextureName.c_str(), Device, Type);
			}
			else
			{
				PrintToConsole(("The object cannot have a Texture applied: " + TextureName), 3);
			}

			// Show error in output log for failure to load the mesh.
			if (!LoadStat)
			{
				PrintToConsole(("The texture could not be opened and applied to the object: " + TextureName), 2);
			}
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Open Texture operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_OpenModelFile()
	{
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "3D Object files (*.obj)\0*.obj";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick a Model";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&OFN))
		{
			std::string FilenameStr = Filename;
			PrintToConsole("Loading model: " + FilenameStr);

			// Get the level name from the selected file.
			//std::vector<std::string> Chunks = SplitString(Filename, '\\');
			//std::string ModelName = Chunks[Chunks.size() - 1];
			std::string ModelName = PGameplayStatics::SliceFilepathByDir(FilenameStr, "Assets");

			PStaticMesh* CurrMesh = dynamic_cast<PStaticMesh*>(Environment.SelectedObject);

			if (CurrMesh)
			{
				PrintToConsole("Applying new Model in the world: " + ModelName);
				bool LoadStat = CurrMesh->LoadMesh(ModelName.c_str(), Device, Context, true);

				// Show error in output log for failure to load the mesh.
				if (!LoadStat)
				{
					PrintToConsole(("The model could not be opened and applied to the object: " + ModelName), 2);
				}
			}
			else
			{
				PrintToConsole(("The object cannot have a Model applied: " + ModelName), 3);
			}
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Open Model operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_OpenMeshFile()
	{
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "Mesh files (*.mesh)\0*.mesh";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick a Mesh";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&OFN))
		{
			std::string FilenameStr = Filename;
			PrintToConsole("Loading mesh: " + FilenameStr);

			// Get the level name from the selected file.
			//std::vector<std::string> Chunks = SplitString(Filename, '\\');
			//std::string ModelName = Chunks[Chunks.size() - 1];
			std::string ModelName = PGameplayStatics::SliceFilepathByDir(FilenameStr, "Assets");

			PSkeletalMesh* CurrMesh = dynamic_cast<PSkeletalMesh*>(Environment.SelectedObject);

			if (CurrMesh)
			{
				PrintToConsole("Applying new Mesh in the world: " + ModelName);
				bool LoadStat = CurrMesh->LoadMesh(ModelName.c_str(), Device, Context, true);

				// Show error in output log for failure to load the mesh.
				if (!LoadStat)
				{
					PrintToConsole(("The mesh could not be opened and applied to the object: " + ModelName), 2);
				}
			}
			else
			{
				PrintToConsole(("The object cannot have a Mesh applied: " + ModelName), 3);
			}
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Open Model operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_OpenAnimFile()
	{
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "Animation files (*.anim)\0*.anim";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick an Animation";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		if (GetOpenFileNameA(&OFN))
		{
			std::string FilenameStr = Filename;
			PrintToConsole("Loading animation: " + FilenameStr);

			// Get the level name from the selected file.
			//std::vector<std::string> Chunks = SplitString(Filename, '\\');
			//std::string ModelName = Chunks[Chunks.size() - 1];
			std::string ModelName = PGameplayStatics::SliceFilepathByDir(FilenameStr, "Assets");

			PSkeletalMesh* CurrMesh = dynamic_cast<PSkeletalMesh*>(Environment.SelectedObject);

			if (CurrMesh)
			{
				PrintToConsole("Applying new Animation in the world: " + ModelName);
				bool LoadStat = CurrMesh->PlayAnimation(ModelName.c_str());

				// Show error in output log for failure to load the mesh.
				if (!LoadStat)
				{
					PrintToConsole(("The animation could not be opened and applied to the mesh: " + ModelName), 2);
				}
			}
			else
			{
				PrintToConsole(("No mesh to apply animation to: " + ModelName), 3);
			}
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Open Model operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_ImportFBXFile()
	{
		std::string FilenameStr;		// This will hold the 3D Mesh filepath to import from.
		std::string ModelName;			// This will hold the name of the 3D mesh without a filepath preceeding it.
		std::string DestinationStr;		// This will hold the destination of the new .MESH file.

		// File structure for the importing mesh.
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "Skeletal Mesh files (*.fbx)\0*.fbx";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick an FBX Model to Import";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		// Get the Mesh to import.
		if (GetOpenFileNameA(&OFN))
		{
			std::string FilenameStr = Filename;
			PrintToConsole("Finding mesh: " + FilenameStr);

			// Get the level name from the selected file.
			std::vector<std::string> SplitVals = PGameplayStatics::SplitString(FilenameStr, '\\');
			ModelName = SplitVals[SplitVals.size() - 1];

			// Pick destination folder/directory.
			if (PGameplayStatics::SplitString(ModelName, '.')[1] == "fbx" || PGameplayStatics::SplitString(ModelName, '.')[1] == "FBX")
			{
				char MeshFilename[500];
				OPENFILENAME OFN;
				ZeroMemory(&MeshFilename, sizeof(MeshFilename));
				ZeroMemory(&OFN, sizeof(OFN));
				OFN.lStructSize = sizeof(OFN);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = "Mesh files (*.mesh)\0*.mesh";
				OFN.lpstrDefExt = ".mesh";
				OFN.lpstrFile = MeshFilename;
				OFN.nMaxFile = 500;
				OFN.lpstrTitle = "Pick where to Save the Mesh";
				OFN.Flags = OFN_DONTADDTORECENT;

				if (GetSaveFileNameA(&OFN))
				{
					PrintToConsole("Importing \".FBX\" file: \"" + FilenameStr + "\" as: \"" + MeshFilename + "\"");

					FBXExporter MeshExport = FBXExporter(FBXExporter::CONSTYPE::MESH, FilenameStr.c_str(), MeshFilename);
				}
				else
				{
					// Check for errors.
					switch (CommDlgExtendedError())
					{
					case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
					case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
					case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
					case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
					case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
					case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
					case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
					case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
					case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
					case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
					case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
					case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
					case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
					case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
					case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
					default:					PrintToConsole("You cancelled the Save File operation.", 3); break;
					}
				}
			}
			else
			{
				PrintToConsole("It appears this file was not a \".FBX\" file type. Try picking an \"FBX\" Skeletal Mesh.", 2);
			}
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Open Model operation.", 3); break;
			}
		}

		return 0;
	}

	int PRender::CreateWindow_ImportFBXAnimation()
	{
		std::string FilenameStr;		// This will hold the 3D Mesh filepath to import from.
		std::string ModelName;			// This will hold the name of the 3D mesh without a filepath preceeding it.
		std::string DestinationStr;		// This will hold the destination of the new .MESH file.

		// File structure for the importing mesh.
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "Skeletal Mesh files (*.fbx)\0*.fbx";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick an FBX Animation to Import";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		// Get the Mesh to import.
		if (GetOpenFileNameA(&OFN))
		{
			std::string FilenameStr = Filename;
			PrintToConsole("Finding animation: " + FilenameStr);

			// Get the level name from the selected file.
			std::vector<std::string> SplitVals = PGameplayStatics::SplitString(FilenameStr, '\\');
			ModelName = SplitVals[SplitVals.size() - 1];

			// Pick destination folder/directory.
			if (PGameplayStatics::SplitString(ModelName, '.')[1] == "fbx" || PGameplayStatics::SplitString(ModelName, '.')[1] == "FBX")
			{
				char MeshFilename[500];
				OPENFILENAME OFN;
				ZeroMemory(&MeshFilename, sizeof(MeshFilename));
				ZeroMemory(&OFN, sizeof(OFN));
				OFN.lStructSize = sizeof(OFN);
				OFN.hwndOwner = hwnd;
				OFN.lpstrFilter = "Animation files (*.anim)\0*.anim";
				OFN.lpstrDefExt = ".anim";
				OFN.lpstrFile = MeshFilename;
				OFN.nMaxFile = 500;
				OFN.lpstrTitle = "Pick where to Save the Animation";
				OFN.Flags = OFN_DONTADDTORECENT;

				if (GetSaveFileNameA(&OFN))
				{
					PrintToConsole("Importing \".FBX\" file: \"" + FilenameStr + "\" as: \"" + MeshFilename + "\"");

					FBXExporter MeshExport = FBXExporter(FBXExporter::CONSTYPE::ANIM, FilenameStr.c_str(), MeshFilename);
				}
				else
				{
					// Check for errors.
					switch (CommDlgExtendedError())
					{
					case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
					case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
					case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
					case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
					case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
					case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
					case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
					case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
					case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
					case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
					case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
					case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
					case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
					case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
					case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
					default:					PrintToConsole("You cancelled the Save File operation.", 3); break;
					}
				}
			}
			else
			{
				PrintToConsole("It appears this file was not a \".FBX\" file type. Try picking an \"FBX\" Skeletal Mesh.", 2);
			}
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Open Model operation.", 3); break;
			}
		}

		return 0;
	}

	std::string PRender::CreateWindow_PickLevelDialog()
	{
		std::string FilenameStr;		// This will hold the 3D Mesh filepath to import from.
		std::string ModelName;			// This will hold the name of the 3D mesh without a filepath preceeding it.
		std::string DestinationStr;		// This will hold the destination of the new .MESH file.

		// File structure for the importing mesh.
		char Filename[500];
		OPENFILENAME OFN;
		ZeroMemory(&Filename, sizeof(Filename));
		ZeroMemory(&OFN, sizeof(OFN));
		OFN.lStructSize = sizeof(OFN);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = "Polyn Level (*.plevel)\0*.plevel";
		OFN.lpstrFile = Filename;
		OFN.nMaxFile = 500;
		OFN.lpstrTitle = "Pick a Level to Open";
		OFN.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

		// Get the Mesh to import.
		if (GetOpenFileNameA(&OFN))
		{
			// Get the level name from the selected file.
			std::vector<std::string> SplitVals = PGameplayStatics::SplitString(Filename, '\\');
			std::string FilenameStr = SplitVals[SplitVals.size() - 1];

			// Pick destination folder/directory.
			if (PGameplayStatics::SplitString(FilenameStr, '.')[1] == "plevel" || PGameplayStatics::SplitString(FilenameStr, '.')[1] == "PLEVEL")
			{
				return (PGameplayStatics::GetGameDirectory() + FilenameStr);
			}
			else
			{
				PrintToConsole("It appears this file was not a \".PLEVEL\" file type. Try picking a \"PLEVEL\" Level.", 2);
			}
		}
		else
		{
			// Check for errors.
			switch (CommDlgExtendedError())
			{
			case CDERR_DIALOGFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_FINDRESFAILURE:	PrintToConsole("CDERR_FINDRESFAILURE", 2);  break;
			case CDERR_INITIALIZATION:	PrintToConsole("CDERR_INITIALIZATION", 2);  break;
			case CDERR_LOADRESFAILURE:	PrintToConsole("CDERR_LOADRESFAILURE", 2);  break;
			case CDERR_LOADSTRFAILURE:	PrintToConsole("CDERR_LOADSTRFAILURE", 2);  break;
			case CDERR_LOCKRESFAILURE:	PrintToConsole("CDERR_LOCKRESFAILURE", 2);  break;
			case CDERR_MEMALLOCFAILURE: PrintToConsole("CDERR_MEMALLOCFAILURE", 2); break;
			case CDERR_MEMLOCKFAILURE:	PrintToConsole("CDERR_MEMLOCKFAILURE", 2);  break;
			case CDERR_NOHINSTANCE:		PrintToConsole("CDERR_NOHINSTANCE", 2);     break;
			case CDERR_NOHOOK:			PrintToConsole("CDERR_NOHOOK", 2);          break;
			case CDERR_NOTEMPLATE:		PrintToConsole("CDERR_NOTEMPLATE", 2);      break;
			case CDERR_STRUCTSIZE:		PrintToConsole("CDERR_STRUCTSIZE", 2);      break;
			case FNERR_BUFFERTOOSMALL:	PrintToConsole("FNERR_BUFFERTOOSMALL", 2);  break;
			case FNERR_INVALIDFILENAME: PrintToConsole("FNERR_INVALIDFILENAME", 2); break;
			case FNERR_SUBCLASSFAILURE: PrintToConsole("FNERR_SUBCLASSFAILURE", 2); break;
			default:					PrintToConsole("You cancelled the Open Model operation.", 3); break;
			}
		}

		return "";
	}

	bool PRender::LoadEngineIni()
	{
		char IniBuffer[150];
		std::vector<std::string> IniList;
		std::string DefaultValue;

		// RENDERER.COLORS
		//
		// Selected object highlight color.
		DefaultValue = (std::to_string(GUI_Color_SeletedObjectHighlight.x) + " " + std::to_string(GUI_Color_SeletedObjectHighlight.y) + " " + std::to_string(GUI_Color_SeletedObjectHighlight.z) + " " + std::to_string(GUI_Color_SeletedObjectHighlight.w));
		GetPrivateProfileString("Renderer.Colors", "ObjectHighlight", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_SeletedObjectHighlight = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Debug grid color.
		DefaultValue = (std::to_string(GUI_Color_DebugGrid.x) + " " + std::to_string(GUI_Color_DebugGrid.y) + " " + std::to_string(GUI_Color_DebugGrid.z) + " " + std::to_string(GUI_Color_DebugGrid.w));
		GetPrivateProfileString("Renderer.Colors", "DebugGridColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_DebugGrid = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// EDITOR.COLORS
		//
		// Normal text color.
		DefaultValue = (std::to_string(GUI_Color_TextColor.x) + " " + std::to_string(GUI_Color_TextColor.y) + " " + std::to_string(GUI_Color_TextColor.z) + " " + std::to_string(GUI_Color_TextColor.w));
		GetPrivateProfileString("Editor.Colors", "TextColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_TextColor = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };
		
		// Title background color.
		DefaultValue = (std::to_string(GUI_Color_TitleBg.x) + " " + std::to_string(GUI_Color_TitleBg.y) + " " + std::to_string(GUI_Color_TitleBg.z) + " " + std::to_string(GUI_Color_TitleBg.w));
		GetPrivateProfileString("Editor.Colors", "TitleBackgroundColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_TitleBg = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Active title background color.
		DefaultValue = (std::to_string(GUI_Color_TitleActiveBg.x) + " " + std::to_string(GUI_Color_TitleActiveBg.y) + " " + std::to_string(GUI_Color_TitleActiveBg.z) + " " + std::to_string(GUI_Color_TitleActiveBg.w));
		GetPrivateProfileString("Editor.Colors", "ActiveTitleBackgroundColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_TitleActiveBg = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Window background color.
		DefaultValue = (std::to_string(GUI_Color_WindowBg.x) + " " + std::to_string(GUI_Color_WindowBg.y) + " " + std::to_string(GUI_Color_WindowBg.z) + " " + std::to_string(GUI_Color_WindowBg.w));
		GetPrivateProfileString("Editor.Colors", "WindowBackgroundColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_WindowBg = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };
		
		// Window border color.
		DefaultValue = (std::to_string(GUI_Color_TitleBorderClr.x) + " " + std::to_string(GUI_Color_TitleBorderClr.y) + " " + std::to_string(GUI_Color_TitleBorderClr.z) + " " + std::to_string(GUI_Color_TitleBorderClr.w));
		GetPrivateProfileString("Editor.Colors", "WindowBorderColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_TitleBorderClr = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// HIERARCHY.COLORS
		//
		// Hierarchy idle text color.
		DefaultValue = (std::to_string(GUI_Color_HierarchyButtonText.x) + " " + std::to_string(GUI_Color_HierarchyButtonText.y) + " " + std::to_string(GUI_Color_HierarchyButtonText.z) + " " + std::to_string(GUI_Color_HierarchyButtonText.w));
		GetPrivateProfileString("Hierarchy.Colors", "HiTextColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_HierarchyButtonText = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Hierarchy selected text color.
		DefaultValue = (std::to_string(GUI_Color_HierarchyButtonTextSelect.x) + " " + std::to_string(GUI_Color_HierarchyButtonTextSelect.y) + " " + std::to_string(GUI_Color_HierarchyButtonTextSelect.z) + " " + std::to_string(GUI_Color_HierarchyButtonTextSelect.w));
		GetPrivateProfileString("Hierarchy.Colors", "HiSelectedTextColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_HierarchyButtonTextSelect = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Hierarchy idle button color.
		DefaultValue = (std::to_string(GUI_Color_HierarchyButton.x) + " " + std::to_string(GUI_Color_HierarchyButton.y) + " " + std::to_string(GUI_Color_HierarchyButton.z) + " " + std::to_string(GUI_Color_HierarchyButton.w));
		GetPrivateProfileString("Hierarchy.Colors", "HiButtonColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_HierarchyButton = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Hierarchy button color on hover.
		DefaultValue = (std::to_string(GUI_Color_HierarchyButtonHover.x) + " " + std::to_string(GUI_Color_HierarchyButtonHover.y) + " " + std::to_string(GUI_Color_HierarchyButtonHover.z) + " " + std::to_string(GUI_Color_HierarchyButtonHover.w));
		GetPrivateProfileString("Hierarchy.Colors", "HiButtonHoverColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_HierarchyButtonHover = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Hierarchy selected button color.
		DefaultValue = (std::to_string(GUI_Color_HierarchyButtonSelected.x) + " " + std::to_string(GUI_Color_HierarchyButtonSelected.y) + " " + std::to_string(GUI_Color_HierarchyButtonSelected.z) + " " + std::to_string(GUI_Color_HierarchyButtonSelected.w));
		GetPrivateProfileString("Hierarchy.Colors", "HiSelectedButtonColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_HierarchyButtonSelected = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// Hierarchy button color on hover when the object selected is active.
		DefaultValue = (std::to_string(GUI_Color_HierarchyButtonActiveHover.x) + " " + std::to_string(GUI_Color_HierarchyButtonActiveHover.y) + " " + std::to_string(GUI_Color_HierarchyButtonActiveHover.z) + " " + std::to_string(GUI_Color_HierarchyButtonActiveHover.w));
		GetPrivateProfileString("Hierarchy.Colors", "HiButtonHoverSelectedColor", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		IniList = PGameplayStatics::SplitString(IniBuffer, ' ');
		GUI_Color_HierarchyButtonActiveHover = { stof(IniList[0]), stof(IniList[1]), stof(IniList[2]), stof(IniList[3]) };

		// EDITOR.PREFERENCES
		//
		// Should the selected object be unselected when you click away from it? The click must be in the renderer, clicking in menus will not unselect the object.
		DefaultValue = (GUI_Pref_UnselectOnHover == true ? "true" : "false");
		GetPrivateProfileString("Editor.Preferences", "UnselectOnLoseFocus", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		if (std::strcmp(IniBuffer, "true") == 0)
		{
			GUI_Pref_UnselectOnHover = true;
		}
		else
		{
			GUI_Pref_UnselectOnHover = false;
		}

		// EDITOR.STARTUP
		//
		// What map should open when the engine opens?
		DefaultValue = GUI_Pref_StartupLevel;
		GetPrivateProfileString("Editor.Startup", "StartupMap", DefaultValue.c_str(), IniBuffer, sizeof(IniBuffer), (PGameplayStatics::GetMainDirectory() + "Configurations/Engine.ini").c_str());
		GUI_Pref_StartupLevel = IniBuffer;


		return true;
	}
}
