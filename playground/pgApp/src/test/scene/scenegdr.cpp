#include "scenegdr.h"
#include "engine/utils/mathutils.h"

using namespace Diligent;

const uint16_t SceneGdr::s_maxNoofProps = 10;
const uint16_t SceneGdr::s_maxNoofInstances = 2048;

void SceneGdr::create()
{
    m_totalInstancesCount = 0;

    m_noofProps = 0;

    m_props = new Prop[s_maxNoofProps];

    // first create space for some materials
    m_materials = new MaterialColor[s_maxNoofProps];
    m_noofMaterials = 0;

    // add a ground plane
    {
        Prop& prop = m_props[m_noofProps++];

        prop.m_renderPass = RenderPass::MainPass;

        createCubeMesh(prop);

        prop.m_noofInstances = 1;
        prop.m_instances = new InstanceData[prop.m_noofInstances];

        // prop.m_instances->m_world = float4x4::Scale(100.0f, 0.1f, 100.0f) *
        // float4x4::RotationZ(0.0f) * float4x4::RotationY(0.0f) *
        // float4x4::RotationX(0.0f) *
        // float4x4::Translation(0.0f, 0.0f, 0.0f);
        ade::mtxSRT(prop.m_instances->m_world, 100.0f, 0.1f, 100.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                   0.0f);

        float4 temp;
        setVector4(temp, -0.5f, -0.5f, -0.5f, 1.0f);
        ade::vec4MulMtx(prop.m_instances->m_bboxMin, temp, prop.m_instances->m_world);

        setVector4(temp, 0.5f, 0.5f, 0.5f, 1.0f);
        ade::vec4MulMtx(prop.m_instances->m_bboxMax, temp, prop.m_instances->m_world);

        prop.m_materialID = m_noofMaterials;
        setVector4(m_materials[prop.m_materialID].m_color, 0.0f, 0.6f, 0.0f, 1.0f);
        m_noofMaterials++;

        m_totalInstancesCount += prop.m_noofInstances;
    }

    // add a few instances of the occluding mesh
    {
        Prop& prop = m_props[m_noofProps++];

        prop.m_renderPass = RenderPass::All;

        // create prop
        createCubeMesh(prop);

        // add a few instances of the wall mesh
        prop.m_noofInstances = 25;
        prop.m_instances = new InstanceData[prop.m_noofInstances];
        for (int i = 0; i < prop.m_noofInstances; i++) {
            // calculate world position
            ade::mtxSRT(prop.m_instances[i].m_world, 40.0f, 10.0f, 0.1f, 0.0f,
                       (rand01() * 120.0f - 60.0f) * 3.1459f / 180.0f, 0.0f,
                       rand01() * 100.0f - 50.0f, 5.0f, rand01() * 100.0f - 50.0f);

            // calculate bounding box and transform to world space
            float4 temp;
            setVector4(temp, -0.5f, -0.5f, -0.5f, 1.0f);
            ade::vec4MulMtx(prop.m_instances[i].m_bboxMin, temp, prop.m_instances[i].m_world);

            setVector4(temp, 0.5f, 0.5f, 0.5f, 1.0f);
            ade::vec4MulMtx(prop.m_instances[i].m_bboxMax, temp, prop.m_instances[i].m_world);
        }

        // set the material ID. Will be used in the shader to select the material
        prop.m_materialID = m_noofMaterials;

        // add a "material" for this prop
        setVector4(m_materials[prop.m_materialID].m_color, 0.0f, 0.0f, 1.0f, 0.0f);
        m_noofMaterials++;

        m_totalInstancesCount += prop.m_noofInstances;
    }

    // add a few "regular" props
    {
        // add cubes
        {
            Prop& prop = m_props[m_noofProps++];

            prop.m_renderPass = RenderPass::MainPass;

            createCubeMesh(prop);

            prop.m_noofInstances = 200;
            prop.m_instances = new InstanceData[prop.m_noofInstances];
            for (int i = 0; i < prop.m_noofInstances; i++) {
                ade::mtxSRT(prop.m_instances[i].m_world, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f,
                           rand01() * 100.0f - 50.0f, 1.0f, rand01() * 100.0f - 50.0f);

                float4 temp;
                setVector4(temp, -0.5f, -0.5f, -0.5f, 1.0f);
                ade::vec4MulMtx(prop.m_instances[i].m_bboxMin, temp, prop.m_instances[i].m_world);

                setVector4(temp, 0.5f, 0.5f, 0.5f, 1.0f);
                ade::vec4MulMtx(prop.m_instances[i].m_bboxMax, temp, prop.m_instances[i].m_world);
            }

            prop.m_materialID = m_noofMaterials;
            setVector4(m_materials[prop.m_materialID].m_color, 1.0f, 1.0f, 0.0f, 1.0f);
            m_noofMaterials++;

            m_totalInstancesCount += prop.m_noofInstances;
        }

        // add some more cubes
        {
            Prop& prop = m_props[m_noofProps++];

            prop.m_renderPass = RenderPass::MainPass;

            createCubeMesh(prop);

            prop.m_noofInstances = 300;
            prop.m_instances = new InstanceData[prop.m_noofInstances];
            for (int i = 0; i < prop.m_noofInstances; i++) {
                ade::mtxSRT(prop.m_instances[i].m_world, 2.0f, 4.0f, 2.0f, 0.0f, 0.0f, 0.0f,
                           rand01() * 100.0f - 50.0f, 2.0f, rand01() * 100.0f - 50.0f);

                float4 temp;
                setVector4(temp, -0.5f, -0.5f, -0.5f, 1.0f);
                ade::vec4MulMtx(prop.m_instances[i].m_bboxMin, temp, prop.m_instances[i].m_world);

                setVector4(temp, 0.5f, 0.5f, 0.5f, 1.0f);
                ade::vec4MulMtx(prop.m_instances[i].m_bboxMax, temp, prop.m_instances[i].m_world);
            }

            prop.m_materialID = m_noofMaterials;
            setVector4(m_materials[prop.m_materialID].m_color, 1.0f, 0.0f, 0.0f, 1.0f);
            m_noofMaterials++;

            m_totalInstancesCount += prop.m_noofInstances;
        }
    }
}
