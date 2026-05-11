#include "graphics/meshes.h"

#include <cfloat>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../third_party/tiny_gltf.h"

objeto triangulo;
objeto cubo;
objeto suelo;
objeto pared;
objeto avion;
objeto sun;
objeto crear_cubo(void)
{
    objeto obj;

    // Cubo centrado en el origen.
    // Cada vértice tiene:
    // x, y, z, nx, ny, nz
    // Es decir: posición + normal.
    GLfloat vertices[] = {
        // Cara frontal (z = 0.5), normal (0, 0, 1)
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,

        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,

        // Cara trasera (z = -0.5), normal (0, 0, -1)
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,

         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,

        // Cara izquierda (x = -0.5), normal (-1, 0, 0)
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,

        // Cara derecha (x = 0.5), normal (1, 0, 0)
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,

         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,

        // Cara superior (y = 0.5), normal (0, 1, 0)
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,

        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,

        // Cara inferior (y = -0.5), normal (0, -1, 0)
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f
    };

    obj.Nv = 36;

    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributo 0: posición del vértice
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(GLfloat),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // Atributo 1: normal del vértice
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(GLfloat),
        (void*)(3 * sizeof(GLfloat))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return obj;
}

objeto crear_mesh_uv(
    GLfloat* vertices,
    int numFloats
)
{
 objeto obj;

 obj.Nv = numFloats / 8;

 glGenVertexArrays(1, &obj.VAO);
 glBindVertexArray(obj.VAO);

 GLuint VBO;

 glGenBuffers(1, &VBO);

 glBindBuffer(GL_ARRAY_BUFFER, VBO);

 glBufferData(
     GL_ARRAY_BUFFER,
     numFloats * sizeof(GLfloat),
     vertices,
     GL_STATIC_DRAW
 );

 // Posición
 glVertexAttribPointer(
     0,
     3,
     GL_FLOAT,
     GL_FALSE,
     8 * sizeof(GLfloat),
     (void*)0
 );

 glEnableVertexAttribArray(0);

 // Normal
 glVertexAttribPointer(
     1,
     3,
     GL_FLOAT,
     GL_FALSE,
     8 * sizeof(GLfloat),
     (void*)(3 * sizeof(GLfloat))
 );

 glEnableVertexAttribArray(1);

 // UV
 glVertexAttribPointer(
     2,
     2,
     GL_FLOAT,
     GL_FALSE,
     8 * sizeof(GLfloat),
     (void*)(6 * sizeof(GLfloat))
 );

 glEnableVertexAttribArray(2);

 glBindVertexArray(0);

 return obj;
}

