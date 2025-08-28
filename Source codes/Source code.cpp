#define FREEGLUT_STATIC 
#define FREEGLUT_STATIC 
#include <GL/freeglut.h> 
#include <cstdlib>
#include <stdio.h> 
#include <vector>
#include <iostream>
#include <cmath>
#include "plane.h"
#include "stick.h"
#include "tower_upper.h"
#include "tower_bottom.h"
#include "fence.h"
#include "tree1.h"
#include "tree2.h"
#define M_PI 3.1415926

// Variables to control camera rotation
float cameraAngleX = 0.0f;
float cameraAngleY = 0.0f;
float cameraDistance = 25.0f;  // Distance from the center

std::vector<GLuint> textures;  // Vector to store multiple texture IDs

const float STICK_BASE_X = -2.1f;
const float STICK_BASE_Y = 4.2f;
const float STICK_LENGTH = 10.0f;

// Variables to control animation
float rotation_angle = 0.0f;
float phase_swing = 0.0f;
const float ROTATION_SPEED = 20.0f;  
const float SWING_SPEED = 2.0f;   
const float SWING_ANGLE_MAX = 10.0f; 
clock_t last_time = clock();  // Record the time when the program starts

// Variables for changing modes
bool isFirstPerson = false;
bool lightingEnabled = true;
int phaseDifferenceMode = 1;

// Record the information for each tree
struct Tree {
    float x;
    float z;
    int type; 
    float scale;
    float rotation;
};

std::vector<Tree> trees;  // Vector to store tree objects

void initializeTrees() {
    // Generate coordinates in a square grid
    for (float x = -35.0f; x <= 35.0f; x += 10.0f) {
        for (float z = -35.0f; z <= 35.0f; z += 10.0f) {
            // Avoid positions generated within the centeral area
            if (abs(x) < 20.0f && abs(z) < 20.0f) {
                continue;
            }

            // Generate random offsets to make the position more natural
            float xOffset = ((rand() % 7) - 3);
            float zOffset = ((rand() % 7) - 3);

            // Create a tree object and set its properties
            Tree tree;
            tree.x = x + xOffset;
            tree.z = z + zOffset;
            tree.type = rand() % 2;  // There are two kinds of trees, 0 for tree1 and 1 for tree2
            // Randomly generate a scale within a range
            tree.scale = 0.07f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (0.09f - 0.07f);
            tree.rotation = static_cast<float>(rand() % 360);   // Randomly generate a rotation angle

            // Push the object into the vector
            trees.push_back(tree);
        }
    }
}

// Read the bmp file and generate a texture id
GLuint LoadTexture(const char path[256]) {
    GLint imagewidth, imageheight, pixellength;
    GLubyte* pixeldata;
    FILE* pfile;

    const std::string baseFolder = "Textures/"; 
    std::string fullPath = baseFolder + path;
    fopen_s(&pfile, fullPath.c_str(), "rb");
    if (pfile == 0) {
        std::cerr << "Error: Unable to open file " << path << std::endl;
        exit(0);
    }

    fseek(pfile, 0x0012, SEEK_SET);
    fread(&imagewidth, sizeof(imagewidth), 1, pfile);
    fread(&imageheight, sizeof(imageheight), 1, pfile);

    pixellength = imagewidth * 3;
    while (pixellength % 4 != 0) pixellength++;
    pixellength *= imageheight;

    pixeldata = (GLubyte*)malloc(pixellength);
    if (pixeldata == 0) {
        std::cerr << "Error: Unable to allocate memory for image " << path << std::endl;
        fclose(pfile);
        exit(0);
    }

    fseek(pfile, 54, SEEK_SET);
    fread(pixeldata, pixellength, 1, pfile);
    fclose(pfile);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imagewidth, imageheight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixeldata);
    free(pixeldata);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set texture environment mode
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    return textureID;
}

