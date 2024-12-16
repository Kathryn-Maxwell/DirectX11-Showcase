#pragma once
// Tessellation plane mesh.
// Builds a plane subdivided into quads each with 12 control points for tessellation
// Overrides sendData() function for different primitive topology

#include "BaseMesh.h"

class TessellationPlaneMesh : public BaseMesh
{
public:
	TessellationPlaneMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMINT2 _size);
	~TessellationPlaneMesh();

	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST) override;
protected:
	void initBuffers(ID3D11Device* device);
	int clamp(int _value, int _min, int _max);

	XMINT2 size;
};