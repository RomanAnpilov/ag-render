//
//  AppDelegate.hpp
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include "MTKViewDelegate.hpp"

class AppDelegate : public NS::ApplicationDelegate
{
public:
    ~AppDelegate();
    
    NS::Menu* createMenuBar();
    
    virtual void applicationWillFinishLaunching(NS::Notification* pNotification) override;
    virtual void applicationDidFinishLaunching(NS::Notification* pNotification) override;
    virtual bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* pSender) override;
    
private:
    NS::Window* _pWindow;
    MTK::View* _pMtkView;
    MTL::Device* _pDevice;
    MTKViewDelegate* _pViewDelegate = nullptr;
};
