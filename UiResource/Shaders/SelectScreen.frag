uniform sampler2D qt_Texture0;
uniform vec4 lineColor;
uniform vec2 bmpSize;
uniform float pointSize;
uniform vec2 topLeft;
uniform vec2 bottomRight;
uniform bool boxEditing;
uniform bool showInfo;
uniform vec2 infoBoxTL;
uniform vec2 infoBoxBR;
uniform vec2 whiteBoxTL;
uniform vec2 whiteBoxBR;
uniform vec2 zoomBoxTL;
uniform vec2 zoomBoxSize;
uniform float zoomPixel;
uniform vec2 mousePos;

varying vec4 qt_TexCoord0;


void main(void)
{
    vec2 pos = floor(qt_TexCoord0.xy * bmpSize);
    float potSiz = pointSize * 0.5;
    if (boxEditing)
    {
        if (abs(pos.y - topLeft.y) <= potSiz || abs(pos.y - bottomRight.y) <= potSiz)
        {
            if(abs(pos.x - topLeft.x) <= potSiz || abs(pos.x - bottomRight.x) <= potSiz || abs(pos.x - (bottomRight.x + topLeft.x) * 0.5) <= potSiz)
            {
                gl_FragColor = lineColor;
                return;
            }
            else if (pos.x >= topLeft.x && pos.x <= bottomRight.x && ( abs(pos.y - topLeft.y) < 0.1 || abs(pos.y - bottomRight.y) < 0.1 ))
            {
                gl_FragColor = lineColor;
                return;
            }
        }
        else if(pos.y > topLeft.y && pos.y < bottomRight.y && ( abs(pos.x - topLeft.x) <= potSiz || abs(pos.x - bottomRight.x) <= potSiz))
        {
            if (abs(pos.y - (bottomRight.y + topLeft.y) * 0.5) <= potSiz || abs(pos.x - topLeft.x) < 0.1 || abs(pos.x - bottomRight.x) < 0.1)
            {
                gl_FragColor = lineColor;
                return;
            }
        }
    }
    else
    {
        if (abs(pos.y - topLeft.y) <= potSiz || abs(pos.y - bottomRight.y) <= potSiz)
        {
            if (pos.x >= topLeft.x && pos.x <= bottomRight.x)
            {
                gl_FragColor = lineColor;
                return;
            }
        }
        if (abs(pos.x - topLeft.x) <= potSiz || abs(pos.x - bottomRight.x) < potSiz)
        {
            if (pos.y >= topLeft.y && pos.y <= bottomRight.y)
            {
                gl_FragColor = lineColor;
                return;
            }
        }
    }
    if ( showInfo && pos.x >= infoBoxTL.x && pos.x <= infoBoxBR.x
         && pos.y >= infoBoxTL.y && pos.y <= infoBoxBR.y )
    {
        if ( pos.x >= whiteBoxTL.x && pos.x <= whiteBoxBR.x
             && pos.y >= whiteBoxTL.y && pos.y <= whiteBoxBR.y )
        {
            if ( pos.x >= zoomBoxTL.x && pos.x <= ( zoomBoxTL.x + zoomBoxSize.x - 1.0 )
                 && pos.y >= zoomBoxTL.y && ( pos.y <= zoomBoxTL.y + zoomBoxSize.y - 1.0 ) )
            {
                vec2 t = floor(( pos - zoomBoxTL ) / zoomPixel);
                vec2 s = ( t - vec2(14.0, 10.0) + mousePos + 0.5 ) / bmpSize;
                gl_FragColor = texture2D(qt_Texture0, s);
                if ( t.x == 14.0 || t.y == 10.0 )
                {
                    gl_FragColor = mix( gl_FragColor, lineColor, 0.5 );
                }
            }
            else
            {
                gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
            }
        }
        else
        {
            gl_FragColor = vec4(texture2D(qt_Texture0, qt_TexCoord0.st).rgb * 0.2, 1.0);
        }
    }
    else
    {
        gl_FragColor = texture2D(qt_Texture0, qt_TexCoord0.st);
        if ( pos.x <= topLeft.x || pos.x >= bottomRight.x ||
             pos.y <= topLeft.y || pos.y >= bottomRight.y )
        {
            gl_FragColor.r *= 0.5;
            gl_FragColor.g *= 0.5;
            gl_FragColor.b *= 0.5;
        }
    }
}
