
struct GeometryInput
{
    uint primitive : INDEX;
};

struct PixelInput
{
    float4 posH : SV_POSITION;
};

struct MeshData
{
    float4 fMin;
    float4 fMax;
};

ConstantBuffer<MeshData> Instance : register(b0);

struct ViewData
{
    row_major matrix ViewProj;
};
ConstantBuffer<ViewData> View : register(b1);

const static float3 _kBoxMin = float3(-1.0f, -1.0f, -1.0f);
const static float3 _kBoxMax = float3(+1.0f, +1.0f, +1.0f);
const static float3 _kBoxVerts[8] =
{
    float3(_kBoxMax.x, _kBoxMax.y, _kBoxMin.z),
	float3(_kBoxMax.x, _kBoxMin.y, _kBoxMin.z),
	float3(_kBoxMax),
	float3(_kBoxMax.x, _kBoxMin.y, _kBoxMax.z),
	float3(_kBoxMin.x, _kBoxMax.y, _kBoxMin.z),
    float3(_kBoxMin),
	float3(_kBoxMin.x, _kBoxMax.y, _kBoxMax.z),
	float3(_kBoxMin.x, _kBoxMin.y, _kBoxMax.z)
};

const static uint _kusBoxIndeces[36] =
{
    4, 2, 0,
    2, 7, 3,
    6, 5, 7,
    1, 7, 5,
    0, 3, 1,
    4, 1, 5,
    4, 6, 2,
    2, 6, 7,
    6, 4, 5,
    1, 3, 7,
    0, 2, 3,
    4, 0, 1
};

[maxvertexcount(36)]
void main(point GeometryInput input[1], inout LineStream<PixelInput> lineStream)
{
	// temp box values 
    float4 _kBoxVertsW[8];

	// calcualting half and extends of the AABB
    float3 fCenter = (Instance.fMax.xyz + Instance.fMin.xyz) * 0.5f;
    float3 fHalfSize = (Instance.fMax.xyz - Instance.fMin.xyz) * 0.5f;

	// converting all 8 box vertexes to Homo coordinate
	[unroll]
    for (uint i = 0; i < 8; ++i)
    {
        _kBoxVertsW[i].xyz = _kBoxVerts[i] * fHalfSize + fCenter;
        _kBoxVertsW[i] = mul(float4(_kBoxVertsW[i].xyz, 1.0f), View.ViewProj);
    }
	
	// for each pair of line, adding to stream
	[unroll]
    for (i = 0; i < 36; i += 3)
    {
        lineStream.Append((PixelInput) _kBoxVertsW[_kusBoxIndeces[i + 0]]);
        lineStream.Append((PixelInput) _kBoxVertsW[_kusBoxIndeces[i + 1]]);
        lineStream.Append((PixelInput) _kBoxVertsW[_kusBoxIndeces[i + 2]]);
		
        lineStream.RestartStrip();
    }
}