// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//







int main()
{
	

#pragma endregion Window init




	//DXGI_SWAP_CHAIN_DESC swapDesc = {};
	


	//Microsoft::WRL::ComPtr<ID3D11Device> device;
	//ID3D11DeviceContext* context;
	//IDXGISwapChain* swapChain;



	

	//ID3D11Texture2D* backTex;
	//ID3D11RenderTargetView* rtv;



	//ID3DBlob* vertexBC = nullptr;
	//ID3DBlob* errorVertexCode = nullptr;
	

	//ID3D11InputLayout* layout;
	

	//DirectX::XMFLOAT4 points[8] = {
		
	};


	






	MSG msg = {};
	bool isExitRequested = false;
	while (!isExitRequested) {
		// Handle the windows messages.
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT) {
			isExitRequested = true;
		}

		

		


		

		if (totalTime > 1.0f) {
			float fps = frameCount / totalTime;

			totalTime -= 1.0f;

			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %f"), fps);
			SetWindowText(hWnd, text);

			frameCount = 0;
		}

	
	}

	std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