objeto crear_caja_texturizada()
{
    std::vector<GLfloat> vertices;

    auto pushVertex = [&](glm::vec3 p, glm::vec3 n, glm::vec2 uv)
    {
        vertices.push_back(p.x);
        vertices.push_back(p.y);
        vertices.push_back(p.z);
        vertices.push_back(n.x);
        vertices.push_back(n.y);
        vertices.push_back(n.z);
        vertices.push_back(uv.x);
        vertices.push_back(uv.y);
    };

    auto pushQuad = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 n)
    {
        pushVertex(a, n, glm::vec2(0.0f, 0.0f));
        pushVertex(b, n, glm::vec2(1.0f, 0.0f));
        pushVertex(c, n, glm::vec2(1.0f, 1.0f));
        pushVertex(a, n, glm::vec2(0.0f, 0.0f));
        pushVertex(c, n, glm::vec2(1.0f, 1.0f));
        pushVertex(d, n, glm::vec2(0.0f, 1.0f));
    };

    pushQuad(
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f),
        glm::vec3( 0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    pushQuad(
        glm::vec3( 0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3( 0.5f,  0.5f, -0.5f),
        glm::vec3(0.0f, 0.0f, -1.0f)
    );

    pushQuad(
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f,  0.5f),
        glm::vec3(-0.5f,  0.5f, -0.5f),
        glm::vec3(-1.0f, 0.0f, 0.0f)
    );

    pushQuad(
        glm::vec3(0.5f, -0.5f,  0.5f),
        glm::vec3(0.5f, -0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f, -0.5f),
        glm::vec3(0.5f,  0.5f,  0.5f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    pushQuad(
        glm::vec3(-0.5f, 0.5f,  0.5f),
        glm::vec3( 0.5f, 0.5f,  0.5f),
        glm::vec3( 0.5f, 0.5f, -0.5f),
        glm::vec3(-0.5f, 0.5f, -0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    pushQuad(
        glm::vec3(-0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f, -0.5f),
        glm::vec3( 0.5f, -0.5f,  0.5f),
        glm::vec3(-0.5f, -0.5f,  0.5f),
        glm::vec3(0.0f, -1.0f, 0.0f)
    );

    return crear_mesh_uv(
        vertices.data(),
        (int)vertices.size()
    );
}

bool cargarImagenDummy(
    tinygltf::Image* image,
    const int image_idx,
    std::string* err,
    std::string* warn,
    int req_width,
    int req_height,
    const unsigned char* bytes,
    int size,
    void* user_data
)
{
    (void)image_idx;
    (void)err;
    (void)warn;
    (void)req_width;
    (void)req_height;
    (void)bytes;
    (void)size;
    (void)user_data;

    image->width = 1;
    image->height = 1;
    image->component = 4;
    image->bits = 8;
    image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    image->image = {255, 255, 255, 255};

    return true;
}

objeto crear_mesh_gltf_color(GLfloat* vertices, int numFloats)
{
    objeto obj;

    obj.Nv = numFloats / 11;

    glGenVertexArrays(1, &obj.VAO);
    glBindVertexArray(obj.VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        numFloats * sizeof(GLfloat),
        vertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (void*)(3 * sizeof(GLfloat))
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (void*)(6 * sizeof(GLfloat))
    );
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(
        3,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (void*)(8 * sizeof(GLfloat))
    );
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    return obj;
}

objeto cargar_modelo_gltf(
    const char* ruta,
    bool flipUVs
)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;

    loader.SetImageLoader(cargarImagenDummy, nullptr);

    std::string err;
    std::string warn;

    bool ok = loader.LoadBinaryFromFile(&model, &err, &warn, ruta);

    if (!warn.empty())
    {
        std::cout << "TinyGLTF warning: " << warn << std::endl;
    }

    if (!err.empty())
    {
        std::cout << "TinyGLTF error: " << err << std::endl;
    }

    if (!ok)
    {
        std::cout << "No se pudo cargar el modelo GLB: " << ruta << std::endl;
        std::cout << "Se usa cubo como fallback." << std::endl;
        return crear_cubo();
    }

    std::vector<GLfloat> vertices;

    auto componentSize = [](int componentType) -> size_t
    {
        switch (componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return 1;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return 2;
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return 4;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return 8;
        default:
            return 0;
        }
    };

    auto getDataPtr = [&](const tinygltf::Accessor& accessor, size_t index) -> const unsigned char*
    {
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        size_t compSize = componentSize(accessor.componentType);
        size_t numComp = tinygltf::GetNumComponentsInType(accessor.type);
        size_t stride = accessor.ByteStride(bufferView);

        if (stride == 0)
        {
            stride = compSize * numComp;
        }

        size_t offset = bufferView.byteOffset + accessor.byteOffset + index * stride;

        return buffer.data.data() + offset;
    };

    auto readIndex = [&](const tinygltf::Accessor& accessor, size_t index) -> unsigned int
    {
        const unsigned char* ptr = getDataPtr(accessor, index);

        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return *(const unsigned char*)ptr;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return *(const unsigned short*)ptr;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return *(const unsigned int*)ptr;
        default:
            return 0;
        }
    };

    auto readVec3 = [&](const tinygltf::Accessor& accessor, size_t index) -> glm::vec3
    {
        const unsigned char* ptr = getDataPtr(accessor, index);
        const float* f = (const float*)ptr;

        return glm::vec3(f[0], f[1], f[2]);
    };

    auto readVec2 = [&](const tinygltf::Accessor& accessor, size_t index) -> glm::vec2
    {
        const unsigned char* ptr = getDataPtr(accessor, index);
        const float* f = (const float*)ptr;

        if (flipUVs)
        {
            return glm::vec2(f[0], 1.0f - f[1]);
        }

        return glm::vec2(f[0], f[1]);
    };

    auto readColor = [&](const tinygltf::Accessor& accessor, size_t index) -> glm::vec3
    {
        const unsigned char* ptr = getDataPtr(accessor, index);

        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
        {
            const float* f = (const float*)ptr;

            return glm::vec3(f[0], f[1], f[2]);
        }

        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
        {
            const unsigned char* c = (const unsigned char*)ptr;

            return glm::vec3(
                c[0] / 255.0f,
                c[1] / 255.0f,
                c[2] / 255.0f
            );
        }

        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            const unsigned short* c = (const unsigned short*)ptr;

            return glm::vec3(
                c[0] / 65535.0f,
                c[1] / 65535.0f,
                c[2] / 65535.0f
            );
        }

        return glm::vec3(0.8f);
    };

    auto getMaterialColor = [&](int materialIndex) -> glm::vec3
    {
        if (materialIndex < 0 || materialIndex >= (int)model.materials.size())
        {
            return glm::vec3(0.8f);
        }

        const tinygltf::Material& mat = model.materials[materialIndex];

        if (mat.pbrMetallicRoughness.baseColorFactor.size() >= 3)
        {
            return glm::vec3(
                (float)mat.pbrMetallicRoughness.baseColorFactor[0],
                (float)mat.pbrMetallicRoughness.baseColorFactor[1],
                (float)mat.pbrMetallicRoughness.baseColorFactor[2]
            );
        }

        return glm::vec3(0.8f);
    };

    auto getNodeMatrix = [&](const tinygltf::Node& node) -> glm::mat4
    {
        glm::mat4 M(1.0f);

        if (node.matrix.size() == 16)
        {
            for (int col = 0; col < 4; col++)
            {
                for (int row = 0; row < 4; row++)
                {
                    M[col][row] = (float)node.matrix[col * 4 + row];
                }
            }

            return M;
        }

        glm::vec3 translation(0.0f);
        glm::vec3 scale(1.0f);
        glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);

        if (node.translation.size() == 3)
        {
            translation = glm::vec3(
                (float)node.translation[0],
                (float)node.translation[1],
                (float)node.translation[2]
            );
        }

        if (node.scale.size() == 3)
        {
            scale = glm::vec3(
                (float)node.scale[0],
                (float)node.scale[1],
                (float)node.scale[2]
            );
        }

        if (node.rotation.size() == 4)
        {
            rotation = glm::quat(
                (float)node.rotation[3],
                (float)node.rotation[0],
                (float)node.rotation[1],
                (float)node.rotation[2]
            );
        }

        M =
            glm::translate(glm::mat4(1.0f), translation) *
            glm::toMat4(rotation) *
            glm::scale(glm::mat4(1.0f), scale);

        return M;
    };

    auto pushVertex = [&](glm::vec3 p, glm::vec3 n, glm::vec2 uv, glm::vec3 color)
    {
        vertices.push_back(p.x);
        vertices.push_back(p.y);
        vertices.push_back(p.z);
        vertices.push_back(n.x);
        vertices.push_back(n.y);
        vertices.push_back(n.z);
        vertices.push_back(uv.x);
        vertices.push_back(uv.y);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
    };

    std::function<void(int, glm::mat4)> processNode;

    processNode = [&](int nodeIndex, glm::mat4 parentTransform)
    {
        const tinygltf::Node& node = model.nodes[nodeIndex];
        glm::mat4 nodeTransform = parentTransform * getNodeMatrix(node);

        if (node.mesh >= 0)
        {
            const tinygltf::Mesh& mesh = model.meshes[node.mesh];

            for (const tinygltf::Primitive& primitive : mesh.primitives)
            {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
                {
                    continue;
                }

                auto posIt = primitive.attributes.find("POSITION");

                if (posIt == primitive.attributes.end())
                {
                    continue;
                }

                const tinygltf::Accessor& posAccessor = model.accessors[posIt->second];
                const tinygltf::Accessor* normalAccessor = nullptr;
                const tinygltf::Accessor* uvAccessor = nullptr;
                const tinygltf::Accessor* indexAccessor = nullptr;
                const tinygltf::Accessor* colorAccessor = nullptr;

                auto normalIt = primitive.attributes.find("NORMAL");

                if (normalIt != primitive.attributes.end())
                {
                    normalAccessor = &model.accessors[normalIt->second];
                }

                auto uvIt = primitive.attributes.find("TEXCOORD_0");

                if (uvIt != primitive.attributes.end())
                {
                    uvAccessor = &model.accessors[uvIt->second];
                }

                auto colorIt = primitive.attributes.find("COLOR_0");

                if (colorIt != primitive.attributes.end())
                {
                    colorAccessor = &model.accessors[colorIt->second];
                }

                if (primitive.indices >= 0)
                {
                    indexAccessor = &model.accessors[primitive.indices];
                }

                glm::vec3 materialColor = getMaterialColor(primitive.material);
                glm::mat3 normalMatrix =
                    glm::mat3(glm::transpose(glm::inverse(nodeTransform)));

                size_t indexCount =
                    indexAccessor != nullptr
                    ? indexAccessor->count
                    : posAccessor.count;

                for (size_t i = 0; i + 2 < indexCount; i += 3)
                {
                    unsigned int idx0 = indexAccessor ? readIndex(*indexAccessor, i + 0) : (unsigned int)(i + 0);
                    unsigned int idx1 = indexAccessor ? readIndex(*indexAccessor, i + 1) : (unsigned int)(i + 1);
                    unsigned int idx2 = indexAccessor ? readIndex(*indexAccessor, i + 2) : (unsigned int)(i + 2);

                    glm::vec3 p0 = glm::vec3(nodeTransform * glm::vec4(readVec3(posAccessor, idx0), 1.0f));
                    glm::vec3 p1 = glm::vec3(nodeTransform * glm::vec4(readVec3(posAccessor, idx1), 1.0f));
                    glm::vec3 p2 = glm::vec3(nodeTransform * glm::vec4(readVec3(posAccessor, idx2), 1.0f));

                    glm::vec3 n0;
                    glm::vec3 n1;
                    glm::vec3 n2;

                    if (normalAccessor)
                    {
                        n0 = glm::normalize(normalMatrix * readVec3(*normalAccessor, idx0));
                        n1 = glm::normalize(normalMatrix * readVec3(*normalAccessor, idx1));
                        n2 = glm::normalize(normalMatrix * readVec3(*normalAccessor, idx2));
                    }
                    else
                    {
                        glm::vec3 faceNormal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
                        n0 = faceNormal;
                        n1 = faceNormal;
                        n2 = faceNormal;
                    }

                    glm::vec2 uv0(0.0f);
                    glm::vec2 uv1(0.0f);
                    glm::vec2 uv2(0.0f);

                    if (uvAccessor)
                    {
                        uv0 = readVec2(*uvAccessor, idx0);
                        uv1 = readVec2(*uvAccessor, idx1);
                        uv2 = readVec2(*uvAccessor, idx2);
                    }

                    glm::vec3 c0 = materialColor;
                    glm::vec3 c1 = materialColor;
                    glm::vec3 c2 = materialColor;

                    if (colorAccessor)
                    {
                        c0 = readColor(*colorAccessor, idx0);
                        c1 = readColor(*colorAccessor, idx1);
                        c2 = readColor(*colorAccessor, idx2);
                    }

                    pushVertex(p0, n0, uv0, c0);
                    pushVertex(p1, n1, uv1, c1);
                    pushVertex(p2, n2, uv2, c2);
                }
            }
        }

        for (int childIndex : node.children)
        {
            processNode(childIndex, nodeTransform);
        }
    };

    if (model.scenes.empty())
    {
        std::cout << "El GLB no tiene escenas. Se usa cubo como fallback." << std::endl;
        return crear_cubo();
    }

    int sceneIndex = model.defaultScene >= 0 ? model.defaultScene : 0;
    const tinygltf::Scene& scene = model.scenes[sceneIndex];

    for (int nodeIndex : scene.nodes)
    {
        processNode(nodeIndex, glm::mat4(1.0f));
    }

    if (vertices.empty())
    {
        std::cout << "El GLB no contiene triangulos cargables. Se usa cubo como fallback." << std::endl;
        return crear_cubo();
    }

    glm::vec3 minP(FLT_MAX);
    glm::vec3 maxP(-FLT_MAX);

    for (size_t i = 0; i < vertices.size(); i += 11)
    {
        glm::vec3 p(
            vertices[i + 0],
            vertices[i + 1],
            vertices[i + 2]
        );

        minP = glm::min(minP, p);
        maxP = glm::max(maxP, p);
    }

    glm::vec3 center = (minP + maxP) * 0.5f;
    glm::vec3 size = maxP - minP;

    float maxDim = glm::max(size.x, glm::max(size.y, size.z));

    if (maxDim <= 0.0001f)
    {
        maxDim = 1.0f;
    }

    for (size_t i = 0; i < vertices.size(); i += 11)
    {
        vertices[i + 0] = (vertices[i + 0] - center.x) / maxDim;
        vertices[i + 1] = (vertices[i + 1] - center.y) / maxDim;
        vertices[i + 2] = (vertices[i + 2] - center.z) / maxDim;
    }

    std::cout << "Modelo GLB cargado correctamente: " << ruta << std::endl;
    std::cout << "Vertices cargados: " << vertices.size() / 11 << std::endl;

    return crear_mesh_gltf_color(
        vertices.data(),
        (int)vertices.size()
    );
}

