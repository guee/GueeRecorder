uniform sampler2D qt_Texture0;
uniform sampler2D qt_Texture1;
uniform vec2 imageSize;
uniform vec2 mbSize;
varying vec4 qt_TexCoord0;


void main(void)
{
    vec2 pos = floor(qt_TexCoord0.xy * mbSize) * 16.0;
    pos.x = (pos.x + 1.0) / imageSize.x;
    pos.y = (pos.y + 1.0) / imageSize.y;
    vec2 siz = 2.0 / imageSize;
    for ( int x = 0; x < 8; ++x )
    {
        vec2 p = vec2(pos.x + float(x) * siz.x, pos.y);
        for ( int y = 0; y < 8; ++y )
        {
            vec3 pix0 = texture2D(qt_Texture0, p).rgb;
            vec3 pix1 = texture2D(qt_Texture1, p).rgb;
            if ( pix0 != pix1)
            {
                gl_FragColor.r = 0.0;
                return;
            }
            p.y += siz.y;
        }
    }
    gl_FragColor.r = 1.0;

//    if (pos.x < 0.5)
//    {
//        pos.x *= 2.0;
//        vec3 pix0 = texture2D(qt_Texture0, pos).rgb;
//        gl_FragColor.r = (pix0.r + pix0.g + pix0.b) / 3.0;
//    }
//    else
//    {
//        pos.x = (pos.x - 0.5) * 2.0;
//        vec3 pix0 = texture2D(qt_Texture1, pos).rgb;
//        gl_FragColor.r = (pix0.r + pix0.g + pix0.b) / 3.0;
//    }
}
