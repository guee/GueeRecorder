uniform sampler2D qt_Texture0;

varying vec4 qt_TexCoord0;
uniform vec2 textureSize;
uniform int yuvFormat;        //0：正常 RGB/RGBA等。
                                //1=AYUV444, 2=YUV444, 3=YUV420P, 4=YUV422P
                                //5=YV12, 6=NV12, 7=NV21, 8=YV16,
                                //10=NV16, 11=UYVY, 12=YUYV, 13=Y

uniform float contrast;     //对比度：-1 ~ 1。 0表示无变化，负数降低对比度，正数增大对比度。
uniform float bright;       //亮度：-1 ~ 1。 0表示无变化，负数降低亮度，正数增大亮度。
uniform float saturation;   //饱和度：-1 ~ 1。 0表示无变化，负数降低饱和度，正数增大饱和度。
uniform float hue;          //色相：-1 ~ 1。 0表示无变化，负数逆时针旋转（360度），正数顺时针旋转（360度）。
                            //色相可把参数范围控制为 -0.5 ~ 0.5，表示正负180度，合为360度。
                            //其它参数（对比度、亮度、饱和度）也可以缩小值范围，避免用户调整得过大。
uniform bool hueDye;
uniform float transparence; //透明度 0 ~ 1。0是全透明，1是不透明。


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

vec3 rgb2hsl(vec3 rgb)
{
    //返回的 vec3 中， r 表示 h， g 表示 s, b 表示 l，均为归一化的值，包括色相 s。
    float h, s, l, v;
    float maxValue = max(rgb.r, max(rgb.g, rgb.b));
    float minValue = min(rgb.r, min(rgb.g, rgb.b));
    v = maxValue - minValue;
    l = (maxValue + minValue) * 0.5;
    if (v == 0.0 || l == 0.0)
    {
        s = h = 0.0;
    }
    else
    {
        //计算色深
        //s = v / ( 1.0 - abs( 2.0 * l - 1.0));
        s = l <= 0.5 ? v/(2.0*l) : v/(2.0-2.0*l);
        if (maxValue == rgb.r)
            h = ((rgb.g - rgb.b) / v + (rgb.g < rgb.b ? 6.0 : 0.0)) / 6.0;
        else if (maxValue == rgb.g)
            h = ((rgb.b - rgb.r) / v + 2.0) / 6.0;
        else
            h = ((rgb.r - rgb.g) / v + 4.0) / 6.0;
    }
    return vec3(h,s,l);
}
float hue2rgb(float p, float q, float t)
{
    if (t < 0.0)
        ++t;
    else if (t > 1.0)
        --t;
    if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if (t < 1.0/2.0) return q;
    if (t < 2.0/3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
    return p;
}
vec3 hsl2rgb(float h, float s, float l)
{
    float r, g, b;
    if (s == 0.0)
    {
        r = g = b = l;
    }
    else
    {
        float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
        float p = 2.0 * l - q;
        r = hue2rgb(p, q, h + 1.0/3.0);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0/3.0);
    }
    return vec3(r, g, b);
}

void main(void)
{
    vec4 pix = getRGB_Pixel(qt_TexCoord0.st);
    if (contrast != 0.0 || bright != 0.0 || saturation != 0.0)
    {
        float c = contrast >= 0.0 ? 1.0 + contrast * 9.0 : 1.0 + contrast * 9.0 / 10.0;
        float b = bright >= 0.0 ? 1.0 + bright * 9.0 : 1.0 + bright * 9.0 / 10.0;
        float s = saturation >= 0.0 ? 1.0/(1.0 + saturation * 9.0) : 1.0 + saturation * 1.0;
        vec3 hsl = rgb2hsl(pix.rgb);
        if (contrast >= 0.0)
        {
            hsl.b = hsl.b <= 0.5 ? pow(hsl.b * 2.0, c) * 0.5 : (1.0 - pow((1.0 - (hsl.b - 0.5) * 2.0), c)) * 0.5 + 0.5;
        }
        else
        {
            hsl.b = hsl.b * c + (1.0 - c) * 0.5;
            hsl.g = hsl.g * c;
        }
        hsl.b = bright >= 0.0 ? 1.0 - pow( 1.0 - hsl.b, b) : pow( hsl.b, 1.0 / b);
        hsl.g = saturation >= 0.0 ? pow( hsl.g, s) : hsl.g * s;
        if (hueDye)
        {
            hsl.r = hue;
        }
        else
        {
            hsl.r = hsl.r + hue + 2.0;
            hsl.r -= floor(hsl.r);
        }
        pix.rgb = hsl2rgb(hsl.r, hsl.g, hsl.b);
    }
    pix.a *= transparence;
    gl_FragColor = pix;
}
