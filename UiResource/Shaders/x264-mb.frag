highp float;
highp int;
uniform sampler2D qt_Texture0;
uniform sampler2D qt_Texture1;
uniform vec2 imageSize;
uniform vec2 mbSize;
varying vec4 qt_TexCoord0;

varying float offsetX[16];

void main(void)
{
    vec2 pos = (floor(qt_TexCoord0.xy * mbSize) * 16.0 + 0.5) / imageSize;
    pos.y = 1.0 - pos.y;
    vec2 siz = 1.0 / imageSize;

    gl_FragColor.a = 1.0;
    gl_FragColor.r = 1.0;
    float length = 0.0;

    for ( int y = 0; y < 16; ++y )
    {
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 1.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 1.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 3.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 3.0, pos.y)).rgb);

        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 5.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 5.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 7.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 7.0, pos.y)).rgb);

        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 9.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 9.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 11.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 11.0, pos.y)).rgb);

        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 13.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 13.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 15.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 15.0, pos.y)).rgb);

//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 1.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 1.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 3.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 3.0, pos.y)).r);

//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 5.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 5.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 7.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 7.0, pos.y)).r);

//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 9.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 9.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 11.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 11.0, pos.y)).r);

//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 13.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 13.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).r);
//        length += abs(texture2D(qt_Texture0, vec2(pos.x + siz.x * 15.0, pos.y)).r- texture2D(qt_Texture1, vec2(pos.x + siz.x * 15.0, pos.y)).r);
pos.y -= siz.y;
    }

//    gl_FragColor.a = 1.0;
//    for ( int x = 0; x < 16; ++x )
//    {
//        vec2 p = vec2(pos.x + float(x) * siz.x, pos.y);
//        for ( int y = 0; y < 16; ++y )
//        {
//            vec3 pix0 = texture2D(qt_Texture0, p).rgb;
//            vec3 pix1 = texture2D(qt_Texture1, p).rgb;
//            if ( pix0 != pix1)
//            {
//                gl_FragColor.r = 0.0;
//                return;
//            }
//            p.y -= siz.y;
//        }
//    }
    if (length > 0.0)
        gl_FragColor.r = 0.0;

     //gl_FragColor.r = isSame ? 1.0 : 0.0;
}
