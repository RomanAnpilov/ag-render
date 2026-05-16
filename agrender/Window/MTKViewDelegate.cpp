//
//  MTKViewDelegate.cpp
//  agrender
//
//  Created by ANPILOV Roman on 05.02.2026.
//

#include "AssetImporter.hpp"
#include "MTKViewDelegate.hpp"
#include <iostream>

MTKViewDelegate::MTKViewDelegate( MTL::Device* pDevice, MTK::View* pView )
: MTK::ViewDelegate()
{
    _pRenderer = new Renderer(pDevice, pView);
//    AssetImporter importer;
//    CpuModel model = importer.loadModel("/Users/ruaapr3/Developer/agrender/BoxTextured.gltf");
}

MTKViewDelegate::~MTKViewDelegate()
{
    delete _pRenderer;
}

void MTKViewDelegate::drawInMTKView( MTK::View* pView )
{
    _pRenderer->draw( pView );
}
