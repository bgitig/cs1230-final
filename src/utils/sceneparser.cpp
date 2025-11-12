#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>


void dfsSceneGraph(RenderData &renderData, SceneNode* node, glm::mat4 parentCTM) {
    // update CTM
    glm::mat4 childCTM = parentCTM;
    for (SceneTransformation* transformation : node->transformations) {
        switch (transformation->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            childCTM = glm::translate(childCTM, transformation->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            childCTM = glm::scale(childCTM, transformation->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            childCTM = glm::rotate(childCTM, transformation->angle,transformation->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            childCTM = childCTM * transformation->matrix;
            break;
        default:
            break;
        }
    }

    // for each primitive you encounter, you should:
    // (1) construct a RenderShapeData object using the primitive and its corresponding CTM
    // (2) append the RenderShapeData onto renderData.shapes.

    // if primitive
    for (ScenePrimitive* primitive : node->primitives) {
        glm::mat3 ictm = glm::inverse(glm::transpose(glm::mat3(childCTM)));
        RenderShapeData obj = RenderShapeData{*primitive, childCTM, ictm};
        renderData.shapes.push_back(obj);
    }


    // if light
    for (SceneLight* light : node->lights) {
        renderData.lights.push_back(
            SceneLightData{
                light->id,
                light->type,
                light->color,
                light->function,
                childCTM * glm::vec4(0,0,0,1),
                childCTM * light->dir,
                light->penumbra,
                light->angle,
                light->width,
                light->height
            }
            );
    }

    // do dfs
    for (SceneNode* child : node->children) {
        dfsSceneGraph(renderData, child, childCTM);
    }


}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }
    // Task 5: populate renderData with global data, and camera data;
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    // Task 6: populate renderData's list of primitives and their transforms.
    //         This will involve traversing the scene graph, and we recommend you
    //         create a helper function to do so!
    SceneNode* root = fileReader.getRootNode();
    renderData.shapes.clear();
    glm::mat4 identity = glm::mat4(1.0f);
    dfsSceneGraph(renderData, root, identity);

    return true;
}
