/************************************************************
 * Handout: rotate-cube-new.cpp (A Sample Code for Shader-Based OpenGL ---
                                 for OpenGL version 3.1 and later)
 * Originally from Ed Angel's textbook "Interactive Computer Graphics" 6th Ed
              sample code "example3.cpp" of Chapter 4.
 * Moodified by Yi-Jen Chiang to include the use of a general rotation function
   Rotate(angle, x, y, z), where the vector (x, y, z) can have length != 1.0,
   and also to include the use of the function NormalMatrix(mv) to return the
   normal matrix (mat3) of a given model-view matrix mv (mat4).

   (The functions Rotate() and NormalMatrix() are added to the file "mat-yjc-new.h"
   by Yi-Jen Chiang, where a new and correct transpose function "transpose1()" and
   other related functions such as inverse(m) for the inverse of 3x3 matrix m are
   also added; see the file "mat-yjc-new.h".)

 * Extensively modified by Yi-Jen Chiang for the program structure and user
   interactions. See the function keyboard() for the keyboard actions.
   Also extensively re-structured by Yi-Jen Chiang to create and use the new
   function drawObj() so that it is easier to draw multiple objects. Now a floor
   and a rotating cube are drawn.

** Perspective view of a color cube using LookAt() and Perspective()

** Colors are assigned to each vertex and then the rasterizer interpolates
   those colors across the triangles.
**************************************************************/
#include "Angel-yjc.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define PI 3.14159

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;       /* shader program object id */

// Vertex Buffer Objects
GLuint cube_buffery;    /* vertex buffer object id for y axis */
GLuint floor_buffer;    /* vertex buffer object id for floor */
GLuint cube_bufferx;    /* vertex buffer object id for x axis */
GLuint cube_bufferz;    /* vertex buffer object id for z axis */
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint shadow_buffer;

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 50.0;

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye; // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation.
int beginFlag = 0; // flag to begin rolling

int cubeFlag = 1;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'

const int cube_NumVertices = 36; // (6 faces)*(2 triangles/face)*(3 vertices/triangle)
int sphere_NumVertices = 0; // will be changed after reading file input

// flags for rolling segments
bool flagAB = true;
bool flagBC = false;
bool flagCA = false;

// flags for menu options
bool flagPointSourceLight = false;
bool flagSpotlightLight = true;
bool flagShadow = true;
bool flagWireframe = false;
bool flagLighting = true;
int fogFlag = 0; // 0: no fog; 1: linear fog; 2: exponential fog; 3: exponential square fog
bool shadowblendFlag = true;
int floortextureFlag = 1;
int verticalFlag = 0;
int eyeFlag = 0;
int sphereCheckerFlag = 1;
int spheretextureFlag = 1;

// matrix M
mat4 totalRotation(vec4(1.0, 0.0, 0.0, 0.0),
                   vec4(0.0, 1.0, 0.0, 0.0),
                   vec4(0.0, 0.0, 1.0, 0.0),
                   vec4(0.0, 0.0, 0.0, 1.0));

// shadow matrix
mat4 shadowMat(vec4(12.0,  0.0,  0.0, 182.0),
               vec4( 0.0,  0.0,  0.0,   0.0),
               vec4( 0.0,  0.0, 12.0,  39.0),
               vec4( 0.0, -1.0,  0.0,  12.0));

// y axis
#if 0
point3 cube_pointsy[cube_NumVertices]; // positions for all vertices
vec3 cube_colorsy[cube_NumVertices]; // colors for all vertices
#endif
#if 1
point3 cube_pointsy[100];
vec3 cube_colorsy[100];
#endif

// x axis
point3 cube_pointsx[100];
vec3 cube_colorsx[100];

// z axis
point3 cube_pointsz[100];
vec3 cube_colorsz[100];

// floor
const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
vec3 floor_normals[floor_NumVertices];

// sphere
point3 sphere_points[1000000];
vec3 sphere_normals_flat[1000000];
vec3 sphere_normals_smooth[1000000];

#define ImageWidth  32
#define ImageHeight 32
GLubyte Image[ImageHeight][ImageWidth][4];

#define    stripeImageWidth 32
GLubyte stripeImage[4*stripeImageWidth];

static GLuint texName;

using namespace std;

// Vertices of y axis
point3 verticesy[8] = {
    point3( -0.05,  0.0,  0.05),
    point3( -0.05, 50.0,  0.05),
    point3(  0.05, 50.0,  0.05),
    point3(  0.05,  0.0,  0.05),
    point3( -0.05,  0.0, -0.05),
    point3( -0.05, 50.0, -0.05),
    point3(  0.05, 50.0, -0.05),
    point3(  0.05,  0.0, -0.05)
};

