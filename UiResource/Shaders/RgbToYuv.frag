uniform sampler2D qt_Texture0;
uniform int PlaneType;
uniform int YCbCrType;      //0=Bt.601[0~255], 1=Bt.601[0~219], 2=Bt.709[0~255], 3=Bt.709[0~219]
uniform vec2 texureSize;
uniform vec2 alignSize;

varying vec4 qt_TexCoord0;
varying vec4 RGB_Y;
varying vec4 RGB_U;
varying vec4 RGB_V;

void main(void)
{
    vec2 pos = qt_TexCoord0.st * texureSize;

    //if (0 == PlaneType)     //Vid_CSP_I420
//    {
//        if (pos.y < alignSize.y)
//        {
//            pos.y /= alignSize.y;
//            gl_FragColor = vec4(texture2D(qt_Texture0, pos).rgb, 1.0);
//            gl_FragColor.r = dot(gl_FragColor, RGB_Y);
//        }
//        else
//        {

//            gl_FragColor.r = 1.0;
//            gl_FragColor.g = 0.0;
//            gl_FragColor.b = 0.0;
//            gl_FragColor.a = 1.0;
//        }
//    }
gl_FragColor = vec4(texture2D(qt_Texture0, qt_TexCoord0.st).rgb, 1.0);
}
