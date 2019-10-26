#pragma once

#include "engine/engine.h"
#include "engine/scene/sceneass.h"

#include "../mesh/meshprop.h"

class SceneGdr : public ade::SceneAss
{
	Prop*	m_props;
	Material* m_materials;
	uint16_t m_noofProps;
	uint16_t m_noofMaterials;
	uint16_t m_totalInstancesCount;

    static const uint16_t s_maxNoofProps;
    static const uint16_t s_maxNoofInstances;

  public:
    SceneGdr() : ade::SceneAss() {}

    void create();
};