// Vertices of x axis
point3 verticesx[8] = {
    point3(   0.0, -0.05,  0.05),
    point3(   0.0,  0.05,  0.05),
    point3(  50.0,  0.05,  0.05),
    point3(  50.0, -0.05,  0.05),
    point3(   0.0, -0.05, -0.05),
    point3(   0.0,  0.05, -0.05),
    point3(  50.0,  0.05, -0.05),
    point3(  50.0, -0.05, -0.05)
};

// Vertices of z axis
point3 verticesz[8] = {
    point3( -0.05, -0.05,  50.0),
    point3( -0.05,  0.05,  50.0),
    point3(  0.05,  0.05,  50.0),
    point3(  0.05, -0.05,  50.0),
    point3( -0.05, -0.05,  -0.0),
    point3( -0.05,  0.05,  -0.0),
    point3(  0.05,  0.05,  -0.0),
    point3(  0.05, -0.05,  -0.0)
};

// RGBA colors
color3 vertex_colors[8] = {
    color3( 0.0, 0.0, 0.0),  // black
    color3( 1.0, 0.0, 0.0),  // red
    color3( 1.0, 1.0, 0.0),  // yellow
    color3( 0.0, 1.0, 0.0),  // green
    color3( 0.0, 0.0, 1.0),  // blue
    color3( 1.0, 0.0, 1.0),  // magenta
    color3( 1.0, 1.0, 1.0),  // white
    color3( 0.0, 1.0, 1.0)   // cyan
};

// Vertices of floor
point3 floor_vertices[4] = {
    point3(  5.0,  0.0,  8.0),
    point3(  5.0,  0.0, -4.0),
    point3( -5.0,  0.0, -4.0),
    point3( -5.0,  0.0,  8.0)
};

vec2 floor_texCoord[6] = {
    vec2(0.0, 0.0),
    vec2(0.0, 5.0),
    vec2(6.0, 5.0),
    vec2(6.0, 5.0),
    vec2(6.0, 0.0),
    vec2(0.0, 0.0)
};

// Model-view and projection matrices uniform location
GLuint  model_view, projection;

/*----- Shader Lighting Parameters -----*/

color4 global_light_ambient( 1.0, 1.0, 1.0, 1.0);

color4 directional_light_ambient( 0.0, 0.0, 0.0, 1.0 );
color4 directional_light_diffuse( 0.8, 0.8, 0.8, 1.0 );
color4 directional_light_specular( 0.2, 0.2, 0.2, 1.0 );
vec4 directional_light_direction( 0.1, 0.0, -1.0, 0.0 );

color4 positional_light_ambient( 0.0, 0.0, 0.0, 1.0 );
color4 positional_light_diffuse( 1.0, 1.0, 1.0, 1.0 );
color4 positional_light_specular( 1.0, 1.0, 1.0, 1.0 );
point4 light_position( -14.0, 12.0, -3.0, 1.0 );

float const_att = 2.0;
float linear_att = 0.01;
float quad_att = 0.001;

point4 spotlight_pointat( -6.0, 0.0, -4.5, 1.0 );
vec4 spotlight_direction = spotlight_pointat - light_position;
float exp_val = 15.0;
float cutoff_angle = 20.0 * PI / 180.0;

color4 floor_material_ambient( 0.2, 0.2, 0.2, 1.0 );
color4 floor_material_diffuse( 0.0, 1.0, 0.0, 1.0 );
color4 floor_material_specular( 0.0, 0.0, 0.0, 1.0 );
float  floor_material_shininess = 100.0;

color4 sphere_material_ambient( 0.2, 0.2, 0.2, 1.0 );
color4 sphere_material_diffuse( 1.0, 0.84, 0.0, 1.0 );
color4 sphere_material_specular( 1.0, 0.84, 0.0, 1.0 );
float  sphere_material_shininess = 125.0;

void SetUp_Lighting_Uniform_Vars(mat4 mv, string object);

//----------------------------------------------------------------------------
int Index = 0; // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

