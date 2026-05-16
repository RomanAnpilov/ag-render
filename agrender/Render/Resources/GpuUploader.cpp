//
//  GpuUploader.cpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include "GpuUploader.hpp"

void GpuModelUploader::initialize(MTL::Device *device)
{
    this->device = device->retain();
}

GpuModel GpuModelUploader::upload(const CpuModel &model)
{
    std::vector<CpuVertex> verticies;
    std::vector<uint32_t> indicies;
    
    GpuModel gpu_model;
    
    for (const CpuMesh& cpu_mesh: model.meshes)
    {
        GpuMesh gpu_mesh;
        
        for (const CpuPrimitive& cpu_primitive: cpu_mesh.primitives)
        {
            GpuPrimitiveRange gpu_primitive_range;
            
            gpu_primitive_range.index_count = static_cast<uint32_t>(cpu_primitive.indices.size());
            gpu_primitive_range.vertex_offset = static_cast<uint32_t>(verticies.size());
            gpu_primitive_range.index_offset = static_cast<uint32_t>(indicies.size());
            
            verticies.insert(verticies.end(), cpu_primitive.vertices.begin(), cpu_primitive.vertices.end());
            
            for (uint32_t index: cpu_primitive.indices)
            {
                indicies.push_back(index + gpu_primitive_range.index_offset);
            }
            
            gpu_mesh.primitives.push_back(gpu_primitive_range);
        }
        
        gpu_model.meshses.push_back(gpu_mesh);
    }
    
    // TODO: Remove storage mode shared, make it with blit copy
    gpu_model.vertexBuffer = device->newBuffer(verticies.data(), verticies.size() * sizeof(CpuVertex), MTL::ResourceStorageModeShared);
    gpu_model.indexBuffer = device->newBuffer(indicies.data(), indicies.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
    
    return gpu_model;
}
