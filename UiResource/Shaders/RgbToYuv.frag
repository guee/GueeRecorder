
uniform sampler2D qt_Texture0;
uniform int PlaneType; //0=Y, 1=U, 2=V, 3=UV, 4=VU
uniform vec4 RGB_Y;
uniform vec4 RGB_U;
uniform vec4 RGB_V;

varying vec4 qt_TexCoord0;


void main(void)
{
    gl_FragColor = vec4(texture2D(qt_Texture0, qt_TexCoord0.st).rgb, 1.0);
    if (PlaneType == 0)
    {
        gl_FragColor.r = dot(gl_FragColor, RGB_Y);
    }
    else if (PlaneType == 1)
    {
        gl_FragColor.r = dot(gl_FragColor, RGB_U);
    }
    else if (PlaneType == 2)
    {
        gl_FragColor.r = dot(gl_FragColor, RGB_V);
    }
 //   gl_FragColor.r = 1.0;
}