// quad(): generate two triangles for each face and assign colors to the vertices
void quad( int a, int b, int c, int d, color3 cube_colors[], point3 cube_points[], const point3 vertices[], int color)
{
    cube_colors[Index] = vertex_colors[color]; cube_points[Index] = vertices[a]; Index++;
    cube_colors[Index] = vertex_colors[color]; cube_points[Index] = vertices[b]; Index++;
    cube_colors[Index] = vertex_colors[color]; cube_points[Index] = vertices[c]; Index++;

    cube_colors[Index] = vertex_colors[color]; cube_points[Index] = vertices[c]; Index++;
    cube_colors[Index] = vertex_colors[color]; cube_points[Index] = vertices[d]; Index++;
    cube_colors[Index] = vertex_colors[color]; cube_points[Index] = vertices[a]; Index++;
}
//----------------------------------------------------------------------------
// generate 12 triangles: 36 vertices and 36 colors
void colorcube(color3 cube_colors[], point3 cube_points[], const point3 vertices[], int color)
{
    quad( 1, 0, 3, 2, cube_colors, cube_points, vertices, color);
    quad( 2, 3, 7, 6, cube_colors, cube_points, vertices, color);
    quad( 3, 0, 4, 7, cube_colors, cube_points, vertices, color);
    quad( 6, 5, 1, 2, cube_colors, cube_points, vertices, color);
    quad( 4, 5, 6, 7, cube_colors, cube_points, vertices, color);
    quad( 5, 4, 0, 1, cube_colors, cube_points, vertices, color);
    Index = 0;
}
//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
    floor_normals[0] = vec3(0.0, 1.0, 0.0); floor_points[0] = floor_vertices[0];
    floor_normals[1] = vec3(0.0, 1.0, 0.0); floor_points[1] = floor_vertices[3];
    floor_normals[2] = vec3(0.0, 1.0, 0.0); floor_points[2] = floor_vertices[2];

    floor_normals[3] = vec3(0.0, 1.0, 0.0); floor_points[3] = floor_vertices[2];
    floor_normals[4] = vec3(0.0, 1.0, 0.0); floor_points[4] = floor_vertices[1];
    floor_normals[5] = vec3(0.0, 1.0, 0.0); floor_points[5] = floor_vertices[0];
}

void setspherenormals() {
    for (int i = 0; i < sphere_NumVertices; i+=3) {
        vec4 u = sphere_points[i+1] - sphere_points[i];
        vec4 v = sphere_points[i+2] - sphere_points[i];
        
        vec3 normal = normalize( cross(u, v) );
        
        sphere_normals_flat[i] = normal;
        sphere_normals_flat[i+1] = normal;
        sphere_normals_flat[i+2] = normal;
        
        sphere_normals_smooth[i] = sphere_points[i];
        sphere_normals_smooth[i+1] = sphere_points[i+1];
        sphere_normals_smooth[i+2] = sphere_points[i+2];
    }
}

void image_set_up(void)
{
 int i, j, c;
 
 /* --- Generate checkerboard image to the image array ---*/
  for (i = 0; i < ImageHeight; i++)
    for (j = 0; j < ImageWidth; j++)
      {
       c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

       if (c == 1) /* white */
    {
         c = 255;
     Image[i][j][0] = (GLubyte) c;
         Image[i][j][1] = (GLubyte) c;
         Image[i][j][2] = (GLubyte) c;
    }
       else  /* green */
    {
         Image[i][j][0] = (GLubyte) 0;
         Image[i][j][1] = (GLubyte) 150;
         Image[i][j][2] = (GLubyte) 0;
    }

       Image[i][j][3] = (GLubyte) 255;
      }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

/*--- Generate 1D stripe image to array stripeImage[] ---*/
  for (j = 0; j < stripeImageWidth; j++) {
     /* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
        When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
      */
    stripeImage[4*j] = (GLubyte)    255;
    stripeImage[4*j+1] = (GLubyte) ((j>4) ? 255 : 0);
    stripeImage[4*j+2] = (GLubyte) 0;
    stripeImage[4*j+3] = (GLubyte) 255;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
/*----------- End 1D stripe image ----------------*/

/*--- texture mapping set-up is to be done in
      init() (set up texture objects),
      display() (activate the texture object to be used, etc.)
      and in shaders.
 ---*/

} /* end function */

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    colorcube(cube_colorsy, cube_pointsy, verticesy, 5);

#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
#endif

    // Create and initialize a vertex buffer object for y axis, to be used in display()
    glGenBuffers(1, &cube_buffery);
    glBindBuffer(GL_ARRAY_BUFFER, cube_buffery);

#if 0
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_pointsy) + sizeof(cube_colorsy),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_pointsy), cube_pointsy);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_pointsy), sizeof(cube_colorsy),
                    cube_colorsy);
