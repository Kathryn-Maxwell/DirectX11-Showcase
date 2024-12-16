#include "TessellationPlaneMesh.h"

// initialise buffers and load texture.
TessellationPlaneMesh::TessellationPlaneMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMINT2 _size)
{
	size = _size;
	initBuffers(device);
}

// Release resources.
TessellationPlaneMesh::~TessellationPlaneMesh()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

// Build plane (with texture coordinates and normals).
void TessellationPlaneMesh::initBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// TERRAIN_RESOLUTION² number of quads, each with 4 verts
	VertexType* vertices;
	vertexCount = (size.x) * (size.y) * 4;
	vertices = new VertexType[vertexCount];

	// TERRAIN_RESOLUTION² number of quads, each with 12 control points, so neighbouring tessellation may be calculated
	unsigned long* indices;
	indexCount = (size.x) * (size.y) * 12;
	indices = new unsigned long[indexCount];

	// UV coords.
	float u = 0;
	float v = 0;
	XMFLOAT2 increment = XMFLOAT2(1.0f / (float)size.x, 1.0f / (float)size.y);

	for (int x = 0; x < (size.x); x++)
	{
		for (int z = 0; z < (size.y); z++)
		{
			int vertexIndex = (x * size.x + z) * 4;
			int indiceIndex = (x * size.x + z) * 12;

			// Top left
			vertices[vertexIndex].position = XMFLOAT3((float)x, 0.0f, (float)z + 1.0f);
			vertices[vertexIndex].texture = XMFLOAT2(u, v + increment.y);
			vertices[vertexIndex].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			indices[indiceIndex] = vertexIndex;

			// Bottom left.
			vertices[vertexIndex + 1].position = XMFLOAT3((float)x, 0.0f, (float)z);
			vertices[vertexIndex + 1].texture = XMFLOAT2(u, v);
			vertices[vertexIndex + 1].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			indices[indiceIndex + 1] = vertexIndex + 1;

			// Bottom right
			vertices[vertexIndex + 2].position = XMFLOAT3((float)x + 1.0f, 0.0f, (float)z);
			vertices[vertexIndex + 2].texture = XMFLOAT2(u + increment.x, v);
			vertices[vertexIndex + 2].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			indices[indiceIndex + 2] = vertexIndex + 2;

			// Top right.
			vertices[vertexIndex + 3].position = XMFLOAT3((float)x + 1.0f, 0.0f, (float)z + 1);
			vertices[vertexIndex + 3].texture = XMFLOAT2(u + increment.x, v + increment.y);
			vertices[vertexIndex + 3].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			indices[indiceIndex + 3] = vertexIndex + 3;

			// right
			indices[indiceIndex + 4] = clamp(vertexIndex + (size.x * 4) + 2, 0, vertexCount); // Bottom right of right quad
			indices[indiceIndex + 5] = clamp(vertexIndex + (size.x * 4) + 3, 0, vertexCount); // Top right of right quad

			// up
			indices[indiceIndex + 6] = clamp(vertexIndex + 4, 0, vertexCount); // Top left of 'above' quad
			indices[indiceIndex + 7] = clamp(vertexIndex + 4 + 3, 0, vertexCount); // Top right of 'above' quad

			// left
			indices[indiceIndex + 8] = clamp(vertexIndex - (size.x * 4) + 1, 0, vertexCount); // Bottom left of left quad
			indices[indiceIndex + 9] = clamp(vertexIndex - (size.x * 4), 0, vertexCount); // Top left of left quad

			// down
			indices[indiceIndex + 10] = clamp(vertexIndex - 4 + 1, 0, vertexCount); // Bottom left of 'below' quad
			indices[indiceIndex + 11] = clamp(vertexIndex - 4 + 2, 0, vertexCount); // Bottom right of 'below' quad

			v += increment.y;
		}

		v = 0;
		u += increment.x;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	// Now create the vertex buffer.
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;
	// Create the index buffer.
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

// Override sendData() to change topology type. Control point patch list is required for tessellation.
void TessellationPlaneMesh::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	// Set the type of primitive that should be rendered from this vertex buffer, in this case control patch for tessellation.
	deviceContext->IASetPrimitiveTopology(top);
}

// Helper function to clamp a value between a minimum and maximim
int TessellationPlaneMesh::clamp(int _value, int _min, int _max) {
	return (max(min((_value), (_max)), (_min)));
}