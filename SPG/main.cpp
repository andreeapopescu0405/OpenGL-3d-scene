#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>

#include "textures.h"

const float PI = 3.1415926535f;

// ===================== CAMERA =====================
static float camX = 0.0f;
static float camY = 18.0f;
static float camZ = 55.0f;

static float yaw = -90.0f;
static float pitch = -12.0f;

static float dirX = 0.0f;
static float dirY = 0.0f;
static float dirZ = -1.0f;

// ===================== STRUCTURI =====================
struct LampPos {
    float x, z;
};

LampPos lamps[] = {
    { -18.0f,  18.0f },
    {  18.0f,  18.0f },
    { -18.0f, -38.0f },
    {  18.0f, -38.0f }
};

const int lampCount = 4;

// ===================== CAMERA =====================
void updateCameraDirection()
{
    float yawRad = yaw * PI / 180.0f;
    float pitchRad = pitch * PI / 180.0f;

    dirX = cosf(yawRad) * cosf(pitchRad);
    dirY = sinf(pitchRad);
    dirZ = sinf(yawRad) * cosf(pitchRad);

    float len = sqrtf(dirX * dirX + dirY * dirY + dirZ * dirZ);
    if (len > 0.0001f) {
        dirX /= len;
        dirY /= len;
        dirZ /= len;
    }
}

// ===================== RELIEF / TEREN =====================
float terrainHeight(float x, float z)
{
    float h = 0.0f;

    // deal central
    float dx1 = x;
    float dz1 = z + 18.0f;
    float d1 = sqrtf(dx1 * dx1 + dz1 * dz1);
    h += 3.0f * expf(-(d1 * d1) / 260.0f);

    // deal stânga
    float dx2 = x + 55.0f;
    float dz2 = z + 5.0f;
    float d2 = sqrtf(dx2 * dx2 + dz2 * dz2);
    h += 4.5f * expf(-(d2 * d2) / 180.0f);

    // deal dreapta
    float dx3 = x - 55.0f;
    float dz3 = z + 5.0f;
    float d3 = sqrtf(dx3 * dx3 + dz3 * dz3);
    h += 4.5f * expf(-(d3 * d3) / 180.0f);

    // variații mici de relief
    h += 0.25f * sinf(x * 0.18f) * cosf(z * 0.16f);
    h += 0.15f * sinf((x + z) * 0.12f);

    return h;
}

void computeNormal(float x, float z, float& nx, float& ny, float& nz)
{
    float eps = 0.3f;

    float hL = terrainHeight(x - eps, z);
    float hR = terrainHeight(x + eps, z);
    float hD = terrainHeight(x, z - eps);
    float hU = terrainHeight(x, z + eps);

    nx = hL - hR;
    ny = 2.0f * eps;
    nz = hD - hU;

    float len = sqrtf(nx * nx + ny * ny + nz * nz);
    if (len > 0.0001f) {
        nx /= len;
        ny /= len;
        nz /= len;
    }
}

