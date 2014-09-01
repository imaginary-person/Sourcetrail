#include "cxxtest/TestSuite.h"

#include "data/parser/cxx/CxxParser.h"
#include "data/parser/ParseFunction.h"
#include "data/parser/ParseLocation.h"
#include "data/parser/ParserClient.h"
#include "data/parser/ParseTypeUsage.h"
#include "data/parser/ParseVariable.h"
#include "utility/text/TextAccess.h"

class CxxParserTestSuite: public CxxTest::TestSuite
{
public:
	void test_cxx_parser_finds_global_class_definition()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A\n"
			"{\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->classes.size(), 1);
		TS_ASSERT_EQUALS(client->classes[0], "A <1:1 <1:7 1:7> 3:1>");
	}

	void test_cxx_parser_finds_global_class_forward_declaration()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A;\n"
		);

		TS_ASSERT_EQUALS(client->classes.size(), 1);
		TS_ASSERT_EQUALS(client->classes[0], "A <1:7 1:7>");
	}

	void test_cxx_parser_finds_nested_class_definition()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A\n"
			"{\n"
			"public:\n"
			"	class B;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->classes.size(), 2);
		TS_ASSERT_EQUALS(client->classes[0], "A <1:1 <1:7 1:7> 5:1>");
		TS_ASSERT_EQUALS(client->classes[1], "public A::B <4:8 4:8>");
	}

	void test_cxx_parser_finds_class_definition_in_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace a\n"
			"{\n"
			"	class B;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->classes.size(), 1);
		TS_ASSERT_EQUALS(client->classes[0], "a::B <3:8 3:8>");
	}

	void test_cxx_parser_finds_global_struct_definition()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"struct A\n"
			"{\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->structs.size(), 1);
		TS_ASSERT_EQUALS(client->structs[0], "A <1:1 <1:8 1:8> 3:1>");
	}

	void test_cxx_parser_finds_global_struct_forward_declaration()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"struct A;\n"
		);

		TS_ASSERT_EQUALS(client->structs.size(), 1);
		TS_ASSERT_EQUALS(client->structs[0], "A <1:8 1:8>");
	}

	void test_cxx_parser_finds_struct_definition_in_class()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A\n"
			"{\n"
			"	struct B\n"
			"	{\n"
			"	};\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->structs.size(), 1);
		TS_ASSERT_EQUALS(client->structs[0], "private A::B <3:2 <3:9 3:9> 5:2>");
	}

	void test_cxx_parser_finds_struct_definition_in_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace A\n"
			"{\n"
			"	struct B\n"
			"	{\n"
			"	};\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->structs.size(), 1);
		TS_ASSERT_EQUALS(client->structs[0], "A::B <3:2 <3:9 3:9> 5:2>");
	}

	void test_cxx_parser_finds_variable_definitions_in_global_scope()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int x;\n"
			"const int y = 0;\n"
			"static int z;\n"
			"class A;\n"
			"A* b;"
		);

		TS_ASSERT_EQUALS(client->globalVariables.size(), 4);
		TS_ASSERT_EQUALS(client->globalVariables[0], "int x <1:5 1:5>");
		TS_ASSERT_EQUALS(client->globalVariables[1], "int const y <2:11 2:11>");
		TS_ASSERT_EQUALS(client->globalVariables[2], "static int z <3:12 3:12>");
		TS_ASSERT_EQUALS(client->globalVariables[3], "A * b <5:4 5:4>");
	}

	void test_cxx_parser_finds_variable_definitions_in_namespace_scope()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace n"
			"{\n"
			"	int x;\n"
			"	class A;\n"
			"	A* b;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->globalVariables.size(), 2);
		TS_ASSERT_EQUALS(client->globalVariables[0], "int n::x <2:6 2:6>");
		TS_ASSERT_EQUALS(client->globalVariables[1], "n::A * n::b <4:5 4:5>");
	}

	void test_cxx_parser_finds_field_in_nested_class()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class B\n"
			"{\n"
			"public:\n"
			"	class C\n"
			"	{\n"
			"	private:\n"
			"		static const int amount;\n"
			"	};\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->fields.size(), 1);
		TS_ASSERT_EQUALS(client->fields[0], "private static int const B::C::amount <7:20 7:25>");
	}

	void test_cxx_parser_finds_fields_in_class_with_access_type()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A\n"
			"{\n"
			"	int a;"
			"public:\n"
			"	A() : d(0) {};\n"
			"	int b;"
			"protected:\n"
			"	static int c;\n"
			"private:\n"
			"	const int d;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->fields.size(), 4);
		TS_ASSERT_EQUALS(client->fields[0], "private int A::a <3:6 3:6>");
		TS_ASSERT_EQUALS(client->fields[1], "public int A::b <5:6 5:6>");
		TS_ASSERT_EQUALS(client->fields[2], "protected static int A::c <6:13 6:13>");
		TS_ASSERT_EQUALS(client->fields[3], "private int const A::d <8:12 8:12>");
	}

	void test_cxx_parser_finds_function_in_global_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int ceil(float a)\n"
			"{\n"
			"	return static_cast<int>(a) + 1;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->functions.size(), 1);
		TS_ASSERT_EQUALS(client->functions[0], "int ceil(float) <1:1 <1:5 1:8> 4:1>");
	}

	void test_cxx_parser_finds_function_in_anonymous_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace\n"
			"{\n"
			"	int sum(int a, int b);\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->functions.size(), 1);
		TS_ASSERT_EQUALS(client->functions[0], "int (anonymous namespace)::sum(int, int) <3:6 3:8>");
	}

	void test_cxx_parser_finds_static_function_in_global_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"static int ceil(float a)\n"
			"{\n"
			"	return static_cast<int>(a) + 1;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->functions.size(), 1);
		TS_ASSERT_EQUALS(client->functions[0], "static int ceil(float) <1:1 <1:12 1:15> 4:1>");
	}

	void test_cxx_parser_finds_method_declaration()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class B\n"
			"{\n"
			"public:\n"
			"	B();\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->methods.size(), 1);
		TS_ASSERT_EQUALS(client->methods[0], "public void B::B() <4:2 4:2>");
	}

	void test_cxx_parser_finds_method_declaration_and_definition()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class B\n"
			"{\n"
			"public:\n"
			"	B();\n"
			"};\n"
			"B::B()\n"
			"{\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->methods.size(), 2);
		TS_ASSERT_EQUALS(client->methods[0], "public void B::B() <4:2 4:2>");
		TS_ASSERT_EQUALS(client->methods[1], "public void B::B() <6:1 <6:4 6:4> 8:1>");
	}

	void test_cxx_parser_finds_virtual_method()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class B\n"
			"{\n"
			"public:\n"
			"	virtual void process();\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->methods.size(), 1);
		TS_ASSERT_EQUALS(client->methods[0], "public virtual void B::process() <4:15 4:21>");
	}

	void test_cxx_parser_finds_pure_virtual_method()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class B\n"
			"{\n"
			"protected:\n"
			"	virtual void process() = 0;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->methods.size(), 1);
		TS_ASSERT_EQUALS(client->methods[0], "protected pure virtual void B::process() <4:15 4:21>");
	}

	void test_cxx_parser_finds_method_declared_in_nested_class()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class B\n"
			"{\n"
			"	class C\n"
			"	{\n"
			"		bool isGreat() const;\n"
			"	};\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->methods.size(), 1);
		TS_ASSERT_EQUALS(client->methods[0], "private _Bool B::C::isGreat() const <5:8 5:14>");
	}

	void test_cxx_parser_finds_named_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace A\n"
			"{\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->namespaces.size(), 1);
		TS_ASSERT_EQUALS(client->namespaces[0], "A <1:1 <1:11 1:11> 3:1>");
	}

	void test_cxx_parser_finds_anonymous_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace\n"
			"{\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->namespaces.size(), 1);
		TS_ASSERT_EQUALS(client->namespaces[0], "(anonymous) <1:1 3:1>");
	}

	void test_cxx_parser_finds_enum_defined_in_global_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"enum E\n"
			"{\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->enums.size(), 1);
		TS_ASSERT_EQUALS(client->enums[0], "E <1:1 <1:6 1:6> 3:1>");
	}

	void test_cxx_parser_finds_enum_defined_in_class()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class B\n"
			"{\n"
			"public:\n"
			"	enum Z\n"
			"	{\n"
			"	};\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->enums.size(), 1);
		TS_ASSERT_EQUALS(client->enums[0], "public B::Z <4:2 <4:7 4:7> 6:2>");
	}

	void test_cxx_parser_finds_enum_defined_in_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace n\n"
			"{\n"
			"	enum Z\n"
			"	{\n"
			"	};\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->enums.size(), 1);
		TS_ASSERT_EQUALS(client->enums[0], "n::Z <3:2 <3:7 3:7> 5:2>");
	}

	void test_cxx_parser_finds_enum_field_in_global_enum()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"enum E\n"
			"{\n"
			"	P\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->enumFields.size(), 1);
		TS_ASSERT_EQUALS(client->enumFields[0], "E::P <3:2 3:2>");
	}

	void test_cxx_parser_finds_typedef_in_global_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"typedef unsigned int uint;\n"
		);

		TS_ASSERT_EQUALS(client->typedefs.size(), 1);
		TS_ASSERT_EQUALS(client->typedefs[0], "unsigned int -> uint <1:22 1:25>");
	}

	void test_cxx_parser_finds_typedef_in_named_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace test\n"
			"{\n"
			"	typedef unsigned int uint;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->typedefs.size(), 1);
		TS_ASSERT_EQUALS(client->typedefs[0], "unsigned int -> test::uint <3:23 3:26>");
	}

	void test_cxx_parser_finds_typedef_in_anonymous_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace\n"
			"{\n"
			"	typedef unsigned int uint;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->typedefs.size(), 1);
		TS_ASSERT_EQUALS(client->typedefs[0], "unsigned int -> (anonymous namespace)::uint <3:23 3:26>");
	}

	void test_cxx_parser_finds_typedef_that_uses_type_defined_in_named_namespace()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"namespace test\n"
			"{\n"
			"	struct TestStruct{};\n"
			"}\n"
			"typedef test::TestStruct globalTestStruct;\n"
		);

		TS_ASSERT_EQUALS(client->typedefs.size(), 1);
		TS_ASSERT_EQUALS(client->typedefs[0], "test::TestStruct -> globalTestStruct <5:26 5:41>");
	}

	void test_cxx_parser_finds_global_variable_with_typedef_type()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"typedef unsigned int uint;\n"
			"uint number;\n"
		);

		TS_ASSERT_EQUALS(client->typedefs.size(), 1);
		TS_ASSERT_EQUALS(client->typedefs[0], "unsigned int -> uint <1:22 1:25>");

		TS_ASSERT_EQUALS(client->globalVariables.size(), 1);
		TS_ASSERT_EQUALS(client->globalVariables[0], "uint number <2:6 2:11>");
	}

	void test_cxx_parser_finds_global_variable_with_qualified_typedef_type()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"typedef unsigned int* uint;\n"
			"const uint* number;\n"
		);

		TS_ASSERT_EQUALS(client->typedefs.size(), 1);
		TS_ASSERT_EQUALS(client->typedefs[0], "unsigned int * -> uint <1:23 1:26>");

		TS_ASSERT_EQUALS(client->globalVariables.size(), 1);
		TS_ASSERT_EQUALS(client->globalVariables[0], "uint const * number <2:13 2:18>");
	}

	void test_cxx_parser_finds_public_inheritance()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A {};\n"
			"class B : public A {};\n"
		);

		TS_ASSERT_EQUALS(client->inheritances.size(), 1);
		TS_ASSERT_EQUALS(client->inheritances[0], "B : public A <2:11 2:18>");
	}

	void test_cxx_parser_finds_protected_inheritance()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A {};\n"
			"class B : protected A {};\n"
		);

		TS_ASSERT_EQUALS(client->inheritances.size(), 1);
		TS_ASSERT_EQUALS(client->inheritances[0], "B : protected A <2:11 2:21>");
	}

	void test_cxx_parser_finds_private_inheritance()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A {};\n"
			"class B : private A {};\n"
		);

		TS_ASSERT_EQUALS(client->inheritances.size(), 1);
		TS_ASSERT_EQUALS(client->inheritances[0], "B : private A <2:11 2:19>");
	}

	void test_cxx_parser_finds_multiple_inheritance()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A {};\n"
			"class B {};\n"
			"class C\n"
			"	: public A\n"
			"	, private B\n"
			"{};\n"
		);

		TS_ASSERT_EQUALS(client->inheritances.size(), 2);
		TS_ASSERT_EQUALS(client->inheritances[0], "C : public A <4:4 4:11>");
		TS_ASSERT_EQUALS(client->inheritances[1], "C : private B <5:4 5:12>");
	}

	void test_cxx_parser_finds_call_in_function()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int sum(int a, int b)\n"
			"{\n"
			"	return a + b;\n"
			"}\n"
			"int main()\n"
			"{\n"
			"	sum(1, 2);\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "int main() -> int sum(int, int) <7:2 7:10>");
	}

	void test_cxx_parser_finds_call_in_function_with_right_signature()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int sum(int a, int b)\n"
			"{\n"
			"	return a + b;\n"
			"}\n"
			"void func()\n"
			"{\n"
			"}\n"
			"void func(bool right)\n"
			"{\n"
			"	sum(1, 2);\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "void func(_Bool) -> int sum(int, int) <10:2 10:10>");
	}

	void test_cxx_parser_finds_call_to_function_with_right_signature()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int sum(int a, int b)\n"
			"{\n"
			"	return a + b;\n"
			"}\n"
			"float sum(float a, float b)\n"
			"{\n"
			"	return a + b;\n"
			"}\n"
			"int main()\n"
			"{\n"
			"	sum(1, 2);\n"
			"	sum(1.0f, 0.5f);\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 2);
		TS_ASSERT_EQUALS(client->calls[0], "int main() -> int sum(int, int) <11:2 11:10>");
		TS_ASSERT_EQUALS(client->calls[1], "int main() -> float sum(float, float) <12:2 12:16>");
	}

	void test_cxx_parser_finds_call_within_call_in_function()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int sum(int a, int b)\n"
			"{\n"
			"	return a + b;\n"
			"}\n"
			"int main()\n"
			"{\n"
			"	return sum(1, sum(2, 3));\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 2);
		TS_ASSERT_EQUALS(client->calls[0], "int main() -> int sum(int, int) <7:9 7:25>");
		TS_ASSERT_EQUALS(client->calls[1], "int main() -> int sum(int, int) <7:16 7:24>");
	}

	void test_cxx_parser_finds_call_in_method()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int sum(int a, int b)\n"
			"{\n"
			"	return a + b;\n"
			"}\n"
			"class App\n"
			"{\n"
			"	int main()\n"
			"	{\n"
			"		return sum(1, 2);\n"
			"	}\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "int App::main() -> int sum(int, int) <9:10 9:18>");
	}

	void test_cxx_parser_finds_constructor_call()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class App\n"
			"{\n"
			"public:\n"
			"	App() {}\n"
			"};\n"
			"int main()\n"
			"{\n"
			"	App app;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "int main() -> void App::App() <8:6 8:6>");
	}

	void test_cxx_parser_finds_constructor_without_definition_call()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class App\n"
			"{\n"
			"};\n"
			"int main()\n"
			"{\n"
			"	App app;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "int main() -> void App::App() <6:6 6:6>");
	}

	void test_cxx_parser_finds_constructor_call_of_field()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class Item\n"
			"{\n"
			"};\n"
			"class App\n"
			"{\n"
			"public:\n"
			"	App() {}\n"
			"	Item item;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "void App::App() -> void Item::Item() <7:2 7:2>");
	}

	void test_cxx_parser_finds_constructor_call_of_field_in_initialization_list()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class Item\n"
			"{\n"
			"public:\n"
			"	Item(int n) {}\n"
			"};\n"
			"class App\n"
			"{\n"
			"	App()\n"
			"		: item(1)"
			"	{}\n"
			"	Item item;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "void App::App() -> void Item::Item(int) <9:5 9:11>");
	}

	void test_cxx_parser_finds_function_call_within_constructor_call_of_field_in_initialization_list()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int one() { return 1; }\n"
			"class Item\n"
			"{\n"
			"public:\n"
			"	Item(int n) {}\n"
			"};\n"
			"class App\n"
			"{\n"
			"	App()\n"
			"		: item(one())"
			"	{}\n"
			"	Item item;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 2);
		TS_ASSERT_EQUALS(client->calls[0], "void App::App() -> void Item::Item(int) <10:5 10:15>");
		TS_ASSERT_EQUALS(client->calls[1], "void App::App() -> int one() <10:10 10:14>");
	}

	void test_cxx_parser_finds_copy_constructor_call()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class App\n"
			"{\n"
			"public:\n"
			"	App() {}\n"
			"	App(const App& other) {}\n"
			"};\n"
			"int main()\n"
			"{\n"
			"	App app;\n"
			"	App app2(app);\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 2);
		TS_ASSERT_EQUALS(client->calls[0], "int main() -> void App::App() <9:6 9:6>");
		TS_ASSERT_EQUALS(client->calls[1], "int main() -> void App::App(class App const &) <10:6 10:14>");
	}

	void test_cxx_parser_finds_global_constructor_call()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class App\n"
			"{\n"
			"public:\n"
			"	App() {}\n"
			"};\n"
			"App app;\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "app -> void App::App() <6:5 6:5>");
	}

	void test_cxx_parser_finds_global_function_call()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int one() { return 1; }\n"
			"int a = one();\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 1);
		TS_ASSERT_EQUALS(client->calls[0], "a -> int one() <2:9 2:13>");
	}

	void test_cxx_parser_finds_operator_call()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class App\n"
			"{\n"
			"public:\n"
			"	void operator+(int a)\n"
			"	{\n"
			"	}\n"
			"};\n"
			"int main()\n"
			"{\n"
			"	App app;\n"
			"	app + 2;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->calls.size(), 2);
		TS_ASSERT_EQUALS(client->calls[0], "int main() -> void App::App() <10:6 10:6>");
		TS_ASSERT_EQUALS(client->calls[1], "int main() -> void App::operator+(int) <11:2 11:8>");
	}

	void test_cxx_parser_finds_usage_of_global_variable_in_function()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int bar;\n"
			"\n"
			"int main()\n"
			"{\n"
			"	bar = 1;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->usages.size(), 1);
		TS_ASSERT_EQUALS(client->usages[0], "int main() -> bar <5:2 5:4>");
	}

	void test_cxx_parser_finds_usage_of_global_variable_in_method()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int bar;\n"
			"\n"
			"class App\n"
			"{\n"
			"	void foo()\n"
			"	{\n"
			"		bar = 1;\n"
			"	}\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->usages.size(), 1);
		TS_ASSERT_EQUALS(client->usages[0], "void App::foo() -> bar <7:3 7:5>");
	}

	void test_cxx_parser_finds_usage_of_field_in_method()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class App\n"
			"{\n"
			"	void foo()\n"
			"	{\n"
			"		bar = 1;\n"
			"		this->bar = 2;\n"
			"	}\n"
			"	int bar;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->usages.size(), 2);
		TS_ASSERT_EQUALS(client->usages[0], "void App::foo() -> App::bar <5:3 5:5>");
		TS_ASSERT_EQUALS(client->usages[1], "void App::foo() -> App::bar <6:3 6:11>");
	}

	void test_cxx_parser_finds_usage_of_field_in_initialization_list()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class App\n"
			"{\n"
			"	App()\n"
			"		: bar(42)\n"
			"	{}\n"
			"	int bar;\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->usages.size(), 1);
		TS_ASSERT_EQUALS(client->usages[0], "void App::App() -> App::bar <4:5 4:7>");
	}

	void test_cxx_parser_finds_return_type_use_in_function()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"double PI()\n"
			"{\n"
			"	return 3.14159265359;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 1);
		TS_ASSERT_EQUALS(client->typeUses[0], "double <1:1 1:6>");
	}

	void test_cxx_parser_finds_return_and_parameter_type_uses_in_function()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int ceil(float a)\n"
			"{\n"
			"	return static_cast<int>(a) + 1;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 2);
		TS_ASSERT_EQUALS(client->typeUses[0], "int <1:1 1:3>");
		TS_ASSERT_EQUALS(client->typeUses[1], "float <1:10 1:14>");
	}

	void test_cxx_parser_finds_parameter_type_uses_in_constructor()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A\n"
			"{\n"
			"	A(int a, bool b, float c, int d);\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 4);
		TS_ASSERT_EQUALS(client->typeUses[0], "int <3:4 3:6>");
		TS_ASSERT_EQUALS(client->typeUses[1], "_Bool <3:11 3:15>");
		TS_ASSERT_EQUALS(client->typeUses[2], "float <3:19 3:23>");
		TS_ASSERT_EQUALS(client->typeUses[3], "int <3:28 3:30>");
	}

	void test_cxx_parser_finds_type_uses_in_function_body()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int main()\n"
			"{\n"
			"	int a = 42;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 2);
		TS_ASSERT_EQUALS(client->typeUses[0], "int <1:1 1:3>");
		TS_ASSERT_EQUALS(client->typeUses[1], "int main() -> int <3:2 3:4>");
	}

	void test_cxx_parser_finds_type_uses_in_method_body()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A\n"
			"{\n"
			"	int main()\n"
			"	{\n"
			"		int a = 42;\n"
			"		return a;\n"
			"	}\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 2);
		TS_ASSERT_EQUALS(client->typeUses[0], "int <3:2 3:4>");
		TS_ASSERT_EQUALS(client->typeUses[1], "int A::main() -> int <5:3 5:5>");
	}

	void test_cxx_parser_finds_type_uses_in_loops_and_conditions()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"int main()\n"
			"{\n"
			"	if (true)\n"
			"	{\n"
			"		int a = 42;\n"
			"	}\n"
			"	for (int i = 0; i < 10; i++)\n"
			"	{\n"
			"		int b = i * 2;\n"
			"	}\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 4);
		TS_ASSERT_EQUALS(client->typeUses[0], "int <1:1 1:3>");
		TS_ASSERT_EQUALS(client->typeUses[1], "int main() -> int <5:3 5:5>");
		TS_ASSERT_EQUALS(client->typeUses[2], "int main() -> int <7:7 7:9>");
		TS_ASSERT_EQUALS(client->typeUses[3], "int main() -> int <9:3 9:5>");
	}

	void test_cxx_parser_finds_type_uses_of_classes_in_functions()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A {};\n"
			"int main()\n"
			"{\n"
			"	A a;\n"
			"}\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 2);
		TS_ASSERT_EQUALS(client->typeUses[0], "int <2:1 2:3>");
		TS_ASSERT_EQUALS(client->typeUses[1], "int main() -> A <4:2 4:2>");
	}

	void test_cxx_parser_finds_type_uses_of_base_class_in_derived_constructor()
	{
		std::shared_ptr<TestParserClient> client = parseCode(
			"class A\n"
			"{\n"
			"public:\n"
			"	A(int n) {}"
			"};\n"
			"class B : public A\n"
			"{\n"
			"public:\n"
			"	B() : A(42) {}\n"
			"};\n"
		);

		TS_ASSERT_EQUALS(client->typeUses.size(), 2);
		TS_ASSERT_EQUALS(client->typeUses[0], "int <4:4 4:6>");
		TS_ASSERT_EQUALS(client->typeUses[1], "void B::B() -> A <8:8 8:8>");
	}

	void test_cxx_parser_parses_multiple_files()
	{
		TestParserClient client;
		CxxParser parser(&client);

		std::vector<std::string> filePaths;
		filePaths.push_back("data/CxxParserTestSuite/header.h");
		filePaths.push_back("data/CxxParserTestSuite/code.cpp");
		parser.parseFiles(filePaths);

		TS_ASSERT_EQUALS(client.typedefs.size(), 1);
		TS_ASSERT_EQUALS(client.classes.size(), 4);
		TS_ASSERT_EQUALS(client.enums.size(), 1);
		TS_ASSERT_EQUALS(client.enumFields.size(), 2);
		TS_ASSERT_EQUALS(client.functions.size(), 2);
		TS_ASSERT_EQUALS(client.fields.size(), 4);
		TS_ASSERT_EQUALS(client.globalVariables.size(), 2);
		TS_ASSERT_EQUALS(client.methods.size(), 5);
		TS_ASSERT_EQUALS(client.namespaces.size(), 2);
		TS_ASSERT_EQUALS(client.structs.size(), 1);

		TS_ASSERT_EQUALS(client.inheritances.size(), 1);
		TS_ASSERT_EQUALS(client.calls.size(), 2);
		TS_ASSERT_EQUALS(client.usages.size(), 3);
		TS_ASSERT_EQUALS(client.typeUses.size(), 8);
	}

