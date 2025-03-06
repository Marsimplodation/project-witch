#ifndef MATERIAL_H
#define MATERIAL_H

#define MAX_TEXTURES 1024
#define MAX_MATERIALS 1024
struct Material {
    vec4 albedo;
    ivec4 texInfos;
};
  const int DIFFUSE = 0;
  const int NORMAL = 1;
#endif
