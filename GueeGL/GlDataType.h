#ifndef GUEEGLDATATYPE_H
#define GUEEGLDATATYPE_H
namespace Guee
{
	//https://www.khronos.org/registry/OpenGL-Refpages/gl4/
	struct GL_DataTypeBytes
	{
		GLenum symbolicConstant;		//OpenGL 类型符号常量
		int32_t	bytes;
	};
	//顶点数据的外部存储格式
	const	GL_DataTypeBytes GL_VertexDataTypes[] = {
        {GL_BYTE,                           sizeof(GLbyte)},
        {GL_UNSIGNED_BYTE,                  sizeof(GLubyte)},
        {GL_SHORT,                          sizeof(GLshort)},
        {GL_UNSIGNED_SHORT,                 sizeof(GLushort)},
        {GL_INT,                            sizeof(GLint)},
        {GL_UNSIGNED_INT,                   sizeof(GLuint)},

        {GL_HALF_FLOAT,                     sizeof(GLhalf)},    //16位 S1E5M10 半精度浮点
        {GL_FLOAT,                          sizeof(GLfloat)},
        {GL_DOUBLE,                         sizeof(GLdouble)},
        {GL_FIXED,                          sizeof(GLfixed)},   //有符号16位定点
        {GL_INT_2_10_10_10_REV,             sizeof(GLuint)},    //压缩数据类型
        {GL_UNSIGNED_INT_2_10_10_10_REV,    sizeof(GLuint)},    //压缩数据类型
        {GL_UNSIGNED_INT_10F_11F_11F_REV,   sizeof(GLuint)}     //压缩数据类型
	};
	//像素数据外部存储格式
	const	GL_DataTypeBytes GL_PixelDataTypes[] = {
        {GL_UNSIGNED_BYTE,                  sizeof(GLubyte)},
        {GL_BYTE,                           sizeof(GLbyte)},
        {GL_UNSIGNED_SHORT,                 sizeof(GLushort)},
        {GL_SHORT,                          sizeof(GLshort)},
        {GL_UNSIGNED_INT,                   sizeof(GLuint)},
        {GL_INT,                            sizeof(GLint)},
        {GL_HALF_FLOAT,                     sizeof(GLhalf)},    //16位 S1E5M10 半精度浮点
        {GL_FLOAT,                          sizeof(GLfloat)},
        {GL_UNSIGNED_BYTE_3_3_2,            sizeof(GLubyte)},
        {GL_UNSIGNED_BYTE_2_3_3_REV,        sizeof(GLubyte)},
        {GL_UNSIGNED_SHORT_5_6_5,           sizeof(GLushort)},
        {GL_UNSIGNED_SHORT_5_6_5_REV,       sizeof(GLushort)},
        {GL_UNSIGNED_SHORT_4_4_4_4,         sizeof(GLushort)},
        {GL_UNSIGNED_SHORT_4_4_4_4_REV,     sizeof(GLushort)},
        {GL_UNSIGNED_SHORT_5_5_5_1,         sizeof(GLushort)},
        {GL_UNSIGNED_SHORT_1_5_5_5_REV,     sizeof(GLushort)},
        {GL_UNSIGNED_INT_8_8_8_8,           sizeof(GLuint)},
        {GL_UNSIGNED_INT_8_8_8_8_REV,       sizeof(GLuint)},
        {GL_UNSIGNED_INT_10_10_10_2,        sizeof(GLuint)},
        {GL_UNSIGNED_INT_2_10_10_10_REV,    sizeof(GLuint)}
	};

	//纹理目标类型
	const	GLenum GL_TexutreTargets[] = {
		GL_TEXTURE_1D,
		GL_TEXTURE_2D,
		GL_TEXTURE_3D,
		GL_TEXTURE_1D_ARRAY,
		GL_TEXTURE_2D_ARRAY,
		GL_TEXTURE_RECTANGLE,
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_CUBE_MAP_ARRAY,
		GL_TEXTURE_BUFFER,
		GL_TEXTURE_2D_MULTISAMPLE,
		GL_TEXTURE_2D_MULTISAMPLE_ARRAY
	};

	//纹理像素采样时，插值的方式
	const	GLenum GL_TextureFilters[] = {
        GL_NEAREST,                 //最邻近过滤，获得最靠近纹理坐标点的像素。
        GL_LINEAR,                  //线性插值过滤，获取坐标点附近4个像素的加权平均值。
        GL_NEAREST_MIPMAP_NEAREST,  //选择最邻近的mip层，并使用最邻近过滤。
        GL_NEAREST_MIPMAP_LINEAR,   //对两个mip层使用最邻近过滤后，对的采样的结果进行加权平均。
        GL_LINEAR_MIPMAP_NEAREST,   //选择最邻近的mip层，使用线性插值算法进行过滤。
        GL_LINEAR_MIPMAP_LINEAR     //对两个mip层使用线性插值过滤后，对的采样的结果进行加权平均，又称三线性mipmap
	};