#endif
#if 1
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point3)*cube_NumVertices + sizeof(vec3)*cube_NumVertices,
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(point3) * cube_NumVertices, cube_pointsy);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point3) * cube_NumVertices,
                    sizeof(vec3) * cube_NumVertices,
                    cube_colorsy);
#endif

    colorcube(cube_colorsx, cube_pointsx, verticesx, 1);

    // Create and initialize a vertex buffer object for x axis, to be used in display()
    glGenBuffers(1, &cube_bufferx);
    glBindBuffer(GL_ARRAY_BUFFER, cube_bufferx);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point3)*cube_NumVertices + sizeof(vec3)*cube_NumVertices,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(point3) * cube_NumVertices, cube_pointsx);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point3) * cube_NumVertices,
                    sizeof(vec3) * cube_NumVertices,
                    cube_colorsx);

    colorcube(cube_colorsz, cube_pointsz, verticesz, 4);

    // Create and initialize a vertex buffer object for z axis, to be used in display()
    glGenBuffers(1, &cube_bufferz);
    glBindBuffer(GL_ARRAY_BUFFER, cube_bufferz);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point3)*cube_NumVertices + sizeof(vec3)*cube_NumVertices,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(point3) * cube_NumVertices, cube_pointsz);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point3) * cube_NumVertices,
                    sizeof(vec3) * cube_NumVertices,
                    cube_colorsz);

    setspherenormals();

    // Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point3)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(point3) * sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point3) * sphere_NumVertices,
                    sizeof(vec3) * sphere_NumVertices,
                    sphere_normals_smooth);

    // Create and initialize a vertex buffer object for shadow, to be used in display()
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(point3)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
         NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(point3) * sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER,
                    sizeof(point3) * sphere_NumVertices,
                    sizeof(vec3) * sphere_NumVertices,
                    sphere_normals_smooth);
    
    image_set_up();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    /*--- Create and Initialize a texture object ---*/
    glGenTextures(1, &texName);      // Generate texture obj name(s)

    glActiveTexture( GL_TEXTURE0 );  // Set the active texture unit to be 0
    glBindTexture(GL_TEXTURE_2D, texName); // Bind the texture to this texture unit

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);
    
    glActiveTexture( GL_TEXTURE1 );  // Set the active texture unit to be 1
    glBindTexture(GL_TEXTURE_1D, texName); // Bind the texture to this texture unit

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
    
    floor();     
    // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_normals) + sizeof(floor_texCoord),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_normals),
                    floor_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_normals), sizeof(floor_texCoord),
                    floor_texCoord);
    
    
 // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader42.glsl", "fshader42.glsl");
    
    glEnable( GL_DEPTH_TEST );
    glClearColor(0.529, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
}


