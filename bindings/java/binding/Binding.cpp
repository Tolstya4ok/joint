#include <binding/Binding.hpp>

#include <joint/Joint.h>
#include <joint/devkit/accessors/MakeAccessor.hpp>
#include <joint/devkit/util/CppWrappers.hpp>
#include <joint/devkit/util/Holder.hpp>

#include <memory>
#include <string.h>

#include <stdio.h>

#include <binding/JointJavaContext.hpp>
#include <binding/Module.hpp>
#include <binding/Object.hpp>
#include <jni.h>
#include <utils/Utils.hpp>


namespace joint {
namespace java {
namespace binding
{

    using namespace devkit;


    Binding::Binding()
    { GetLogger().Debug() << "Created"; }


    Binding::~Binding()
    { GetLogger().Debug() << "Destroying"; }


    JointCore_Error Binding::LoadModule(JointCore_ManifestHandle moduleManifest, JointCore_ModuleAccessor* outModule) JOINT_DEVKIT_NOEXCEPT
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN
        *outModule = accessors::MakeAccessor<Module>(moduleManifest);
        JOINT_DEVKIT_CPP_WRAP_END
    }


}}}
