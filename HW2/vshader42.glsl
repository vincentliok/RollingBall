/***************************
 * File: vshader42.glsl:
 *   A simple vertex shader.
 *
 * - Vertex attributes (positions & colors) for all vertices are sent
 *   to the GPU via a vertex buffer object created in the OpenGL program.
 *
 * - This vertex shader uses the Model-View and Projection matrices passed
 *   on from the OpenGL program as uniform variables of type mat4.
 ***************************/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in vec3 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;
out vec4 color;
out float z;
out vec2 texCoord;

uniform bool isPointSource;
uniform bool isSpotlight;

uniform vec4 GlobalAmbientProduct, PositionalAmbientProduct, PositionalDiffuseProduct, PositionalSpecularProduct;
uniform vec4 DirectionalAmbientProduct, DirectionalDiffuseProduct, DirectionalSpecularProduct;
uniform mat3 Normal_Matrix;
uniform vec4 SpotLightDirection;
uniform vec4 DirectionalLightDirection;
uniform vec4 LightPosition;
uniform float Shininess;

uniform mat4 model_view;
uniform mat4 projection;

uniform bool isSphere;
uniform bool isFloor;
uniform bool isAxisX;
uniform bool isAxisY;
uniform bool isAxisZ;
uniform bool isShadow;

uniform float ConstAtt;
uniform float LinearAtt;
uniform float QuadAtt;

uniform float ExpVal;
uniform float CutoffAngle;

uniform bool isWireframe;
uniform bool isLighting;

uniform int fogFlagIn;
flat out int fogFlagOut;
flat out int isFloorFragment;
flat out int isSphereFragment;

uniform int verticalFlag;
uniform int eyeFlag;
uniform int sphereCheckerFlag;
flat out int sphereCheckerFlagFragment;
uniform int spheretextureFlag;

void main() 
{
    vec4 vPosition4 = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);
    
    if (isSphere || isFloor) {
        
        // directional light
        
        vec3 pos = (model_view * vPosition4).xyz;
        
        vec3 L = normalize( -DirectionalLightDirection.xyz );
        vec3 E = normalize( -pos );
        vec3 H = normalize( L + E );

        vec3 N = normalize(Normal_Matrix * vNormal);

        if ( dot(N, E) < 0 ) N = -N;
        
        float attenuation = 1.0;
        
        vec4 global = GlobalAmbientProduct;
        
        vec4 ambient = DirectionalAmbientProduct;

        float d = max( dot(L, N), 0.0 );
        vec4  diffuse = d * DirectionalDiffuseProduct;

        float s = pow( max(dot(N, H), 0.0), Shininess );
        vec4  specular = s * DirectionalSpecularProduct;

        if( dot(L, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }
        
        color = global + (attenuation * (ambient + diffuse + specular));
        
        // positional light
        
        L = normalize( LightPosition.xyz - pos );
        H = normalize( L + E );
        
        float dist = abs(distance(LightPosition.xyz, pos));
        
        if (isPointSource) {
            attenuation = 1.0 / (ConstAtt + (LinearAtt * dist) + (QuadAtt * (dist * dist)));
        }
        if (isSpotlight) {
            if (dot(normalize(SpotLightDirection.xyz), -L) < cos(CutoffAngle)) {
                attenuation = 0.0;
            }
            else {
                attenuation = pow(dot(normalize(SpotLightDirection.xyz), -L), ExpVal) / (ConstAtt + (LinearAtt * dist) + (QuadAtt * (dist * dist)));
            }
        }
        
        ambient = PositionalAmbientProduct;

        d = max( dot(L, N), 0.0 );
        diffuse = d * PositionalDiffuseProduct;

        s = pow( max(dot(N, H), 0.0), Shininess );
        specular = s * PositionalSpecularProduct;

        if( dot(L, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }
        
        color += (attenuation * (ambient + diffuse + specular));
    }

    if (isAxisX) {
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }

    if (isAxisY) {
        color = vec4(1.0, 0.0, 1.0, 1.0);
    }

    if (isAxisZ) {
        color = vec4(0.0, 0.0, 1.0, 1.0);
    }

    if (isShadow) {
        color = vec4(0.25, 0.25, 0.25, 0.65);
    }
    
    if (isSphere && (isWireframe || !isLighting)) {
        color = vec4(1.0, 0.84, 0.0, 1.0);
    }
    
    if (isFloor && !isLighting) {
        color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    
    gl_Position = projection * model_view * vPosition4;
    
    vec4 pos = projection * model_view * vPosition4;
    vec4 pos2 = model_view * vPosition4;
    z = pos.z;
    
    fogFlagOut = fogFlagIn;
    if (isFloor) {
        texCoord = vTexCoord;
        isFloorFragment = 1;
    }
    else {
        isFloorFragment = 0;
    }
    if (isSphere && sphereCheckerFlag == 0 && spheretextureFlag == 1) {
        isSphereFragment = 1;
        
        if (verticalFlag == 1 && eyeFlag == 0) {
            texCoord[0] = 2.5 * vPosition.x;
        }
        if (verticalFlag == 0 && eyeFlag == 0) {
            texCoord[0] = 1.5 * (vPosition.x + vPosition.y + vPosition.z);
        }
        if (verticalFlag == 1 && eyeFlag == 1) {
            texCoord[0] = 2.5 * pos2.x;
        }
        if (verticalFlag == 0 && eyeFlag == 1) {
            texCoord[0] = 1.5 * (pos2.x + pos2.y + pos2.z);
        }
    }
    if (isSphere && sphereCheckerFlag == 1 && spheretextureFlag == 1) {
        isSphereFragment = 1;
        
        if (verticalFlag == 1 && eyeFlag == 0) {
            texCoord[0] = 0.75 * (vPosition.x + 1);
            texCoord[1] = 0.75 * (vPosition.y + 1);
        }
        if (verticalFlag == 0 && eyeFlag == 0) {
            texCoord[0] = 0.45 * (vPosition.x + vPosition.y + vPosition.z);
            texCoord[1] = 0.45 * (vPosition.x - vPosition.y + vPosition.z);
        }
        if (verticalFlag == 1 && eyeFlag == 1) {
            texCoord[0] = 0.75 * (pos2.x + 1);
            texCoord[1] = 0.75 * (pos2.y + 1);
        }
        if (verticalFlag == 0 && eyeFlag == 1) {
            texCoord[0] = 0.45 * (pos2.x + pos2.y + pos2.z);
            texCoord[1] = 0.45 * (pos2.x - pos2.y + pos2.z);
        }
    }
    if (!isSphere) {
        isSphereFragment = 0;
    }
    
    sphereCheckerFlagFragment = sphereCheckerFlag;
} 