void SetUp_Lighting_Uniform_Vars(mat4 mv, string object)
{
    color4 global_ambient_product;
    color4 positional_ambient_product;
    color4 positional_diffuse_product;
    color4 positional_specular_product;
    color4 directional_ambient_product;
    color4 directional_diffuse_product;
    color4 directional_specular_product;
    float material_shininess;
    
    bool isSphere = false;
    bool isFloor = false;
    bool isAxisX = false;
    bool isAxisY = false;
    bool isAxisZ = false;
    bool isShadow = false;
    
    if (object == "sphere") {
        isSphere = true;
        global_ambient_product = global_light_ambient * sphere_material_ambient;
        directional_ambient_product = directional_light_ambient * sphere_material_ambient;
        directional_diffuse_product = directional_light_diffuse * sphere_material_diffuse;
        directional_specular_product = directional_light_specular * sphere_material_specular;
        material_shininess = sphere_material_shininess;
    }
    
    if (object == "floor") {
        isFloor = true;
        global_ambient_product = global_light_ambient * floor_material_ambient;
        directional_ambient_product = directional_light_ambient * floor_material_ambient;
        directional_diffuse_product = directional_light_diffuse * floor_material_diffuse;
        directional_specular_product = directional_light_specular * floor_material_specular;
        material_shininess = floor_material_shininess;
    }
    
    if (object == "sphere" && (flagPointSourceLight || flagSpotlightLight)) {
        isSphere = true;
        global_ambient_product = global_light_ambient * sphere_material_ambient;
        positional_ambient_product = positional_light_ambient * sphere_material_ambient;
        positional_diffuse_product = positional_light_diffuse * sphere_material_diffuse;
        positional_specular_product = positional_light_specular * sphere_material_specular;
        material_shininess = sphere_material_shininess;
    }
    
    if (object == "floor" && (flagPointSourceLight || flagSpotlightLight)) {
        isFloor = true;
        global_ambient_product = global_light_ambient * floor_material_ambient;
        positional_ambient_product = positional_light_ambient * floor_material_ambient;
        positional_diffuse_product = positional_light_diffuse * floor_material_diffuse;
        positional_specular_product = positional_light_specular * floor_material_specular;
        material_shininess = floor_material_shininess;
    }
    
    if (object == "xaxis") {
        isAxisX = true;
    }
    
    if (object == "yaxis") {
        isAxisY = true;
    }
    
    if (object == "zaxis") {
        isAxisZ = true;
    }
    
    if (object == "shadow") {
        isShadow = true;
    }
    
    // link variables
    
    glUniform4fv( glGetUniformLocation(program, "GlobalAmbientProduct"),
          1, global_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "PositionalAmbientProduct"),
          1, positional_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "PositionalDiffuseProduct"),
          1, positional_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "PositionalSpecularProduct"),
          1, positional_specular_product );
    glUniform4fv( glGetUniformLocation(program, "DirectionalAmbientProduct"),
          1, directional_ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DirectionalDiffuseProduct"),
          1, directional_diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "DirectionalSpecularProduct"),
          1, directional_specular_product );

    glUniform4fv( glGetUniformLocation(program, "SpotLightDirection"),
             1, mv * spotlight_direction);
    
    glUniform4fv( glGetUniformLocation(program, "DirectionalLightDirection"),
             1, directional_light_direction);
    
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
             1, mv * light_position);
    
    glUniform1f(glGetUniformLocation(program, "Shininess"),
                material_shininess );
    
    glUniform1f(glGetUniformLocation(program, "isSphere"),
                isSphere );
    glUniform1f(glGetUniformLocation(program, "isFloor"),
                isFloor );
    glUniform1f(glGetUniformLocation(program, "isAxisX"),
                isAxisX );
    glUniform1f(glGetUniformLocation(program, "isAxisY"),
                isAxisY );
    glUniform1f(glGetUniformLocation(program, "isAxisZ"),
                isAxisZ );
    glUniform1f(glGetUniformLocation(program, "isShadow"),
                isShadow );
    glUniform1f(glGetUniformLocation(program, "isWireframe"),
                flagWireframe );
    glUniform1f(glGetUniformLocation(program, "isLighting"),
                flagLighting );
    
    glUniform1f(glGetUniformLocation(program, "ConstAtt"),
                const_att);
    glUniform1f(glGetUniformLocation(program, "LinearAtt"),
                linear_att);
    glUniform1f(glGetUniformLocation(program, "QuadAtt"),
                quad_att);
    
    glUniform1f(glGetUniformLocation(program, "ExpVal"),
                exp_val);
    glUniform1f(glGetUniformLocation(program, "CutoffAngle"),
                cutoff_angle);
    
    glUniform1f(glGetUniformLocation(program, "isPointSource"),
                flagPointSourceLight);
    glUniform1f(glGetUniformLocation(program, "isSpotlight"),
                flagSpotlightLight);
    
    glUniform1i(glGetUniformLocation(program, "fogFlagIn"),
                fogFlag);
    
    glUniform1i(glGetUniformLocation(program, "floortextureFlag"),
                floortextureFlag);
    
    glUniform1i(glGetUniformLocation(program, "verticalFlag"),
                verticalFlag);
    
    glUniform1i(glGetUniformLocation(program, "eyeFlag"),
                eyeFlag);
    
    glUniform1i(glGetUniformLocation(program, "sphereCheckerFlag"),
                sphereCheckerFlag);
    
    glUniform1i(glGetUniformLocation(program, "spheretextureFlag"),
                spheretextureFlag);
    
}

//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices, bool usesTexture)
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 3, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
               BUFFER_OFFSET(sizeof(point3) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    GLuint vTexCoord;
    if (usesTexture) {
        GLuint vTexCoord = glGetAttribLocation( program, "vTexCoord" );
        glEnableVertexAttribArray( vTexCoord );
        glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
                   BUFFER_OFFSET(sizeof(point3) * num_vertices * 2));
    }
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
    if (usesTexture) {
        glDisableVertexAttribArray(vTexCoord);
    }
}
//----------------------------------------------------------------------------
void display( void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );

