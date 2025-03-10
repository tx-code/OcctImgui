#define BOOST_TEST_MODULE Core SerializableObject Tests
#include <boost/test/unit_test.hpp>

#include "core/SerializableObject.h"
#include <memory>
#include <string>
#include <vector>

using namespace Core;

class Person: public SerializableObject
{
public:
    Property<std::string> name {"Unknown"};
    Property<int> age {0};
    Property<double> height {0.0};

    Person()
    {
        registerProperty("name", name);
        registerProperty("age", age);
        registerProperty("height", height);
    }
};

BOOST_AUTO_TEST_CASE(serializable_object_test)
{
    Person person;
    person.name.set("John Doe");
    person.age.set(30);
    person.height.set(185.0);

    auto json = person.serialize();
    BOOST_CHECK(json.contains("name"));
    BOOST_CHECK(json["name"].get<std::string>() == "John Doe");
    BOOST_CHECK(json.contains("age"));
    BOOST_CHECK(json["age"].get<int>() == 30);
    BOOST_CHECK(json.contains("height"));
    BOOST_CHECK(json["height"].get<double>() == 185.0);

    // 测试反序列化
    Person deserializedPerson;
    BOOST_CHECK(deserializedPerson.deserialize(json));
    BOOST_CHECK(deserializedPerson.name.get() == "John Doe");
    BOOST_CHECK(deserializedPerson.age.get() == 30);
    BOOST_CHECK(deserializedPerson.height.get() == 185.0);

    // 测试文件序列化
    const std::string filename = TEST_DATA_DIR "/person.json";

    // 测试文件反序列化
    Person deserializedFromFile;
    BOOST_CHECK(deserializedFromFile.loadFromFile(filename));
    BOOST_CHECK(deserializedFromFile.name.get() == "John Doe");
    BOOST_CHECK(deserializedFromFile.age.get() == 30);
    BOOST_CHECK(deserializedFromFile.height.get() == 185.0);
}