void setupLighting() {
    glEnable(GL_LIGHTING); // Enable lighting
    glEnable(GL_LIGHT0); // Enable a light source

    // Define light properties
    GLfloat lightPosition[] = { 50.0f, 50.0f, 50.0f, 1.0f }; 
    GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };  // Ambient light make the scene more bright
  
    // Set the light properties
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
}

void setMaterial(int type) {
    if (type == 0) {
        // Define material properties for the airplane carousel
        GLfloat matAmbient[] = { 0.6f, 0.6f, 0.6f, 1.0f };   
        GLfloat matDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat matSpecular[] = { 0.8f, 0.8f, 0.8f, 1.0f }; 
        GLfloat matShininess[] = { 20.0f };                 
        // Set the material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    }else{
        // Define material properties for the environment objects
        GLfloat matAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
        GLfloat matDiffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f }; 
        GLfloat matSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 
        GLfloat matShininess[] = { 0.0f };            
        // Set the material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    }
}

void myinit(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);  // Use smooth shading for better lighting effect
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);

    // Enable face culling to render only front faces
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    initializeTrees();

    setupLighting();

    // Load multiple textures and add to vector
    textures.push_back(LoadTexture("tower_bottom_texture.bmp"));
    textures.push_back(LoadTexture("tower_upper_texture.bmp"));
    textures.push_back(LoadTexture("stick_texture.bmp"));
    textures.push_back(LoadTexture("plane_texture1.bmp"));
    textures.push_back(LoadTexture("plane_texture2.bmp"));

    textures.push_back(LoadTexture("fence_texture.bmp"));
    textures.push_back(LoadTexture("tree1_texture.bmp"));
    textures.push_back(LoadTexture("tree2_texture.bmp"));

    textures.push_back(LoadTexture("ground_texture.bmp"));
    textures.push_back(LoadTexture("sky_top_texture.bmp"));
    textures.push_back(LoadTexture("sky_side1_texture.bmp"));
    textures.push_back(LoadTexture("sky_side2_texture.bmp"));
    textures.push_back(LoadTexture("sky_side3_texture.bmp"));
    textures.push_back(LoadTexture("sky_side4_texture.bmp"));
}

void updateAnimation() {
    clock_t current_time = clock();
    float deltaTime = float(current_time - last_time) / CLOCKS_PER_SEC;
    last_time = current_time;
   
    rotation_angle += ROTATION_SPEED * deltaTime;
    if (rotation_angle > 360.0f) {
        rotation_angle -= 360.0f;
    }

    phase_swing += SWING_SPEED * deltaTime;
    if (phase_swing > 360.0f) {
        phase_swing -= 360.0f;
    }
}

// Draw model using the data in the header
void drawModel(const float vertices[][3], int vertCount, const int faces[][3][3], int faceCount,
    const float tex_coords[][2], const float normals[][3], GLuint textureID, float x, float y, float z, float scale) {

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);

    // Bind the texture to be used
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < faceCount; ++i) {
        for (int j = 0; j < 3; ++j) {
            int v = faces[i][j][0];  // Vertex index
            int vt = faces[i][j][1]; // Texture coordinate index
            int vn = faces[i][j][2]; // Normal index

            if (vn != -1) {  // If normal index is available, use it
                glNormal3f(normals[vn][0], normals[vn][1], normals[vn][2]);
            }
            if (vt != -1) {  // If texture coordinate is available
                glTexCoord2f(tex_coords[vt][0], tex_coords[vt][1]);
            }
            glVertex3f(vertices[v][0], vertices[v][1], vertices[v][2]);
        }
    }
    glEnd();
    glPopMatrix();
}

