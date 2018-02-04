#include <joint/public/Array.h>

#include <joint/devkit/log/Logger.hpp>
#include <joint/devkit/util/CppWrappers.hpp>

#include <atomic>
#include <string.h>
#include <vector>
#include <iostream>

static const char* LoggerName = "Joint.Core";
JOINT_DEVKIT_LOGGER(LoggerName)

struct Joint_Array
{
    virtual ~Joint_Array() = default;

    virtual JointCore_SizeT GetSize() const = 0;
    virtual void Set(JointCore_SizeT index, JointCore_Value value) = 0;
    virtual JointCore_Value Get(JointCore_SizeT index) const = 0;

    virtual int IncRef() = 0;
    virtual int DecRef() = 0;
};


namespace {

    enum class ArrayTypeId {
        POD,
        UTF8,
        STRUCT,
        OBJ,
        ARRAY
    };

    template<typename RawElementType_, JointCore_TypeId JointCoreTypeId_, ArrayTypeId ArrayTypeId_>
    struct Joint_ArrayImpl : public Joint_Array {
        JointCore_Type _elementType;
        std::vector<JointCore_Value> _elements;
        std::atomic<int> _refCount;

        Joint_ArrayImpl(JointCore_Type elementType, std::size_t size)
                : _elementType(DeepCopyType(elementType)), _elements(size), _refCount(1) {
            for (auto &e : _elements)
                e = MakeDefaultValue(elementType);
        }

        ~Joint_ArrayImpl() override {
            for (auto &e : _elements)
                ReleaseValue(_elementType, e);
            DeepDeleteType(_elementType);
        }


        Joint_ArrayImpl(const Joint_Array &) = delete;

        Joint_ArrayImpl &operator=(const Joint_Array &) = delete;

        JointCore_SizeT GetSize() const override { return (JointCore_SizeT) _elements.size(); }

        void Set(JointCore_SizeT index, JointCore_Value value) override {
            ReleaseValue(_elementType, _elements[index]);
            _elements[index] = DeepCopyValue(_elementType, value);
        }

        JointCore_Value Get(JointCore_SizeT index) const override { return _elements[index]; }

        int IncRef() override { return ++_refCount; }

        int DecRef() override { return --_refCount; }

    private:
        static JointCore_Value DeepCopyValue(JointCore_Type type, JointCore_Value value) {
            JointCore_Value result = value;
            switch (type.id) {
                case JOINT_CORE_TYPE_UTF8: {
                    char *tmp = new char[strlen(value.utf8) + 1];
                    strcpy(tmp, value.utf8);
                    result.utf8 = tmp;
                }
                    break;
                case JOINT_CORE_TYPE_STRUCT: {
                    auto &sd = *type.payload.structDescriptor;
                    result.members = new JointCore_Value[sd.membersCount];
                    for (int32_t i = 0; i < sd.membersCount; ++i)
                        result.members[i] = DeepCopyValue(sd.memberTypes[i], value.members[i]);
                }
                    break;
                case JOINT_CORE_TYPE_ARRAY:
                    Joint_IncRefArray(result.array);
                    break;
                case JOINT_CORE_TYPE_OBJ:
                    JOINT_CORE_DECREF_ACCESSOR(result.obj);
                    break;
                default:
                    break;
            }
            return result;
        }

        static void ReleaseValue(JointCore_Type type, JointCore_Value value) {
            switch (type.id) {
                case JOINT_CORE_TYPE_UTF8:
                    delete[] value.utf8;
                    break;
                case JOINT_CORE_TYPE_STRUCT: {
                    auto &sd = *type.payload.structDescriptor;
                    for (int32_t i = 0; i < sd.membersCount; ++i)
                        ReleaseValue(sd.memberTypes[i], value.members[i]);
                    delete[] value.members;
                }
                    break;
                case JOINT_CORE_TYPE_ARRAY:
                    Joint_DecRefArray(value.array);
                    break;
                case JOINT_CORE_TYPE_OBJ:
                    JOINT_CORE_DECREF_ACCESSOR(value.obj);
                    break;
                default:
                    break;
            }
        }

        static JointCore_Value MakeDefaultValue(JointCore_Type type) {
            JointCore_Value result = JointCore_Value();
            switch (type.id) {
                case JOINT_CORE_TYPE_I32:
                    result.i32 = 0;
                    break;
                case JOINT_CORE_TYPE_F32:
                    result.f32 = 0.f;
                    break;
                case JOINT_CORE_TYPE_F64:
                    result.f64 = 0.0;
                    break;
                case JOINT_CORE_TYPE_UTF8: {
                    char *tmp = new char[1];
                    *tmp = '\0';
                    result.utf8 = tmp;
                }
                    break;
                case JOINT_CORE_TYPE_STRUCT: {
                    auto &sd = *type.payload.structDescriptor;
                    result.members = new JointCore_Value[sd.membersCount];
                    for (int32_t i = 0; i < sd.membersCount; ++i)
                        result.members[i] = MakeDefaultValue(sd.memberTypes[i]);
                }
                    break;
                case JOINT_CORE_TYPE_ARRAY:
                    break;
                default:
                    break;
            }
            return result;
        }

