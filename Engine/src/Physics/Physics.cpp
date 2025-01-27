#include "JoltImpl.h"
#include "types/types.h"

namespace {
    PhysicsSystem& getPhysicsSystem() {
	static PhysicsSystem physics_system;
        return physics_system;
    }

    TempAllocatorImpl& getTempAllocator() {
        static TempAllocatorImpl temp_allocator(30 * 1024 * 1024);
        return temp_allocator;
    }

    JobSystemThreadPool& getJobSystem() {
        static JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
        return job_system;
    }
    const uint cMaxBodies = 1024;

    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;
    BPLayerInterfaceImpl broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;
}
void initPhysics() {
    RegisterDefaultAllocator();
    Factory::sInstance = new Factory();
    RegisterTypes();

    // Now we can create the actual physics system.
    getPhysicsSystem().Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
    getPhysicsSystem().SetGravity(Vec3(0,-9.81f, 0));

    BodyInterface &body_interface = getPhysicsSystem().GetBodyInterface();

    // Next we can create a rigid body to serve as the floor, we make a large box
    // Create the settings for the collision volume (the shape).
    // Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
    BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
    floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

    // Create the shape
    ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
    ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

    // Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
    BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

    // Create the actual rigid body
    Body *floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

    // Add it to the world
    body_interface.AddBody(floor->GetID(), EActivation::DontActivate);

}
glm::mat4 ConvertToGLM(const JPH::Mat44& jphMatrix) {
    glm::mat4 glmMatrix;

    // Copy each element from JPH::Mat44 to glm::mat4
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            glmMatrix[col][row] = jphMatrix(row, col); // Note: glm::mat4 uses column-major order
        }
    }

    return glmMatrix;
}

void createRigidBody(Instance & instance, Scene & scene) {
    BodyInterface &bodyInterface = getPhysicsSystem().GetBodyInterface();
    PhysicsComponent phys = {
        JPH::BodyID(),                             // Default initialize bodyID
	0x0,
        new JPH::BoxShapeSettings(JPH::Vec3(1.0f, 1.0f, 1.0f)) // Initialize shapeSettings
    };
    BodyCreationSettings cubeSettings(phys.shapeSettings, RVec3(0.0_r, 0.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
    phys.body = bodyInterface.CreateBody(cubeSettings);
    phys.bodyID = phys.body->GetID();
    bodyInterface.AddBody(phys.bodyID, EActivation::Activate); 
    scene.registry.emplace<PhysicsComponent>(instance.entity, phys);
};

void AddLinearVelocity(Instance & instance, Vec3 v, Scene & scene) {
    auto & phys = scene.registry.get<PhysicsComponent>(instance.entity);
    BodyInterface &bodyInterface = getPhysicsSystem().GetBodyInterface();
    bodyInterface.SetLinearVelocity(phys.bodyID, v);
}


void updatePhysics(float deltaTime, Scene &scene) {
    auto view = scene.registry.view<TransformComponent, PhysicsComponent>();
    BodyInterface &bodyInterface = getPhysicsSystem().GetBodyInterface();
    //update locations from viewport and logic interaction
    for (auto & entity : view) {
	auto & phys = scene.registry.get<PhysicsComponent>(entity); 
	auto & transform = scene.registry.get<TransformComponent>(entity);
	
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(transform.mat, scale, rotation, translation, skew,perspective);

	// Rebuild the matrix without the scale
	glm::mat4 normalizedMat = glm::mat4_cast(rotation);
	normalizedMat[3] = transform.mat[3]; // Keep the translation


	JPH::Mat44 mat = JPH::Mat44(
	    JPH::Vec4(normalizedMat[0][0], normalizedMat[0][1], normalizedMat[0][2], normalizedMat[0][3]),
	    JPH::Vec4(normalizedMat[1][0], normalizedMat[1][1], normalizedMat[1][2], normalizedMat[1][3]),
	    JPH::Vec4(normalizedMat[2][0], normalizedMat[2][1], normalizedMat[2][2], normalizedMat[2][3]),
	    JPH::Vec4(normalizedMat[3][0], normalizedMat[3][1], normalizedMat[3][2], normalizedMat[3][3])
	);
	bodyInterface.SetPositionAndRotation(phys.bodyID, mat.GetTranslation(), mat.GetRotation().GetQuaternion(), EActivation::Activate);
    }
    getPhysicsSystem().Update(deltaTime, 1, &getTempAllocator(), &getJobSystem());

    for (auto & entity : view) {
	auto & phys = scene.registry.get<PhysicsComponent>(entity); 
	auto & transform = scene.registry.get<TransformComponent>(entity);
	auto mat = phys.body->GetWorldTransform();
	transform.mat = ConvertToGLM(mat);
    }
}
