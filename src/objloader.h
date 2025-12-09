#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>

class OBJLoader {
public:
    static bool loadOBJ(
        const std::string& path,
        std::vector<float>& out_vertices  // Format: x,y,z, nx,ny,nz (6 floats per vertex)
        );
};

#endif // OBJLOADER_H
