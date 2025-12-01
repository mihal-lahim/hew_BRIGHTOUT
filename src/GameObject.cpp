#include "GameObject.h"
#include "model.h"
#include "cube.h" // モデルがない場合の描画用

using namespace DirectX;

void GameObject::Draw() const
{
    XMMATRIX scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
    XMMATRIX trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
    XMMATRIX world = scale * rot * trans;

    if (m_pModel) {
        ModelDraw(m_pModel, world);
    }
    else if (m_TextureId != -1) {
        // モデルがない場合はテクスチャ付きのキューブを描画
        Cube_Draw(m_TextureId, world);
    }
}

AABB GameObject::GetAABB() const
{
    // 単純なAABB計算
    XMFLOAT3 min = { m_Position.x - m_Scale.x * 0.5f, m_Position.y, m_Position.z - m_Scale.z * 0.5f };
    XMFLOAT3 max = { m_Position.x + m_Scale.x * 0.5f, m_Position.y + m_Scale.y, m_Position.z + m_Scale.z * 0.5f };
    return AABB(min, max);
}