objeto crear_suelo()
{
    GLfloat vertices[] =
    {
        // Cara superior

        -0.5f, 0.5f,  0.5f,   0,1,0,   0,0,
         0.5f, 0.5f,  0.5f,   0,1,0,   4,0,
         0.5f, 0.5f, -0.5f,   0,1,0,   4,4,

        -0.5f, 0.5f,  0.5f,   0,1,0,   0,0,
         0.5f, 0.5f, -0.5f,   0,1,0,   4,4,
        -0.5f, 0.5f, -0.5f,   0,1,0,   0,4
    };

    return crear_mesh_uv(
        vertices,
        sizeof(vertices) / sizeof(GLfloat)
    );
}

objeto crear_pared()
{
    GLfloat vertices[] =
    {
        // Cara izquierda

        -0.5f, -0.5f, -0.5f,   -1,0,0,   0,0,
        -0.5f, -0.5f,  0.5f,   -1,0,0,   0,2,
        -0.5f,  0.5f,  0.5f,   -1,0,0,   2,2,

        -0.5f, -0.5f, -0.5f,   -1,0,0,   0,0,
        -0.5f,  0.5f,  0.5f,   -1,0,0,   2,2,
        -0.5f,  0.5f, -0.5f,   -1,0,0,   2,0,

        // Cara derecha

         0.5f, -0.5f,  0.5f,    1,0,0,   0,0,
         0.5f, -0.5f, -0.5f,    1,0,0,   0,2,
         0.5f,  0.5f, -0.5f,    1,0,0,   2,2,

         0.5f, -0.5f,  0.5f,    1,0,0,   0,0,
         0.5f,  0.5f, -0.5f,    1,0,0,   2,2,
         0.5f,  0.5f,  0.5f,    1,0,0,   2,0
    };

    return crear_mesh_uv(
        vertices,
        sizeof(vertices) / sizeof(GLfloat)
    );
}

