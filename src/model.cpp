#include "direct3d.h"
#include "model.h"
using namespace DirectX;
#include "DirectXTex.h"
#include "shader3d.h"
#include "texture.h"

// 頂点構造体
struct Vertex3d
{
	XMFLOAT3 position; // 頂点座標
	XMFLOAT3 normal;   // 法線
	XMFLOAT4 color;    // 色
	XMFLOAT2 texcoord; // テクスチャ座標
};

static int g_WhiteTexId = -1;

MODEL* ModelLoad(const char* FileName, float scale, bool isBlender)
{
	MODEL* model = new MODEL;

	const std::string modelPath(FileName);

	model->AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(model->AiScene);

	model->VertexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes];
	model->IndexBuffer = new ID3D11Buffer * [model->AiScene->mNumMeshes];

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// 頂点バッファ生成
		{
			Vertex3d* vertex = new Vertex3d[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{

				if (isBlender == true)
				{
					// ブレンダー用
					vertex[v].position = XMFLOAT3(mesh->mVertices[v].x, -mesh->mVertices[v].z, mesh->mVertices[v].y);
					vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, -mesh->mNormals[v].z, mesh->mNormals[v].y);

				}
				if (isBlender == false)
				{
					// Maya用
					vertex[v].position = XMFLOAT3(mesh->mVertices[v].x * scale, mesh->mVertices[v].y * scale, mesh->mVertices[v].z * scale);
					vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				}

				vertex[v].texcoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				vertex[v].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			D3D11_BUFFER_DESC bd{};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(Vertex3d) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd{};
			sd.pSysMem = vertex;

			Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer[m]);

			delete[] vertex;
		}

		// インデックスバッファ生成
		{
			unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				assert(face->mNumIndices == 3);

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd{};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd{};
			sd.pSysMem = index;

			Direct3D_GetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer[m]);

			delete[] index;
		}
	}

	//テクスチャ読み込み
	for (int i = 0; i < model->AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = model->AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture;
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICMemory((const uint8_t*)aitexture->pcData, aitexture->mWidth, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(Direct3D_GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &texture);
		assert(texture);

		model->Texture[aitexture->mFilename.data] = texture;
	}

	g_WhiteTexId = Texture_Load(L"texture/siro.png");// サーフェースカラー用

	return model;
}

void ModelRelease(MODEL* model)
{
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		model->VertexBuffer[m]->Release();
		model->IndexBuffer[m]->Release();
	}

	delete[] model->VertexBuffer;
	delete[] model->IndexBuffer;

	for (std::pair<const std::string, ID3D11ShaderResourceView*> pair : model->Texture)
	{
		pair.second->Release();
	}

	aiReleaseImport(model->AiScene);

	delete model;
}

void ModelDraw(const MODEL* model, const DirectX::XMMATRIX& mtxWorld)
{
	// シェーダーを描画パイプラインに設定
	Shader3d_Begin();

	for (int i = 0; i < model->AiScene->mNumMeshes; i++)
	{
		// 頂点バッファを描画パイプラインに設定
		UINT stride = sizeof(Vertex3d);
		UINT offset = 0;

		// プリミティブトポロジ設定
		Direct3D_GetContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[i], &stride, &offset);

		// 頂点インデックスを描画をパイプラインに設定
		Direct3D_GetContext()->IASetIndexBuffer(model->IndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

		// ワールド変換行列を計算
		Shader3d_SetWorldMatrix(mtxWorld);

		// プリミティブトポロジ設定
		Direct3D_GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//テクスチャの設定

		aiString texture;
		aiMaterial* aimaterial = model->AiScene->mMaterials[model->AiScene->mMeshes[i]->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

		if (texture.length != 0)
		{
			Direct3D_GetContext()->PSSetShaderResources(0, 1, &model->Texture.at(texture.data));
			Shader3d_SetMaterialDiffuse({ 1.0f,1.0f,1.0f,1.0f });
		}
		else
		{
			Texture_SetTexture(g_WhiteTexId);
			aiColor3D diffuse;
			aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			Shader3d_SetMaterialDiffuse({ diffuse.r,diffuse.g,diffuse.b,1.0f });
		}


		// ポリゴン描画命令発行
		Direct3D_GetContext()->DrawIndexed(model->AiScene->mMeshes[i]->mNumFaces * 3, 0, 0);
	}
}