// ===================== LUMINĂ / FOG =====================
void setupLighting()
{
    GLfloat lightPos[] = { 30.0f, 45.0f, 20.0f, 1.0f };
    GLfloat ambient[] = { 0.14f, 0.14f, 0.14f, 1.0f };
    GLfloat diffuse[] = { 0.48f, 0.48f, 0.44f, 1.0f };
    GLfloat specular[] = { 0.15f, 0.15f, 0.15f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

void setupLampLights()
{
    for (int i = 0; i < lampCount; i++) {
        float x = lamps[i].x;
        float z = lamps[i].z;
        float y = terrainHeight(x, z) + 7.3f;

        GLfloat pos[] = { x, y, z, 1.0f };
        GLfloat ambient[] = { 0.05f, 0.04f, 0.02f, 1.0f };
        GLfloat diffuse[] = { 1.35f, 1.20f, 0.85f, 1.0f };
        GLfloat specular[] = { 1.20f, 1.10f, 0.80f, 1.0f };

        GLenum lightId = GL_LIGHT1 + i;

        glLightfv(lightId, GL_POSITION, pos);
        glLightfv(lightId, GL_AMBIENT, ambient);
        glLightfv(lightId, GL_DIFFUSE, diffuse);
        glLightfv(lightId, GL_SPECULAR, specular);

        glLightf(lightId, GL_CONSTANT_ATTENUATION, 0.8f);
        glLightf(lightId, GL_LINEAR_ATTENUATION, 0.010f);
        glLightf(lightId, GL_QUADRATIC_ATTENUATION, 0.0008f);
    }
}

void setupFog()
{
    GLfloat fogColor[] = { 0.60f, 0.82f, 1.00f, 1.0f };
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 150.0f);
    glFogf(GL_FOG_END, 280.0f);
}

// ===================== SKYBOX COMPLET =====================
void drawSkybox()
{
    if (!skyTexture) return;

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_FOG);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTexture);
    glColor3f(1.0f, 1.0f, 1.0f);

    const float s = 260.0f;
    const float yBottom = -120.0f;
    const float yTop = 140.0f;

    glPushMatrix();
    glTranslatef(camX, 0.0f, camZ);

    // spate
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, yBottom, -s);
    glTexCoord2f(1, 0); glVertex3f(s, yBottom, -s);
    glTexCoord2f(1, 1); glVertex3f(s, yTop, -s);
    glTexCoord2f(0, 1); glVertex3f(-s, yTop, -s);
    glEnd();

    // față
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(s, yBottom, s);
    glTexCoord2f(1, 0); glVertex3f(-s, yBottom, s);
    glTexCoord2f(1, 1); glVertex3f(-s, yTop, s);
    glTexCoord2f(0, 1); glVertex3f(s, yTop, s);
    glEnd();

    // stânga
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, yBottom, s);
    glTexCoord2f(1, 0); glVertex3f(-s, yBottom, -s);
    glTexCoord2f(1, 1); glVertex3f(-s, yTop, -s);
    glTexCoord2f(0, 1); glVertex3f(-s, yTop, s);
    glEnd();

    // dreapta
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(s, yBottom, -s);
    glTexCoord2f(1, 0); glVertex3f(s, yBottom, s);
    glTexCoord2f(1, 1); glVertex3f(s, yTop, s);
    glTexCoord2f(0, 1); glVertex3f(s, yTop, -s);
    glEnd();

    // sus
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, yTop, -s);
    glTexCoord2f(1, 0); glVertex3f(s, yTop, -s);
    glTexCoord2f(1, 1); glVertex3f(s, yTop, s);
    glTexCoord2f(0, 1); glVertex3f(-s, yTop, s);
    glEnd();

    // jos
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, yBottom, s);
    glTexCoord2f(1, 0); glVertex3f(s, yBottom, s);
    glTexCoord2f(1, 1); glVertex3f(s, yBottom, -s);
    glTexCoord2f(0, 1); glVertex3f(-s, yBottom, -s);
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glEnable(GL_FOG);
}

// ===================== MUNȚI / ORIZONT =====================
void drawMountains()
{
    if (!mountainsTexture) return;

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mountainsTexture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.05f);

    glColor4f(1.0f, 1.0f, 1.0f, 0.92f);

    const int segments = 9;
    const float totalWidth = 300.0f;
    const float segWidth = totalWidth / segments;
    const float leftStart = -totalWidth / 2.0f;
    const float bottomY = -8.0f;
    const float topY = 26.0f;
    const float z = -150.0f;

    for (int i = 0; i < segments; i++) {
        float x1 = leftStart + i * segWidth;
        float x2 = x1 + segWidth;
        bool flip = (i % 2 == 1);

        glBegin(GL_QUADS);
        if (!flip) {
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x1, bottomY, z);
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x2, bottomY, z);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x2, topY, z);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x1, topY, z);
        }
        else {
            glTexCoord2f(1.0f, 0.0f); glVertex3f(x1, bottomY, z);
            glTexCoord2f(0.0f, 0.0f); glVertex3f(x2, bottomY, z);
            glTexCoord2f(0.0f, 1.0f); glVertex3f(x2, topY, z);
            glTexCoord2f(1.0f, 1.0f); glVertex3f(x1, topY, z);
        }
        glEnd();
    }

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
}

// ===================== TEREN =====================
void drawTerrain()
{
    if (grassTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.30f, 0.68f, 0.28f);
    }

    float minX = -300.0f, maxX = 300.0f;
    float minZ = -300.0f, maxZ = 300.0f;
    float step = 4.0f;

    for (float z = minZ; z < maxZ; z += step) {
        glBegin(GL_TRIANGLE_STRIP);

        for (float x = minX; x <= maxX; x += step) {
            float nx, ny, nz;

            float y1 = terrainHeight(x, z);
            computeNormal(x, z, nx, ny, nz);
            glNormal3f(nx, ny, nz);
            glTexCoord2f((x - minX) * 0.03f, (z - minZ) * 0.03f);
            glVertex3f(x, y1, z);

            float y2 = terrainHeight(x, z + step);
            computeNormal(x, z + step, nx, ny, nz);
            glNormal3f(nx, ny, nz);
            glTexCoord2f((x - minX) * 0.03f, (z + step - minZ) * 0.03f);
            glVertex3f(x, y2, z + step);
        }

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}

