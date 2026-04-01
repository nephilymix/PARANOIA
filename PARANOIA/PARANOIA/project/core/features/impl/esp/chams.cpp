#include <stdafx.hpp>

namespace features::esp {

    struct gpu_vertex_t {
        float position[3];
        float normal[3];
        uint32_t bone_indices;
        float weights[4];
    };

    struct cb_view_projection_t {
        float vp[4][4];
        float screen_w;
        float screen_h;
        float pad[2];
    };

    struct cb_bone_matrices_t {
        float bones[128][16];
    };

    struct cb_material_t {
        float color[4];
        float wire_color[4];
        int   render_mode;
        float alpha;
        int   material_type;
        float rim_power;
        float cam_pos[3];
        float pad;
    };

    static const char* s_shader = R"(
        cbuffer CBViewProjection : register(b0) {
            row_major float4x4 g_VP;
            float g_ScreenW;
            float g_ScreenH;
            float2 _pad0;
        };
        cbuffer CBBoneMatrices : register(b1) {
            row_major float4x4 g_Bones[128];
        };
        cbuffer CBMaterial : register(b2) {
            float4 g_FillCol;
            float4 g_WireCol;
            int    g_Mode;
            float  g_Alpha;
            int    g_MatType; 
            float  g_Rim;     
            float3 g_CamPos;  
            float  _pad1;
        };
        struct VS_IN {
            float3 Pos : POSITION;
            float3 Norm : NORMAL;
            uint4  Bones : BLENDINDICES;
            float4 Weights : BLENDWEIGHT;
        };
        struct PS_IN {
            float4 Pos : SV_POSITION;
            float3 WPos : TEXCOORD0;
            float3 WNorm: TEXCOORD1;
        };
        PS_IN VS_Skinning(VS_IN input) {
            PS_IN output = (PS_IN)0;
            float4 sp = float4(0,0,0,0);
            float3 sn = float3(0,0,0);
            [unroll] for (int i=0; i<4; i++) {
                float w = input.Weights[i];
                if (w > 0.0001f) {
                    uint b = input.Bones[i];
                    sp += mul(g_Bones[b], float4(input.Pos, 1.0f)) * w;
                    sn += mul((float3x3)g_Bones[b], input.Norm) * w;
                }
            }
            output.WPos = sp.xyz;
            output.WNorm = normalize(sn);
            
            float4 clip = mul(g_VP, sp);
            if (clip.w < 0.01f) {
                output.Pos = float4(0,0,-1,1);
                return output;
            }
            float inv_w = 1.0f / clip.w;
            float nx = clip.x * inv_w;
            float ny = clip.y * inv_w;
            float sx = g_ScreenW * 0.5f + nx * g_ScreenW * 0.5f;
            float sy = g_ScreenH * 0.5f - ny * g_ScreenH * 0.5f;
            output.Pos = float4((sx/g_ScreenW)*2.0f-1.0f, -((sy/g_ScreenH)*2.0f-1.0f), 0.5f, 1.0f);
            return output;
        }
        float4 PS_Fill(PS_IN input) : SV_TARGET {
            return float4(g_FillCol.rgb, g_FillCol.a * g_Alpha);
        }
        float4 PS_Wire(PS_IN input) : SV_TARGET {
            return float4(g_WireCol.rgb, g_WireCol.a * g_Alpha);
        }
    )";

    bool c_mesh_renderer::initialize(ID3D11Device* device, ID3D11DeviceContext* context, const std::string& mesh_path) {
        m_device = device;
        m_context = context;

        if (!load_glb(mesh_path)) return false;
        if (!setup_gpu()) return false;
        if (!upload_mesh()) return false;

        m_ready = true;
        return true;
    }

    void c_mesh_renderer::shutdown() {
        m_mesh.release();
        if (m_vs) { m_vs->Release(); m_vs = nullptr; }
        if (m_ps_fill) { m_ps_fill->Release(); m_ps_fill = nullptr; }
        if (m_ps_wire) { m_ps_wire->Release(); m_ps_wire = nullptr; }
        if (m_layout) { m_layout->Release(); m_layout = nullptr; }
        if (m_cb_vp) { m_cb_vp->Release(); m_cb_vp = nullptr; }
        if (m_cb_bones) { m_cb_bones->Release(); m_cb_bones = nullptr; }
        if (m_cb_mat) { m_cb_mat->Release(); m_cb_mat = nullptr; }
        if (m_rs_fill) { m_rs_fill->Release(); m_rs_fill = nullptr; }
        if (m_rs_wire) { m_rs_wire->Release(); m_rs_wire = nullptr; }
        if (m_blend) { m_blend->Release(); m_blend = nullptr; }
        if (m_depth) { m_depth->Release(); m_depth = nullptr; }
        m_ready = false;
    }

    std::vector<bone_matrix_3x4_t> c_mesh_renderer::convert_cached_bones(const systems::bones::data& bones, int needed_count) {
        std::vector<bone_matrix_3x4_t> matrices(needed_count);

        // We avoid memory reads by using the existing bone cache
        int limit = (std::min)(needed_count, static_cast<int>(bones.count)); // Ensure we dont out of bounds

        for (int i = 0; i < limit; i++) {
            const auto& b = bones.bones[i];
            auto& mat = matrices[i];

            // Convert quaternion rotation to 3x3 rotation matrix, assuming bone.rotation is a math::vector4 or similar quat
            float qx = b.rotation.x, qy = b.rotation.y, qz = b.rotation.z, qw = b.rotation.w;
            float xx = qx * qx, yy = qy * qy, zz = qz * qz;
            float xy = qx * qy, xz = qx * qz, yz = qy * qz;
            float wx = qw * qx, wy = qw * qy, wz = qw * qz;

            mat.m[0][0] = 1.f - 2.f * (yy + zz);
            mat.m[0][1] = 2.f * (xy - wz);
            mat.m[0][2] = 2.f * (xz + wy);
            mat.m[0][3] = b.position.x;

            mat.m[1][0] = 2.f * (xy + wz);
            mat.m[1][1] = 1.f - 2.f * (xx + zz);
            mat.m[1][2] = 2.f * (yz - wx);
            mat.m[1][3] = b.position.y;

            mat.m[2][0] = 2.f * (xz - wy);
            mat.m[2][1] = 2.f * (yz + wx);
            mat.m[2][2] = 1.f - 2.f * (xx + yy);
            mat.m[2][3] = b.position.z;
        }

        return matrices;
    }

    void c_mesh_renderer::render_player(const systems::bones::data& bones, const settings::esp::player::chams& cfg) {
        if (!m_ready || !cfg.enabled) return;

        int needed = static_cast<int>(m_mesh.gltf_to_game_bone_map.size());
        needed = (std::min)(needed, 128);

        auto game_bones = convert_cached_bones(bones, needed);
        if (game_bones.empty()) return;

        int render_mode = cfg.wireframe ? 2 : 0;

        draw_command_t cmd{};
        cmd.mesh = &m_mesh;
        cmd.fill_color = cfg.fill_color;
        cmd.wire_color = cfg.wire_color;
        cmd.alpha = cfg.alpha;
        cmd.render_mode = render_mode;
        cmd.material_type = cfg.material_type;

        size_t bone_count = m_mesh.inverse_bind_matrices.size();
        cmd.combined_matrices.resize(bone_count);

        for (size_t i = 0; i < bone_count; i++) {
            int game_idx = (i < m_mesh.gltf_to_game_bone_map.size()) ? m_mesh.gltf_to_game_bone_map[i] : 0;
            if (game_idx >= 0 && game_idx < static_cast<int>(game_bones.size())) {
                cmd.combined_matrices[i] = game_bones[game_idx].multiply(m_mesh.inverse_bind_matrices[i]);
            }
            else {
                cmd.combined_matrices[i] = bone_matrix_3x4_t::identity();
            }
        }

        m_draws.push_back(std::move(cmd));
    }

    void c_mesh_renderer::flush() {
        if (!m_ready || m_draws.empty()) return;

        // Save states
        ID3D11RasterizerState* old_rs;
        m_context->RSGetState(&old_rs);
        ID3D11BlendState* old_bs;
        float old_bf[4];
        UINT old_sm;
        m_context->OMGetBlendState(&old_bs, old_bf, &old_sm);
        ID3D11DepthStencilState* old_dss;
        UINT old_sr;
        m_context->OMGetDepthStencilState(&old_dss, &old_sr);

        // Map VP
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(m_context->Map(m_cb_vp, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* data = static_cast<cb_view_projection_t*>(mapped.pData);

            // NOTE: Retrieve ViewMatrix properly based on PARANOIA implementation
            const auto& view_matrix = systems::g_view.get_matrix(); // Check your systems::g_view implementation
            memcpy(data->vp, view_matrix.data(), sizeof(float) * 16);

            const auto [w, h] = zdraw::get_display_size();
            data->screen_w = static_cast<float>(w);
            data->screen_h = static_cast<float>(h);
            m_context->Unmap(m_cb_vp, 0);
        }

        m_context->IASetInputLayout(m_layout);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->VSSetShader(m_vs, nullptr, 0);

        float blend_factor[4] = { 0,0,0,0 };
        m_context->OMSetBlendState(m_blend, blend_factor, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_depth, 0);

        ID3D11Buffer* vs_cbs[3] = { m_cb_vp, m_cb_bones, m_cb_mat };
        m_context->VSSetConstantBuffers(0, 3, vs_cbs);
        ID3D11Buffer* ps_cbs[1] = { m_cb_mat };
        m_context->PSSetConstantBuffers(0, 1, ps_cbs);

        for (auto& cmd : m_draws) {
            // Map bones
            if (SUCCEEDED(m_context->Map(m_cb_bones, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                auto* bdata = static_cast<cb_bone_matrices_t*>(mapped.pData);
                memset(bdata, 0, sizeof(cb_bone_matrices_t));
                size_t c = (std::min)(cmd.combined_matrices.size(), size_t(128));
                for (size_t i = 0; i < c; i++) cmd.combined_matrices[i].to_4x4(bdata->bones[i]);
                m_context->Unmap(m_cb_bones, 0);
            }

            UINT stride = sizeof(gpu_vertex_t);
            UINT offset = 0;
            m_context->IASetVertexBuffers(0, 1, &cmd.mesh->gpu_vertex_buffer, &stride, &offset);
            m_context->IASetIndexBuffer(cmd.mesh->gpu_index_buffer, DXGI_FORMAT_R32_UINT, 0);

            auto update_mat = [&](int mode) {
                if (SUCCEEDED(m_context->Map(m_cb_mat, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
                    auto* mdata = static_cast<cb_material_t*>(mapped.pData);
                    mdata->color[0] = cmd.fill_color.r / 255.f; mdata->color[1] = cmd.fill_color.g / 255.f;
                    mdata->color[2] = cmd.fill_color.b / 255.f; mdata->color[3] = cmd.fill_color.a / 255.f;
                    mdata->wire_color[0] = cmd.wire_color.r / 255.f; mdata->wire_color[1] = cmd.wire_color.g / 255.f;
                    mdata->wire_color[2] = cmd.wire_color.b / 255.f; mdata->wire_color[3] = cmd.wire_color.a / 255.f;
                    mdata->render_mode = mode; mdata->alpha = cmd.alpha;
                    mdata->material_type = cmd.material_type;
                    m_context->Unmap(m_cb_mat, 0);
                }
                };

            if (cmd.render_mode == 0 || cmd.render_mode == 2) {
                update_mat(0);
                m_context->RSSetState(m_rs_fill);
                m_context->PSSetShader(m_ps_fill, nullptr, 0);
                m_context->DrawIndexed(cmd.mesh->index_count, 0, 0);
            }
            if (cmd.render_mode == 1 || cmd.render_mode == 2) {
                update_mat(1);
                m_context->RSSetState(m_rs_wire);
                m_context->PSSetShader(m_ps_wire, nullptr, 0);
                m_context->DrawIndexed(cmd.mesh->index_count, 0, 0);
            }
        }
        m_draws.clear();

        // Restore state
        m_context->RSSetState(old_rs);
        m_context->OMSetBlendState(old_bs, old_bf, old_sm);
        m_context->OMSetDepthStencilState(old_dss, old_sr);
        if (old_rs) old_rs->Release();
        if (old_bs) old_bs->Release();
        if (old_dss) old_dss->Release();
    }

    bool c_mesh_renderer::setup_gpu() {
        ID3DBlob* vs_blob = nullptr;
        ID3DBlob* ps_fill_blob = nullptr;
        ID3DBlob* ps_wire_blob = nullptr;

        D3DCompile(s_shader, strlen(s_shader), nullptr, nullptr, nullptr, "VS_Skinning", "vs_5_0", 0, 0, &vs_blob, nullptr);
        D3DCompile(s_shader, strlen(s_shader), nullptr, nullptr, nullptr, "PS_Fill", "ps_5_0", 0, 0, &ps_fill_blob, nullptr);
        D3DCompile(s_shader, strlen(s_shader), nullptr, nullptr, nullptr, "PS_Wire", "ps_5_0", 0, 0, &ps_wire_blob, nullptr);

        if (!vs_blob || !ps_fill_blob || !ps_wire_blob) return false;

        m_device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &m_vs);
        m_device->CreatePixelShader(ps_fill_blob->GetBufferPointer(), ps_fill_blob->GetBufferSize(), nullptr, &m_ps_fill);
        m_device->CreatePixelShader(ps_wire_blob->GetBufferPointer(), ps_wire_blob->GetBufferSize(), nullptr, &m_ps_wire);

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                             D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        m_device->CreateInputLayout(layout, 4, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &m_layout);

        vs_blob->Release(); ps_fill_blob->Release(); ps_wire_blob->Release();

        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DYNAMIC; desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.ByteWidth = sizeof(cb_view_projection_t); m_device->CreateBuffer(&desc, nullptr, &m_cb_vp);
        desc.ByteWidth = sizeof(cb_bone_matrices_t); m_device->CreateBuffer(&desc, nullptr, &m_cb_bones);
        desc.ByteWidth = sizeof(cb_material_t); m_device->CreateBuffer(&desc, nullptr, &m_cb_mat);

        D3D11_RASTERIZER_DESC rsd{};
        rsd.FillMode = D3D11_FILL_SOLID; rsd.CullMode = D3D11_CULL_NONE; m_device->CreateRasterizerState(&rsd, &m_rs_fill);
        rsd.FillMode = D3D11_FILL_WIREFRAME; m_device->CreateRasterizerState(&rsd, &m_rs_wire);

        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE; bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        m_device->CreateBlendState(&bd, &m_blend);

        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = FALSE; dsd.StencilEnable = FALSE;
        m_device->CreateDepthStencilState(&dsd, &m_depth);

        return true;
    }

    bool c_mesh_renderer::upload_mesh() {
        if (m_mesh.vertices.empty()) return false;
        std::vector<gpu_vertex_t> gv(m_mesh.vertices.size());
        for (size_t i = 0; i < m_mesh.vertices.size(); i++) {
            auto& s = m_mesh.vertices[i];
            gv[i].position[0] = s.position.x; gv[i].position[1] = s.position.y; gv[i].position[2] = s.position.z;
            gv[i].normal[0] = s.normal.x; gv[i].normal[1] = s.normal.y; gv[i].normal[2] = s.normal.z;
            gv[i].bone_indices = (s.joint_indices[0] & 0xFF) | ((s.joint_indices[1] & 0xFF) << 8) | ((s.joint_indices[2] & 0xFF) << 16) | ((s.joint_indices[3] & 0xFF) << 24);
            gv[i].weights[0] = s.weights[0]; gv[i].weights[1] = s.weights[1]; gv[i].weights[2] = s.weights[2]; gv[i].weights[3] = s.weights[3];
        }

        D3D11_BUFFER_DESC vd{}; vd.ByteWidth = gv.size() * sizeof(gpu_vertex_t); vd.Usage = D3D11_USAGE_IMMUTABLE; vd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vdat{}; vdat.pSysMem = gv.data();
        m_device->CreateBuffer(&vd, &vdat, &m_mesh.gpu_vertex_buffer);

        D3D11_BUFFER_DESC id{}; id.ByteWidth = m_mesh.indices.size() * sizeof(uint32_t); id.Usage = D3D11_USAGE_IMMUTABLE; id.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA idat{}; idat.pSysMem = m_mesh.indices.data();
        m_device->CreateBuffer(&id, &idat, &m_mesh.gpu_index_buffer);
        return true;
    }

    // Keep your GLB parsing logic here (omitted partially to keep sample focused on render updates, 
    // but just drop the c_mesh_loader::load_glb content inside this function using nlohmann::json)
    bool c_mesh_renderer::load_glb(const std::string& path) {
        // Your exact json parser logic goes here from snippet
        // ...
        return true;
    }

}