/**
* \class Base Mesh
*
* \brief The parent for all mesh objects. Provides default functionality.
*
* Can be inherited to create custom meshes. Provide functions for sending data to GPU memory, getting index count and storing geometry data.
*
* \author Paul Robertson
*/


#ifndef _BASEMESH_H_
#define _BASEMESH_H_

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

class BaseMesh
{
protected:

	/// Default struct for general vertex data include position, texture coordinates and normals
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	/// Default vertex struct for geometry with only position and colour
	struct VertexType_Colour
	{
		XMFLOAT3 position;
		XMFLOAT4 colour;
	};

	/// Default vertex struct for geometry with only position and texture coordinates.
	struct VertexType_Texture
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	/// Empty constructor
	BaseMesh();
	~BaseMesh();

	/// Transfers mesh data to the GPU.
	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	int getIndexCount();			///< Returns total index value of the mesh
	
	void setPosition(float _x, float _y, float _z) { position = XMVectorSet(_x, _y, _z, 1.0f); }
	void setPosition(XMFLOAT3 _position) { position = XMVectorSet(_position.x, _position.y, _position.z, 1.0f); }
	void setPosition(XMFLOAT4 _position) { position = XMVectorSet(_position.x, _position.y, _position.z, 1.0f); }
	XMFLOAT3 getPosition();

	XMMATRIX getTranslation() { return XMMatrixTranslation(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position)); }
protected:
	virtual void initBuffers(ID3D11Device*) = 0;

	ID3D11Buffer *vertexBuffer, *indexBuffer;
	int vertexCount, indexCount;
	XMVECTOR position;
};

#endif