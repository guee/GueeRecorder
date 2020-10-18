highp float;
highp int;
uniform sampler2D qt_Texture0;
uniform sampler2D qt_Texture1;
uniform vec2 imageSize;
uniform vec2 mbSize;
uniform bool flip;
varying vec4 qt_TexCoord0;

varying float offsetX[16];

void main(void)
{
    vec2 siz = 1.0 / imageSize;
    vec2 pos = qt_TexCoord0.st;
    float length = 0.0;
    pos.x -= siz.x * 7.0;
    pos.y -= siz.y * 7.0;

    if (flip)
    {
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);
    }
    else
    {
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 2.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 2.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 6.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 6.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 10.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 10.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 14.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 14.0, pos.y)).rgb);

        pos.y += siz.y * 2.0;
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 0.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 0.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 4.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 4.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 8.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 8.0, pos.y)).rgb);
        length += distance(texture2D(qt_Texture0, vec2(pos.x + siz.x * 12.0, pos.y)).rgb, texture2D(qt_Texture1, vec2(pos.x + siz.x * 12.0, pos.y)).rgb);
    }
    gl_FragColor.a = (length > 0.0) ? 0.0 : 1.0;
}
