//
//  AssetImporter.cpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include "AssetImporter.hpp"

CpuModel AssetImporter::loadModel(std::filesystem::path path)
{
    CpuModel model;
    auto scene = load_gltf_scene(path);
    
    for (auto& mesh: scene.meshes)
    {
        model.meshes.push_back(load_mesh(scene, mesh));
    }
    
    std::cout << "Model loaded by path: " << path << std::endl;
    std::cout << "Count of meshes: " << model.meshes.size() << std::endl;
    
    for (auto mesh: model.meshes)
    {
        std::cout << "\n" << "Mesh name: " << mesh.name << std::endl;
        std::cout << "Count of primitives per mesh: " << mesh.primitives.size() << std::endl;
    }
    
    return model;
}

fastgltf::Asset AssetImporter::load_gltf_scene(std::filesystem::path path)
{
    auto data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None)
    {
        std::cout << "here log about error" << std::endl;
    }
    
    constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers;
    
    auto asset = parser.loadGltf(data.get(), path.parent_path(), gltfOptions);
    if (auto error = asset.error(); error != fastgltf::Error::None)
    {
        std::cout << "here log about error" << std::endl;
    }
    
    return std::move(asset.get());
}

CpuMesh AssetImporter::load_mesh(fastgltf::Asset& gltf_asset, fastgltf::Mesh& gltf_mesh)
{
    CpuMesh cpu_mesh;
    cpu_mesh.name = gltf_mesh.name;
    
    for (const auto& mesh_primitive : gltf_mesh.primitives)
    {
        CpuPrimitive cpu_primitive;
        
        // load indexes
        {
            fastgltf::Accessor& index_accessor = gltf_asset.accessors[mesh_primitive.indicesAccessor.value()];
            cpu_primitive.indices.reserve(index_accessor.count);
            fastgltf::iterateAccessor<std::uint32_t>(gltf_asset, index_accessor, [&](std::uint32_t idx) {
                cpu_primitive.indices.push_back(idx);
            });
        }
        
        // load vertex position
        {
            fastgltf::Accessor& position_accessor = gltf_asset.accessors[mesh_primitive.findAttribute("POSITION")->accessorIndex];
            cpu_primitive.vertices.resize(position_accessor.count);
            fastgltf::iterateAccessorWithIndex<vector_float3>(gltf_asset, position_accessor, [&](vector_float3 position, size_t index) {
                CpuVertex cpu_vertex;
                cpu_vertex.position = position;
                cpu_primitive.vertices[index] = cpu_vertex;
            });
        }
        
        // load vertex normals
        {
            auto normals = mesh_primitive.findAttribute("NORMAL");
            
            if (normals != mesh_primitive.attributes.end()) {
                fastgltf::Accessor& normals_accessor = gltf_asset.accessors[(*normals).accessorIndex];
                fastgltf::iterateAccessorWithIndex<vector_float3>(gltf_asset, normals_accessor, [&](vector_float3 normal, size_t index) {
                    cpu_primitive.vertices[index].normal = normal;
                });
            }
        }
        
        // load UVs
        {
            auto uv = mesh_primitive.findAttribute("TEXCOORD_0");
            if (uv != mesh_primitive.attributes.end()) {
                fastgltf::Accessor& uv_accessor = gltf_asset.accessors[(*uv).accessorIndex];
                fastgltf::iterateAccessorWithIndex<vector_float2>(gltf_asset, uv_accessor, [&](vector_float2 uv, size_t index) {
                    cpu_primitive.vertices[index].uv = uv;
                });
            }
        }
        
        // load vertex colors
        {
            auto colors = mesh_primitive.findAttribute("COLOR_0");
            if (colors != mesh_primitive.attributes.end()) {
                fastgltf::Accessor& color_accessor = gltf_asset.accessors[(*colors).accessorIndex];
                fastgltf::iterateAccessorWithIndex<vector_float4>(gltf_asset, color_accessor,[&](vector_float4 color, size_t index) {
                    cpu_primitive.vertices[index].color = color;
                });
            }
        }
        
        cpu_mesh.primitives.push_back(cpu_primitive);
    }
    
    return cpu_mesh;
}