/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);
    
    mat4 sphereMat;
    mat4 mv;
    
    glUniform1i( glGetUniformLocation(program, "texture_2D"), 0 );
    glUniform1i( glGetUniformLocation(program, "texture_1D"), 1 );
    
    glDepthMask(GL_FALSE);
    
    // floor
    
    mv = LookAt(eye, at, up);
    
    SetUp_Lighting_Uniform_Vars(mv, "floor");
    
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    mat3 normal_matrix = NormalMatrix(mv, 0);
    glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
               1, GL_TRUE, normal_matrix );
    
    if (floorFlag == 1) // Filled floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(floor_buffer, floor_NumVertices, true);  // draw the floor

    // for rolling segment AB

    if (flagAB) {
        vec4 A(-4.0, 1.0, 4.0, 1.0);
        vec4 B(3.0, 1.0, -4.0, 1.0);
        vec4 AB = B - A;
        vec4 rotAxVec = cross(up, AB);

        vec4 translationVec = normalize(AB);
        float d = angle * 2.0 * PI / 360.0;
        translationVec *= d;
        translationVec += A;

        sphereMat = Translate(translationVec) * Rotate(angle, rotAxVec.x, rotAxVec.y, rotAxVec.z) * totalRotation;

        if (d > length(AB)) {
            flagAB = false;
            flagBC = true;
            totalRotation = Rotate(angle, rotAxVec.x, rotAxVec.y, rotAxVec.z) * totalRotation;
            angle = 0;
        }
    }

    // for rolling segment BC

    if (flagBC) {
        vec4 B( 3.0, 1.0, -4.0, 1.0);
        vec4 C(-3.0, 1.0, -3.0, 1.0);
        vec4 BC = C - B;
        vec4 rotAxVec = cross(up, BC);

        vec4 translationVec = normalize(BC);
        float d = angle * 2.0 * PI / 360.0;
        translationVec *= d;
        translationVec += B;

        sphereMat = Translate(translationVec) * Rotate(angle, rotAxVec.x, rotAxVec.y, rotAxVec.z) * totalRotation;

        if (d > length(BC)) {
            flagBC = false;
            flagCA = true;
            totalRotation = Rotate(angle, rotAxVec.x, rotAxVec.y, rotAxVec.z) * totalRotation;
            angle = 0;
        }
    }

    // for rolling segment CA

    if (flagCA) {
        vec4 C(-3.0, 1.0, -3.0, 1.0);
        vec4 A(-4.0, 1.0, 4.0, 1.0);
        vec4 CA = A - C;
        vec4 rotAxVec = cross(up, CA);

        vec4 translationVec = normalize(CA);
        float d = angle * 2.0 * PI / 360.0;
        translationVec *= d;
        translationVec += C;

        sphereMat = Translate(translationVec) * Rotate(angle, rotAxVec.x, rotAxVec.y, rotAxVec.z) * totalRotation;

        if (d > length(CA)) {
            flagCA = false;
            flagAB = true;
            totalRotation = Rotate(angle, rotAxVec.x, rotAxVec.y, rotAxVec.z) * totalRotation;
            angle = 0;
        }
    }

    // shadow
    
    if (flagShadow) {
        
        if (shadowblendFlag) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        SetUp_Lighting_Uniform_Vars(mv, "shadow");

        mv = LookAt(eye, at, up) * Translate(-15.5, 0, -3) * shadowMat * sphereMat;

        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
        normal_matrix = NormalMatrix(mv, 0);
        glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
                   1, GL_TRUE, normal_matrix );
        
        if (flagWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        drawObj(shadow_buffer, sphere_NumVertices, false);  // draw the shadow
        
        if (shadowblendFlag) {
            glDisable(GL_BLEND);
        }
            
    }
    
    glDepthMask(GL_TRUE);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // floor
    
    mv = LookAt(eye, at, up);

    SetUp_Lighting_Uniform_Vars(mv, "floor");

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    normal_matrix = NormalMatrix(mv, 0);
    glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
               1, GL_TRUE, normal_matrix );

    if (floorFlag == 1) // Filled floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(floor_buffer, floor_NumVertices, true);  // draw the floor

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // sphere
    
    SetUp_Lighting_Uniform_Vars(mv, "sphere");

    mv = LookAt(eye, at, up) * sphereMat;

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    normal_matrix = NormalMatrix(mv, 0);
    glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
               1, GL_TRUE, normal_matrix );

    if (flagWireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    drawObj(sphere_buffer, sphere_NumVertices, false);  // draw the sphere

    // axes
    
    mv = LookAt(eye, at, up);

    SetUp_Lighting_Uniform_Vars(mv, "yaxis");

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    normal_matrix = NormalMatrix(mv, 0);
    glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
               1, GL_TRUE, normal_matrix );

    if (cubeFlag == 1) // Filled cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(cube_buffery, cube_NumVertices, false);  // draw the y axis

    SetUp_Lighting_Uniform_Vars(mv, "xaxis");

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    normal_matrix = NormalMatrix(mv, 0);
    glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
               1, GL_TRUE, normal_matrix );

    if (cubeFlag == 1) // Filled cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(cube_bufferx, cube_NumVertices, false);  // draw the x axis

    SetUp_Lighting_Uniform_Vars(mv, "zaxis");

    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    normal_matrix = NormalMatrix(mv, 0);
    glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
               1, GL_TRUE, normal_matrix );

    if (cubeFlag == 1) // Filled cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(cube_bufferz, cube_NumVertices, false);  // draw the z axis
    
    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
    //angle += 0.02;
    angle += 2.0;    //YJC: change this value to adjust the sphere rotation speed.
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;

        case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

        case 'b': case 'B': // Begin rolling
            if (beginFlag == 0)  {
                beginFlag = 1;
                animationFlag = 1;
                glutIdleFunc(idle);
            }
            break;

	case ' ':  // reset to initial viewer/eye position
	    eye = init_eye;
	    break;
            
    case 'v': verticalFlag = 1; break;
    case 'V': verticalFlag = 1; break;
    
    case 's': verticalFlag = 0; break;
    case 'S': verticalFlag = 0; break;
            
    case 'o': eyeFlag = 0; break;
    case 'O': eyeFlag = 0; break;
    
    case 'e': eyeFlag = 1; break;
    case 'E': eyeFlag = 1; break;
    }
    
    glutPostRedisplay();
}

