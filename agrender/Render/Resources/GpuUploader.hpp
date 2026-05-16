//
//  GpuUploader.hpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#pragma once

#include <Metal/Metal.hpp>
#include <vector>
#include "AssetImporter.hpp"

struct GpuPrimitiveRange
{
    uint32_t vertex_offset = 0;
    uint32_t index_offset = 0;
    uint32_t index_count = 0;
};

struct GpuMesh
{
    std::vector<GpuPrimitiveRange> primitives;
};

struct GpuModel
{
    MTL::Buffer* vertexBuffer = nullptr;
    MTL::Buffer* indexBuffer = nullptr;
    
    std::vector<GpuMesh> meshses;
};

class GpuModelUploader
{
public:
    void initialize(MTL::Device* device);
    
    GpuModel upload(const CpuModel& model);
private:
    MTL::Device* device;
};
