#include <joint/devkit/accessors/MakeAccessor.hpp>
#include <joint/devkit/marshalling/ValueMarshaller.hpp>
#include <joint/devkit/util/CppWrappers.hpp>
#include <joint/devkit/util/JointException.hpp>
#include <joint/devkit/util/StackStorage.hpp>
#include <joint/devkit/util/StringBuilder.hpp>

#include <binding/JavaBindingInfo.hpp>
#include <binding/JointJavaContext.hpp>
#include <binding/Marshaller.hpp>
#include <binding/Object.hpp>
#include <utils/Utils.hpp>


namespace joint {
namespace java {
namespace binding
{

    using namespace devkit;

    Object::Object(JNIEnv* env, JObjGlobalRef accessor)
        : _accessor(std::move(accessor))
    {
        JointJavaContext::Accessor a(_accessor.Weak(env));
        _obj = a.GetObj().Global();
        _interfaceDesc = a.GetInterfaceDescriptor().Global();
        _nativeInterfaceDesc = JointJavaContext::InterfaceDescriptor(_interfaceDesc.Weak(env)).GetNative();
    }


    Object::~Object()
    { }


    JointCore_Error Object::CastObject(JointCore_InterfaceId interfaceId, JointCore_InterfaceChecksum checksum, JointCore_ObjectAccessor* outAccessor) JOINT_DEVKIT_NOEXCEPT
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        auto jvm = JointJavaCoreContext::GetJvm();
        auto env = GetJavaEnv(jvm);

        auto iid = JointJavaContext::InterfaceId::Make(env, JStringLocalRef::StealLocal(env, JAVA_CALL(env->NewStringUTF(interfaceId))));

        auto jaccessor = GetAccessor(env);
        JObjLocalRef new_jaccessor = JointJavaContext::Accessor(jaccessor).Cast(iid);
        if (!new_jaccessor)
            return JOINT_CORE_ERROR_CAST_FAILED;

        *outAccessor = accessors::MakeAccessor<Object>(env, new_jaccessor.Global());

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Object::InvokeMethod(JointCore_SizeT methodId, const JointCore_Parameter* paramsPtr, JointCore_SizeT paramsCount, JointCore_RetValue* outRetValue) JOINT_DEVKIT_NOEXCEPT
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        const auto& m_desc = _nativeInterfaceDesc->GetMethod(methodId);

        StackStorage<jvalue, 1024> params_storage;

        auto jvm = JointJavaCoreContext::GetJvm();
        auto env = GetJavaEnv(jvm);

        jvalue* jparams = nullptr;
        if (paramsCount)
        {
            JavaMarshaller m(env);
            jparams = params_storage.Make(paramsCount);

            for (size_t i = 0; i < paramsCount; ++i)
                jparams[i] = ValueMarshaller::FromJoint<jvalue>(ValueDirection::Parameter, m_desc.GetParamType(i), paramsPtr[i].value, m);
        }

        JObjLocalRef retValueLocalGuard;

        jvalue j_res;
        jmethodID j_method = m_desc.GetUserData().Id;
        switch (m_desc.GetRetType().GetJointType().id)
        {
        case JOINT_CORE_TYPE_VOID:
            env->CallVoidMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_BOOL:
            j_res.z = env->CallBooleanMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_U8:
        case JOINT_CORE_TYPE_I8:
            j_res.b = env->CallByteMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_U16:
        case JOINT_CORE_TYPE_I16:
            j_res.s = env->CallShortMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_U32:
        case JOINT_CORE_TYPE_I32:
            j_res.i = env->CallIntMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_U64:
        case JOINT_CORE_TYPE_I64:
            j_res.j = env->CallLongMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_F32:
            j_res.f = env->CallFloatMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_F64:
            j_res.d = env->CallDoubleMethodA(_obj.Get(), j_method, jparams);
            break;
        case JOINT_CORE_TYPE_UTF8:
        case JOINT_CORE_TYPE_ENUM:
        case JOINT_CORE_TYPE_OBJ:
        case JOINT_CORE_TYPE_STRUCT:
        case JOINT_CORE_TYPE_ARRAY:
            j_res.l = env->CallObjectMethodA(_obj.Get(), j_method, jparams);
            retValueLocalGuard = JObjLocalRef::StealLocal(env, j_res.l);
            break;
        default:
            JOINT_DEVKIT_THROW(JOINT_CORE_ERROR_NOT_IMPLEMENTED);
        }

        outRetValue->releaseValue = &Object::ReleaseRetValue;
        if (!env->ExceptionCheck())
        {
            if (m_desc.GetRetType().GetJointType().id != JOINT_CORE_TYPE_VOID)
            {
                RetValueAllocator alloc;
                outRetValue->result.value = ValueMarshaller::ToJoint(ValueDirection::Return, m_desc.GetRetType(), j_res, JavaMarshaller(env), alloc);
            }

            return JOINT_CORE_ERROR_NONE;
        }
        else
        {
            const auto& c = JointJavaContext::Instance();

            jthrowable raw_throwable = env->ExceptionOccurred();
            env->ExceptionClear();
            auto ex = JThrowableLocalRef::StealLocal(env, raw_throwable);

            jlong jointExLong = JAVA_CALL(env->CallStaticLongMethod(c.ExceptionUtils_cls.Get(), c.ExceptionUtils_toJointExceptionHandle, ex.Get()));
            outRetValue->result.ex = reinterpret_cast<JointCore_Exception_Handle>(jointExLong);

            return JOINT_CORE_ERROR_EXCEPTION;
        }

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Object::ReleaseRetValue(JointCore_Type type, JointCore_Value value)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN
        switch(type.id)
        {
        case JOINT_CORE_TYPE_UTF8:
            delete[] value.utf8;
            break;
        case JOINT_CORE_TYPE_STRUCT:
            for (int32_t i = 0; i < type.payload.structDescriptor->membersCount; ++i)
                ReleaseRetValue(type.payload.structDescriptor->memberTypes[i], value.members[i]);
            delete[] value.members;
            break;
        default:
            break;
        }
        JOINT_DEVKIT_CPP_WRAP_END
    }

}}}
