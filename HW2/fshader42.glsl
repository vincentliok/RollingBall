/*****************************
 * File: fshader42.glsl
 *       A simple fragment shader
 *****************************/

#version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in vec4 color;
in float z;
in vec2 texCoord;
flat in int fogFlagOut;
flat in int isFloorFragment;
flat in int isSphereFragment;
out vec4 fColor;

uniform sampler2D texture_2D;
uniform sampler1D texture_1D;
uniform int floortextureFlag;

flat in int sphereCheckerFlagFragment;

void main() 
{
    vec4 newColor = color;
    if (isFloorFragment == 1 && floortextureFlag == 1) {
        newColor = color * texture( texture_2D, texCoord );
    }
    
    if (isSphereFragment == 1 && sphereCheckerFlagFragment == 0) {
        newColor = color * texture( texture_1D, texCoord[0] );
    }
    
    if (isSphereFragment == 1 && sphereCheckerFlagFragment == 1) {
        vec4 texColor = texture( texture_2D, texCoord );
        if (texColor.x < 0.5) {
            texColor = vec4(0.9, 0.1, 0.1, 1.0);
        }
        newColor = color * texColor;
    }
    
    if (fogFlagOut == 0) {
        fColor = newColor;
    }
    
    if (fogFlagOut == 1) {
        vec4 fogColor = vec4(0.7, 0.7, 0.7, 0.5);
        float start = 0.0;
        float end = 18.0;
        float fogEquation = (end - z) / (end - start);
        fColor = mix(fogColor, newColor, clamp(fogEquation, 0.0, 1.0));
    }
    
    if (fogFlagOut == 2) {
        float density = 0.09;
        vec4 fogColor = vec4(0.7, 0.7, 0.7, 0.5);
        float fogEquation = exp(-density * z);
        fColor = mix(fogColor, newColor, clamp(fogEquation, 0.0, 1.0));
    }
    
    if (fogFlagOut == 3) {
        float density = 0.09;
        vec4 fogColor = vec4(0.7, 0.7, 0.7, 0.5);
        float fogEquation = exp(-pow(density * z, 2));
        fColor = mix(fogColor, newColor, clamp(fogEquation, 0.0, 1.0));
    }
} 