void myMouse(int button, int state, int x, int y) {
    
    // if right mouse button pressed, toggle animation
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && beginFlag) {
        animationFlag = 1 - animationFlag;
        if (animationFlag == 1) glutIdleFunc(idle);
        else                    glutIdleFunc(NULL);
    }
}

void menu(int id) {
    switch(id) {
            
        // reset viewer position
        case 1:
            eye = init_eye;
            animationFlag = 1;
            beginFlag = 1;
            glutIdleFunc(idle);
            break;
            
        // quit
        case 2:
            exit(0);
            break;
            
        case 3:
            flagWireframe = true;
            break;
    }
    glutPostRedisplay();
}

void shadow_menu(int id) {
    switch(id) {
            
        case 1:
            flagShadow = true;
            break;
            
        case 2:
            flagShadow = false;
            break;
    }
    glutPostRedisplay();
}

void lighting_menu(int id) {
    switch(id) {
            
        case 1:
            flagLighting = true;
            break;
            
        case 2:
            flagLighting = false;
            break;
    }
    glutPostRedisplay();
}

void shading_menu(int id) {
    switch(id) {
            
        case 1:
            flagWireframe = false;
            glGenBuffers(1, &sphere_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);

            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(point3)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
                 NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                            sizeof(point3) * sphere_NumVertices, sphere_points);
            glBufferSubData(GL_ARRAY_BUFFER,
                            sizeof(point3) * sphere_NumVertices,
                            sizeof(vec3) * sphere_NumVertices,
                            sphere_normals_flat);
            break;
            
        
        case 2:
            flagWireframe = false;
            glGenBuffers(1, &sphere_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);

            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(point3)*sphere_NumVertices + sizeof(vec3)*sphere_NumVertices,
                 NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                            sizeof(point3) * sphere_NumVertices, sphere_points);
            glBufferSubData(GL_ARRAY_BUFFER,
                            sizeof(point3) * sphere_NumVertices,
                            sizeof(vec3) * sphere_NumVertices,
                            sphere_normals_smooth);
            break;
    }
    glutPostRedisplay();
}

void lightsource_menu(int id) {
    switch(id) {
            
        case 1:
            flagSpotlightLight = true;
            flagPointSourceLight = false;
            break;
            
        
        case 2:
            flagSpotlightLight = false;
            flagPointSourceLight = true;
            break;
    }
    glutPostRedisplay();
}

void fog_menu(int id) {
    switch(id) {
            
        case 1:
            fogFlag = 0;
            break;
        
        case 2:
            fogFlag = 1;
            break;
            
        case 3:
            fogFlag = 2;
            break;
            
        case 4:
            fogFlag = 3;
            break;
    }
    glutPostRedisplay();
}

