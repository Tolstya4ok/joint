#ifndef JOINT_UTILS_JOINTEXCEPTION_HPP
#define JOINT_UTILS_JOINTEXCEPTION_HPP


#include <joint/Joint.h>

#include <stdexcept>
#include <type_traits>


namespace joint
{

#define JOINT_PP_CAT1(A_, B_) A_##B_
#define JOINT_PP_CAT(A_, B_) JOINT_PP_CAT1(A_, B_)

#define JOINT_PP_STR1(...) #__VA_ARGS__
#define JOINT_PP_STR(...) JOINT_PP_STR1(__VA_ARGS__)

#define JOINT_SOURCE_LOCATION __FILE__ ":" JOINT_PP_STR(__LINE__)

#define JOINT_THROW(...) do { throw ::joint::MakeException(__VA_ARGS__); } while (false)
#define JOINT_CHECK(Expr_, ...) do { if (!(Expr_)) { GetLogger().Error() << "JOINT_CHECK failed: " #Expr_ " in function " << __func__ << " at " << JOINT_SOURCE_LOCATION; JOINT_THROW(__VA_ARGS__); } } while (false)

	class JointException : public std::exception
	{
	private:
		Joint_Error		_err;

	public:
		JointException(Joint_Error err) : _err(err) { }
		virtual const char* what() const throw() { return Joint_ErrorToString(_err); }
		virtual ~JointException() throw() { }

		Joint_Error GetError() const { return _err; }
	};


	inline JointException MakeException(Joint_Error err) { return JointException(err); }

	template < typename T_, typename Enabler_ = typename std::enable_if<std::is_base_of<std::exception, T_>::value>::type >
	T_ MakeException(T_&& ex) { return ex; }

	template < typename T_, typename Enabler_ = typename std::enable_if<std::is_constructible<std::string, T_>::value>::type >
	std::runtime_error MakeException(T_&& msg) { return std::runtime_error(std::string(msg)); }

}

#endif
