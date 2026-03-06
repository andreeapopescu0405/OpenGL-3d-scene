#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include "textures.h"

static float camX = 0.0f;
static float camY = 8.0f;
static float camZ = 26.0f;

static float lookX = 0.0f;
static float lookY = 3.5f;
static float lookZ = -10.0f;

float terrainHeight(float x, float z)
{
    float h = 0.0f;

    float dx1 = x;
    float dz1 = z + 12.0f;
    float d1 = sqrtf(dx1 * dx1 + dz1 * dz1);
    h += 3.2f * expf(-(d1 * d1) / 180.0f);

    float dx2 = x + 16.0f;
    float dz2 = z + 10.0f;
    float d2 = sqrtf(dx2 * dx2 + dz2 * dz2);
    h += 1.6f * expf(-(d2 * d2) / 90.0f);

    float dx3 = x - 15.0f;
    float dz3 = z + 11.0f;
    float d3 = sqrtf(dx3 * dx3 + dz3 * dz3);
    h += 1.8f * expf(-(d3 * d3) / 100.0f);

    h += 0.18f * sinf(x * 0.22f) * cosf(z * 0.20f);

 

    return h;
}

void computeNormal(float x, float z, float& nx, float& ny, float& nz)
{
    float eps = 0.2f;

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

void setupLighting()
{
    GLfloat lightPos[] = { 25.0f, 35.0f, 20.0f, 1.0f };
    GLfloat ambient[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    GLfloat diffuse[] = { 0.90f, 0.90f, 0.85f, 1.0f };
    GLfloat specular[] = { 0.20f, 0.20f, 0.20f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

void drawSkybox()
{
    if (!skyTexture) return;

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    float s = 300.0f;

    glPushMatrix();
    glTranslatef(camX, 0.0f, camZ);

    // perete spate
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, -20.0f, -s);
    glTexCoord2f(1, 0); glVertex3f(s, -20.0f, -s);
    glTexCoord2f(1, 1); glVertex3f(s, 140.0f, -s);
    glTexCoord2f(0, 1); glVertex3f(-s, 140.0f, -s);
    glEnd();

    // stanga
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-s, -20.0f, -s);
    glTexCoord2f(1, 0); glVertex3f(-s, -20.0f, s);
    glTexCoord2f(1, 1); glVertex3f(-s, 140.0f, s);
    glTexCoord2f(0, 1); glVertex3f(-s, 140.0f, -s);
    glEnd();

    // dreapta
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(s, -20.0f, s);
    glTexCoord2f(1, 0); glVertex3f(s, -20.0f, -s);
    glTexCoord2f(1, 1); glVertex3f(s, 140.0f, -s);
    glTexCoord2f(0, 1); glVertex3f(s, 140.0f, s);
    glEnd();

   

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

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

    // putin estompati, sa para mai departati
    glColor4f(0.92f, 0.92f, 0.92f, 0.78f);

    const int segments = 7;
    const float totalWidth = 220.0f;
    const float segWidth = totalWidth / segments;

    const float leftStart = -totalWidth / 2.0f;

    // coborati mai jos
    const float bottomY = -4.0f;
    const float topY = 14.5f;

    // mai departe in spate
    const float z = -95.0f;

    for (int i = 0; i < segments; i++) {
        float x1 = leftStart + i * segWidth;
        float x2 = x1 + segWidth;

        // alternam textura normal / oglindit
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

    float minX = -60.0f, maxX = 60.0f;
    float minZ = -60.0f, maxZ = 45.0f;
    float step = 1.5f;

    for (float z = minZ; z < maxZ; z += step) {
        glBegin(GL_TRIANGLE_STRIP);

        for (float x = minX; x <= maxX; x += step) {
            float nx, ny, nz;

            float y1 = terrainHeight(x, z);
            computeNormal(x, z, nx, ny, nz);
            glNormal3f(nx, ny, nz);
            glTexCoord2f((x - minX) * 0.10f, (z - minZ) * 0.10f);
            glVertex3f(x, y1, z);

            float y2 = terrainHeight(x, z + step);
            computeNormal(x, z + step, nx, ny, nz);
            glNormal3f(nx, ny, nz);
            glTexCoord2f((x - minX) * 0.10f, (z + step - minZ) * 0.10f);
            glVertex3f(x, y2, z + step);
        }

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        camX, camY, camZ,
        lookX, lookY, lookZ,
        0.0f, 1.0f, 0.0f
    );

    setupLighting();

    drawSkybox();
    drawMountains();
    drawTerrain();

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

void keyboard(unsigned char key, int, int)
{
    switch (key) {
    case 27:
        freeTextures();
        std::exit(0);
        break;

    case 'w':
        camZ -= 1.2f;
        lookZ -= 1.2f;
        break;
    case 's':
        camZ += 1.2f;
        lookZ += 1.2f;
        break;
    case 'a':
        camX -= 1.2f;
        lookX -= 1.2f;
        break;
    case 'd':
        camX += 1.2f;
        lookX += 1.2f;
        break;
    case 'q':
        camY += 0.8f;
        lookY += 0.8f;
        break;
    case 'e':
        camY -= 0.8f;
        lookY -= 0.8f;
        break;
    }

    glutPostRedisplay();
}

void init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glShadeModel(GL_SMOOTH);

    glClearColor(0.60f, 0.82f, 1.00f, 1.0f);

  

    loadTextures();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 700);
    glutCreateWindow("Scena 3D");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}