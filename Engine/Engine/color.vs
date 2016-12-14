// TYPEDEF
struct VertexInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};
struct HullInputType
{
    float3 position : POSITION;
    float4 color: COLOR;
};

// Vertex Shader
HullInputType ColorVertexShader(VertexInputType input)
{
    HullInputType output;

    // Pass vertices and color data through to hull shader
    output.position = input.position;
    output.color = input.color;

    return output;
}