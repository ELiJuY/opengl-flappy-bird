#pragma once

#include <core/gpo_wrapper.h>

extern objeto triangulo;
extern objeto cubo;
extern objeto suelo;
extern objeto pared;
extern objeto avion;
extern objeto sun;

objeto crear_cubo();
objeto crear_caja_texturizada();

objeto crear_mesh_uv(
    GLfloat* vertices,
    int numFloats
);
objeto cargar_modelo_gltf(const char* ruta, bool flipUV);
objeto crear_suelo();

objeto crear_pared();
