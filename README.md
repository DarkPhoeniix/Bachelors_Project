# Bachelor's Project: Graphics Engine on DirectX12

This project focuses on the development of a graphics engine using DirectX12. The engine implements several techniques to enhance 
rendering performance and visual quality, including frustum culling, dynamic Level of Detail (LoD), occlusion culling. 
The project's goal is to optimize graphics rendering to achieve high frame rates and detailed visual output suitable for real-time 
applications.

### Features
- [x] Frustum Culling: Implemented to increase FPS by rendering only objects within the camera's view.
- [x] Dynamic Level of Detail (LoD): Adjusts the complexity of 3D models based on their distance from the camera to optimize performance.
- [x] Occlusion Culling: Uses D3D12_QUERY to avoid rendering objects occluded by others, enhancing performance.
- [x] Depth Prepass: Tested to potentially reduce overdraw, although it initially led to worsened FPS.
- [x] Performance Analysis: Utilizes PIX events for detailed performance profiling and analysis.

---
### Requirements:
- Windows 10/11
- Visual Studio 2019 or later
- DirectX12-compatible GPU
- PIX for Windows: Download PIX

### Running the Application
- Camera Controls: Use W, A, S, D to move the camera, and RMB pressed to look around.
