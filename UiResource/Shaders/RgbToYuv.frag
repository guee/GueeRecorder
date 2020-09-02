uniform sampler2D qt_Texture0;
uniform int PlaneType;
uniform int YCbCrType;      //0=Bt.601[0~255], 1=Bt.601[0~219], 2=Bt.709[0~255], 3=Bt.709[0~219]

varying vec4 qt_TexCoord0;
varying vec4 RGB_Y;
varying vec4 RGB_U;
varying vec4 RGB_V;

void main(void)
{
    gl_FragColor = texture2D(qt_Texture0, qt_TexCoord0.st);
    return;
    vec4 pix = texture2D(qt_Texture0, qt_TexCoord0.st);
    if (0 == PlaneType)     //Y
    {
        pix.a = 1.0;
        gl_FragColor.r = dot(pix, RGB_Y);
    }
    else if (1 == PlaneType)     //U
    {
        pix.a = 1.0;
        gl_FragColor.r = dot(pix, RGB_U);
    }
    else if (2 == PlaneType)     //V
    {
        pix.a = 1.0;
        gl_FragColor.r = dot(pix, RGB_V);
    }
    else if (3 == PlaneType)    //UV
    {
        pix.a = 1.0;
        gl_FragColor.r = dot(pix, RGB_U);
        gl_FragColor.g = dot(pix, RGB_V);
    }
    else if (4 == PlaneType)    //VU
    {
        pix.a = 1.0;
        gl_FragColor.r = dot(pix, RGB_V);
        gl_FragColor.g = dot(pix, RGB_U);
    }
    else if (5 == PlaneType)     //YUYV
    {
        pix.a = 1.0;
        gl_FragColor.r = dot(pix, RGB_V);
        gl_FragColor.g = dot(pix, RGB_U);
    }
    else if (6 == PlaneType)     //UYVY
    {
    }
    else if (7 == PlaneType)     //AYUV
    {
    }
    else if (8 == PlaneType)     //BGR
    {
    }
}