// ===================== DRUM OVAL ÎNCHIS =====================
void drawRoad()
{
    bool useTexture = (roadTexture != 0);

    if (useTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, roadTexture);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.16f, 0.16f, 0.16f);
    }

    const int slices = 240;
    const float outerA = 36.0f;
    const float outerB = 24.0f;
    const float innerA = 26.0f;
    const float innerB = 16.0f;
    const float centerZ = -10.0f;

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; i++) {
        float t = 2.0f * PI * i / slices;
        float ct = cosf(t);
        float st = sinf(t);

        float xOuter = outerA * ct;
        float zOuter = outerB * st + centerZ;

        float xInner = innerA * ct;
        float zInner = innerB * st + centerZ;

        float yOuter = terrainHeight(xOuter, zOuter) + 0.25f;
        float yInner = terrainHeight(xInner, zInner) + 0.25f;

        glNormal3f(0.0f, 1.0f, 0.0f);

        if (useTexture) {
            glTexCoord2f(i * 0.18f, 1.0f); glVertex3f(xOuter, yOuter, zOuter);
            glTexCoord2f(i * 0.18f, 0.0f); glVertex3f(xInner, yInner, zInner);
        }
        else {
            glVertex3f(xOuter, yOuter, zOuter);
            glVertex3f(xInner, yInner, zInner);
        }
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// ===================== ACOPERIȘ =====================
void drawRoof(float w, float h, float d)
{
    glBegin(GL_TRIANGLES);
    // față
    glNormal3f(0.0f, 0.7f, 0.7f);
    glVertex3f(-w / 2, 0.0f, d / 2);
    glVertex3f(w / 2, 0.0f, d / 2);
    glVertex3f(0.0f, h, d / 2);

    // spate
    glNormal3f(0.0f, 0.7f, -0.7f);
    glVertex3f(-w / 2, 0.0f, -d / 2);
    glVertex3f(0.0f, h, -d / 2);
    glVertex3f(w / 2, 0.0f, -d / 2);
    glEnd();

    glBegin(GL_QUADS);
    // pantă stânga
    glNormal3f(-0.8f, 0.6f, 0.0f);
    glVertex3f(-w / 2, 0.0f, -d / 2);
    glVertex3f(-w / 2, 0.0f, d / 2);
    glVertex3f(0.0f, h, d / 2);
    glVertex3f(0.0f, h, -d / 2);

    // pantă dreapta
    glNormal3f(0.8f, 0.6f, 0.0f);
    glVertex3f(w / 2, 0.0f, d / 2);
    glVertex3f(w / 2, 0.0f, -d / 2);
    glVertex3f(0.0f, h, -d / 2);
    glVertex3f(0.0f, h, d / 2);
    glEnd();
}

// ===================== CLĂDIRI =====================
void drawBuilding(float x, float z, float w, float h, float d)
{
    float y = terrainHeight(x, z);

    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    glColor3f(0.90f, 0.82f, 0.65f);
    glPushMatrix();
    glTranslatef(0.0f, y + h * 0.5f, 0.0f);
    glScalef(w, h, d);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.82f, 0.72f, 0.56f);
    glPushMatrix();
    glTranslatef(0.0f, y + h + 1.3f, 0.0f);
    glScalef(w * 0.85f, 2.0f, d * 0.85f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.60f, 0.18f, 0.12f);
    glPushMatrix();
    glTranslatef(0.0f, y + h + 2.3f, 0.0f);
    drawRoof(w * 1.05f, 2.5f, d * 1.05f);
    glPopMatrix();

    glPopMatrix();
}