	//当坐标超出纹理边界时，获取像素的方式
	const	GLenum GL_TextureWarps[] = {
        GL_CLAMP_TO_EDGE,           //使用边缘的最后一个像素
        GL_CLAMP,                   //使用取自纹理边框的像素，在2x2的数组中加权取得像素，实际由于硬件不支持，这种方式通常和 ClampToEdge 结果一样。
        GL_CLAMP_TO_BORDER,         //使用边框纹理单元，如果没设置边框，就使用边框颜色常量（GL_TEXTURE_BORDER_COLOR）。
        GL_REPEAT,                  //重复纹理坐标，忽略纹理坐标的整数部分。
        GL_MIRRORED_REPEAT          //对纹理坐标镜象。
	};

	//纹理数据的源格式
	const	GLenum GL_SourceFormats[] = {
		GL_RED,
		GL_RG,
		GL_RGB,
		GL_BGR,
		GL_RGBA,
		GL_BGRA,
		GL_RED_INTEGER,
		GL_RG_INTEGER,
		GL_RGB_INTEGER,
		GL_BGR_INTEGER,
		GL_RGBA_INTEGER,
		GL_BGRA_INTEGER,
		GL_STENCIL_INDEX,
		GL_DEPTH_COMPONENT,
		GL_DEPTH_STENCIL
	};

	//纹理数据的基本内部格式
    const	GLenum GL_Base_InternalFormats[] = {	//RGBA, Depth and Stencil Values		Internal Components
        GL_DEPTH_COMPONENT,                     //Depth									D
        GL_DEPTH_STENCIL,                       //Depth, Stencil						D,S
        GL_RED,                                 //Red									R
        GL_RG,                                  //Red, Green							R,G
        GL_RGB,                                 //Red, Green, Blue						R,G,B
        GL_RGBA                                 //Red, Green, Blue, Alpha				R,G,B,A
	};