        static JointCore_Type DeepCopyType(JointCore_Type type) {
            JointCore_Type result;
            result.id = type.id;
            switch (type.id) {
                case JOINT_CORE_TYPE_OBJ:
                    result.payload.interfaceChecksum = type.payload.interfaceChecksum;
                    break;
                case JOINT_CORE_TYPE_STRUCT: {
                    result.payload.structDescriptor = new JointCore_StructDescriptor;
                    auto &src_sd = *type.payload.structDescriptor;
                    auto &dst_sd = *result.payload.structDescriptor;
                    dst_sd.membersCount = src_sd.membersCount;
                    dst_sd.memberTypes = new JointCore_Type[dst_sd.membersCount];
                    for (int32_t i = 0; i < dst_sd.membersCount; ++i)
                        dst_sd.memberTypes[i] = DeepCopyType(src_sd.memberTypes[i]);
                }
                    break;
                default:
                    break;
            }
            return result;
        }

        static void DeepDeleteType(JointCore_Type type) {
            if (type.id != JOINT_CORE_TYPE_STRUCT)
                return;

            auto &sd = *type.payload.structDescriptor;
            for (int32_t i = 0; i < sd.membersCount; ++i)
                DeepDeleteType(sd.memberTypes[i]);

            delete[] sd.memberTypes;
            delete type.payload.structDescriptor;
        }
    };

    template<typename RawElementType_, JointCore_TypeId JointCoreTypeId_>
    struct ValueRefGetter;

#define DEFINE_VALUE_REF_GETTER(JOINT_TYPE_, RAW_TYPE_) \
    template<typename RawElementType_> \
    struct ValueRefGetter<RawElementType_, JOINT_TYPE_> \
    { \
        static RawElementType_& Get(JointCore_Value* value) { return value->RAW_TYPE_;} \
    };

    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_I8,   i8);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_I16,  i16);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_I32,  i32);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_I64,  i64);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_U8,   u8);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_U16,  u16);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_U32,  i32);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_U64,  i64);
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_BOOL, b)
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_F32,  f32)
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_F64,  f64)
    DEFINE_VALUE_REF_GETTER(JOINT_CORE_TYPE_ENUM, e)

    template<typename RawElementType_, JointCore_TypeId JointCoreTypeId_>
    struct Joint_ArrayImpl<RawElementType_, JointCoreTypeId_, ArrayTypeId::POD> : public Joint_Array {
        std::vector<RawElementType_> _elements;
        std::atomic<int> _refCount;

        Joint_ArrayImpl(JointCore_Type elementType, std::size_t size)
            : _elements(size), _refCount(1)
        { }

        Joint_ArrayImpl(const Joint_Array &) = delete;

        Joint_ArrayImpl &operator=(const Joint_Array &) = delete;

        JointCore_SizeT GetSize() const override { return (JointCore_SizeT) _elements.size(); }

        void Set(JointCore_SizeT index, JointCore_Value value) override
        {
            _elements[index] = ValueRefGetter<RawElementType_, JointCoreTypeId_>::Get(&value);
        }

        JointCore_Value Get(JointCore_SizeT index) const override
        {
            JointCore_Value value;
            ValueRefGetter<RawElementType_, JointCoreTypeId_>::Get(&value) = _elements[index];
            return value;
        }

        int IncRef() override { return ++_refCount; }
        int DecRef() override { return --_refCount; }
    };
}

extern "C"
{
    JointCore_Error Joint_MakeArray(JointCore_Type elementType, JointCore_SizeT size, JointCore_ArrayHandle* outArray)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN
        if (elementType.id == JOINT_CORE_TYPE_I64)
            *outArray = new Joint_ArrayImpl<int64_t, JOINT_CORE_TYPE_I64, ArrayTypeId::POD>(elementType, size);
        else
            *outArray = new Joint_ArrayImpl<JointCore_Value, JOINT_CORE_TYPE_STRUCT, ArrayTypeId::STRUCT>(elementType, size);
        JOINT_DEVKIT_CPP_WRAP_END
    }


    void Joint_IncRefArray(JointCore_ArrayHandle handle)
    {
        if (handle == JOINT_CORE_NULL_HANDLE)
            return;

        if (handle->IncRef() <= 1)
            JOINT_DEVKIT_TERMINATE("Inconsistent reference counter!");
    }


    void Joint_DecRefArray(JointCore_ArrayHandle handle)
    {
        if (handle == JOINT_CORE_NULL_HANDLE)
            return;

        auto refs = handle->DecRef();
        if (refs < 0)
            JOINT_DEVKIT_TERMINATE("Inconsistent reference counter!");

        if (refs == 0)
        {
            JOINT_DEVKIT_CPP_WRAP_BEGIN
            delete handle;
            JOINT_DEVKIT_CPP_WRAP_END_VOID
        }
    }


    JointCore_Error Joint_ArrayGetSize(JointCore_ArrayHandle handle, JointCore_SizeT* outSize)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN
        JOINT_DEVKIT_CHECK(handle != JOINT_CORE_NULL_HANDLE, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outSize, JOINT_CORE_ERROR_INVALID_PARAMETER);
        *outSize = (JointCore_SizeT)handle->GetSize();
        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_ArraySet(JointCore_ArrayHandle handle, JointCore_SizeT index, JointCore_Value value)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN
        JOINT_DEVKIT_CHECK(handle != JOINT_CORE_NULL_HANDLE, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(index < handle->GetSize(), JOINT_CORE_ERROR_INDEX_OUT_OF_RANGE);
        handle->Set(index, value);
        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_ArrayGet(JointCore_ArrayHandle handle, JointCore_SizeT index, JointCore_Value* outValue)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN
        JOINT_DEVKIT_CHECK(handle != JOINT_CORE_NULL_HANDLE, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(index < handle->GetSize(), JOINT_CORE_ERROR_INDEX_OUT_OF_RANGE);
        *outValue = handle->Get(index);
        JOINT_DEVKIT_CPP_WRAP_END
    }

}
