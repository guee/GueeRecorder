uniform sampler2D qt_Texture0;
varying vec4 qt_TexCoord0;

uniform int _intVar;
uniform int _intArr[5];
uniform float _floatVar;
uniform float _floatArr[6];
uniform vec2 _vec2ver;
uniform vec2 _vec2Arr[7];
uniform ivec2 _ivec2ver;
uniform ivec2 _ivec2Arr[8];
uniform vec3 _vec3Arr[1];
struct StrUct
{
    float fa;
    float fArr[3];
    int   ia;
    int   iArr[4];
    vec2  va;
    vec2  vArr[5];
    mat4  mArr[6];
};
uniform StrUct _sVar;
uniform StrUct _sArr[6];


void main(void)
{
    float a = float(_intVar) + float(_intArr[0]) + float(_intArr[1]) + float(_intArr[2]);
    float b = _floatVar + _floatArr[0] + _floatArr[1] + _floatArr[2] + _vec3Arr[0].x;

    a += _vec2ver.x;
    b += _vec2Arr[6].y;
    int c = _ivec2ver.r + _ivec2Arr[5].r + _ivec2Arr[6].r + _ivec2Arr[7].r;
    float d = float(c) * a * b;
    float e = _sVar.fa + _sVar.fArr[1];
    float f = float(_sVar.ia + _sVar.iArr[1]) * _sArr[1].va.x + _sArr[0].vArr[3].g;


    gl_FragColor.x = float(c);
    gl_FragColor.y = d;
    gl_FragColor.z = e;
    gl_FragColor.a = f;
}
