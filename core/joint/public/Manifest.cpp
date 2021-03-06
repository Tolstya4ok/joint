#include <joint/public/Manifest.h>

#include <joint/devkit/log/Logger.hpp>
#include <joint/devkit/util/CppWrappers.hpp>
#include <joint/private/JsonNode.hpp>
#include <joint/private/JsonParser.hpp>

#include <atomic>


using namespace joint;


static const char* LoggerName = "Joint.Core";
JOINT_DEVKIT_LOGGER(LoggerName)

extern "C"
{

    struct Joint_ManifestNode : public joint::JsonNode
    {
        Joint_ManifestNode(Joint_ManifestNode&& other)
            : joint::JsonNode(std::move(other))
        { }

        Joint_ManifestNode(joint::JsonNode&& other)
            : joint::JsonNode(std::move(other))
        { }
    };


    struct Joint_Manifest
    {
        std::atomic<int>           refCount{1};
        const Joint_ManifestNode   rootNode;
        const std::string          location;

        Joint_Manifest(Joint_ManifestNode&& rootNode, std::string location)
            : rootNode(std::move(rootNode)), location(std::move(location))
        { }
    };
}


inline std::ostream& operator << (std::ostream& s, const JointCore_ManifestHandle m)
{
    if (!m)
        s << "null";
    else
        s << "{ location: " << m->location << ", root: " << m->rootNode << " }";
    return s;
}


extern "C"
{

    JointCore_Error Joint_ReadManifestFromFile(const char* path, JointCore_ManifestHandle* outManifest)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(outManifest, JOINT_CORE_ERROR_INVALID_PARAMETER);

        GetLogger().Info() << "ReadManifestFromFile(path: " << (path ? path : "null") << ")";
        JsonNode n = JsonParser::Parse(path);

        std::string path_str(path);
        size_t last_slash = path_str.find_last_of("/\\");

        std::string location = (last_slash == std::string::npos) ? "." : path_str.substr(0, last_slash);

        *outManifest = new Joint_Manifest(std::move(n), std::move(location));
        GetLogger().Debug() << "Manifest: " << *outManifest;

        JOINT_DEVKIT_CPP_WRAP_END
    }


    void Joint_IncRefManifest(JointCore_ManifestHandle handle)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        if (!handle)
            return;

        ++handle->refCount;

        JOINT_DEVKIT_CPP_WRAP_END_VOID
    }


    void Joint_DecRefManifest(JointCore_ManifestHandle handle)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        if (!handle)
            return;

        if (--handle->refCount == 0)
            delete handle;

        JOINT_DEVKIT_CPP_WRAP_END_VOID
    }


    JointCore_Error Joint_GetManifestLocation(JointCore_ManifestHandle manifest, const char** outLocation)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(manifest, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outLocation, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outLocation = manifest->location.c_str();

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestRootNode(JointCore_ManifestHandle manifest, JointCore_ManifestNodeHandle* outNode)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(manifest, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outNode, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outNode = &manifest->rootNode;

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeType(JointCore_ManifestNodeHandle node, JointCore_ManifestNodeType* outType)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outType, JOINT_CORE_ERROR_INVALID_PARAMETER);

        switch (node->GetType())
        {
        case JsonNode::Type::Null:    *outType = JOINT_CORE_MANIFEST_NODE_NULL; break;
        case JsonNode::Type::Boolean: *outType = JOINT_CORE_MANIFEST_NODE_BOOLEAN; break;
        case JsonNode::Type::Integer: *outType = JOINT_CORE_MANIFEST_NODE_INTEGER; break;
        case JsonNode::Type::Float:   *outType = JOINT_CORE_MANIFEST_NODE_FLOAT; break;
        case JsonNode::Type::String:  *outType = JOINT_CORE_MANIFEST_NODE_STRING; break;
        case JsonNode::Type::Array:   *outType = JOINT_CORE_MANIFEST_NODE_ARRAY; break;
        case JsonNode::Type::Object:  *outType = JOINT_CORE_MANIFEST_NODE_OBJECT; break;
        default: JOINT_DEVKIT_THROW(JOINT_CORE_ERROR_INVALID_PARAMETER);
        }

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeBooleanValue(JointCore_ManifestNodeHandle node, JointCore_Bool* outVal)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outVal, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outVal = (JointCore_Bool)node->AsBoolean();

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeIntegerValue(JointCore_ManifestNodeHandle node, int64_t* outVal)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outVal, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outVal = node->AsInteger();

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeFloatValue(JointCore_ManifestNodeHandle node, double* outVal)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outVal, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outVal = node->AsFloat();

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeStringValue(JointCore_ManifestNodeHandle node, const char** outVal)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outVal, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outVal = node->AsString().c_str();

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeChildrenCount(JointCore_ManifestNodeHandle node, JointCore_SizeT* outCount)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outCount, JOINT_CORE_ERROR_INVALID_PARAMETER);

        switch (node->GetType())
        {
        case JsonNode::Type::Array:  *outCount = (JointCore_SizeT)node->AsArray().size(); break;
        case JsonNode::Type::Object: *outCount = (JointCore_SizeT)node->AsObject().size(); break;
        default: JOINT_DEVKIT_THROW(JOINT_CORE_ERROR_INVALID_PARAMETER);
        }

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeArrayElement(JointCore_ManifestNodeHandle node, JointCore_SizeT index, JointCore_ManifestNodeHandle* outNode)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outNode, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outNode = static_cast<JointCore_ManifestNodeHandle>(&node->AsArray().at(index));

        JOINT_DEVKIT_CPP_WRAP_END
    }


    JointCore_Error Joint_GetManifestNodeObjectElementByKey(JointCore_ManifestNodeHandle node, const char* key, JointCore_ManifestNodeHandle* outValue)
    {
        JOINT_DEVKIT_CPP_WRAP_BEGIN

        JOINT_DEVKIT_CHECK(node, JOINT_CORE_ERROR_INVALID_PARAMETER);
        JOINT_DEVKIT_CHECK(outValue, JOINT_CORE_ERROR_INVALID_PARAMETER);

        *outValue = static_cast<JointCore_ManifestNodeHandle>(const_cast<JsonNode*>(&node->AsObject().at(key)));

        JOINT_DEVKIT_CPP_WRAP_END
    }

}
