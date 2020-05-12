#ifndef GLVDIEODEFINE_H
#define GLVDIEODEFINE_H

const char* vertexSrc = "attribute vec4 vertexIn;\
                         attribute vec2 textureIn;\
                         varying vec2 textureOut;\
                         void main(void)\
                         {\
                            gl_Position = vertexIn;\
                            textureOut = textureIn;\
                         }";
const char* fragmentSrc =
#if defined(WIN32)
                          "#ifdef GL_ES\n"
                          "precision mediump float;\n"
                          "#endif\n"
#endif
                          "varying vec2 textureOut; \
                          uniform sampler2D tex_y; \
                          uniform sampler2D tex_u; \
                          uniform sampler2D tex_v; \
                          void main(void)\
                          {\
                            vec3 yuv; \
                            vec3 rgb; \
                            yuv.x = texture2D(tex_y, textureOut).r; \
                            yuv.x = 1.164*(yuv.x - 0.0625);\
                            yuv.y = texture2D(tex_u, textureOut).r - 0.5;\
                            yuv.z = texture2D(tex_v, textureOut).r - 0.5;\
                            rgb.x = yuv.x + 1.596023559570*yuv.z;\
                            rgb.y = yuv.x - 0.3917694091796875*yuv.y - 0.8129730224609375*yuv.z;\
                            rgb.z = yuv.x + 2.017227172851563*yuv.y;\
                            gl_FragColor = vec4(rgb, 1);\
                           }";

#endif // GLVDIEODEFINE_H