void drawSkybox() {
    // Disable lighting to make sure the skybox is rendered correctly
    glDisable(GL_LIGHTING);

    // Set skybox size
    float skyboxSize = 40.0f;

    // Draw skybox, bottom is at y = 0, faces inward
    // Bottom face
    glBindTexture(GL_TEXTURE_2D, textures[8]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-skyboxSize, 0.0f, -skyboxSize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(skyboxSize, 0.0f, -skyboxSize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(skyboxSize, 0.0f, skyboxSize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-skyboxSize, 0.0f, skyboxSize);
    glEnd();

    // Top face
    glBindTexture(GL_TEXTURE_2D, textures[9]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-skyboxSize, skyboxSize, -skyboxSize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(skyboxSize, skyboxSize, -skyboxSize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(skyboxSize, skyboxSize, skyboxSize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-skyboxSize, skyboxSize, skyboxSize);
    glEnd();

    // Front face
    glBindTexture(GL_TEXTURE_2D, textures[10]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-skyboxSize, 0.0f, skyboxSize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(skyboxSize, 0.0f, skyboxSize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(skyboxSize, skyboxSize, skyboxSize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-skyboxSize, skyboxSize, skyboxSize);
    glEnd();

    // Back face
    glBindTexture(GL_TEXTURE_2D, textures[11]); 
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-skyboxSize, 0.0f, -skyboxSize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-skyboxSize, skyboxSize, -skyboxSize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(skyboxSize, skyboxSize, -skyboxSize);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(skyboxSize, 0.0f, -skyboxSize);
    glEnd();

    // Left face
    glBindTexture(GL_TEXTURE_2D, textures[12]); 
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-skyboxSize, 0.0f, -skyboxSize);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-skyboxSize, 0.0f, skyboxSize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-skyboxSize, skyboxSize, skyboxSize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-skyboxSize, skyboxSize, -skyboxSize);
    glEnd();

    // Right face 
    glBindTexture(GL_TEXTURE_2D, textures[13]); 
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(skyboxSize, 0.0f, -skyboxSize);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(skyboxSize, skyboxSize, -skyboxSize);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(skyboxSize, skyboxSize, skyboxSize);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(skyboxSize, 0.0f, skyboxSize);
    glEnd();

    // Re-enable lighting after drawing the skybox
    if (lightingEnabled)
        glEnable(GL_LIGHTING);
}

void drawSurroundings() {
    setMaterial(1);

    // Fence
    drawModel(fence_vertices, sizeof(fence_vertices) / sizeof(fence_vertices[0]),
        fence_faces, sizeof(fence_faces) / sizeof(fence_faces[0]),
        fence_tex_coords, fence_normals, textures[5], 0, 0.0f, 0, 0.07f);

    // Trees
    for (const Tree& tree : trees) {
        glPushMatrix();
        glTranslatef(tree.x, 0.0f, tree.z);
        glRotatef(tree.rotation, 0.0f, 1.0f, 0.0f);
        if (tree.type == 0) {
            drawModel(tree1_vertices, sizeof(tree1_vertices) / sizeof(tree1_vertices[0]),
                tree1_faces, sizeof(tree1_faces) / sizeof(tree1_faces[0]),
                tree1_tex_coords, tree1_normals, textures[6], 0.0f, 0.0f, 0.0f, tree.scale);
        }
        else {
            drawModel(tree2_vertices, sizeof(tree2_vertices) / sizeof(tree2_vertices[0]),
                tree2_faces, sizeof(tree2_faces) / sizeof(tree2_faces[0]),
                tree2_tex_coords, tree2_normals, textures[7], 0.0f, 0.0f, 0.0f, tree.scale);
        }
        glPopMatrix();
    }
}

std::pair<float, float> CalculatePlanePosition(float phase_offset = 0.0f) {
    float swing_angle = SWING_ANGLE_MAX * sin(phase_swing + phase_offset);
    float radians = swing_angle * (M_PI / 180.0f);  // convert angle (degree) to radians
    // calculate the end position of the stick
    float x = STICK_BASE_X - STICK_LENGTH * cos(radians); // x position of stick end (negative x direction)
    float y = STICK_BASE_Y - STICK_LENGTH * sin(radians); // y position of stick end
    return std::make_pair(x, y);
}

void drawAirplaneCarousel() {
    updateAnimation();
    setMaterial(0);

    // Tower bottom (static)
    drawModel(tower_bottom_vertices, sizeof(tower_bottom_vertices) / sizeof(tower_bottom_vertices[0]),
        tower_bottom_faces, sizeof(tower_bottom_faces) / sizeof(tower_bottom_faces[0]),
        tower_bottom_tex_coords, tower_bottom_normals, textures[0], 0.0f, 0.0f, 0.0f, 0.05f);
    // Tower upper (rotating)
    glPushMatrix();
    glRotatef(rotation_angle, 0.0f, -1.0f, 0.0f); // rotate around y axis (including the sticks and planes)
    drawModel(tower_upper_vertices, sizeof(tower_upper_vertices) / sizeof(tower_upper_vertices[0]),
        tower_upper_faces, sizeof(tower_upper_faces) / sizeof(tower_upper_faces[0]),
        tower_upper_tex_coords, tower_upper_normals, textures[1], 0.0f, 0.0f, 0.0f, 0.05f);

    // Sticks and Planes
    for (int i = 0; i < 6; ++i) { 
        float angleOffset = i * 60.0f;  // Draw one every 60 degrees
        // Phase offset would change according to the mode
        float phaseOffset = i * (M_PI / phaseDifferenceMode); 
        float swingAngle = SWING_ANGLE_MAX * sin(phase_swing + phaseOffset);

        // Stick
        glPushMatrix();
        glRotatef(angleOffset, 0.0f, -1.0f, 0.0f); 
        glTranslatef(STICK_BASE_X, STICK_BASE_Y, 0.0f);  // move to the pivot
        glRotatef(swingAngle, 0.0f, 0.0f, 1.0f); // swing up and down
        drawModel(stick_vertices, sizeof(stick_vertices) / sizeof(stick_vertices[0]),
            stick_faces, sizeof(stick_faces) / sizeof(stick_faces[0]),
            stick_tex_coords, stick_normals, textures[2], 0.0f, 0.0f, 0.0f, 0.05f);
        glPopMatrix();

        // Plane
        std::pair<float, float> position = CalculatePlanePosition(phaseOffset);
        glPushMatrix();
        glRotatef(angleOffset, 0.0f, -1.0f, 0.0f);
        glTranslatef(position.first, position.second, 0.0f);  // draw plane on the end of the stick
        GLuint planeTexture = i % 2 == 0 ? textures[3] : textures[4];  // different color for adjacent planes
        drawModel(plane_vertices, sizeof(plane_vertices) / sizeof(plane_vertices[0]),
            plane_faces, sizeof(plane_faces) / sizeof(plane_faces[0]),
            plane_tex_coords, plane_normals, planeTexture, 0.0f, 0.0f, 0.0f, 0.05f);
        glPopMatrix();
    }
    glPopMatrix();
}

void setCamera() {
    glLoadIdentity();
    if (isFirstPerson) {
        // First person view
        std::pair<float, float> plane_position = CalculatePlanePosition();

        // calculate plane postion considering the rotation
        float rotationRadians = rotation_angle * (M_PI / 180.0f);
        float rotatedPlaneX = cos(rotationRadians) * plane_position.first;
        float rotatedPlaneZ = sin(rotationRadians) * plane_position.first;

        // Adjust camera position to create the view of sitting inside the plane
        float camX = rotatedPlaneX * 1.1f;
        float camY = plane_position.second + 2.5f; 
        float camZ = rotatedPlaneZ * 1.1f;

       // Combine rotation angle and camera angle 
       // so that the sight would move as rotation and can be controlled by keyboard
        float combinedAngleY = cameraAngleY + rotationRadians; 
        float lookAtX = camX + sinf(combinedAngleY) * cosf(cameraAngleX);
        float lookAtY = camY + sinf(cameraAngleX);
        float lookAtZ = camZ - cosf(combinedAngleY) * cosf(cameraAngleX);

        gluLookAt(camX, camY, camZ, lookAtX, lookAtY, lookAtZ, 0.0f, 1.0f, 0.0f); 
    }
    else {  
        // Third person view
        float initialY = 6.0f; 
        float groundMargin = 0.1f;    // margin to avoid looking below the ground

        float camX = cameraDistance * sinf(cameraAngleY) * cosf(cameraAngleX);
        float camY = cameraDistance * sinf(cameraAngleX) + initialY;
        float camZ = cameraDistance * cosf(cameraAngleY) * cosf(cameraAngleX);
        if (camY < 0){
            camY = 0;
            camX = cameraDistance * sinf(cameraAngleY);
            camZ = cameraDistance * cosf(cameraAngleY);
        }
        gluLookAt(camX, camY + groundMargin, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    setCamera();
    drawSkybox();
    drawSurroundings();
    drawAirplaneCarousel();

    glutSwapBuffers();
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': 
        cameraAngleX += 0.05f;  // Look up
        // Prevent angles from exceeding 90 degrees (with a small margin)
        if (cameraAngleX > M_PI/2 - 0.1f) { 
            cameraAngleX = M_PI/2 - 0.1f;
        }
        break;
    case 's':  
        cameraAngleX -= 0.05f;  // Look down
        // Prevent angles from exceeding -90 degrees (with a small margin)
        if (cameraAngleX < -M_PI/2 + 0.1f) { 
            cameraAngleX = -M_PI/2 + 0.1f;
        }
        break;
    case 'a': 
        cameraAngleY -= 0.05f;  // Turn left
        break;
    case 'd': 
        cameraAngleY += 0.05f;  // Turn right
        break;
    case 'q':
        cameraDistance -= 0.5f; // Zoom in
        // Prevent going inside the center model
        if (cameraDistance < 2.0f)
            cameraDistance = 2.0f;
        break;
    case 'e':
        cameraDistance += 0.5f; // Zoom out
        // Prevent going out of the skybox
        if (cameraDistance > 40.0f)
            cameraDistance = 40.0f;
        break;
    case 'f':
        isFirstPerson = true;
        // Reset the camera
        cameraAngleX = 0.0f; 
        cameraAngleY = 0.0f;
        break;
    case 't':
        isFirstPerson = false;
        // Reset the camera
        cameraAngleX = 0.0f; 
        cameraAngleY = 0.0f;
        cameraDistance = 25.0f;
        break;
    case 'p': 
        phaseDifferenceMode++;
        if (phaseDifferenceMode > 3) {
            phaseDifferenceMode = 1;  // switch between 1,2,3
        }
        break;
    case 'l':
        lightingEnabled = !lightingEnabled;
        break;
    case 27:
        exit(0);  // Escape for exit
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void menu(int item) {
    keyboard((unsigned int)item, 0, 0);
}

void createMenu() {
    // Create submenu for selecting view mode
    int viewMenu = glutCreateMenu(menu);
    glutAddMenuEntry("First Person (f)", 'f');
    glutAddMenuEntry("Third Person (t)", 't');

    // Create main menu
    int menuId = glutCreateMenu(menu);
    glutAddMenuEntry("     === Control Menu ===", '\0');
    glutAddMenuEntry("                         ", '\0');
    glutAddSubMenu("Select View", viewMenu);
    glutAddMenuEntry("Lighting On/Off (l)", 'l');
    glutAddMenuEntry("Change Plane Swing Pattern (p)", 'p');
    glutAddMenuEntry("Quit (Esc)", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void myReshape(GLsizei w, GLsizei h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0 * (GLfloat)w / (GLfloat)h, 0.1, 300.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1280,720);
    glutIdleFunc(display);
    glutCreateWindow("Airplane Carousel Scene");
   
    myinit();
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    createMenu();
    glutMainLoop();
}
