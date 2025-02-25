#pragma once

#include "DXObjects/Heap.h"
#include "DXObjects/DescriptorHeap.h"
#include "DXObjects/RootSignature.h"
#include "DXObjects/Texture.h"
#include "DXObjects/StatisticsQuery.h"
#include "Scene/Camera.h"
#include "Scene/Scene.h"
#include "Render/Frame.h"
#include "Window/IWindowEventListener.h"

class DXRenderer : public Core::Events::IWindowEventListener
{
public:
    DXRenderer(HWND windowHandle);
    ~DXRenderer();

    virtual void SetScene(const std::string& filepath);
    virtual bool LoadContent(TaskGPU* loadTask);
    virtual void UnloadContent();

    virtual void OnUpdate(Core::Events::UpdateEvent& e) override;
    virtual void OnRender(Core::Events::RenderEvent& e, Frame& frame) override;
    virtual void OnKeyPressed(Core::Events::KeyEvent& e) override;
    virtual void OnKeyReleased(Core::Events::KeyEvent& e) override {}
    virtual void OnMouseMoved(Core::Events::MouseMoveEvent& e) override;
    virtual void OnMouseButtonPressed(Core::Events::MouseButtonEvent& e) override;
    virtual void OnMouseButtonReleased(Core::Events::MouseButtonEvent& e) override;
    virtual void OnMouseScroll(Core::Events::MouseScrollEvent& e) override {}
    virtual void OnResize(Core::Events::ResizeEvent& e) override {}

private:
    ComPtr<ID3D12Device2> _DXDevice;
    HWND _windowHandle;

    Core::RootSignature _renderPipeline;
    Core::RootSignature _AABBpipeline;
    Core::RootSignature _depthPrepassPipeline;
    Core::RootSignature _occlusionPipeline;

    std::shared_ptr<Core::Resource> _ambient;

    Core::DescriptorHeap _texDescHeap;
    std::shared_ptr<Core::Texture> _tex;

    std::string _scenePath;
    Scene _scene;
    Camera _camera;
    bool _isCameraMoving;
    float _deltaTime;

    bool _contentLoaded;

#if defined(_DEBUG)
    Core::StatisticsQuery _statsQuery;
#endif
};