// ===================== COPACI =====================
void drawTree(float x, float z)
{
    float y = terrainHeight(x, z);

    glPushMatrix();
    glTranslatef(x, y, z);

    glColor3f(0.45f, 0.28f, 0.10f);
    glPushMatrix();
    glTranslatef(0.0f, 1.5f, 0.0f);
    glScalef(0.5f, 3.0f, 0.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.10f, 0.55f, 0.15f);
    glPushMatrix();
    glTranslatef(0.0f, 3.4f, 0.0f);
    glutSolidSphere(1.4f, 16, 16);
    glPopMatrix();

    glColor3f(0.14f, 0.65f, 0.20f);
    glPushMatrix();
    glTranslatef(0.0f, 4.5f, 0.0f);
    glutSolidSphere(1.0f, 16, 16);
    glPopMatrix();

    glPopMatrix();
}

// ===================== STÂLPI DE ILUMINAT =====================
void drawLamp(float x, float z)
{
    float y = terrainHeight(x, z);

    glPushMatrix();
    glTranslatef(x, y, z);

    GLfloat poleSpec[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    GLfloat poleShiny[] = { 40.0f };
    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, poleSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, poleShiny);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);

    glColor3f(0.38f, 0.38f, 0.38f);
    glPushMatrix();
    glTranslatef(0.0f, 3.5f, 0.0f);
    glScalef(0.3f, 7.0f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    GLfloat bulbEmission[] = { 0.95f, 0.85f, 0.35f, 1.0f };
    GLfloat bulbSpec[] = { 1.0f, 0.95f, 0.70f, 1.0f };
    GLfloat bulbShiny[] = { 90.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, bulbEmission);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, bulbSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, bulbShiny);

    glColor3f(1.0f, 0.95f, 0.65f);
    glPushMatrix();
    glTranslatef(0.0f, 7.3f, 0.0f);
    glutSolidSphere(0.55f, 20, 20);
    glPopMatrix();

    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);

    glPopMatrix();
}