private:
	class TestParserClient: public ParserClient
	{
	public:
		virtual Id onTypedefParsed(
			const ParseLocation& location, const std::string& fullName, const ParseTypeUsage& underlyingType,
			AccessType access
		)
		{
			std::string str = addAccessPrefix(underlyingType.dataType.getFullTypeName() + " -> " + fullName, access);
			typedefs.push_back(addLocationSuffix(str, location));
			return 0;
		}

		virtual Id onClassParsed(
			const ParseLocation& location, const std::string& fullName, AccessType access,
			const ParseLocation& scopeLocation)
		{
			classes.push_back(addLocationSuffix(addAccessPrefix(fullName, access), location, scopeLocation));
			return 0;
		}

		virtual Id onStructParsed(
			const ParseLocation& location, const std::string& fullName, AccessType access,
			const ParseLocation& scopeLocation)
		{
			structs.push_back(addLocationSuffix(addAccessPrefix(fullName, access), location, scopeLocation));
			return 0;
		}

		virtual Id onGlobalVariableParsed(const ParseLocation& location, const ParseVariable& variable)
		{
			globalVariables.push_back(addLocationSuffix(variableStr(variable), location));
			return 0;
		}

		virtual Id onFieldParsed(const ParseLocation& location, const ParseVariable& variable, AccessType access)
		{
			fields.push_back(addLocationSuffix(addAccessPrefix(variableStr(variable), access), location));
			return 0;
		}

		virtual Id onFunctionParsed(
			const ParseLocation& location, const ParseFunction& function, const ParseLocation& scopeLocation
		){
			functions.push_back(addLocationSuffix(functionStr(function), location, scopeLocation));

			addTypeUse(function.returnType);
			for (const ParseTypeUsage& parameter : function.parameters)
			{
				addTypeUse(parameter);
			}
			return 0;
		}

		virtual Id onMethodParsed(
			const ParseLocation& location, const ParseFunction& method, AccessType access, AbstractionType abstraction,
			const ParseLocation& scopeLocation
		){
			std::string str = functionStr(method);
			str = addAbstractionPrefix(str, abstraction);
			str = addAccessPrefix(str, access);
			str = addLocationSuffix(str, location, scopeLocation);
			methods.push_back(str);

			addTypeUse(method.returnType);
			for (const ParseTypeUsage& parameter : method.parameters)
			{
				addTypeUse(parameter);
			}
			return 0;
		}

		virtual Id onNamespaceParsed(
			const ParseLocation& location, const std::string& fullName, const ParseLocation& scopeLocation)
		{
			namespaces.push_back(addLocationSuffix(fullName, location, scopeLocation));
			return 0;
		}

		virtual Id onEnumParsed(
			const ParseLocation& location, const std::string& fullName, AccessType access,
			const ParseLocation& scopeLocation)
		{
			enums.push_back(addLocationSuffix(addAccessPrefix(fullName, access), location, scopeLocation));
			return 0;
		}

		virtual Id onEnumFieldParsed(const ParseLocation& location, const std::string& fullName)
		{
			enumFields.push_back(addLocationSuffix(fullName, location));
			return 0;
		}

		virtual Id onInheritanceParsed(
			const ParseLocation& location, const std::string& fullName, const std::string& baseName, AccessType access)
		{
			std::string str = fullName + " : " + addAccessPrefix(baseName, access);
			inheritances.push_back(addLocationSuffix(str, location));
			return 0;
		}

		virtual Id onCallParsed(
			const ParseLocation& location, const ParseFunction& caller, const ParseFunction& callee)
		{
			calls.push_back(addLocationSuffix(functionStr(caller) + " -> " + functionStr(callee), location));
			return 0;
		}

		virtual Id onCallParsed(
			const ParseLocation& location, const ParseVariable& caller, const ParseFunction& callee)
		{
			calls.push_back(addLocationSuffix(caller.fullName + " -> " + functionStr(callee), location));
			return 0;
		}

		virtual Id onFieldUsageParsed(
			const ParseLocation& location, const ParseFunction& user, const std::string& usedName)
		{
			usages.push_back(addLocationSuffix(functionStr(user) + " -> " + usedName, location));
			return 0;
		}

		virtual Id onGlobalVariableUsageParsed(
			const ParseLocation& location, const ParseFunction& user, const std::string& usedName)
		{
			usages.push_back(addLocationSuffix(functionStr(user) + " -> " + usedName, location));
			return 0;
		}

		virtual Id onTypeUsageParsed(const ParseTypeUsage& type, const ParseFunction& function)
		{
			addTypeUse(type, function);
			return 0;
		}

		std::vector<std::string> typedefs;
		std::vector<std::string> classes;
		std::vector<std::string> enums;
		std::vector<std::string> enumFields;
		std::vector<std::string> functions;
		std::vector<std::string> fields;
		std::vector<std::string> globalVariables;
		std::vector<std::string> methods;
		std::vector<std::string> namespaces;
		std::vector<std::string> structs;

		std::vector<std::string> inheritances;
		std::vector<std::string> calls;
		std::vector<std::string> usages;
		std::vector<std::string> typeUses;

	private:
		void addTypeUse(const ParseTypeUsage& use)
		{
			if (use.location.isValid())
			{
				typeUses.push_back(addLocationSuffix(use.dataType.getFullTypeName(), use.location));
			}
		}

		void addTypeUse(const ParseTypeUsage& use, const ParseFunction& func)
		{
			if (use.location.isValid())
			{
				typeUses.push_back(
					addLocationSuffix(functionStr(func) + " -> " + use.dataType.getFullTypeName(), use.location)
				);
			}
		}
	};

	std::shared_ptr<TestParserClient> parseCode(std::string code) const
	{
		std::shared_ptr<TestParserClient> client = std::make_shared<TestParserClient>();
		CxxParser parser(client.get());
		parser.parseFile(TextAccess::createFromString(code));
		return client;
	}
};
