attribute highp vec4 vertexIn;
attribute highp vec2 textureIn;
varying highp vec2 textureOut;

void main(void)
{
    gl_Position = vertexIn;
    textureOut = textureIn;
}
