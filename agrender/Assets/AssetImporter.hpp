//
//  AssetImporter.hpp
//  agrender
//
//  Created by ANPILOV Roman on 16.05.2026.
//

#include <filesystem>
#include <iostream>
#include <simd/simd.h>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>

struct CpuVertex
{
    vector_float3 position;
    vector_float2 uv;
    vector_float3 normal;
    vector_float4 color;
};

// logic to form gpu buffers and info
//GeoSurface newSurface;
//newSurface.startIndex = (uint32_t)indices.size();
//newSurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;
struct CpuPrimitive
{
    std::vector<CpuVertex> vertices;
    std::vector<uint32_t> indices;
};

struct CpuMesh
{
    std::string name;
    std::vector<CpuPrimitive> primitives;
};

struct CpuModel
{
    std::vector<CpuMesh> meshes;
};

class AssetImporter
{
public:
    CpuModel loadModel(std::filesystem::path path);
    
    fastgltf::Asset load_gltf_scene(std::filesystem::path path);
    CpuMesh load_mesh(fastgltf::Asset &gltf_asset, fastgltf::Mesh& gltf_mesh);
private:
    fastgltf::Parser parser;
};

template <>
struct fastgltf::ElementTraits<vector_float3> : fastgltf::ElementTraitsBase<vector_float3, AccessorType::Vec3, float> {};

template <>
struct fastgltf::ElementTraits<vector_float2> : fastgltf::ElementTraitsBase<vector_float2, AccessorType::Vec2, float> {};

template <>
struct fastgltf::ElementTraits<vector_float4> : fastgltf::ElementTraitsBase<vector_float4, AccessorType::Vec4, float> {};
