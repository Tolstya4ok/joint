#ifndef UTILS_UTILS_HPP
#define UTILS_UTILS_HPP


#include <joint/devkit/ExceptionInfo.hpp>
#include <joint/devkit/JointException.hpp>
#include <joint/devkit/ScopeExit.hpp>
#include <joint/devkit/StringBuilder.hpp>

#include <utils/JPtr.hpp>


#define JNI_WRAP_CPP_BEGIN \
	try {

#define JNI_WRAP_CPP_END(OkRetVal_, FailRetVal_) \
		return (OkRetVal_); \
	} catch (const std::exception& ex) { \
		GetLogger().Error() << JOINT_SOURCE_LOCATION ": " << ex; \
		::joint::java::ThrowJavaException(env, ex.what()); \
		return (FailRetVal_); \
	}

#define JNI_WRAP_CPP_END_VOID() \
	} catch (const std::exception& ex) { \
		GetLogger().Error() << JOINT_SOURCE_LOCATION ": " << ex; \
		::joint::java::ThrowJavaException(env, ex.what()); \
	}


#define JAVA_CALL(...) JAVA_CALL_EX(env, __VA_ARGS__)
#define JAVA_CALL_EX(Env_, ...) ::joint::java::JavaCallImpl(Env_, (__VA_ARGS__), JOINT_SOURCE_LOCATION)

#define JAVA_CALL_VOID(...) JAVA_CALL_VOID_EX(env, __VA_ARGS__)
#define JAVA_CALL_VOID_EX(Env_, ...) do { __VA_ARGS__; ::joint::java::JavaCallImpl(Env_, 0, JOINT_SOURCE_LOCATION); } while (false)


namespace joint {
namespace java
{

	class StringDataHolder
	{
	private:
		JNIEnv*            _env;
		jstring            _strObj;
		const char*        _data;

	public:
		StringDataHolder(JStringWeakRef str)
		{ Init(str.GetEnv(), str.Get()); }

		~StringDataHolder()
		{ _env->ReleaseStringUTFChars(_strObj, _data); }

		const char* GetData() const
		{ return _data; }

	private:
		void Init(JNIEnv* env, jstring str)
		{
			_env = JOINT_DEVKIT_REQUIRE_NOT_NULL(env);
			_strObj = JOINT_DEVKIT_REQUIRE_NOT_NULL(str);
			jboolean is_copy = false;
			_data = _env->GetStringUTFChars(_strObj, &is_copy);
		}
	};


	inline std::string ToStdString(JStringTempRef str)
	{ return StringDataHolder(str.Weak()).GetData(); }


	inline void ThrowJavaException(JNIEnv* env, const std::string& msg)
	{
		const char* LoggerName = "Joint.Java.Utils";

		auto jmsg = JStringLocalRef::StealLocal(env, env->NewStringUTF(msg.c_str()));
		auto RuntimeException_cls = JClassLocalRef::StealLocal(env, env->FindClass("java/lang/RuntimeException"));
		if (!RuntimeException_cls)
			JOINT_TERMINATE("Could not find class java.lang.RuntimeException");

		jmethodID RuntimeException_ctor_id = env->GetMethodID(RuntimeException_cls.Get(), "<init>", "(Ljava/lang/String;)V");
		if (!RuntimeException_ctor_id)
			JOINT_TERMINATE("Could not find java.lang.RuntimeException(java.lang.String) constructor");

		auto ex = JThrowableLocalRef::StealLocal(env, reinterpret_cast<jthrowable>(env->NewObject(RuntimeException_cls.Get(), RuntimeException_ctor_id, jmsg.Get())));
		if (!ex)
			JOINT_TERMINATE("Could not create java.lang.RuntimeException object");

		env->Throw(ex.Get());
	}


#define DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION \
		do { \
			if (env->ExceptionCheck()) \
			{ \
				env->ExceptionDescribe(); \
				JOINT_TERMINATE_EX("Joint.Java.Utils", "Got an exception from java while processing another one"); \
			} \
		} while (false)

