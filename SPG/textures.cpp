#include "textures.h"
#include "stb_image.h"

#include <iostream>
#include <algorithm>
#include <cmath>

GLuint grassTexture = 0;
GLuint skyTexture = 0;
GLuint mountainsTexture = 0;

static GLuint loadTexture2D(const char* path, bool flip = true)
{
    int w, h, channels;

    stbi_set_flip_vertically_on_load(flip);
    unsigned char* data = stbi_load(path, &w, &h, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path;
        const char* reason = stbi_failure_reason();
        if (reason) std::cerr << " | reason: " << reason;
        std::cerr << "\n";
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_LUMINANCE;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    gluBuild2DMipmaps(GL_TEXTURE_2D, format, w, h, format, GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return tex;
}

// Folositor daca muntele e PNG cu fundal deschis sau JPG cu fundal aproape alb/gri
static GLuint loadMountainTextureWithAlphaKey(const char* path, bool flip = true)
{
    int w, h, channels;

    stbi_set_flip_vertically_on_load(flip);
    unsigned char* src = stbi_load(path, &w, &h, &channels, 3);

    if (!src) {
        std::cerr << "Failed to load texture: " << path;
        const char* reason = stbi_failure_reason();
        if (reason) std::cerr << " | reason: " << reason;
        std::cerr << "\n";
        return 0;
    }

    unsigned char* rgba = new unsigned char[w * h * 4];

    for (int i = 0; i < w * h; i++) {
        unsigned char r = src[i * 3 + 0];
        unsigned char g = src[i * 3 + 1];
        unsigned char b = src[i * 3 + 2];

        rgba[i * 4 + 0] = r;
        rgba[i * 4 + 1] = g;
        rgba[i * 4 + 2] = b;

        int avg = (r + g + b) / 3;
        int maxv = std::max(r, std::max(g, b));
        int minv = std::min(r, std::min(g, b));
        int spread = maxv - minv;

        // transparent pentru alb, gri foarte deschis sau albastru foarte deschis de fundal
        if ((avg > 200 && spread < 35) || (b > 180 && r > 150 && g > 150))
            rgba[i * 4 + 3] = 0;
        else
            rgba[i * 4 + 3] = 255;
    }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(src);
    delete[] rgba;

    return tex;
}

void loadTextures()
{
    grassTexture = loadTexture2D("grass.jpg", true);
    skyTexture = loadTexture2D("sky.jpg", true);

    // daca ai PNG transparent sau aproape transparent
    mountainsTexture = loadMountainTextureWithAlphaKey("mountains.png", true);

    // daca muntele tau este JPG, foloseste:
    // mountainsTexture = loadMountainTextureWithAlphaKey("mountains.jpg", true);
}

void freeTextures()
{
    if (grassTexture) {
        glDeleteTextures(1, &grassTexture);
        grassTexture = 0;
    }

    if (skyTexture) {
        glDeleteTextures(1, &skyTexture);
        skyTexture = 0;
    }

    if (mountainsTexture) {
        glDeleteTextures(1, &mountainsTexture);
        mountainsTexture = 0;
    }
}