	//纹理数据的定长内部格式”
	struct GL_Sized_InternalFormat
	{
		GLenum	sized;
		GLenum	base;
		union
		{
			GLint	redBits;
			GLint	depthBits;
		};
		union
		{
			GLint	greenBits;
			GLint	stencilBits;
		};
		GLint	blueBits;
		GLint	alphaBits;
		GLint	shardBits;
	};
	const	GL_Sized_InternalFormat GL_Sized_InternalFormats[] = {
        //Sized Internal Format		Base Internal Format	Red Bits        Green Bits      Blue Bits	Alpha Bits	Shared Bits
        {GL_R8,						GL_RED,					{8},            {0},            0,          0,          0},
        {GL_R8_SNORM,				GL_RED,					{8/*s8*/},      {0},            0,          0,          0},
        {GL_R16,					GL_RED,					{16},           {0},            0,          0,          0},
        {GL_R16_SNORM,				GL_RED,					{16/*s16*/},    {0},            0,          0,          0},
        {GL_RG8,					GL_RG,					{8},            {8},            0,          0,          0},
        {GL_RG8_SNORM,				GL_RG,					{8/*s8*/},      {8/*s8*/},      0,          0,          0},
        {GL_RG16,					GL_RG,					{16},           {16},           0,          0,          0},
        {GL_RG16_SNORM,				GL_RG,					{16/*s16*/},    {16/*s16*/},    0,          0,          0},
        {GL_R3_G3_B2,				GL_RGB,					{3},            {3},            2,          0,          0},
        {GL_RGB4,					GL_RGB,					{4},            {4},            4,          0,          0},
        {GL_RGB5,					GL_RGB,					{5},            {5},            5,          0,          0},
        {GL_RGB8,					GL_RGB,					{8},            {8},            8,          0,          0},
        {GL_RGB8_SNORM,				GL_RGB,					{8/*s8*/},      {8/*s8*/},      8/*s8*/,    0,          0},
        {GL_RGB10,					GL_RGB,					{10},           {10},           10,         0,          0},
        {GL_RGB12,					GL_RGB,					{12},           {12},           12,         0,          0},
        {GL_RGB16_SNORM,			GL_RGB,					{16},           {16},           16,         0,          0},
        {GL_RGBA2,					GL_RGB,					{2},            {2},            2,          2,          0},
        {GL_RGBA4,					GL_RGB,					{4},            {4},			4,			4,          0},
        {GL_RGB5_A1,				GL_RGBA,				{5},            {5},			5,			1,          0},
        {GL_RGBA8,					GL_RGBA,				{8},            {8},			8,			8,          0},
        {GL_RGBA8_SNORM,			GL_RGBA,				{8/*s8*/},      {8/*s8*/},      8/*s8*/,	8/*s8*/,    0},
        {GL_RGB10_A2,				GL_RGBA,				{10},           {10},           10,			2,          0},
        {GL_RGB10_A2UI,				GL_RGBA,				{10/*ui10*/},   {10/*ui10*/},   10/*ui10*/,	2/*ui2*/,   0},
        {GL_RGBA12,					GL_RGBA,				{12},			{12},           12,			12,         0},
        {GL_RGBA16,					GL_RGBA,				{16},			{16},           16,			16,         0},
        {GL_SRGB8,					GL_RGB,					{8},			{8},            8,          0,          0},
        {GL_SRGB8_ALPHA8,			GL_RGBA,				{8},			{8},            8,			8,          0},
        {GL_R16F,					GL_RED,					{16/*f16*/},    {0},            0,          0,          0},
        {GL_RG16F,					GL_RG,					{16/*f16*/},	{16/*f16*/},    0,          0,          0},
        {GL_RGB16F,					GL_RGB,					{16/*f16*/},	{16/*f16*/},	16/*f16*/,  0,          0},
        {GL_RGBA16F,				GL_RGBA,				{16/*f16*/},	{16/*f16*/},	16/*f16*/,	16/*f16*/,  0},
        {GL_R32F,					GL_RED,					{32/*f32*/},    {0},            0,          0,          0},
        {GL_RG32F,					GL_RG,					{32/*f32*/},    {32/*f32*/},    0,          0,          0},
        {GL_RGB32F,					GL_RGB,					{32/*f32*/},	{32/*f32*/},	32/*f32*/,  0,          0},
        {GL_RGBA32F,				GL_RGBA,				{32/*f32*/},	{32/*f32*/},	32/*f32*/,	32/*f32*/,  0},
        {GL_R11F_G11F_B10F,			GL_RGB,					{11/*f11*/},	{11/*f11*/},	10/*f10*/,  0,          0},
        {GL_RGB9_E5,				GL_RGB,					{9},            {9},			9,			0,			5},
        {GL_R8I,					GL_RED,					{8/*i8*/},      {0},            0,          0,          0},
        {GL_R8UI,					GL_RED,					{8/*ui8*/},     {0},            0,          0,          0},
        {GL_R16I,					GL_RED,					{16/*i16*/},    {0},            0,          0,          0},
        {GL_R16UI,					GL_RED,					{16/*ui16*/},   {0},            0,          0,          0},
        {GL_R32I,					GL_RED,					{32/*i32*/},    {0},            0,          0,          0},
        {GL_R32UI,					GL_RED,					{32/*ui32*/},   {0},            0,          0,          0},
        {GL_RG8I,					GL_RG,					{8/*i8*/},      {8/*i8*/},      0,          0,          0},
        {GL_RG8UI,					GL_RG,					{8/*ui8*/},     {8/*ui8*/},     0,          0,          0},
        {GL_RG16I,					GL_RG,					{16/*i16*/},	{16/*i16*/},    0,          0,          0},
        {GL_RG16UI,					GL_RG,					{16/*ui16*/},	{16/*ui16*/},   0,          0,          0},
        {GL_RG32I,					GL_RG,					{32/*i32*/},	{32/*i32*/},    0,          0,          0},
        {GL_RG32UI,					GL_RG,					{32/*ui32*/},	{32/*ui32*/},   0,          0,          0},
        {GL_RGB8I,					GL_RGB,					{8/*i8*/},      {8/*i8*/},      8/*i8*/,    0,          0},
        {GL_RGB8UI,					GL_RGB,					{8/*ui8*/},     {8/*ui8*/},     8/*ui8*/,   0,          0},
        {GL_RGB16I,					GL_RGB,					{16/*i16*/},	{16/*i16*/},	16/*i16*/,  0,          0},
        {GL_RGB16UI,				GL_RGB,					{16/*ui16*/},	{16/*ui16*/},	16/*ui16*/, 0,          0},
        {GL_RGB32I,					GL_RGB,					{32/*i32*/},	{32/*i32*/},	32/*i32*/,  0,          0},
        {GL_RGB32UI,				GL_RGB,					{32/*ui32*/},	{32/*ui32*/},	32/*ui32*/, 0,          0},
        {GL_RGBA8I,					GL_RGBA,				{8/*i8*/},      {8/*i8*/},      8/*i8*/,	8/*i8*/,    0},
        {GL_RGBA8UI,				GL_RGBA,				{8/*ui8*/},     {8/*ui8*/},     8/*ui8*/,	8/*ui8*/,   0},
        {GL_RGBA16I,				GL_RGBA,				{16/*i16*/},	{16/*i16*/},	16/*i16*/,	16/*i16*/,  0},
        {GL_RGBA16UI,				GL_RGBA,				{16/*ui16*/},	{16/*ui16*/},	16/*ui16*/,	16/*ui16*/, 0},
        {GL_RGBA32I,				GL_RGBA,				{32/*i32*/},	{32/*i32*/},	32/*i32*/,	32/*i32*/,  0},
        {GL_RGBA32UI,				GL_RGBA,				{32/*ui32*/},	{32/*ui32*/},	32/*ui32*/,	32/*ui32*/, 0},
		//Sized Depth and Stencil Internal Formats
        {GL_DEPTH_COMPONENT16,		GL_DEPTH_COMPONENT,		{16},           {0},            0,          0,          0},
        {GL_DEPTH_COMPONENT24,		GL_DEPTH_COMPONENT,		{24},           {0},            0,          0,          0},
        {GL_DEPTH_COMPONENT32,		GL_DEPTH_COMPONENT,		{32},           {0},            0,          0,          0},
        {GL_DEPTH_COMPONENT32F,		GL_DEPTH_COMPONENT,		{32/*f32*/},    {0},            0,          0,          0},
        {GL_DEPTH24_STENCIL8,		GL_DEPTH_STENCIL,		{24},			{8},            0,          0,          0},
        {GL_DEPTH32F_STENCIL8,		GL_DEPTH_STENCIL,		{32/*f32*/},	{8},            0,          0,          0},
        {GL_STENCIL_INDEX8,			GL_STENCIL_INDEX,		{0},			{8},            0,          0,          0}
	};

