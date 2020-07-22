#ifdef GL_ES
precision mediump float;
#endif
varying vec2 textureOut;
uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;
uniform sampler2D tex_img;
void main(void)
{
#ifdef FRAME_RGB
    gl_FragColor = vec4(texture2D(tex_img, textureOut).rgb, 1);
#else
    vec3 yuv;
    vec3 rgb;
    yuv.x = texture2D(tex_y, textureOut).r;
    yuv.x = 1.164*(yuv.x - 0.0625);
    yuv.y = texture2D(tex_u, textureOut).r - 0.5;
    yuv.z = texture2D(tex_v, textureOut).r - 0.5;
    rgb.x = yuv.x + 1.596023559570*yuv.z;
    rgb.y = yuv.x - 0.3917694091796875*yuv.y - 0.8129730224609375*yuv.z;
    rgb.z = yuv.x + 2.017227172851563*yuv.y;
    gl_FragColor = vec4(rgb, 1);
//    vec3 yuv;
//    vec3 rgb;
//    yuv.x = texture2D(tex_y, textureOut).r;
//    yuv.y = texture2D(tex_u, textureOut).r - 0.5;
//    yuv.z = texture2D(tex_v, textureOut).r - 0.5;
//    rgb = mat3( 1,       1,         1,
//                0,       -0.39465,  2.03211,
//                1.13983, -0.58060,  0) * yuv;
//    gl_FragColor = vec4(rgb, 1);
#endif
}