void drawLampGlow(float x, float z)
{
    float y = terrainHeight(x, z) + 7.3f;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    glPushMatrix();
    glTranslatef(x, y, z);
    glColor4f(1.0f, 0.9f, 0.4f, 0.22f);
    glutSolidSphere(1.2f, 18, 18);
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

// ===================== OBIECTE STATICE =====================
void drawStaticObjects()
{
    // 4 clădiri
    drawBuilding(-48.0f, 18.0f, 7.0f, 11.0f, 7.0f);
    drawBuilding(-56.0f, 2.0f, 8.0f, 13.0f, 8.0f);
    drawBuilding(48.0f, 16.0f, 7.0f, 10.0f, 7.0f);
    drawBuilding(56.0f, 0.0f, 8.0f, 14.0f, 8.0f);

    // 8 copaci
    drawTree(-70.0f, -10.0f);
    drawTree(-68.0f, 10.0f);
    drawTree(-60.0f, 28.0f);
    drawTree(-42.0f, -24.0f);

    drawTree(70.0f, -10.0f);
    drawTree(68.0f, 10.0f);
    drawTree(60.0f, 28.0f);
    drawTree(42.0f, -24.0f);

    // 4 stâlpi
    for (int i = 0; i < lampCount; i++) {
        drawLamp(lamps[i].x, lamps[i].z);
        drawLampGlow(lamps[i].x, lamps[i].z);
    }
}

// ===================== UMBRE SIMPLE =====================
void drawSimpleShadowCircle(float x, float z, float radius, float alpha)
{
    float y = terrainHeight(x, z) + 0.03f;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const int segments = 32;

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.0f, 0.0f, 0.0f, alpha);
    glVertex3f(x, y, z);

    for (int i = 0; i <= segments; i++) {
        float t = 2.0f * PI * i / segments;
        float px = x + cosf(t) * radius;
        float pz = z + sinf(t) * radius;
        float py = terrainHeight(px, pz) + 0.03f;

        glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
        glVertex3f(px, py, pz);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void drawOffsetShadow(float objX, float objZ, float radius, float lampX, float lampZ)
{
    float dx = objX - lampX;
    float dz = objZ - lampZ;

    float len = sqrtf(dx * dx + dz * dz);
    if (len < 0.001f) len = 0.001f;

    dx /= len;
    dz /= len;

    float shadowX = objX + dx * 5.0f;
    float shadowZ = objZ + dz * 5.0f;

    float y = terrainHeight(shadowX, shadowZ) + 0.08f;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.0f, 0.0f, 0.0f, 0.28f);

    const int segments = 24;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(shadowX, y, shadowZ);

    for (int i = 0; i <= segments; i++) {
        float t = 2.0f * PI * i / segments;
        float px = shadowX + cosf(t) * radius;
        float pz = shadowZ + sinf(t) * radius;
        float py = terrainHeight(px, pz) + 0.08f;
        glVertex3f(px, py, pz);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_FOG);
    glEnable(GL_LIGHTING);
}

void drawMultipleLampShadows()
{
    // umbre sub clădiri
    drawSimpleShadowCircle(-48.0f, 18.0f, 7.0f, 0.28f);
    drawSimpleShadowCircle(-56.0f, 2.0f, 6.0f, 0.28f);
    drawSimpleShadowCircle(48.0f, 16.0f, 7.0f, 0.28f);
    drawSimpleShadowCircle(56.0f, 0.0f, 6.0f, 0.28f);

    // umbre multiple de la lămpi
    for (int i = 0; i < lampCount; i++) {
        // clădiri
        drawOffsetShadow(-48.0f, 18.0f, 5.0f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(-56.0f, 2.0f, 5.5f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(48.0f, 16.0f, 5.0f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(56.0f, 0.0f, 5.5f, lamps[i].x, lamps[i].z);

        // copaci
        drawOffsetShadow(-70.0f, -10.0f, 1.7f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(-68.0f, 10.0f, 1.7f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(-60.0f, 28.0f, 1.7f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(-42.0f, -24.0f, 1.7f, lamps[i].x, lamps[i].z);

        drawOffsetShadow(70.0f, -10.0f, 1.7f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(68.0f, 10.0f, 1.7f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(60.0f, 28.0f, 1.7f, lamps[i].x, lamps[i].z);
        drawOffsetShadow(42.0f, -24.0f, 1.7f, lamps[i].x, lamps[i].z);
    }
}

// ===================== RENDER =====================
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    updateCameraDirection();

    gluLookAt(
        camX, camY, camZ,
        camX + dirX, camY + dirY, camZ + dirZ,
        0.0f, 1.0f, 0.0f
    );

    setupLighting();
    setupLampLights();

    drawSkybox();
    drawMountains();
    drawTerrain();
    drawMultipleLampShadows();
    drawRoad();
    drawStaticObjects();

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
}

// ===================== CONTROL =====================
/*
W/S - înainte / înapoi
A/D - stânga / dreapta
Q/E - sus / jos
J/L - rotație stânga / dreapta
I/K - rotație sus / jos
ESC - ieșire
*/
void keyboard(unsigned char key, int, int)
{
    updateCameraDirection();

    float moveSpeed = 1.5f;
    float rotSpeed = 3.0f;

    float rightX = -dirZ;
    float rightZ = dirX;

    switch (key) {
    case 27:
        freeTextures();
        std::exit(0);
        break;

    case 'w':
    case 'W':
        camX += dirX * moveSpeed;
        camY += dirY * moveSpeed;
        camZ += dirZ * moveSpeed;
        break;

    case 's':
    case 'S':
        camX -= dirX * moveSpeed;
        camY -= dirY * moveSpeed;
        camZ -= dirZ * moveSpeed;
        break;

    case 'a':
    case 'A':
        camX -= rightX * moveSpeed;
        camZ -= rightZ * moveSpeed;
        break;

    case 'd':
    case 'D':
        camX += rightX * moveSpeed;
        camZ += rightZ * moveSpeed;
        break;

    case 'q':
    case 'Q':
        camY += moveSpeed;
        break;

    case 'e':
    case 'E':
        camY -= moveSpeed;
        if (camY < 2.0f) camY = 2.0f;
        break;

    case 'j':
    case 'J':
        yaw -= rotSpeed;
        break;

    case 'l':
    case 'L':
        yaw += rotSpeed;
        break;

    case 'i':
    case 'I':
        pitch += rotSpeed;
        if (pitch > 89.0f) pitch = 89.0f;
        break;

    case 'k':
    case 'K':
        pitch -= rotSpeed;
        if (pitch < -89.0f) pitch = -89.0f;
        break;
    }

    glutPostRedisplay();
}

// ===================== INIT =====================
void init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);
    glEnable(GL_LIGHT4);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat matSpec[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat matShininess[] = { 16.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

    glShadeModel(GL_SMOOTH);
    glDisable(GL_CULL_FACE);

    glClearColor(0.60f, 0.82f, 1.00f, 1.0f);

    setupFog();
    loadTextures();
}

// ===================== MAIN =====================
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Scena 3D - Proiect");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}