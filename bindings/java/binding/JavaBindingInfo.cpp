#include <binding/JavaBindingInfo.hpp>


namespace joint {
namespace java
{

	Joint_TypeId JavaBindingInfo::GetJointTypeId(const JLocalObjPtr& typeNode) const
	{
		auto env = typeNode.GetEnv();

		JLocalClassPtr TypeDescriptor_cls(env, JAVA_CALL(env->FindClass("org/joint/TypeDescriptor")));
		jfieldID typeId_id = JAVA_CALL(env->GetFieldID(TypeDescriptor_cls, "typeId", "I"));
		return static_cast<Joint_TypeId>(JAVA_CALL(env->GetIntField(typeNode.Get(), typeId_id)));
	}


	Joint_InterfaceChecksum JavaBindingInfo::GetInterfaceChecksum(const JLocalObjPtr& typeNode) const
	{ return Joint_InterfaceChecksum(); }


	JavaBindingInfo::TypeUserData JavaBindingInfo::GetArrayUserData(const JLocalObjPtr& typeNode) const
	{ return TypeUserData(); }


	JavaBindingInfo::TypeUserData JavaBindingInfo::GetObjectUserData(const JLocalObjPtr& typeNode) const
	{
		auto env = typeNode.GetEnv();

		JLocalClassPtr TypeDescriptor_cls(env, JAVA_CALL(env->FindClass("org/joint/TypeDescriptor")));
		jfieldID proxyClass_id = JAVA_CALL(env->GetFieldID(TypeDescriptor_cls, "proxyClass", "Ljava/lang/Class;"));
		JGlobalClassPtr proxy_class(env, static_cast<jclass>(JAVA_CALL(env->GetObjectField(typeNode.Get(), proxyClass_id))));
		JOINT_CHECK(proxy_class, "Invalid TypeDescriptor for interface");
		jmethodID proxy_ctor_id = JAVA_CALL(env->GetMethodID(proxy_class.Get(), "<init>", "(Lorg/joint/JointObject;)V"));
		return TypeUserData{proxy_class, proxy_ctor_id};
	}


	JavaBindingInfo::TypeUserData JavaBindingInfo::GetEnumUserData(const JLocalObjPtr& typeNode) const
	{ return TypeUserData(); }


	JavaBindingInfo::TypeUserData JavaBindingInfo::GetStructUserData(const JLocalObjPtr& typeNode) const
	{ return TypeUserData(); }


	JavaBindingInfo::MethodUserData JavaBindingInfo::GetMethodUserData(const JLocalObjPtr& methodNode) const
	{
		auto env = methodNode.GetEnv();

		JLocalClassPtr MethodDescriptor_cls(env, JAVA_CALL(env->FindClass("org/joint/MethodDescriptor")));

		jfieldID name_id = JAVA_CALL(env->GetFieldID(MethodDescriptor_cls, "name", "Ljava/lang/String;"));
		JLocalStringPtr name(env, reinterpret_cast<jstring>(JAVA_CALL(env->GetObjectField(methodNode.Get(), name_id))));

		jfieldID signature_id = JAVA_CALL(env->GetFieldID(MethodDescriptor_cls, "signature", "Ljava/lang/String;"));
		JLocalStringPtr signature(env, reinterpret_cast<jstring>(JAVA_CALL(env->GetObjectField(methodNode.Get(), signature_id))));

		jfieldID interfaceCls_id = JAVA_CALL(env->GetFieldID(MethodDescriptor_cls, "interfaceCls", "Ljava/lang/Class;"));
		JGlobalClassPtr interfaceCls(env, reinterpret_cast<jclass>(JAVA_CALL(env->GetObjectField(methodNode.Get(), interfaceCls_id))));

		jmethodID id = JAVA_CALL(env->GetMethodID(interfaceCls.Get(), StringDataHolder(env, name.Get()).GetData(), StringDataHolder(env, signature.Get()).GetData()));

		return MethodUserData{interfaceCls, id};
	}


	JLocalObjPtr JavaBindingInfo::GetRetTypeNode(const JLocalObjPtr& methodNode) const
	{
		auto env = methodNode.GetEnv();

		JLocalClassPtr MethodDescriptor_cls(env, JAVA_CALL(env->FindClass("org/joint/MethodDescriptor")));
		jfieldID retType_id = JAVA_CALL(env->GetFieldID(MethodDescriptor_cls, "retType", "Lorg/joint/TypeDescriptor;"));
		return JLocalObjPtr(env, JAVA_CALL(env->GetObjectField(methodNode.Get(), retType_id)));
	}


	JLocalObjPtr JavaBindingInfo::GetArrayElementTypeNode(const JLocalObjPtr& typeNode) const
	{ JOINT_THROW(JOINT_ERROR_NOT_IMPLEMENTED); }


	JavaBindingInfo::Sequence JavaBindingInfo::GetParamsNodes(const JLocalObjPtr& methodNode) const
	{
		auto env = methodNode.GetEnv();

		JLocalClassPtr MethodDescriptor_cls(env, JAVA_CALL(env->FindClass("org/joint/MethodDescriptor")));
		jfieldID paramTypes_id = JAVA_CALL(env->GetFieldID(MethodDescriptor_cls, "paramTypes", "[Lorg/joint/TypeDescriptor;"));
		JLocalObjArrayPtr param_types(env, reinterpret_cast<jobjectArray>(JAVA_CALL(env->GetObjectField(methodNode.Get(), paramTypes_id))));
		return Sequence(param_types);
	}


	JavaBindingInfo::Sequence JavaBindingInfo::GetMethodsNodes(const JLocalObjPtr& ifcNode) const
	{
		auto env = ifcNode.GetEnv();

		JLocalClassPtr InterfaceDescriptor_cls(env, JAVA_CALL(env->FindClass("org/joint/InterfaceDescriptor")));
		jfieldID methods_id = JAVA_CALL(env->GetFieldID(InterfaceDescriptor_cls, "methods", "[Lorg/joint/MethodDescriptor;"));
		JLocalObjArrayPtr methods(env, reinterpret_cast<jobjectArray>(JAVA_CALL(env->GetObjectField(ifcNode.Get(), methods_id))));
		return Sequence(methods);
	}


	JavaBindingInfo::Sequence JavaBindingInfo::GetMembersNodes(const JLocalObjPtr& typeNode) const
	{ JOINT_THROW(JOINT_ERROR_NOT_IMPLEMENTED); }


	JavaBindingInfo::MemberId JavaBindingInfo::GetMemberId(const JLocalObjPtr& memberNode) const
	{ JOINT_THROW(JOINT_ERROR_NOT_IMPLEMENTED); }


	JLocalObjPtr JavaBindingInfo::GetMemberType(const JLocalObjPtr& memberNode) const
	{ JOINT_THROW(JOINT_ERROR_NOT_IMPLEMENTED); }

}}