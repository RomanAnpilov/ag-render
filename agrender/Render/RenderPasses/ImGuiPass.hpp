//
//  ImGuiPass.hpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

class ImGuiPass
{
public:
    ~ImGuiPass();
    
    void initialize(MTL::Device* device, MTK::View* view);
    void draw(MTL4::RenderPassDescriptor* render_pass_descriptor, MTL4::CommandBuffer* command_buffer, MTL4::RenderCommandEncoder* render_encoder, MTL::ResidencySet* residency_set);
private:
    MTL::Device* device;
    MTK::View* view;
};
