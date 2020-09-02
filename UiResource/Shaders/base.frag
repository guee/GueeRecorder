uniform sampler2D qt_Texture0;

varying vec4 qt_TexCoord0;
uniform vec2 textureSize;
uniform int yuvFormat;        //0：正常 RGB/RGBA等。
                                //1=AYUV444, 2=YUV444, 3=YUV420P, 4=YUV422P
                                //5=YV12, 6=NV12, 7=NV21, 8=YV16,
                                //10=NV16, 11=UYVY, 12=YUYV, 13=Y

vec4 getRGB_Pixel( vec2 pos )
{
    vec2 texSize = textureSize;
    float y, u, v;
    if ( yuvFormat == 0 )
    {
        return texture2D(qt_Texture0, pos);
    }
    else if ( yuvFormat == 5 )  //QVideoFrame::Format_YV12:          //YYYY YYYY VV UU
    {
        y = texture2D(qt_Texture0, vec2(pos.x, pos.y / 1.5)).r;
        pos.y = floor(pos.y * texSize.y / 3.0) * 0.5;
        pos.x = (floor(pos.x * 0.5 * texSize.x) + 0.5) / texSize.x + pos.y - floor(pos.y);
        pos.y = (floor(pos.y) + 0.5) / texSize.y + 2.0 / 3.0;
        v = texture2D(qt_Texture0, pos).r;
        pos.y += 1.0/6.0;
        u = texture2D(qt_Texture0, pos).r;
    }
    else if ( yuvFormat == 6 )
    {
        y = texture2D(qt_Texture0, vec2(pos.x, pos.y / 1.5)).r;
        pos.x = (floor(pos.x * texSize.x * 0.5) * 2.0 + 0.5) / texSize.x;
        pos.y = (floor((pos.y + 2.0) * texSize.y / 3.0) + 0.5) / texSize.y;
        u = texture2D(qt_Texture0, pos).r;
        v = texture2D(qt_Texture0, vec2(pos.x + 1.0 / texSize.x, pos.y)).r;
    }
    else if ( yuvFormat == 7 )
    {
        y = texture2D(qt_Texture0, vec2(pos.x, pos.y / 1.5)).r;
        pos.x = (floor(pos.x * texSize.x * 0.5) * 2.0 + 0.5) / texSize.x;
        pos.y = (floor((pos.y + 2.0) * texSize.y / 3.0) + 0.5) / texSize.y;
        u = texture2D(qt_Texture0, vec2(pos.x + 1.0 / texSize.x, pos.y)).r;
        v = texture2D(qt_Texture0, pos).r;
    }
    else if ( yuvFormat == 12 )
    {
        y = texture2D(qt_Texture0, pos).r;
        pos.x = (floor(pos.x * texSize.x * 0.5) * 2.0 + 0.5) / texSize.x;
        u = texture2D(qt_Texture0, pos).a;
        pos.x += 1.0 / texSize.x;
        v = texture2D(qt_Texture0, pos).a;
    }
//    if ( debugMode == 1 )
//    {
//        return clamp( vec4(y + 1.4075 * ( v - 128.0/255.0 ),
//                        y - 0.3455 * ( u - 128.0/255.0 ) - 0.7169 * ( v - 128.0/255.0 ),
//                        y + 1.779  * ( u - 128.0/255.0 ), 1.0), 0.0, 1.0);
//    }

    return clamp(vec4(1.164383 * ( y - 16.0/255.0 ) + 1.596027 * ( v - 128.0/255.0 ),
                1.164383 * ( y - 16.0/255.0 ) - 0.391762 * ( u - 128.0/255.0 ) - 0.812968 * ( v - 128.0/255.0 ),
                1.164383 * ( y - 16.0/255.0 ) + 2.017232 * ( u - 128.0/255.0 ),
                1.0), 0.0, 1.0);


//    else if ( debugMode == 2 )
//    {
//        rgb.b = v;
//        rgb.g = v;
//        rgb.r = v;
//    }
//    else if ( debugMode == 3 )
//    {

//    }
//    else if ( debugMode == 4 )
//    {

//    }

}

void main(void)
{
    gl_FragColor = getRGB_Pixel(qt_TexCoord0.st);
    //gl_FragColor = texture2D(qt_Texture0, qt_TexCoord0.st);
}