	//纹理数据的压缩内部格式”
	struct GL_Compressed_InternalFormat
	{
		GLenum	compressed;
		GLenum	base;
        int32_t	specific;
	};
	const	GL_Compressed_InternalFormat GL_Compressed_InternalFormats[] = {
		//Compressed Internal Format				Base Internal Format		Type
        {GL_COMPRESSED_RED,							GL_RED,						false/*Generic*/},
		{GL_COMPRESSED_RG,							GL_RG,						false/*Generic*/},
		{GL_COMPRESSED_RGB,							GL_RGB,						false/*Generic*/},
		{GL_COMPRESSED_RGBA,						GL_RGBA,					false/*Generic*/},
		{GL_COMPRESSED_SRGB,						GL_RGB,						false/*Generic*/},
		{GL_COMPRESSED_SRGB_ALPHA,					GL_RGBA,					false/*Generic*/},
		{GL_COMPRESSED_RED_RGTC1,					GL_RED,						false/*Generic*/},
		{GL_COMPRESSED_SIGNED_RED_RGTC1,			GL_RED,						false/*Generic*/},
		{GL_COMPRESSED_RG_RGTC2,					GL_RG,						true/*Specific*/},
		{GL_COMPRESSED_SIGNED_RG_RGTC2,				GL_RG,						true/*Specific*/},
		{GL_COMPRESSED_RGBA_BPTC_UNORM,				GL_RGBA,					true/*Specific*/},
		{GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,		GL_RGBA,					true/*Specific*/},
		{GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,		GL_RGB,						true/*Specific*/},
		{GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,		GL_RGB,						true/*Specific*/}
	};

	//Buffer 对象目标类型
	const	GLenum GL_BufferTargets[] = {
		GL_ARRAY_BUFFER,				//Vertex attributes
		GL_ATOMIC_COUNTER_BUFFER,		//Atomic counter storage
		GL_COPY_READ_BUFFER,			//Buffer copy source
		GL_COPY_WRITE_BUFFER,			//Buffer copy destination
		GL_DISPATCH_INDIRECT_BUFFER,	//Indirect compute dispatch commands
		GL_DRAW_INDIRECT_BUFFER,		//Indirect command arguments
		GL_ELEMENT_ARRAY_BUFFER,		//Vertex array indices
		GL_PIXEL_PACK_BUFFER,			//Pixel read target
		GL_PIXEL_UNPACK_BUFFER,			//Texture data source
		GL_QUERY_BUFFER,				//Query result buffer
		GL_SHADER_STORAGE_BUFFER,		//Read - write storage for shaders
		GL_TEXTURE_BUFFER,				//Texture data buffer
		GL_TRANSFORM_FEEDBACK_BUFFER,	//Transform feedback buffer
		GL_UNIFORM_BUFFER				//Uniform block storage
	};

	//Buffer 对象使用方式
	const	GLenum GL_BufferUsages[] = {
		//内容只写入一次，且不会被频繁读取
		GL_STREAM_DRAW,			//应用程序写入，OpenGL读取
		GL_STREAM_READ,			//OpenGL写入，应用程序读取
		GL_STREAM_COPY,			//OpenGL写入，OpenGL读取
		//内容只写入一次，然后反复使用
		GL_STATIC_DRAW,			//应用程序写入，OpenGL读取
		GL_STATIC_READ,			//OpenGL写入，应用程序读取
		GL_STATIC_COPY,			//OpenGL写入，OpenGL读取
		//内容会被反复写入和使用
		GL_DYNAMIC_DRAW,		//应用程序写入，OpenGL读取
		GL_DYNAMIC_READ,		//OpenGL写入，应用程序读取
		GL_DYNAMIC_COPY			//OpenGL写入，OpenGL读取
	};

	//在 Map Buffer 时的访问方式
	const	GLenum GL_BufferMapAccesses[] = {
		GL_READ_ONLY,
		GL_WRITE_ONLY,
		GL_READ_WRITE
	};

	//着色器对象的类型
	const	GLenum GL_ShaderType[] = {
		GL_VERTEX_SHADER,			//顶点着色器
		GL_FRAGMENT_SHADER,			//片元着色器
        GL_GEOMETRY_SHADER,
        GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER,
		GL_COMPUTE_SHADER			//GL version is 4.3 or higher.
	};

