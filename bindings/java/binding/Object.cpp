#include <binding/Object.hpp>

#include <joint/devkit/StringBuilder.hpp>
#include <joint/utils/JointException.hpp>


namespace joint_java {
namespace binding
{

	using namespace joint::devkit;

	//JOINT_DEVKIT_LOGGER("Joint.Java.Object")

	//static void ThrowIfException(JavaVM* jvm, const std::string& methodName = "")
	//{
		//JNIEnv* env = GetJavaEnv(jvm);
		//if (!env->ExceptionCheck())
			//return;

		//jthrowable raw_throwable = env->ExceptionOccurred();
		//env->ExceptionClear();
		//JGlobalThrowablePtr ex = JGlobalThrowablePtr(jvm, raw_throwable);
		//std::string msg("<cannot obtain java exception message>");
		//JLocalClassPtr throwable_class(env, env->FindClass("java/lang/Throwable"));
		//jmethodID toString_id = 0;
		//JLocalStringPtr jmsg;
		//if (throwable_class.get())
			//toString_id = env->GetMethodID(throwable_class.get(), "toString", "()Ljava/lang/String;");
		//if (toString_id)
			//jmsg = JLocalStringPtr(env, (jstring)env->CallObjectMethod(ex.get(), toString_id));
		//if (jmsg)
		//{
			//jboolean is_copy = false;
			//const char* chars = env->GetStringUTFChars(jmsg.get(), &is_copy);
			//msg = chars;
			//env->ReleaseStringUTFChars(jmsg.get(), chars); // TODO: use Holder or something
		//}

		//SC_THROW(JException(ex, methodName, msg));
	//}




	Object::Object(const JGlobalClassPtr& clazz)
		: _class(clazz)
	{
		auto env = _class.GetEnv();
		jmethodID get_methods_desc = env->GetStaticMethodID(clazz, "getMethodsDesc", "()[[Ljava/lang/String;");
		JOINT_CHECK(get_methods_desc, "Method getMethodsDesc not found");
		env->CallStaticObjectMethodA(clazz, get_methods_desc, nullptr);
		//_id = GetJavaEnv(_jvm)->GetMethodID(clazz.get(), name.c_str(), method_str.c_str());
	}


	Object::~Object()
	{
		GetLogger().Warning() << "Object dtor";
	}

}}