	inline devkit::ExceptionInfo GetJavaExceptionInfo(JNIEnv *env)
	{
		using namespace devkit;

		if (!env->ExceptionCheck())
			JOINT_TERMINATE_EX("Joint.Java.Utils", "GetJavaExceptionInfo failed: no active java exception");

		jthrowable raw_throwable = env->ExceptionOccurred();
		env->ExceptionClear();
		auto ex = JThrowableLocalRef::StealLocal(env, raw_throwable);

		auto throwable_class = JClassLocalRef::StealLocal(env, env->FindClass("java/lang/Throwable"));
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		jmethodID toString_id = env->GetMethodID(throwable_class.Get(), "toString", "()Ljava/lang/String;");
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		jmethodID getStackTrace_id = env->GetMethodID(throwable_class.Get(), "getStackTrace", "()[Ljava/lang/StackTraceElement;");
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		auto jmsg = JStringLocalRef::StealLocal(env, env->CallObjectMethod(ex.Get(), toString_id));
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;

		auto ste_class = JClassLocalRef::StealLocal(env, env->FindClass("java/lang/StackTraceElement"));
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		jmethodID ste_getClassName_id = env->GetMethodID(ste_class.Get(), "getClassName", "()Ljava/lang/String;");
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		jmethodID ste_getFileName_id = env->GetMethodID(ste_class.Get(), "getFileName", "()Ljava/lang/String;");
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		jmethodID ste_getLineNumber_id = env->GetMethodID(ste_class.Get(), "getLineNumber", "()I");
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		jmethodID ste_getMethodName_id = env->GetMethodID(ste_class.Get(), "getMethodName", "()Ljava/lang/String;");
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		auto jst = JObjArrayLocalRef::StealLocal(env, env->CallObjectMethod(ex.Get(), getStackTrace_id));
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;

		auto st_len = env->GetArrayLength(jst.Get());
		ExceptionInfo::StackTrace st;

		auto JointException_cls = JClassLocalRef::StealLocal(env, env->FindClass("org/joint/JointException"));
		DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
		if (env->IsInstanceOf(ex.Get(), JointException_cls.Get()))
		{
			jfieldID nativeData_id = env->GetFieldID(JointException_cls.Get(), "nativeData", "J");
			DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
			auto ex_info = reinterpret_cast<const ExceptionInfo*>(env->GetLongField(ex.Get(), nativeData_id));
			DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
			if (ex_info)
			{
				st.reserve(st_len + ex_info->GetStackTrace().size());
				std::copy(ex_info->GetStackTrace().begin(), ex_info->GetStackTrace().end(), std::back_inserter(st));
			}
		}

		st.reserve(st_len);

		for (int i = 0; i < st_len; ++i)
		{
			auto ste = JObjLocalRef::StealLocal(env, env->GetObjectArrayElement(jst.Get(), i));
			DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
			auto class_name = JStringLocalRef::StealLocal(env, env->CallObjectMethod(ste.Get(), ste_getClassName_id));
			DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
			auto file_name = JStringLocalRef::StealLocal(env, env->CallObjectMethod(ste.Get(), ste_getFileName_id));
			DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
			jint line_num = env->CallIntMethod(ste.Get(), ste_getLineNumber_id);
			DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
			auto method_name = JStringLocalRef::StealLocal(env, env->CallObjectMethod(ste.Get(), ste_getMethodName_id));
			DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION;
			st.emplace_back(
					"",
					StringDataHolder(file_name).GetData(),
					line_num,
					"",
					StringBuilder() % StringDataHolder(class_name).GetData() % "." % StringDataHolder(method_name).GetData()
				);
		}

		return ExceptionInfo(StringDataHolder(jmsg).GetData(), st);
	}

#undef DETAIL_JOINT_JAVA_TERMINATE_ON_EXCEPTION


	void ThrowExceptionFromJava(JNIEnv* env, const char* location);

	template < typename T_ >
	T_ JavaCallImpl(JNIEnv *env, T_ result, const char* location)
	{
		if (env->ExceptionCheck())
			ThrowExceptionFromJava(env, location);

		return result;
	}

}}

#endif