	//GLSL Shader 中 Uniform 和 Attribute 的相关信息
    struct GL_GlsVariableInfo
	{
        const char* name;		//Uniform 或 Attribute 在着色器中的名称
        GLint symbolIndex;	//如果是 Attribute，则是类型在 GLSL_AttributeTypes 中的索引
                                //如果是 Uniform，则是类型在 GLSL_UniformTypes 中的索引
        GLint location;		//名称在着色器程序中的索引编号
        GLint elementCount;	//如果为1表示单独的变量，如果大于1则是数组的下标数量
        GLint dataLength;
        GLint dataOffset;
	}; 

	//GLSL Shader 中使用的数据类型与 OpenGL 数据类型的对应关系和格式信息
	struct GL_GlslDataType
	{
        const char* glslTypeName;		//对应的 GLSL 类型名称
        GLenum  symbolicConstant;		//OpenGL 类型符号常量
        GLenum  baseSymbolicConstant;	//对应的 OpenGL 基础类型符号常量
        GLint baseSize;				//基础类型的字节长度
        GLint componentCount;			//此类型中包含的基础类型成员数量
	};
    const	GL_GlslDataType	GLSL_AttributeTypes[] = {
        {"float",       GL_FLOAT,               GL_FLOAT,			sizeof(GLfloat),	1},
        {"vec2",		GL_FLOAT_VEC2,			GL_FLOAT,			sizeof(GLfloat),	2},
        {"vec3",		GL_FLOAT_VEC3,			GL_FLOAT,			sizeof(GLfloat),	3},
        {"vec4",		GL_FLOAT_VEC4,			GL_FLOAT,			sizeof(GLfloat),	4},
        {"mat2",		GL_FLOAT_MAT2,			GL_FLOAT,			sizeof(GLfloat),	4},
        {"mat3",		GL_FLOAT_MAT3,			GL_FLOAT,			sizeof(GLfloat),	9},
        {"mat4",		GL_FLOAT_MAT4,			GL_FLOAT,			sizeof(GLfloat),	16},
        {"mat2x3",      GL_FLOAT_MAT2x3,		GL_FLOAT,			sizeof(GLfloat),	6},
        {"mat2x4",      GL_FLOAT_MAT2x4,		GL_FLOAT,			sizeof(GLfloat),	8},
        {"mat3x2",      GL_FLOAT_MAT3x2,		GL_FLOAT,			sizeof(GLfloat),	6},
        {"mat3x4",      GL_FLOAT_MAT3x4,		GL_FLOAT,			sizeof(GLfloat),	12},
        {"mat4x2",      GL_FLOAT_MAT4x2,		GL_FLOAT,			sizeof(GLfloat),	8},
        {"mat4x3",      GL_FLOAT_MAT4x3,		GL_FLOAT,			sizeof(GLfloat),	12},

        {"double",      GL_DOUBLE,				GL_DOUBLE,			sizeof(GLdouble),	1},
        {"dvec2",       GL_DOUBLE_VEC2,         GL_DOUBLE,			sizeof(GLdouble),	2},
        {"dvec3",       GL_DOUBLE_VEC3,         GL_DOUBLE,			sizeof(GLdouble),	3},
        {"dvec4",       GL_DOUBLE_VEC4,         GL_DOUBLE,			sizeof(GLdouble),	4},
        {"dmat2",       GL_DOUBLE_MAT2,         GL_DOUBLE,			sizeof(GLdouble),	4},
        {"dmat3",       GL_DOUBLE_MAT3,         GL_DOUBLE,			sizeof(GLdouble),	9},
        {"dmat4",       GL_DOUBLE_MAT4,         GL_DOUBLE,			sizeof(GLdouble),	16},
        {"dmat2x3",     GL_DOUBLE_MAT2x3,		GL_DOUBLE,			sizeof(GLdouble),	6},
        {"dmat2x4",     GL_DOUBLE_MAT2x4,		GL_DOUBLE,			sizeof(GLdouble),	8},
        {"dmat3x2",     GL_DOUBLE_MAT3x2,		GL_DOUBLE,			sizeof(GLdouble),	6},
        {"dmat3x4",     GL_DOUBLE_MAT3x4,		GL_DOUBLE,			sizeof(GLdouble),	12},
        {"dmat4x2", 	GL_DOUBLE_MAT4x2,		GL_DOUBLE,			sizeof(GLdouble),	8},
        {"dmat4x3", 	GL_DOUBLE_MAT4x3,		GL_DOUBLE,			sizeof(GLdouble),	12},

        {"int",         GL_INT,                 GL_INT,				sizeof(GLint),		1},
        {"ivec2",       GL_INT_VEC2,			GL_INT,				sizeof(GLint),		2},
        {"ivec3",       GL_INT_VEC3,			GL_INT,				sizeof(GLint),		3},
        {"ivec4",       GL_INT_VEC4,			GL_INT,				sizeof(GLint),		4},

        {"unsigned int",GL_UNSIGNED_INT,		GL_UNSIGNED_INT,	sizeof(GLuint),		1},
        {"uvec2",       GL_UNSIGNED_INT_VEC2,	GL_UNSIGNED_INT,	sizeof(GLuint),		2},
        {"uvec3",       GL_UNSIGNED_INT_VEC3,	GL_UNSIGNED_INT,	sizeof(GLuint),		3},
        {"uvec4",       GL_UNSIGNED_INT_VEC4,	GL_UNSIGNED_INT,	sizeof(GLuint),		4}
	};

