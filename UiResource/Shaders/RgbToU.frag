
uniform sampler2D qt_Texture0;
varying vec4 qt_TexCoord0;

const vec4 RGB_Y = vec4(+0.2568, +0.5041, +0.0979, 16.0/255.0);
const vec4 RGB_U = vec4(-0.1482, -0.2910, +0.4392, 128.0/255.0);
const vec4 RGB_V = vec4(+0.4392, -0.3678, -0.0714, 128.0/255.0);
void main(void)
{
    gl_FragColor = vec4(texture2D(qt_Texture0, qt_TexCoord0.st).rgb, 1.0);
    gl_FragColor.r = dot(gl_FragColor, RGB_U);
}
