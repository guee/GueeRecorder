highp float;
highp int;
uniform sampler2D qt_Texture0;
uniform sampler2D qt_Texture1;
uniform vec2 imageSize;
uniform vec2 mbSize;
varying vec4 qt_TexCoord0;


void main(void)
{
    vec2 pos = (floor(qt_TexCoord0.xy * mbSize) * 16.0 + 0.5) / imageSize;
    pos.y = 1.0 - pos.y;

    gl_FragColor.a = 1.0;

    vec2 siz = 1.0 / imageSize;
    gl_FragColor.a = 1.0;
    for ( int x = 0; x < 16; ++x )
    {
        vec2 p = vec2(pos.x + float(x) * siz.x, pos.y);
        for ( int y = 0; y < 16; ++y )
        {
            vec3 pix0 = texture2D(qt_Texture0, p).rgb;
            vec3 pix1 = texture2D(qt_Texture1, p).rgb;
            if ( pix0 != pix1)
            {
                gl_FragColor.r = 0.0;
                return;
            }
            p.y -= siz.y;
        }
    }
    gl_FragColor.r = 1.0;
}