    const GL_GlslDataType	GLSL_UniformTypes[] = {
        {"float",       GL_FLOAT,               GL_FLOAT,			sizeof(GLfloat),	1},
        {"vec2",		GL_FLOAT_VEC2,          GL_FLOAT,			sizeof(GLfloat),	2},
        {"vec3",		GL_FLOAT_VEC3,          GL_FLOAT,			sizeof(GLfloat),	3},
        {"vec4",		GL_FLOAT_VEC4,          GL_FLOAT,			sizeof(GLfloat),	4},
        {"mat2",		GL_FLOAT_MAT2,          GL_FLOAT,			sizeof(GLfloat),	4},
        {"mat3",		GL_FLOAT_MAT3,          GL_FLOAT,			sizeof(GLfloat),	9},
        {"mat4",		GL_FLOAT_MAT4,          GL_FLOAT,			sizeof(GLfloat),	16},
        {"mat2x3",      GL_FLOAT_MAT2x3,        GL_FLOAT,			sizeof(GLfloat),	6},
        {"mat2x4",      GL_FLOAT_MAT2x4,        GL_FLOAT,			sizeof(GLfloat),	8},
        {"mat3x2",      GL_FLOAT_MAT3x2,        GL_FLOAT,			sizeof(GLfloat),	6},
        {"mat3x4",      GL_FLOAT_MAT3x4,        GL_FLOAT,			sizeof(GLfloat),	12},
        {"mat4x2",      GL_FLOAT_MAT4x2,        GL_FLOAT,			sizeof(GLfloat),	8},
        {"mat4x3",      GL_FLOAT_MAT4x3,        GL_FLOAT,			sizeof(GLfloat),	12},

        {"double",  	GL_DOUBLE,              GL_DOUBLE,			sizeof(GLdouble),	1},
        {"dvec2",       GL_DOUBLE_VEC2,         GL_DOUBLE,			sizeof(GLdouble),	2},
        {"dvec3",       GL_DOUBLE_VEC3,         GL_DOUBLE,			sizeof(GLdouble),	3},
        {"dvec4",       GL_DOUBLE_VEC4,         GL_DOUBLE,			sizeof(GLdouble),	4},
        {"dmat2",       GL_DOUBLE_MAT2,         GL_DOUBLE,			sizeof(GLdouble),	4},
        {"dmat3",       GL_DOUBLE_MAT3,         GL_DOUBLE,			sizeof(GLdouble),	9},
        {"dmat4",       GL_DOUBLE_MAT4,         GL_DOUBLE,			sizeof(GLdouble),	16},
        {"dmat2x3", 	GL_DOUBLE_MAT2x3,       GL_DOUBLE,			sizeof(GLdouble),	6},
        {"dmat2x4",     GL_DOUBLE_MAT2x4,       GL_DOUBLE,			sizeof(GLdouble),	8},
        {"dmat3x2",     GL_DOUBLE_MAT3x2,       GL_DOUBLE,			sizeof(GLdouble),	6},
        {"dmat3x4",     GL_DOUBLE_MAT3x4,       GL_DOUBLE,			sizeof(GLdouble),	12},
        {"dmat4x2", 	GL_DOUBLE_MAT4x2,       GL_DOUBLE,			sizeof(GLdouble),	8},
        {"dmat4x3", 	GL_DOUBLE_MAT4x3,       GL_DOUBLE,			sizeof(GLdouble),	12},

        {"int",         GL_INT,                 GL_INT,				sizeof(GLint),		1},
        {"ivec2",   	GL_INT_VEC2,            GL_INT,				sizeof(GLint),		2},
        {"ivec3",       GL_INT_VEC3,            GL_INT,				sizeof(GLint),		3},
        {"ivec4",       GL_INT_VEC4,            GL_INT,				sizeof(GLint),		4},

        {"unsigned int",GL_UNSIGNED_INT,        GL_UNSIGNED_INT,	sizeof(GLuint),		1},
        {"uvec2",       GL_UNSIGNED_INT_VEC2,   GL_UNSIGNED_INT,	sizeof(GLuint),		2},
        {"uvec3",   	GL_UNSIGNED_INT_VEC3,   GL_UNSIGNED_INT,	sizeof(GLuint),		3},
        {"uvec4",       GL_UNSIGNED_INT_VEC4,   GL_UNSIGNED_INT,	sizeof(GLuint),		4},
	
        {"bool",		GL_BOOL,                GL_BOOL,			sizeof(GLint),		1},
        {"bvec2",   	GL_BOOL_VEC2,           GL_BOOL,			sizeof(GLint),		2},
        {"bvec3",       GL_BOOL_VEC3,           GL_BOOL,			sizeof(GLint),		3},
        {"bvec4",       GL_BOOL_VEC4,           GL_BOOL,			sizeof(GLint),		4},

        {"sampler1D",           GL_SAMPLER_1D,                              GL_SAMPLER,		sizeof(GLuint), 1},
        {"sampler2D",           GL_SAMPLER_2D,                              GL_SAMPLER,		sizeof(GLuint),	1},
        {"sampler3D",           GL_SAMPLER_3D,                              GL_SAMPLER,		sizeof(GLuint),	1},
        {"samplerCube",         GL_SAMPLER_CUBE,                            GL_SAMPLER,		sizeof(GLuint),	1},
        {"sampler1DShadow",     GL_SAMPLER_1D_SHADOW,                       GL_SAMPLER,		sizeof(GLuint),	1},
        {"sampler2DShadow",     GL_SAMPLER_2D_SHADOW,                       GL_SAMPLER,		sizeof(GLuint),	1},
        {"sampler1DArray",      GL_SAMPLER_1D_ARRAY,                        GL_SAMPLER,		sizeof(GLuint),	1},
        {"sampler2DArray",      GL_SAMPLER_2D_ARRAY,                        GL_SAMPLER,		sizeof(GLuint),	1},
        {"sampler1DArrayShadow",GL_SAMPLER_1D_ARRAY_SHADOW,                 GL_SAMPLER,     sizeof(GLuint),	1},
        {"sampler2DArrayShadow",GL_SAMPLER_2D_ARRAY_SHADOW,                 GL_SAMPLER,     sizeof(GLuint),	1},
        {"sampler2DMS",			GL_SAMPLER_2D_MULTISAMPLE,                  GL_SAMPLER,     sizeof(GLuint),	1},
        {"sampler2DMSArray",	GL_SAMPLER_2D_MULTISAMPLE_ARRAY,            GL_SAMPLER,     sizeof(GLuint),	1},
        {"samplerCubeShadow",	GL_SAMPLER_CUBE_SHADOW,                     GL_SAMPLER,     sizeof(GLuint),	1},
        {"samplerBuffer",		GL_SAMPLER_BUFFER,                          GL_SAMPLER,     sizeof(GLuint),	1},
        {"sampler2DRect",       GL_SAMPLER_2D_RECT,                         GL_SAMPLER,     sizeof(GLuint), 1},
        {"sampler2DRectShadow", GL_SAMPLER_2D_RECT_SHADOW,                  GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler1D",          GL_INT_SAMPLER_1D,                          GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler2D",          GL_INT_SAMPLER_2D,                          GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler3D",          GL_INT_SAMPLER_3D,                          GL_SAMPLER,     sizeof(GLuint), 1},
        {"isamplerCube",        GL_INT_SAMPLER_CUBE,                        GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler1DArray",     GL_INT_SAMPLER_1D_ARRAY,                    GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler2DArray",     GL_INT_SAMPLER_2D_ARRAY,                    GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler2DMS",        GL_INT_SAMPLER_2D_MULTISAMPLE,              GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler2DMSArray",   GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,        GL_SAMPLER,     sizeof(GLuint), 1},
        {"isamplerBuffer",      GL_INT_SAMPLER_BUFFER,                      GL_SAMPLER,     sizeof(GLuint), 1},
        {"isampler2DRect",      GL_INT_SAMPLER_2D_RECT,                     GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler1D",          GL_UNSIGNED_INT_SAMPLER_1D,                 GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler2D",          GL_UNSIGNED_INT_SAMPLER_2D,                 GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler3D",          GL_UNSIGNED_INT_SAMPLER_3D,                 GL_SAMPLER,     sizeof(GLuint), 1},
        {"usamplerCube",        GL_UNSIGNED_INT_SAMPLER_CUBE,               GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler2DArray",     GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,           GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler2DArray",     GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,           GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler2DMS",        GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,     GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler2DMSArray",   GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,   GL_SAMPLER, sizeof(GLuint), 1},
        {"usamplerBuffer",      GL_UNSIGNED_INT_SAMPLER_BUFFER,             GL_SAMPLER,     sizeof(GLuint), 1},
        {"usampler2DRect",      GL_UNSIGNED_INT_SAMPLER_2D_RECT,            GL_SAMPLER,     sizeof(GLuint), 1},
        {"image1D",             GL_IMAGE_1D,                                GL_SAMPLER,     sizeof(GLuint), 1},
        {"image2D",             GL_IMAGE_2D,                                GL_SAMPLER,     sizeof(GLuint), 1},
        {"image3D",             GL_IMAGE_3D,                                GL_SAMPLER,     sizeof(GLuint), 1},
        {"image2DRect",         GL_IMAGE_2D_RECT,                           GL_SAMPLER,     sizeof(GLuint), 1},
        {"imageCube",           GL_IMAGE_CUBE,                              GL_SAMPLER,     sizeof(GLuint), 1},
        {"imageBuffer",         GL_IMAGE_BUFFER,                            GL_SAMPLER,     sizeof(GLuint), 1},
        {"image1DArray",        GL_IMAGE_1D_ARRAY,                          GL_SAMPLER,     sizeof(GLuint), 1},
        {"image2DArray",        GL_IMAGE_2D_ARRAY,                          GL_SAMPLER,     sizeof(GLuint), 1},
        {"image2DMS",           GL_IMAGE_2D_MULTISAMPLE,                    GL_SAMPLER,     sizeof(GLuint), 1},
        {"image2DMSArray",      GL_IMAGE_2D_MULTISAMPLE_ARRAY,              GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage1D",            GL_INT_IMAGE_1D,                            GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage2D",            GL_INT_IMAGE_2D,                            GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage3D",            GL_INT_IMAGE_3D,                            GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage2DRect",        GL_INT_IMAGE_2D_RECT,                       GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimageCube",          GL_INT_IMAGE_CUBE,                          GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimageBuffer",        GL_INT_IMAGE_BUFFER,                        GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage1DArray",       GL_INT_IMAGE_1D_ARRAY,                      GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage2DArray",       GL_INT_IMAGE_2D_ARRAY,                      GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage2DMS",          GL_INT_IMAGE_2D_MULTISAMPLE,                GL_SAMPLER,     sizeof(GLuint), 1},
        {"iimage2DMSArray",     GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY,          GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage1D",            GL_UNSIGNED_INT_IMAGE_1D,                   GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage2D",            GL_UNSIGNED_INT_IMAGE_2D,                   GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage3D",            GL_UNSIGNED_INT_IMAGE_3D,                   GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage2DRect",        GL_UNSIGNED_INT_IMAGE_2D_RECT,              GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimageCube",          GL_UNSIGNED_INT_IMAGE_CUBE,                 GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimageBuffer",        GL_UNSIGNED_INT_IMAGE_BUFFER,               GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage1DArray",       GL_UNSIGNED_INT_IMAGE_1D_ARRAY,             GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage2DArray",       GL_UNSIGNED_INT_IMAGE_2D_ARRAY,             GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage2DMS",          GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE,       GL_SAMPLER,     sizeof(GLuint), 1},
        {"uimage2DMSArray",     GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY, GL_SAMPLER,     sizeof(GLuint), 1},
        {"atomic_uint",         GL_UNSIGNED_INT_ATOMIC_COUNTER,             GL_UNSIGNED_INT,sizeof(GLuint), 1}
	};
    #define	ARRAY_SIZE(a) static_cast<int32_t>(sizeof(a)/sizeof(a[0]))
    GLint maxFrameBufferColorAttachments();  // GL_MAX_COLOR_ATTACHMENTS

    GLint maxVertexAttributeCount(); // GL_MAX_VERTEX_ATTRIBS

    GLint maxVertexUniformCount(); //GL_MAX_VERTEX_UNIFORM

    //通过 OpenGL 类型符号常量，取得对应的 GLSL 类型在 GLSL_AttributeTypes 数组中的索引。
    //返回值为 -1 时表示没有找到。
    int32_t glslAttributeTypeIndex(GLenum symbolic);

    //通过 OpenGL 类型符号常量，取得对应的 GLSL 类型在 GLSL_UniformTypes 数组中的索引。
    //返回值为 -1 时表示没有找到。
    int32_t glslUniformTypeIndex(GLenum symbolic);

    //取得 OpenGL 支持的“源像素类型”在 GL_SourceFormats 中的索引。
    //返回值为 -1 时表示没有找到，也就是符号常量 format 不是“源像素类型”。
    int32_t sourceFormatsIndex(GLenum format);

    //取得 OpenGL 支持的“源像素数据格式”在 GL_PixelDataTypes 中的索引。
    //返回值为 -1 时表示没有找到，也就是符号常量 format 不是“源像素数据格式”。
    int32_t pixelDataTypeIndex(GLenum dataType);

    //取得 OpenGL 支持的“基本内部像素类型”在 GL_Base_InternalFormats 中的索引。
    //返回值为 -1 时表示没有找到，也就是符号常量 format 不是“基本内部像素类型”。
    int32_t baseInternalFormatIndex(GLenum format);

    //取得 OpenGL 支持的“定长内部像素类型”在 GL_Sized_InternalFormats 中的索引。
    //返回值为 -1 时表示没有找到，也就是符号常量 format 不是“定长内部像素类型”。
    int32_t sizedInternalFormatIndex(GLenum format);

    //取得 OpenGL 支持的“压缩内部像素类型”在 GL_Compressed_InternalFormats 中的索引。
    //返回值为 -1 时表示没有找到，也就是符号常量 format 不是“压缩内部像素类型”。
    int32_t compressedInternalFormatIndex(GLenum format);
    //取得指定的像素类型符号常量对应的“基本内部像素类型”。
    //返回值为 -1 时表示没有找到，也就是符号常量 format 不是任何一种内部像素类型。
    GLenum mapToBaseInternalFormat(GLenum format);

    //检查一个符号常量是否是内部像素类型
    bool isInternalFormat(GLenum format);
}

#endif  //GUEEGLDATATYPE_H
