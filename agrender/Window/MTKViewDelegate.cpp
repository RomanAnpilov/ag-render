//
//  MTKViewDelegate.cpp
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#include "MTKViewDelegate.hpp"

MTKViewDelegate::MTKViewDelegate( MTL::Device* pDevice, MTK::View* pView )
: MTK::ViewDelegate()
, _pRenderer( new Renderer( pDevice, pView ) )
{}

MTKViewDelegate::~MTKViewDelegate()
{
    delete _pRenderer;
}

void MTKViewDelegate::drawInMTKView( MTK::View* pView )
{
    _pRenderer->draw( pView );
}
