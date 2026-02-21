//
//  TriangleData.hpp
//  agrender
//
//  Created by ANPILOV Roman on 20.02.2026.
//

#include "ShaderTypes.h"

//void configureVertexDataForBuffer(long rotationInDegrees, void *bufferContents);

typedef struct TriangleData {
    VertexData vertex0;
    VertexData vertex1;
    VertexData vertex2;
}
TriangleData;

typedef struct CubeData {
    VertexData vertices[36];
}
CubeData;
