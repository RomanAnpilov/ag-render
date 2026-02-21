//
//  MTKViewDelegate.hpp
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include "Render.hpp"

class MTKViewDelegate : public MTK::ViewDelegate
{
public:
    MTKViewDelegate(MTL::Device* pDevice, MTK::View* pView);
    virtual ~MTKViewDelegate() override;
    virtual void drawInMTKView(MTK::View* pView) override;
    
private:
    Renderer* _pRenderer;
};
