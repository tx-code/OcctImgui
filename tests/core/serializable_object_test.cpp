#define BOOST_TEST_MODULE Core SerializableObject Tests
#include <boost/test/unit_test.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "core/SerializableObject.h"
#include "core/UUIDSerialization.h"
#include <memory>
#include <string>
#include <vector>


using namespace Core;

#define TEST_UUID "01234567-89ab-cdef-0123-456789abcdef"

class Person: public SerializableObject
{
public:
    Property<std::string> name {"Unknown"};
    Property<int> age {0};
    Property<double> height {0.0};
    Property<boost::uuids::uuid> uuid {};

    Person()
    {
        registerProperty("name", name);
        registerProperty("age", age);
        registerProperty("height", height);
        registerProperty("uuid", uuid);
    }
};

BOOST_AUTO_TEST_CASE(serializable_object_test)
{
    Person person;
    person.name.set("John Doe");
    person.age.set(30);
    person.height.set(185.0);
    person.uuid.set(boost::uuids::string_generator()(TEST_UUID));

    auto json = person.serialize();
    BOOST_CHECK(json.contains("name"));
    BOOST_CHECK(json["name"].get<std::string>() == "John Doe");
    BOOST_CHECK(json.contains("age"));
    BOOST_CHECK(json["age"].get<int>() == 30);
    BOOST_CHECK(json.contains("height"));
    BOOST_CHECK(json["height"].get<double>() == 185.0);
    BOOST_CHECK(json.contains("uuid"));
    BOOST_CHECK(json["uuid"].get<std::string>() == TEST_UUID);


    // 测试反序列化
    Person deserializedPerson;
    BOOST_CHECK(deserializedPerson.deserialize(json));
    BOOST_CHECK(deserializedPerson.name.get() == "John Doe");
    BOOST_CHECK(deserializedPerson.age.get() == 30);
    BOOST_CHECK(deserializedPerson.height.get() == 185.0);
    BOOST_CHECK(boost::uuids::to_string(deserializedPerson.uuid.get()) == TEST_UUID);

    // 测试文件序列化
    const std::string filename = TEST_DATA_DIR "/person.json";

    // 测试文件反序列化
    Person deserializedFromFile;
    BOOST_CHECK(deserializedFromFile.loadFromFile(filename));
    BOOST_CHECK(deserializedFromFile.name.get() == "John Doe");
    BOOST_CHECK(deserializedFromFile.age.get() == 30);
    BOOST_CHECK(deserializedFromFile.height.get() == 185.0);
    BOOST_CHECK(boost::uuids::to_string(deserializedFromFile.uuid.get()) == TEST_UUID);
}
