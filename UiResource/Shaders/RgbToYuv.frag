
uniform sampler2D qt_Texture0;
uniform int PlaneType;
uniform int YCbCrType;      //0=Bt.601[0~255], 1=Bt.601[0~219], 2=Bt.709[0~255], 3=Bt.709[0~219]
uniform vec2 surfaceSize;
uniform vec2 alignSize;
uniform vec2 imageSize;

varying vec4 qt_TexCoord0;
varying vec4 RGB_Y;
varying vec4 RGB_U;
varying vec4 RGB_V;

void main(void)
{
    vec2 pos = qt_TexCoord0.st * surfaceSize;

    if (0 == PlaneType)     //Vid_CSP_I420
    {
        if (pos.y < alignSize.y)
        {
            pos.x /= imageSize.x;
            pos.y = 1.0 - (pos.y / imageSize.y);
            gl_FragColor = vec4(texture2D(qt_Texture0, pos).rgb, 1.0);
            gl_FragColor.r = dot(gl_FragColor, RGB_Y);
        }
        else if (pos.y < alignSize.y * 1.25)
        {
//            if (pos.x < surfaceSize.x * 0.5)
//            {
//                pos.x = (floor(pos.x) * 2.0 + 0.5) / imageSize.x;
//                pos.y = 1.0 - (floor(pos.y - alignSize.y) * 4.0 + 0.5 ) / imageSize.y;
//            }
//            else
//            {
//                pos.x = (floor(pos.x - surfaceSize.x * 0.5) * 2.0 + 0.5) / imageSize.x;
//                pos.y = 1.0 - (floor(pos.y - alignSize.y) * 4.0 + 2.5 ) / imageSize.y;
//            }
            if (pos.x < surfaceSize.x * 0.5)
            {
                pos.x = (floor(pos.x) * 2.0 + 1.0) / imageSize.x;
                pos.y = 1.0 - (floor(pos.y - alignSize.y) * 4.0 + 1.0 ) / imageSize.y;
            }
            else
            {
                pos.x = (floor(pos.x - surfaceSize.x * 0.5) * 2.0 + 1.0) / imageSize.x;
                pos.y = 1.0 - (floor(pos.y - alignSize.y) * 4.0 + 3.0 ) / imageSize.y;
            }
            gl_FragColor = vec4(texture2D(qt_Texture0, pos).rgb, 1.0);
            gl_FragColor.r = dot(gl_FragColor, RGB_U);
        }
        else
        {
//            if (pos.x < surfaceSize.x * 0.5)
//            {
//                pos.x = (floor(pos.x) * 2.0 + 0.5) / imageSize.x;
//                pos.y = 1.0 - (floor(pos.y - alignSize.y * 1.25) * 4.0 + 1.5 ) / imageSize.y;
//            }
//            else
//            {
//                pos.x = (floor(pos.x - surfaceSize.x * 0.5) * 2.0 + 0.5) / imageSize.x;
//                pos.y = 1.0 - (floor(pos.y - alignSize.y * 1.25) * 4.0 + 3.5 ) / imageSize.y;
//            }
            if (pos.x < surfaceSize.x * 0.5)
            {
                pos.x = (floor(pos.x) * 2.0 + 1.0) / imageSize.x;
                pos.y = 1.0 - (floor(pos.y - alignSize.y * 1.25) * 4.0 + 1.0 ) / imageSize.y;
            }
            else
            {
                pos.x = (floor(pos.x - surfaceSize.x * 0.5) * 2.0 + 1.0) / imageSize.x;
                pos.y = 1.0 - (floor(pos.y - alignSize.y * 1.25) * 4.0 + 3.0) / imageSize.y;
            }
            gl_FragColor = vec4(texture2D(qt_Texture0, pos).rgb, 1.0);
            gl_FragColor.r = dot(gl_FragColor, RGB_V);
        }
    }
}