void shadowblend_menu(int id) {
    switch(id) {
            
        case 1:
            shadowblendFlag = true;
            break;
            
        
        case 2:
            shadowblendFlag = false;
            break;
    }
    glutPostRedisplay();
}

void floortexture_menu(int id) {
    switch(id) {
            
        case 1:
            floortextureFlag = 1;
            break;
            
        
        case 2:
            floortextureFlag = 0;
            break;
    }
    glutPostRedisplay();
}

void spheretexture_menu(int id) {
    switch(id) {
            
        case 1:
            spheretextureFlag = 1;
            sphereCheckerFlag = 1;
            break;
            
        
        case 2:
            spheretextureFlag = 1;
            sphereCheckerFlag = 0;
            break;
            
        case 3:
            spheretextureFlag = 0;
            break;
    }
    glutPostRedisplay();
}

//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void fileinput()
{
    //try to open file
    cout << "Enter filename: ";
    string filename;
    cin >> filename;
    ifstream ifs(filename);
    if (!ifs) {
        cout << "Failed to open file" << endl;
        exit(1);
    }
    
    //read data into vector
    int num_triangles;
    ifs >> num_triangles;
    
    int num_vertices;
    float x,y,z;
    int index = 0;
    for (int i = 0; i < num_triangles; i++) {
        ifs >> num_vertices;
        for (int j = 0; j < num_vertices; j++) {
            ifs >> x >> y >> z;
            sphere_points[index].x = x;
            sphere_points[index].y = y;
            sphere_points[index].z = z;
            index++;
        }
    }
    
    sphere_NumVertices = index;
    
}
//----------------------------------------------------------------------------
int main( int argc, char **argv )
{
    glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(512, 512);
    glutCreateWindow("Color Cube");

#ifdef __APPLE__ // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err)
    { 
        printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
        exit(1);
    }
#endif

    fileinput();
    
    // Get info of GPU and supported OpenGL version
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(myMouse);
    
    
    // submenus
    int shadow_submenu = glutCreateMenu(shadow_menu);
    glutAddMenuEntry("Yes", 1);
    glutAddMenuEntry("No", 2);
    
    int lighting_submenu = glutCreateMenu(lighting_menu);
    glutAddMenuEntry("Yes", 1);
    glutAddMenuEntry("No", 2);
    
    int shading_submenu = glutCreateMenu(shading_menu);
    glutAddMenuEntry("flat shading", 1);
    glutAddMenuEntry("smooth shading", 2);
    
    int lightsource_submenu = glutCreateMenu(lightsource_menu);
    glutAddMenuEntry("spot light", 1);
    glutAddMenuEntry("point source", 2);
    
    int fog_submenu = glutCreateMenu(fog_menu);
    glutAddMenuEntry("no fog", 1);
    glutAddMenuEntry("linear", 2);
    glutAddMenuEntry("exponential", 3);
    glutAddMenuEntry("exponential square", 4);
    
    int shadowblend_submenu = glutCreateMenu(shadowblend_menu);
    glutAddMenuEntry("Yes", 1);
    glutAddMenuEntry("No", 2);
    
    int floortexture_submenu = glutCreateMenu(floortexture_menu);
    glutAddMenuEntry("Yes", 1);
    glutAddMenuEntry("No", 2);
    
    int spheretexture_submenu = glutCreateMenu(spheretexture_menu);
    glutAddMenuEntry("Yes - Checkerboard", 1);
    glutAddMenuEntry("Yes - Contour Lines", 2);
    glutAddMenuEntry("No", 3);
    
    // menu
    glutCreateMenu(menu);
    glutAddMenuEntry("Default View Point", 1);
    glutAddMenuEntry("Quit", 2);
    glutAddMenuEntry("Wireframe Sphere", 3);
    glutAddSubMenu("Shadow", shadow_submenu);
    glutAddSubMenu("Enable Lighting", lighting_submenu);
    glutAddSubMenu("Shading", shading_submenu);
    glutAddSubMenu("Light Source", lightsource_submenu);
    glutAddSubMenu("Fog Options", fog_submenu);
    glutAddSubMenu("Blending Shadow", shadowblend_submenu);
    glutAddSubMenu("Texture Mapped Ground", floortexture_submenu);
    glutAddSubMenu("Texture Mapped Sphere", spheretexture_submenu);
    glutAttachMenu(GLUT_LEFT_BUTTON);

    init();
    glutMainLoop();
    return 0;
}
