//
//  ImGuiPass.cpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include "ImGuiPass.hpp"
#include "imgui.h"
#include "imgui_impl_metal.h"
#include "imgui_impl_osx.h"

void ImGuiPass::initialize(MTL::Device *device, MTK::View *view)
{
    this->device = device->retain();
    this->view = view->retain();
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    ImGui_ImplMetal_Init(device);
    ImGui_ImplOSX_Init(view);
}

void ImGuiPass::draw(MTL4::RenderPassDescriptor* render_pass_descriptor,
                     MTL4::CommandBuffer* command_buffer,
                     MTL4::RenderCommandEncoder* render_encoder,
                     MTL::ResidencySet* residency_set)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = view->drawableSize().width;
    io.DisplaySize.y = view->drawableSize().height;


    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(render_pass_descriptor);
    ImGui_ImplOSX_NewFrame(view);
    ImGui::NewFrame();


    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;
        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    
    render_encoder->pushDebugGroup(NS::String::string("ImGui Rendering", NS::UTF8StringEncoding));
    ImGui_ImplMetal_RenderDrawData(draw_data, command_buffer, render_encoder, residency_set);
    render_encoder->popDebugGroup();
}

ImGuiPass::~ImGuiPass()
{
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplOSX_Shutdown();
    ImGui::DestroyContext();
}
